/**
 * @file UserService.h
 * @brief 用户服务实现类
 *
 * 实现 IUserService 接口，通过 MySQL 数据库进行实际的数据持久化。
 * 
 * 设计模式：
 *   - 依赖注入：通过静态实例提供全局访问点
 *   - 接口隔离：实现 IUserService 抽象接口
 *
 * @date 2025
 */

#pragma once

#include "IUserService.h"
#include "common/DbPool.h"
#include "common/Logger.h"

#include <mutex>

namespace community {
namespace services {

/**
 * @class UserService
 * @brief 用户服务实现类（MySQL 数据库版）
 *
 * 通过 DbPool 获取数据库连接，执行 SQL 操作。
 * 所有数据库操作都是线程安全的（连接池 + MySQL 自身线程安全机制）。
 *
 * 密码存储：
 *   - 使用 SHA256 + 随机盐值（见 PasswordUtil）
 *   - 存储格式：salt$hash
 */
class UserService : public IUserService {
public:
    UserService() = default;
    ~UserService() override = default;

    // ========== 全局实例管理（依赖注入） ==========

    /**
     * @brief 设置全局服务实例
     */
    static void setInstance(std::shared_ptr<UserService> instance);

    /**
     * @brief 获取全局服务实例
     */
    static std::shared_ptr<UserService> getInstance();

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
    /**
     * @brief 全局服务实例
     */
    static std::shared_ptr<UserService> s_instance_;
    static std::mutex s_mutex_;
};

}  // namespace services
}  // namespace community
