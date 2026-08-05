/**
 * @file User.hpp
 * @brief 用户模型
 *
 * 对应数据库 users 表的用户实体类。
 *
 * 数据库表结构：
 *   id            INT PRIMARY KEY AUTO_INCREMENT
 *   username      VARCHAR(50) UNIQUE NOT NULL   -- 登录用户名
 *   password_hash VARCHAR(255) NOT NULL         -- 密码哈希（bcrypt 等）
 *   real_name     VARCHAR(50)                   -- 真实姓名
 *   phone         VARCHAR(20)                   -- 手机号
 *   role          VARCHAR(20) NOT NULL          -- 角色：admin/property/owner
 *   status        TINYINT DEFAULT 1             -- 状态：1-正常，0-禁用
 *   created_at    DATETIME DEFAULT CURRENT_TIMESTAMP
 *
 * 使用到的 C++ 特性：
 *   - C++11: 统一初始化, =default, explicit 构造
 *   - C++17: std::string_view
 *   - C++20: 三路比较运算符（预留）
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <optional>

// nlohmann/json（项目根目录）
#include "../../json.hpp"

namespace community_system {
namespace models {

/**
 * @enum UserRole
 * @brief 用户角色枚举
 */
enum class UserRole {
    Admin,      ///< 系统管理员
    Property,   ///< 物业人员
    Owner       ///< 业主
};

/**
 * @enum UserStatus
 * @brief 用户状态枚举
 */
enum class UserStatus : int {
    Disabled = 0,  ///< 禁用
    Active   = 1   ///< 正常
};

/**
 * @class User
 * @brief 用户模型类
 *
 * 设计思路：
 *  - 简单的 POCO（Plain Old C++ Object），只有字段和简单方法
 *  - 包含 id 字段，满足 BaseDAO 的 HasId concept 约束
 *  - 提供 toJson() 方法方便序列化
 *  - 提供 fromJson() 静态方法方便反序列化
 */
struct User {
    int          id           = 0;           ///< 主键 ID
    std::string  username;                    ///< 登录用户名
    std::string  passwordHash;                ///< 密码哈希（注意：返回给前端时应清空）
    std::string  realName;                    ///< 真实姓名
    std::string  phone;                       ///< 手机号
    std::string  role;                        ///< 角色字符串：admin/property/owner
    int          status       = 1;           ///< 状态：1-正常，0-禁用
    std::string  createdAt;                   ///< 创建时间

    /**
     * @brief 默认构造函数
     */
    User() = default;

    /**
     * @brief 带参数的构造函数（explicit 防止隐式转换）
     */
    explicit User(int id, std::string_view uname, std::string_view rname = "")
        : id(id), username(uname), realName(rname) {}

    /**
     * @brief 转换为 JSON 对象（用于返回给前端）
     * @param includePassword 是否包含密码哈希（默认 false，安全考虑）
     * @return nlohmann::json
     */
    nlohmann::json toJson(bool includePassword = false) const {
        nlohmann::json j = {
            {"id",         id},
            {"username",   username},
            {"real_name",  realName},
            {"phone",      phone},
            {"role",       role},
            {"status",     status},
            {"created_at", createdAt}
        };
        if (includePassword) {
            j["password_hash"] = passwordHash;
        }
        return j;
    }

    /**
     * @brief 从 JSON 创建 User 对象
     * @param j JSON 对象
     * @return User
     */
    static User fromJson(const nlohmann::json& j) {
        User u;
        u.id           = j.value("id", 0);
        u.username     = j.value("username", "");
        u.passwordHash = j.value("password_hash", "");
        u.realName     = j.value("real_name", "");
        u.phone        = j.value("phone", "");
        u.role         = j.value("role", "owner");
        u.status       = j.value("status", 1);
        u.createdAt    = j.value("created_at", "");
        return u;
    }

    /**
     * @brief 判断是否为管理员
     */
    bool isAdmin() const { return role == "admin"; }

    /**
     * @brief 判断是否为物业人员
     */
    bool isProperty() const { return role == "property"; }

    /**
     * @brief 判断是否为业主
     */
    bool isOwner() const { return role == "owner"; }

    /**
     * @brief 判断账号是否可用（状态正常）
     */
    bool isActive() const { return status == 1; }
};

}  // namespace models
}  // namespace community_system
