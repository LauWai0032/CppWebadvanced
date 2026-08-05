/**
 * @file UserService.cc
 * @brief 用户服务实现 - 基于 MySQL 数据库
 * @date 2025
 */

#include "UserService.h"
#include "common/PasswordUtil.h"
#include <sstream>
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
            LOG_WARN << "用户不存在或已被禁用: " << std::string(username);
            return std::nullopt;
        }

        auto user = models::User::fromDbRow(result.rows[0]);
        
        // 使用 PasswordUtil 验证密码（SHA256 + salt）
        if (!PasswordUtil::verifyPassword(password, user.passwordHash)) {
            LOG_WARN << "用户 " << std::string(username) << " 密码验证失败";
            return std::nullopt;
        }

        // 登录成功，清除密码哈希（安全考虑）
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
        // 对密码进行哈希处理（SHA256 + salt）
        std::string hashedPwd = PasswordUtil::hashPassword(
            user.passwordHash.empty() ? "123456" : user.passwordHash);
        auto safePassword = conn->escape(hashedPwd);
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
            // 密码字段不为空时，加密后更新
            std::string hashedPwd = PasswordUtil::hashPassword(user.passwordHash);
            sets.push_back("password_hash = '" + conn->escape(hashedPwd) + "'");
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
        // status 字段始终更新（0 也有效）
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

}  // namespace services
}  // namespace community
