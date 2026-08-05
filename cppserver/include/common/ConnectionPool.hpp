/**
 * @file ConnectionPool.hpp
 * @brief MySQL 数据库连接池
 *
 * 基于 RAII 思想的线程安全 MySQL 连接池。
 * 采用单例模式（Meyers' Singleton）管理全局连接池实例。
 * 连接通过 std::shared_ptr<MYSQL> 返回，自定义 deleter 实现自动归还。
 *
 * 使用到的 C++ 特性：
 *   - C++11: std::mutex, std::condition_variable, std::queue, std::shared_ptr
 *            自定义 deleter, 线程安全的 static local（Meyers Singleton）
 *   - C++17: std::string_view, std::optional
 *   - C++20: std::make_shared 配合聚合初始化
 *
 * 设计模式：
 *   - 单例模式（Singleton）：全局唯一连接池
 *   - 对象池模式（Object Pool）：复用连接对象
 *   - RAII：通过 shared_ptr deleter 自动归还连接
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <optional>
#include <stdexcept>
#include <chrono>

// MySQL C API 头文件
#include <mysql/mysql.h>

#include "Logger.hpp"

namespace community_system {
namespace common {

/**
 * @struct DBConfig
 * @brief 数据库连接配置参数
 *
 * 使用 struct 聚合配置项，便于从配置文件加载和传递。
 */
struct DBConfig {
    std::string host     = "127.0.0.1";  ///< 数据库地址
    int         port     = 3306;          ///< 数据库端口
    std::string user     = "root";        ///< 用户名
    std::string password = "";            ///< 密码
    std::string database = "community";   ///< 数据库名
    std::string charset  = "utf8mb4";     ///< 字符集
    int         maxSize  = 10;            ///< 最大连接数
    int         minSize  = 2;             ///< 最小连接数（预留）
    int         timeout  = 5;             ///< 连接超时（秒）
};

/**
 * @class ConnectionPool
 * @brief MySQL 连接池单例类
 *
 * 核心设计：
 *  1. 连接池初始化时创建 maxSize 个连接，放入空闲队列
 *  2. getConnection() 从队列头部取出一个连接，若队列为空则阻塞等待
 *  3. 连接通过 std::shared_ptr 包装，自定义 deleter 在引用计数归零时
 *     自动调用 releaseConnection() 将连接归还到队列
 *  4. 使用条件变量实现生产者-消费者模型，线程安全
 *
 * 使用方式：
 *   ConnectionPool::instance().init(config);
 *   auto conn = ConnectionPool::instance().getConnection();
 *   if (conn) { mysql_query(conn.get(), "SELECT ..."); }
 *   // conn 析构时自动归还连接池
 */
class ConnectionPool {
public:
    /**
     * @brief 获取单例实例（Meyers' Singleton，C++11 线程安全）
     * @return ConnectionPool&
     */
    static ConnectionPool& instance() {
        static ConnectionPool inst;
        return inst;
    }

    // 禁止拷贝和移动
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    ConnectionPool(ConnectionPool&&) = delete;
    ConnectionPool& operator=(ConnectionPool&&) = delete;

    /**
     * @brief 初始化连接池
     * @param config 数据库配置
     * @throw std::runtime_error 初始化失败时抛出异常
     *
     * 该方法不是线程安全的，应在程序启动时调用一次
     */
    void init(const DBConfig& config);

    /**
     * @brief 从连接池获取一个连接
     * @return std::shared_ptr<MYSQL> 数据库连接智能指针
     *         若超时无可用连接返回 nullptr
     *
     * 返回的 shared_ptr 带有自定义 deleter，会在引用计数归零时
     * 自动将连接归还到连接池（RAII 思想）。
     */
    std::shared_ptr<MYSQL> getConnection();

    /**
     * @brief 带超时的获取连接（C++17 optional 版本）
     * @param timeoutMs 超时时间（毫秒）
     * @return std::optional<std::shared_ptr<MYSQL>>
     *         获取成功返回 optional 包装的连接，超时返回 nullopt
     *
     * 使用 std::optional 明确表达"可能没有值"的语义，比 nullptr 更安全。
     */
    std::optional<std::shared_ptr<MYSQL>> tryGetConnection(int timeoutMs = 3000);

    /**
     * @brief 获取当前空闲连接数
     * @return size_t 空闲连接数
     */
    size_t idleCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_idleQueue.size();
    }

    /**
     * @brief 获取连接池总连接数
     * @return int 总连接数
     */
    int totalSize() const {
        return m_config.maxSize;
    }

    /**
     * @brief 连接池是否已初始化
     * @return bool
     */
    bool isInitialized() const {
        return m_initialized;
    }

    /**
     * @brief 获取配置（只读）
     * @return const DBConfig&
     */
    const DBConfig& getConfig() const { return m_config; }

    /**
     * @brief 销毁连接池，释放所有连接
     *
     * 由析构函数自动调用，也可手动调用。
     */
    void destroy();

private:
    /**
     * @brief 私有构造函数（单例）
     */
    ConnectionPool() : m_initialized(false) {}

    /**
     * @brief 析构函数，自动释放所有连接
     */
    ~ConnectionPool() {
        destroy();
    }

    /**
     * @brief 创建一个新的 MySQL 连接
     * @return MYSQL* 成功返回连接指针，失败返回 nullptr
     */
    MYSQL* createConnection();

    /**
     * @brief 归还连接到连接池
     * @param conn MySQL 连接指针
     *
     * 该方法由 shared_ptr 的自定义 deleter 调用。
     * 注意：此处 conn 是原始指针，需要保证有效性。
     */
    void releaseConnection(MYSQL* conn);

private:
    DBConfig                  m_config;        ///< 数据库配置
    std::queue<MYSQL*>        m_idleQueue;     ///< 空闲连接队列
    mutable std::mutex        m_mutex;         ///< 互斥锁（保护队列）
    std::condition_variable   m_cond;          ///< 条件变量（等待/唤醒）
    std::atomic<bool>         m_initialized;   ///< 是否已初始化标志
    std::atomic<int>          m_activeCount{0};///< 活跃连接计数（调试用）
};

}  // namespace common
}  // namespace community_system
