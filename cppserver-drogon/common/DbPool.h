/**
 * @file DbPool.h
 * @brief MySQL 数据库连接池
 *
 * 由于 Drogon 1.9.13 编译时未启用 MySQL ORM 模块，
 * 本连接池基于原生 MySQL C API 实现，提供：
 *   - 线程安全的连接获取/归还
 *   - 连接复用（池化）
 *   - 自动重连机制
 *   - RAII 风格的连接管理
 *
 * 设计模式：
 *   - 单例模式：全局唯一的连接池实例
 *   - RAII：DbConnection 智能管理连接的获取与释放
 *
 * 使用方式：
 *   auto conn = DbPool::instance().getConnection();
 *   if (conn) {
 *       auto result = conn->query("SELECT * FROM users");
 *   }
 *
 * @date 2025
 */

#pragma once

#include <mysql/mysql.h>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>
#include <functional>
#include <unordered_map>

namespace community {

/**
 * @struct DbConfig
 * @brief 数据库连接配置
 */
struct DbConfig {
    std::string host     = "127.0.0.1";   ///< 主机地址
    int         port     = 3306;           ///< 端口号
    std::string user     = "root";         ///< 用户名
    std::string password = "";             ///< 密码
    std::string database = "";             ///< 数据库名
    std::string charset  = "utf8mb4";      ///< 字符集
    int         poolSize = 10;             ///< 连接池大小
};

/**
 * @struct DbRow
 * @brief 数据库行数据（字段名 -> 字段值的映射）
 */
using DbRow = std::unordered_map<std::string, std::string>;

/**
 * @struct DbResult
 * @brief 数据库查询结果
 */
struct DbResult {
    std::vector<DbRow> rows;       ///< 结果行
    int affectedRows = 0;          ///< 受影响的行数（INSERT/UPDATE/DELETE）
    int insertId     = 0;          ///< 自增 ID（INSERT）
    bool success     = false;      ///< 是否成功
    std::string error;             ///< 错误信息

    /**
     * @brief 判断查询是否返回了数据
     */
    bool hasData() const { return !rows.empty(); }

    /**
     * @brief 获取行数
     */
    size_t rowCount() const { return rows.size(); }

    /**
     * @brief 获取第一行第一列的值
     */
    std::optional<std::string> scalar() const {
        if (rows.empty() || rows[0].empty()) return std::nullopt;
        return rows[0].begin()->second;
    }
};

/**
 * @class DbConnection
 * @brief 数据库连接封装（RAII）
 *
 * 封装 MYSQL 连接指针，提供便捷的查询接口。
 * 连接归还给连接池时自动释放资源。
 */
class DbConnection {
public:
    /**
     * @brief 构造函数
     * @param conn MYSQL 连接指针
     * @param pool 连接池引用（用于归还连接）
     */
    DbConnection(MYSQL* conn, std::function<void(MYSQL*)> returnFn);

    /**
     * @brief 析构函数 - 自动归还连接到连接池
     */
    ~DbConnection();

    // 禁止拷贝
    DbConnection(const DbConnection&) = delete;
    DbConnection& operator=(const DbConnection&) = delete;

    // 允许移动
    DbConnection(DbConnection&& other) noexcept;
    DbConnection& operator=(DbConnection&& other) noexcept;

    /**
     * @brief 执行 SQL 查询（SELECT）
     * @param sql SQL 语句
     * @return DbResult 查询结果
     */
    DbResult query(std::string_view sql) const;

    /**
     * @brief 执行 SQL 更新（INSERT/UPDATE/DELETE）
     * @param sql SQL 语句
     * @return DbResult 执行结果（含 affectedRows 和 insertId）
     */
    DbResult execute(std::string_view sql) const;

    /**
     * @brief 转义 SQL 字符串（防止注入）
     * @param input 输入字符串
     * @return std::string 转义后的字符串
     */
    std::string escape(std::string_view input) const;

    /**
     * @brief 检查连接是否有效
     */
    bool isValid() const { return conn_ != nullptr; }

    /**
     * @brief 重载 bool 运算符
     */
    explicit operator bool() const { return isValid(); }

private:
    MYSQL* conn_ = nullptr;
    std::function<void(MYSQL*)> returnFn_;
};

/**
 * @class DbPool
 * @brief MySQL 数据库连接池（单例模式）
 *
 * 管理一组 MYSQL 连接，提供线程安全的连接获取/归还机制。
 */
class DbPool {
public:
    /**
     * @brief 获取单例实例
     * @return DbPool& 连接池引用
     */
    static DbPool& instance();

    /**
     * @brief 初始化连接池
     * @param config 数据库配置
     * @return bool 是否初始化成功
     */
    bool init(const DbConfig& config);

    /**
     * @brief 从连接池获取一个连接
     * @return std::unique_ptr<DbConnection> 数据库连接（自动归还）
     *
     * 使用 unique_ptr + 自定义删除器实现 RAII 自动归还。
     * 如果连接池为空且已达上限，则阻塞等待。
     */
    std::unique_ptr<DbConnection> getConnection();

    /**
     * @brief 关闭连接池，释放所有连接
     */
    void shutdown();

    /**
     * @brief 获取当前可用连接数
     */
    size_t availableCount() const;

    // 禁止拷贝和移动
    DbPool(const DbPool&) = delete;
    DbPool& operator=(const DbPool&) = delete;

private:
    DbPool() = default;
    ~DbPool();

    /**
     * @brief 创建一个新的 MySQL 连接
     * @return MYSQL* 连接指针，失败返回 nullptr
     */
    MYSQL* createConnection();

    /**
     * @brief 归还连接到连接池
     * @param conn MYSQL 连接指针
     */
    void returnConnection(MYSQL* conn);

    DbConfig                          config_;        ///< 数据库配置
    std::queue<MYSQL*>                pool_;          ///< 空闲连接队列
    std::mutex                        mutex_;         ///< 互斥锁
    std::condition_variable           cv_;            ///< 条件变量（等待可用连接）
    int                               totalCreated_ = 0;  ///< 已创建的连接总数
    bool                              initialized_ = false; ///< 是否已初始化
};

}  // namespace community
