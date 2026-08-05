/**
 * @file DbPool.cc
 * @brief MySQL 数据库连接池实现
 * @date 2025
 */

#include "DbPool.h"
#include "Logger.h"
#include <stdexcept>
#include <chrono>

namespace community {

// ============== DbConnection 实现 ==============

DbConnection::DbConnection(MYSQL* conn, std::function<void(MYSQL*)> returnFn)
    : conn_(conn), returnFn_(std::move(returnFn)) {}

DbConnection::~DbConnection() {
    if (conn_ && returnFn_) {
        returnFn_(conn_);
    }
}

DbConnection::DbConnection(DbConnection&& other) noexcept
    : conn_(other.conn_), returnFn_(std::move(other.returnFn_)) {
    other.conn_ = nullptr;
}

DbConnection& DbConnection::operator=(DbConnection&& other) noexcept {
    if (this != &other) {
        if (conn_ && returnFn_) {
            returnFn_(conn_);
        }
        conn_ = other.conn_;
        returnFn_ = std::move(other.returnFn_);
        other.conn_ = nullptr;
    }
    return *this;
}

DbResult DbConnection::query(std::string_view sql) const {
    DbResult result;
    if (!conn_) {
        result.error = "连接无效";
        return result;
    }

    // 执行查询
    if (mysql_real_query(conn_, sql.data(), sql.size()) != 0) {
        result.error = mysql_error(conn_);
        LOG_ERROR << "SQL查询失败: " << result.error << " | SQL: " << std::string(sql);
        return result;
    }

    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) {
        // 可能是 INSERT/UPDATE/DELETE
        result.affectedRows = static_cast<int>(mysql_affected_rows(conn_));
        result.insertId = static_cast<int>(mysql_insert_id(conn_));
        result.success = true;
        return result;
    }

    // 获取字段名
    unsigned int numFields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    // 遍历结果行
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        unsigned long* lengths = mysql_fetch_lengths(res);
        DbRow dbRow;
        for (unsigned int i = 0; i < numFields; ++i) {
            std::string value = (row[i] != nullptr) 
                ? std::string(row[i], lengths[i]) 
                : std::string();
            dbRow[fields[i].name] = value;
        }
        result.rows.push_back(std::move(dbRow));
    }

    mysql_free_result(res);
    result.success = true;
    return result;
}

DbResult DbConnection::execute(std::string_view sql) const {
    return query(sql);  // MySQL C API 中 query 和 execute 统一
}

std::string DbConnection::escape(std::string_view input) const {
    if (!conn_) return std::string(input);
    
    std::string output(input.size() * 2 + 1, '\0');
    unsigned long len = mysql_real_escape_string(
        const_cast<MYSQL*>(conn_),
        output.data(),
        input.data(),
        input.size()
    );
    output.resize(len);
    return output;
}

// ============== DbPool 实现 ==============

DbPool& DbPool::instance() {
    static DbPool pool;
    return pool;
}

bool DbPool::init(const DbConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        LOG_WARN << "连接池已初始化，跳过重复初始化";
        return true;
    }

    config_ = config;

    // 初始化 MySQL 库
    if (mysql_library_init(0, nullptr, nullptr) != 0) {
        LOG_ERROR << "MySQL 库初始化失败";
        return false;
    }

    // 预创建连接
    for (int i = 0; i < config_.poolSize; ++i) {
        MYSQL* conn = createConnection();
        if (!conn) {
            LOG_ERROR << "创建数据库连接失败 (第 " << i + 1 << " 个)";
            // 清理已创建的连接
            while (!pool_.empty()) {
                mysql_close(pool_.front());
                pool_.pop();
            }
            return false;
        }
        pool_.push(conn);
        ++totalCreated_;
    }

    initialized_ = true;
    LOG_INFO << "MySQL 连接池初始化完成，连接数: " << config_.poolSize;
    return true;
}

std::unique_ptr<DbConnection> DbPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);

    if (!initialized_) {
        throw std::runtime_error("数据库连接池未初始化");
    }

    // 等待可用连接（最多 5 秒）
    if (pool_.empty()) {
        cv_.wait_for(lock, std::chrono::seconds(5), [this] { return !pool_.empty(); });
    }

    if (pool_.empty()) {
        // 尝试创建新连接（如果未达上限的 2 倍）
        if (totalCreated_ < config_.poolSize * 2) {
            MYSQL* conn = createConnection();
            if (conn) {
                ++totalCreated_;
                return std::make_unique<DbConnection>(conn, [this](MYSQL* c) {
                    returnConnection(c);
                });
            }
        }
        throw std::runtime_error("无法获取数据库连接（连接池已满）");
    }

    MYSQL* conn = pool_.front();
    pool_.pop();
    lock.unlock();

    // 检查连接有效性（使用 ping 检测）
    if (mysql_ping(conn) != 0) {
        LOG_WARN << "数据库连接已断开，重新连接...";
        mysql_close(conn);
        conn = createConnection();
        if (!conn) {
            std::lock_guard<std::mutex> lk(mutex_);
            --totalCreated_;
            throw std::runtime_error("数据库重连失败");
        }
    }

    return std::make_unique<DbConnection>(conn, [this](MYSQL* c) {
        returnConnection(c);
    });
}

void DbPool::returnConnection(MYSQL* conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (conn) {
        pool_.push(conn);
        cv_.notify_one();
    }
}

void DbPool::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!pool_.empty()) {
        mysql_close(pool_.front());
        pool_.pop();
    }
    totalCreated_ = 0;
    initialized_ = false;
    mysql_library_end();
    LOG_INFO << "数据库连接池已关闭";
}

size_t DbPool::availableCount() const {
    return pool_.size();
}

DbPool::~DbPool() {
    // 清理所有连接
    while (!pool_.empty()) {
        mysql_close(pool_.front());
        pool_.pop();
    }
    if (initialized_) {
        mysql_library_end();
    }
}

MYSQL* DbPool::createConnection() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        LOG_ERROR << "mysql_init 失败";
        return nullptr;
    }

    // 设置字符集
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, config_.charset.c_str());

    // 设置连接超时
    unsigned int timeout = 5;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    // 启用自动重连
    bool reconnect = true;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);

    // 建立连接
    if (!mysql_real_connect(
            conn,
            config_.host.c_str(),
            config_.user.c_str(),
            config_.password.c_str(),
            config_.database.c_str(),
            config_.port,
            nullptr,
            0)) {
        LOG_ERROR << "MySQL 连接失败: " << mysql_error(conn);
        mysql_close(conn);
        return nullptr;
    }

    return conn;
}

}  // namespace community
