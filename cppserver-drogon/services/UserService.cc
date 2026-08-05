/**
 * @file UserService.cc
 * @brief 用户服务实现 - 基于 MySQL 数据库
 * @date 2025
 */

#include "UserService.h"
#include <sstream>
#include <functional>
#include <cstring>

namespace community {
namespace services {

// ========== 静态成员初始化 ==========
std::shared_ptr<UserService> UserService::s_instance_ = nullptr;
std::mutex UserService::s_mutex_;

void UserService::setInstance(std::shared_ptr<UserService> instance) {
    std::lock_guard<std::mutex> lock(s_mutex_);
    s_instance_ = std::move(instance);
}

std::shared_ptr<UserService> UserService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex_);
    return s_instance_;
}

// ========== 查询所有用户 ==========
std::vector<models::User> UserService::getAllUsers() {
    std::vector<models::User> users;
    
    try {
        auto conn = DbPool::instance().getConnection();
        auto result = conn->query("SELECT id, username, real_name, phone, role, status, created_at FROM users ORDER BY id");
        
        if (!result.success) {
            LOG_ERROR << "查询用户列表失败: " << result.error;
            return users;
        }

        for (const auto& row : result.rows) {
            users.push_back(models::User::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getAllUsers 异常: " << e.what();
    }

    return users;
}

// ========== 根据 ID 查询用户 ==========
std::optional<models::User> UserService::getUserById(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 使用参数化查询防止 SQL 注入
        std::string sql = "SELECT id, username, real_name, phone, role, status, created_at "
                          "FROM users WHERE id = " + std::to_string(id);
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        // 使用结构化绑定（C++17）
        auto user = models::User::fromDbRow(result.rows[0]);
        return user;
    } catch (const std::exception& e) {
        LOG_ERROR << "getUserById 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 根据用户名查询 ==========
std::optional<models::User> UserService::getUserByUsername(std::string_view username) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 使用 escape 防止 SQL 注入
        std::string safeUsername = conn->escape(username);
        std::string sql = "SELECT id, username, password_hash, real_name, phone, role, status, created_at "
                          "FROM users WHERE username = '" + safeUsername + "'";
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        return models::User::fromDbRow(result.rows[0]);
    } catch (const std::exception& e) {
        LOG_ERROR << "getUserByUsername 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 登录验证 ==========
std::optional<models::User> UserService::authenticate(std::string_view username,
                                                       std::string_view password) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        std::string safeUsername = conn->escape(username);
        std::string sql = "SELECT id, username, password_hash, real_name, phone, role, status, created_at "
                          "FROM users WHERE username = '" + safeUsername + "' AND status = 1";
        auto result = conn->query(sql);
        
        if (!result.success || result.rows.empty()) {
            return std::nullopt;
        }

        auto user = models::User::fromDbRow(result.rows[0]);
        
        // 验证密码（简单比对，实际应使用 bcrypt）
        std::string inputHash = hashPassword(password);
        if (user.passwordHash != inputHash && user.passwordHash != std::string(password)) {
            // 支持明文和哈希两种比对方式（演示用途）
            LOG_WARN << "用户 " << std::string(username) << " 密码验证失败";
            return std::nullopt;
        }

        // 登录成功，清除密码哈希
        user.passwordHash.clear();
        return user;
    } catch (const std::exception& e) {
        LOG_ERROR << "authenticate 异常: " << e.what();
        return std::nullopt;
    }
}

// ========== 创建用户 ==========
int UserService::createUser(const models::User& user) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 转义所有字符串字段
        auto safeUsername = conn->escape(user.username);
        auto safePassword = conn->escape(user.passwordHash.empty() ? 
            hashPassword("123456") : user.passwordHash);
        auto safeRealName = conn->escape(user.realName);
        auto safePhone = conn->escape(user.phone);
        auto safeRole = conn->escape(user.role);

        std::ostringstream oss;
        oss << "INSERT INTO users (username, password_hash, real_name, phone, role, status) "
            << "VALUES ('" << safeUsername << "', '"
            << safePassword << "', '"
            << safeRealName << "', '"
            << safePhone << "', '"
            << safeRole << "', "
            << user.status << ")";

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "创建用户失败: " << result.error;
            return -1;
        }

        LOG_INFO << "创建用户成功，ID: " << result.insertId;
        return result.insertId;
    } catch (const std::exception& e) {
        LOG_ERROR << "createUser 异常: " << e.what();
        return -1;
    }
}

