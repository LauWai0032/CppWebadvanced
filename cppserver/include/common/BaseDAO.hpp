/**
 * @file BaseDAO.hpp
 * @brief 数据库操作基类模板（模板方法模式）
 *
 * 提供通用的 CRUD 操作模板，子类只需实现从结果集到模型的转换。
 * 采用模板方法设计模式：父类定义算法骨架（执行SQL、遍历结果集），
 * 子类实现具体步骤（rowToModel 行转对象）。
 *
 * 使用到的 C++ 特性：
 *   - C++11: 模板元编程, std::vector, std::shared_ptr
 *   - C++17: std::optional, if constexpr, std::string_view
 *   - C++20: concepts（requires 表达式约束模板参数）
 *
 * 设计模式：
 *   - 模板方法模式（Template Method）：CRUD 流程固定，行转换由子类实现
 *   - CRTP（奇异递归模板模式）：通过模板参数 T 实现静态多态
 *
 * @date 2025
 */

#pragma once

#include <vector>
#include <optional>
#include <string>
#include <string_view>
#include <stdexcept>
#include <memory>

#include "../common/ConnectionPool.hpp"
#include "../common/Logger.hpp"

// 引入 nlohmann/json 供子类使用
#include "../../json.hpp"

namespace community_system {
namespace dao {

/**
 * @concept HasId
 * @brief 约束模板参数 T 必须有 id 成员字段
 *
 * 使用 C++20 requires 表达式定义概念，
 * 确保模板参数类型满足 DAO 操作的基本要求（有主键 id）。
 *
 * 用法：template <HasId T> class BaseDAO { ... };
 */
template <typename T>
concept HasId = requires(T t) {
    { t.id } -> std::convertible_to<int>;
};

/**
 * @class BaseDAO
 * @brief 数据库操作模板基类
 * @tparam T 模型类型，必须满足 HasId 概念（有 id 字段）
 *
 * 设计思路：
 *  - 模板参数 T 是业务模型类（如 User）
 *  - BaseDAO 提供通用 CRUD 方法，内部调用连接池获取连接
 *  - 纯虚方法 rowToModel 由子类实现，完成 MYSQL_ROW -> T 的转换
 *  - 子类还需要提供表名（getTableName），用于 SQL 拼接
 *
 * 使用方式：
 *   class UserDAO : public BaseDAO<User> {
 *   public:
 *       std::string getTableName() const override { return "users"; }
 *       User rowToModel(MYSQL_ROW row) override { ... }
 *   };
 */
template <HasId T>
class BaseDAO {
public:
    /**
     * @brief 虚析构函数，确保子类正确析构
     */
    virtual ~BaseDAO() = default;

    /**
     * @brief 根据 ID 查询单条记录
     * @param id 主键 ID
     * @return std::optional<T> 找到返回包装的对象，未找到返回 nullopt
     *
     * 使用 std::optional 明确表达"可能不存在"的语义，
     * 比返回 nullptr 或抛出异常更优雅。
     */
    std::optional<T> findById(int id) {
        std::string sql = "SELECT * FROM " + getTableName() + " WHERE id = " + std::to_string(id);
        auto result = executeQuery(sql);
        if (result.empty()) {
            return std::nullopt;
        }
        return result.front();
    }

    /**
     * @brief 查询所有记录
     * @return std::vector<T> 结果列表
     */
    std::vector<T> findAll() {
        std::string sql = "SELECT * FROM " + getTableName();
        return executeQuery(sql);
    }

    /**
     * @brief 分页查询
     * @param page 页码（从 1 开始）
     * @param pageSize 每页条数
     * @return std::vector<T> 分页结果
     */
    std::vector<T> findByPage(int page, int pageSize) {
        if (page < 1) page = 1;
        if (pageSize < 1) pageSize = 10;
        int offset = (page - 1) * pageSize;
        std::string sql = "SELECT * FROM " + getTableName() +
                          " LIMIT " + std::to_string(pageSize) +
                          " OFFSET " + std::to_string(offset);
        return executeQuery(sql);
    }

