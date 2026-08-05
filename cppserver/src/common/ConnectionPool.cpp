/**
 * @file ConnectionPool.cpp
 * @brief MySQL 连接池实现
 * @date 2025
 */

#include "common/ConnectionPool.hpp"
#include "common/Logger.hpp"

namespace community_system {
namespace common {

// ============== 初始化与销毁 ==============

void ConnectionPool::init(const DBConfig& config) {
    if (m_initialized.load()) {
        Logger::warn("ConnectionPool already initialized, skipping");
        return;
    }

    m_config = config;

    // 合法性校验
    if (m_config.maxSize <= 0) {
        m_config.maxSize = 10;
    }
    if (m_config.port <= 0 || m_config.port > 65535) {
        m_config.port = 3306;
    }

    Logger::info("Initializing MySQL connection pool: host={}, port={}, db={}, maxSize={}",
                 m_config.host, m_config.port, m_config.database, m_config.maxSize);

    // 预创建连接
    for (int i = 0; i < m_config.maxSize; ++i) {
        MYSQL* conn = createConnection();
        if (conn) {
            m_idleQueue.push(conn);
        } else {
            Logger::warn("Failed to create connection #{}", i + 1);
        }
    }

    if (m_idleQueue.empty()) {
        throw std::runtime_error("Failed to create any database connection");
    }

    m_initialized.store(true);
    Logger::info("Connection pool initialized successfully, {} connections created",
                 m_idleQueue.size());
}

void ConnectionPool::destroy() {
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_idleQueue.empty()) {
        MYSQL* conn = m_idleQueue.front();
        m_idleQueue.pop();
        if (conn) {
            mysql_close(conn);
        }
    }
    m_initialized.store(false);
    Logger::info("Connection pool destroyed");
}

// ============== 连接创建 ==============

MYSQL* ConnectionPool::createConnection() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        Logger::error("mysql_init() failed");
        return nullptr;
    }

    // 设置连接超时
    unsigned int timeout = static_cast<unsigned int>(m_config.timeout);
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    // 设置自动重连
    my_bool reconnect = 1;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);

    // 建立连接
    if (!mysql_real_connect(conn,
                             m_config.host.c_str(),
                             m_config.user.c_str(),
                             m_config.password.c_str(),
                             m_config.database.c_str(),
                             m_config.port,
                             nullptr,  // unix socket
                             0)) {   // client flag
        Logger::error("mysql_real_connect() failed: {}", mysql_error(conn));
        mysql_close(conn);
        return nullptr;
    }

    // 设置字符集
    if (mysql_set_character_set(conn, m_config.charset.c_str()) != 0) {
        Logger::warn("Set charset '{}' failed: {}", m_config.charset, mysql_error(conn));
    }

    return conn;
}

// ============== 获取连接 ==============

std::shared_ptr<MYSQL> ConnectionPool::getConnection() {
    // 复用 tryGetConnection，阻塞等待
    auto result = tryGetConnection(0);  // 0 = 无限等待
    return result ? *result : nullptr;
}

std::optional<std::shared_ptr<MYSQL>> ConnectionPool::tryGetConnection(int timeoutMs) {
    if (!m_initialized.load()) {
        Logger::error("ConnectionPool not initialized");
        return std::nullopt;
    }

    std::unique_lock<std::mutex> lock(m_mutex);

    // 等待空闲连接（条件变量 + 谓词）
    if (timeoutMs <= 0) {
        // 无限等待
        m_cond.wait(lock, [this] { return !m_idleQueue.empty(); });
    } else {
        // 带超时等待
        bool got = m_cond.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                                   [this] { return !m_idleQueue.empty(); });
        if (!got) {
            Logger::warn("getConnection timeout after {}ms, all connections busy", timeoutMs);
            return std::nullopt;
        }
    }

    // 从队列头部取出连接
    MYSQL* conn = m_idleQueue.front();
    m_idleQueue.pop();
    ++m_activeCount;
    lock.unlock();

    // 检查连接有效性（心跳检测）
    if (mysql_ping(conn) != 0) {
        Logger::warn("Connection ping failed, reconnecting...");
        mysql_close(conn);
        conn = createConnection();
        if (!conn) {
            Logger::error("Reconnect failed");
            // 失败则减少活跃计数并唤醒等待者
            std::lock_guard<std::mutex> relock(m_mutex);
            --m_activeCount;
            m_cond.notify_one();
            return std::nullopt;
        }
    }

    // 使用自定义 deleter：shared_ptr 析构时自动归还连接到池
    // 这是 RAII 思想的典型应用——将"归还连接"绑定到智能指针的生命周期
    std::shared_ptr<MYSQL> sp(conn, [this](MYSQL* c) {
        if (c) {
            this->releaseConnection(c);
        }
    });

    return sp;
}

// ============== 归还连接 ==============

void ConnectionPool::releaseConnection(MYSQL* conn) {
    if (!conn) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // 如果连接池已销毁，直接释放连接
    if (!m_initialized.load()) {
        mysql_close(conn);
        return;
    }

    m_idleQueue.push(conn);
    --m_activeCount;
    m_cond.notify_one();  // 唤醒一个等待者
}

}  // namespace common
}  // namespace community_system