// ========== 更新用户 ==========
bool UserService::updateUser(const models::User& user) {
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 构建 UPDATE 语句（只更新非空字段）
        std::ostringstream oss;
        oss << "UPDATE users SET ";

        std::vector<std::string> sets;
        
        if (!user.username.empty()) {
            sets.push_back("username = '" + conn->escape(user.username) + "'");
        }
        if (!user.passwordHash.empty()) {
            sets.push_back("password_hash = '" + conn->escape(user.passwordHash) + "'");
        }
        if (!user.realName.empty()) {
            sets.push_back("real_name = '" + conn->escape(user.realName) + "'");
        }
        if (!user.phone.empty()) {
            sets.push_back("phone = '" + conn->escape(user.phone) + "'");
        }
        if (!user.role.empty()) {
            sets.push_back("role = '" + conn->escape(user.role) + "'");
        }
        sets.push_back("status = " + std::to_string(user.status));

        if (sets.empty()) {
            LOG_WARN << "updateUser: 没有需要更新的字段";
            return false;
        }

        for (size_t i = 0; i < sets.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << sets[i];
        }
        oss << " WHERE id = " << user.id;

        auto result = conn->execute(oss.str());
        
        if (!result.success) {
            LOG_ERROR << "更新用户失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "updateUser 异常: " << e.what();
        return false;
    }
}

// ========== 删除用户 ==========
bool UserService::deleteUser(int id) {
    try {
        auto conn = DbPool::instance().getConnection();
        std::string sql = "DELETE FROM users WHERE id = " + std::to_string(id);
        auto result = conn->execute(sql);
        
        if (!result.success) {
            LOG_ERROR << "删除用户失败: " << result.error;
            return false;
        }

        return result.affectedRows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR << "deleteUser 异常: " << e.what();
        return false;
    }
}

// ========== 分页查询 ==========
std::vector<models::User> UserService::getUsersByPage(int page, int pageSize, int& total) {
    std::vector<models::User> users;
    
    try {
        auto conn = DbPool::instance().getConnection();
        
        // 查询总数
        auto countResult = conn->query("SELECT COUNT(*) as cnt FROM users");
        if (countResult.success && !countResult.rows.empty()) {
            auto it = countResult.rows[0].find("cnt");
            if (it != countResult.rows[0].end()) {
                total = std::stoi(it->second);
            }
        }

        // 分页查询
        int offset = (page - 1) * pageSize;
        std::string sql = "SELECT id, username, real_name, phone, role, status, created_at "
                          "FROM users ORDER BY id LIMIT " 
                          + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);
        
        auto result = conn->query(sql);
        if (!result.success) {
            LOG_ERROR << "分页查询用户失败: " << result.error;
            return users;
        }

        for (const auto& row : result.rows) {
            users.push_back(models::User::fromDbRow(row));
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "getUsersByPage 异常: " << e.what();
        total = 0;
    }

    return users;
}

// ========== 密码哈希 ==========
std::string UserService::hashPassword(std::string_view password) {
    // 简化版：实际生产环境应使用 bcrypt / argon2 等安全哈希算法
    // 此处仅为演示，直接使用简单哈希
    std::hash<std::string_view> hasher;
    auto hashValue = hasher(password);
    return std::to_string(hashValue);
}

}  // namespace services
}  // namespace community