    /**
     * @brief 统计表记录总数
     * @return int 记录数
     */
    int countAll() {
        std::string sql = "SELECT COUNT(*) FROM " + getTableName();
        auto conn = community_system::common::ConnectionPool::instance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        if (mysql_query(conn.get(), sql.c_str()) != 0) {
            common::Logger::error("countAll query failed: {}", mysql_error(conn.get()));
            throw std::runtime_error("Query failed: " + std::string(mysql_error(conn.get())));
        }

        MYSQL_RES* res = mysql_store_result(conn.get());
        if (!res) {
            throw std::runtime_error("mysql_store_result failed");
        }

        int count = 0;
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && row[0]) {
            count = std::atoi(row[0]);
        }
        mysql_free_result(res);
        return count;
    }

    /**
     * @brief 根据 ID 删除记录
     * @param id 主键 ID
     * @return bool 是否成功删除
     */
    bool deleteById(int id) {
        std::string sql = "DELETE FROM " + getTableName() + " WHERE id = " + std::to_string(id);
        return executeUpdate(sql) > 0;
    }

    /**
     * @brief 执行自定义查询 SQL
     * @param sql SQL 语句
     * @return std::vector<T> 查询结果
     *
     * 注意：子类可调用此方法执行自定义 SELECT 语句，
     * 但需确保 SELECT 的列顺序与 rowToModel 的解析顺序一致。
     */
    std::vector<T> executeQuery(std::string_view sql) {
        std::vector<T> results;

        // 从连接池获取连接（RAII：函数结束自动归还）
        auto conn = community_system::common::ConnectionPool::instance().getConnection();
        if (!conn) {
            common::Logger::error("executeQuery: failed to get connection");
            return results;
        }

        // 执行查询
        if (mysql_query(conn.get(), sql.data()) != 0) {
            common::Logger::error("executeQuery failed: {}, sql: {}", mysql_error(conn.get()), sql);
            return results;
        }

        // 获取结果集
        MYSQL_RES* res = mysql_store_result(conn.get());
        if (!res) {
            common::Logger::error("mysql_store_result failed: {}", mysql_error(conn.get()));
            return results;
        }

        // 遍历结果集，调用子类的 rowToModel 进行转换（模板方法核心步骤）
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) {
            try {
                results.push_back(rowToModel(row));
            } catch (const std::exception& e) {
                common::Logger::warn("rowToModel exception: {}", e.what());
            }
        }

        mysql_free_result(res);
        return results;
    }

    /**
     * @brief 执行自定义更新 SQL（INSERT / UPDATE / DELETE）
     * @param sql SQL 语句
     * @return int 受影响的行数，失败返回 -1
     */
    int executeUpdate(std::string_view sql) {
        auto conn = community_system::common::ConnectionPool::instance().getConnection();
        if (!conn) {
            common::Logger::error("executeUpdate: failed to get connection");
            return -1;
        }

        if (mysql_query(conn.get(), sql.data()) != 0) {
            common::Logger::error("executeUpdate failed: {}, sql: {}", mysql_error(conn.get()), sql);
            return -1;
        }

        return static_cast<int>(mysql_affected_rows(conn.get()));
    }

protected:
    /**
     * @brief 获取表名（纯虚，由子类实现）
     * @return std::string 表名
     */
    virtual std::string getTableName() const = 0;

    /**
     * @brief 将数据库行转换为模型对象（模板方法的核心步骤）
     * @param row MySQL 结果行
     * @return T 模型对象
     *
     * 这是模板方法模式中的"原语操作"，由具体 DAO 子类实现。
     * 子类负责将 MYSQL_ROW 中的各列映射到模型对象的字段上。
     */
    virtual T rowToModel(MYSQL_ROW row) = 0;

    /**
     * @brief SQL 字符串转义（防 SQL 注入的基础措施）
     * @param conn 数据库连接
     * @param str 原始字符串
     * @return std::string 转义后的字符串
     *
     * 注意：更好的方式是使用预处理语句（prepared statement），
     * 这里提供基础转义供简单场景使用。
     */
    static std::string escapeString(MYSQL* conn, std::string_view str) {
        if (!conn) return std::string(str);
        std::string escaped;
        escaped.resize(str.size() * 2 + 1);
        unsigned long len = mysql_real_escape_string(conn, escaped.data(),
                                                     str.data(),
                                                     static_cast<unsigned long>(str.size()));
        escaped.resize(len);
        return escaped;
    }
};

}  // namespace dao
}  // namespace community_system
