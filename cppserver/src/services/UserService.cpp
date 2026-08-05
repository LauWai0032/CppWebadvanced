/**
 * @file UserService.cpp
 * @brief 用户服务实现（内存模拟版本）
 *
 * 此实现使用内存中的 std::vector 模拟数据库，
 * 便于在 DAO 层和数据库环境未准备好时进行开发和测试。
 *
 * TODO: 后续替换为基于 UserDAO 的真实数据库实现
 *
 * @date 2025
 */

#include "services/UserService.hpp"
#include "common/Logger.hpp"

#include <algorithm>
#include <iostream>

namespace community_system {
namespace services {

// ============== 构造函数 ==============

UserService::UserService() : m_nextId(4) {
    // 初始化模拟数据，对应前端登录页面的 admin/123456
    models::User admin;
    admin.id           = 1;
    admin.username     = "admin";
    admin.passwordHash = "123456";  // 模拟：直接用明文，实际应是哈希
    admin.realName     = "系统管理员";
    admin.phone        = "13800000001";
    admin.role         = "admin";
    admin.status       = 1;
    admin.createdAt    = "2024-01-01 00:00:00";
    m_users.push_back(admin);

    models::User property;
    property.id           = 2;
    property.username     = "property01";
    property.passwordHash = "123456";
    property.realName     = "物业小王";
    property.phone        = "13800000002";
    property.role         = "property";
    property.status       = 1;
    property.createdAt    = "2024-01-02 00:00:00";
    m_users.push_back(property);

    models::User owner;
    owner.id           = 3;
    owner.username     = "owner01";
    owner.passwordHash = "123456";
    owner.realName     = "业主张三";
    owner.phone        = "13800000003";
    owner.role         = "owner";
    owner.status       = 1;
    owner.createdAt    = "2024-01-03 00:00:00";
    m_users.push_back(owner);

    common::Logger::info("UserService initialized with {} mock users", m_users.size());
}

// ============== 查询 ==============

std::vector<models::User> UserService::getAllUsers() {
    std::lock_guard<std::mutex> lock(m_mutex);
    common::Logger::debug("UserService::getAllUsers called, total: {}", m_users.size());
    return m_users;
}

std::optional<models::User> UserService::getUserById(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_users.begin(), m_users.end(),
                           [id](const models::User& u) { return u.id == id; });
    if (it != m_users.end()) {
        return *it;
    }
    common::Logger::warn("UserService::getUserById: user {} not found", id);
    return std::nullopt;
}

std::optional<models::User> UserService::getUserByUsername(std::string_view username) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_users.begin(), m_users.end(),
                           [&](const models::User& u) { return u.username == username; });
    if (it != m_users.end()) {
        return *it;
    }
    return std::nullopt;
}

// ============== 登录验证 ==============

std::optional<models::User> UserService::authenticate(std::string_view username,
                                                      std::string_view password) {
    common::Logger::info("UserService::authenticate: user={}", username);

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_users.begin(), m_users.end(),
                           [&](const models::User& u) {
                               return u.username == username;
                           });

    if (it == m_users.end()) {
        common::Logger::warn("Authenticate failed: user '{}' not found", username);
        return std::nullopt;
    }

    // 模拟密码比对（实际应使用 bcrypt 等哈希算法）
    if (it->passwordHash != password) {
        common::Logger::warn("Authenticate failed: wrong password for user '{}'", username);
        return std::nullopt;
    }

    if (!it->isActive()) {
        common::Logger::warn("Authenticate failed: user '{}' is disabled", username);
        return std::nullopt;
    }

    common::Logger::info("Authenticate success: user '{}' (role={})", username, it->role);
    return *it;
}

// ============== 增删改 ==============

int UserService::createUser(const models::User& user) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // 检查用户名是否已存在
    auto it = std::find_if(m_users.begin(), m_users.end(),
                           [&](const models::User& u) { return u.username == user.username; });
    if (it != m_users.end()) {
        common::Logger::warn("createUser: username '{}' already exists", user.username);
        return -1;
    }

    models::User newUser = user;
    newUser.id = m_nextId++;
    if (newUser.status == 0) newUser.status = 1;  // 默认启用
    if (newUser.role.empty()) newUser.role = "owner";  // 默认业主

    m_users.push_back(newUser);
    common::Logger::info("createUser: id={}, username={}", newUser.id, newUser.username);
    return newUser.id;
}

bool UserService::updateUser(const models::User& user) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_users.begin(), m_users.end(),
                           [&](const models::User& u) { return u.id == user.id; });
    if (it == m_users.end()) {
        common::Logger::warn("updateUser: user {} not found", user.id);
        return false;
    }

    // 更新字段（仅更新有值的字段）
    if (!user.username.empty())     it->username     = user.username;
    if (!user.passwordHash.empty()) it->passwordHash = user.passwordHash;
    if (!user.realName.empty())     it->realName     = user.realName;
    if (!user.phone.empty())        it->phone        = user.phone;
    if (!user.role.empty())         it->role         = user.role;
    if (user.status != 0)           it->status       = user.status;  // 0 可能是禁用，这里简化处理

    common::Logger::info("updateUser: id={} updated successfully", user.id);
    return true;
}

bool UserService::deleteUser(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::remove_if(m_users.begin(), m_users.end(),
                             [id](const models::User& u) { return u.id == id; });
    if (it == m_users.end()) {
        common::Logger::warn("deleteUser: user {} not found", id);
        return false;
    }
    m_users.erase(it, m_users.end());
    common::Logger::info("deleteUser: id={} deleted successfully", id);
    return true;
}

// ============== 分页 ==============

std::vector<models::User> UserService::getUsersByPage(int page, int pageSize, int& total) {
    std::lock_guard<std::mutex> lock(m_mutex);

    total = static_cast<int>(m_users.size());

    if (page < 1) page = 1;
    if (pageSize < 1) pageSize = 10;

    int start = (page - 1) * pageSize;
    if (start >= total) {
        return {};
    }

    int end = std::min(start + pageSize, total);
    std::vector<models::User> result;
    for (int i = start; i < end; ++i) {
        result.push_back(m_users[i]);
    }

    common::Logger::debug("getUsersByPage: page={}, size={}, total={}", page, pageSize, total);
    return result;
}

}  // namespace services
}  // namespace community_system
