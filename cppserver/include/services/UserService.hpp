/**
 * @file UserService.hpp
 * @brief 用户服务实现类
 *
 * 实现 IUserService 接口，当前使用硬编码模拟数据。
 * 待 DAO 层完善后，将替换为基于 BaseDAO + 连接池的真实数据库操作。
 *
 * 使用到的 C++ 特性：
 *   - C++11: override 关键字, std::vector, std::mutex
 *   - C++17: std::optional, std::string_view
 *   - C++20: 三路比较等（预留）
 *
 * 设计模式：
 *   - 实现继承：实现 IUserService 接口
 *   - 单例？不，由 App 通过 unique_ptr 管理生命周期
 *
 * @date 2025
 */

#pragma once

#include "IUserService.hpp"
#include "../common/Logger.hpp"

#include <vector>
#include <string>
#include <string_view>
#include <mutex>
#include <algorithm>

namespace community_system {
namespace services {

/**
 * @class UserService
 * @brief 用户服务实现类
 *
 * 当前版本：内存模拟数据（便于开发和测试）
 * 未来版本：将注入 UserDAO，通过连接池访问 MySQL
 *
 * 注意事项：
 *   - 模拟数据使用 std::vector + mutex 保证基本线程安全
 *   - 真实环境应使用数据库事务和锁
 */
class UserService : public IUserService {
public:
    /**
     * @brief 构造函数：初始化模拟数据
     */
    UserService();

    /**
     * @brief 虚析构函数
     */
    ~UserService() override = default;

    // ========== IUserService 接口实现 ==========

    std::vector<models::User> getAllUsers() override;

    std::optional<models::User> getUserById(int id) override;

    std::optional<models::User> getUserByUsername(std::string_view username) override;

    std::optional<models::User> authenticate(std::string_view username,
                                             std::string_view password) override;

    int createUser(const models::User& user) override;

    bool updateUser(const models::User& user) override;

    bool deleteUser(int id) override;

    std::vector<models::User> getUsersByPage(int page, int pageSize, int& total) override;

private:
    std::vector<models::User> m_users;  ///< 模拟用户数据
    std::mutex                m_mutex;  ///< 保护模拟数据的互斥锁
    int                       m_nextId; ///< 下一个可用 ID
};

}  // namespace services
}  // namespace community_system
