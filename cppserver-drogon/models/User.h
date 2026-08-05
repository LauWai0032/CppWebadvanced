/**
 * @file User.h
 * @brief 用户模型
 *
 * 对应数据库 users 表的用户实体类。
 * 使用 C++20 特性，提供现代化的数据模型定义。
 *
 * 数据库表结构：
 *   id            INT PRIMARY KEY AUTO_INCREMENT
 *   username      VARCHAR(50) UNIQUE NOT NULL
 *   password_hash VARCHAR(255) NOT NULL
 *   real_name     VARCHAR(50)
 *   phone         VARCHAR(20)
 *   role          ENUM('admin','property','owner')
 *   status        TINYINT DEFAULT 1
 *   created_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <json/json.h>
#include "common/DbPool.h"

namespace community {
namespace models {

/**
 * @enum UserRole
 * @brief 用户角色枚举（强类型）
 */
enum class UserRole {
    Admin,     ///< 系统管理员
    Property,  ///< 物业人员
    Owner      ///< 业主
};

/**
 * @brief 角色字符串转枚举
 */
inline UserRole roleFromString(std::string_view s) {
    if (s == "admin") return UserRole::Admin;
    if (s == "property") return UserRole::Property;
    return UserRole::Owner;
}

/**
 * @brief 角色枚举转字符串
 */
inline std::string_view roleToString(UserRole r) {
    switch (r) {
        case UserRole::Admin:    return "admin";
        case UserRole::Property: return "property";
        case UserRole::Owner:    return "owner";
    }
    return "owner";
}

/**
 * @struct User
 * @brief 用户模型结构体
 *
 * 简单的 POCO（Plain Old C++ Object），
 * 提供 toJson/fromDbRow 等转换方法。
 */
struct User {
    int         id           = 0;            ///< 主键 ID
    std::string username;                     ///< 登录用户名
    std::string passwordHash;                 ///< 密码哈希（返回前端时清空）
    std::string realName;                     ///< 真实姓名
    std::string phone;                        ///< 手机号
    std::string role = "owner";               ///< 角色字符串
    int         status       = 1;             ///< 状态：1-正常 0-禁用
    std::string createdAt;                    ///< 创建时间

    User() = default;

    /**
     * @brief 带参数构造（使用 string_view 避免拷贝）
     */
    explicit User(int id, std::string_view uname, std::string_view rname = "")
        : id(id), username(uname), realName(rname) {}

    /**
     * @brief 转换为 JSON 对象
     * @param includePassword 是否包含密码哈希（默认 false，安全考虑）
     */
    Json::Value toJson(bool includePassword = false) const {
        Json::Value j;
        j["id"] = id;
        j["username"] = username;
        j["real_name"] = realName;
        j["phone"] = phone;
        j["role"] = role;
        j["status"] = status;
        j["created_at"] = createdAt;
        if (includePassword) {
            j["password_hash"] = passwordHash;
        }
        return j;
    }

    /**
     * @brief 从 JSON 创建 User 对象
     */
    static User fromJson(const Json::Value& j) {
        User u;
        u.id           = j.get("id", 0).asInt();
        u.username     = j.get("username", "").asString();
        u.passwordHash = j.get("password_hash", "").asString();
        u.realName     = j.get("real_name", "").asString();
        u.phone        = j.get("phone", "").asString();
        u.role         = j.get("role", "owner").asString();
        u.status       = j.get("status", 1).asInt();
        u.createdAt    = j.get("created_at", "").asString();
        return u;
    }

    /**
     * @brief 从数据库行创建 User 对象
     * @param row 数据库查询结果行
     */
    static User fromDbRow(const DbRow& row) {
        User u;
        auto getStr = [&](const std::string& key) -> std::string {
            auto it = row.find(key);
            return (it != row.end()) ? it->second : "";
        };
        auto getInt = [&](const std::string& key, int def = 0) -> int {
            auto s = getStr(key);
            if (s.empty()) return def;
            try { return std::stoi(s); } catch (...) { return def; }
        };

        u.id           = getInt("id");
        u.username     = getStr("username");
        u.passwordHash = getStr("password_hash");
        u.realName     = getStr("real_name");
        u.phone        = getStr("phone");
        u.role         = getStr("role");
        u.status       = getInt("status", 1);
        u.createdAt    = getStr("created_at");
        return u;
    }

    /**
     * @brief 判断是否为管理员
     */
    [[nodiscard]] bool isAdmin() const { return role == "admin"; }

    /**
     * @brief 判断是否为物业人员
     */
    [[nodiscard]] bool isProperty() const { return role == "property"; }

    /**
     * @brief 判断是否为业主
     */
    [[nodiscard]] bool isOwner() const { return role == "owner"; }

    /**
     * @brief 判断账号是否可用
     */
    [[nodiscard]] bool isActive() const { return status == 1; }
};

}  // namespace models
}  // namespace community
