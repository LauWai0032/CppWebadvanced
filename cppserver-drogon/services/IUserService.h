/**
 * @file IUserService.h
 * @brief 用户服务接口（抽象基类）
 *
 * 定义用户相关业务操作的接口。
 * 采用依赖倒置原则：Controller 依赖接口而非具体实现。
 *
 * 使用 std::optional 返回值表达"可能不存在"的语义，
 * 比抛出异常或返回 bool + out 参数更现代、更安全。
 *
 * @date 2025
 */

#pragma once

#include "models/User.h"
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>

namespace community {
namespace services {

/**
 * @class IUserService
 * @brief 用户服务接口
 *
 * 所有用户业务操作的抽象接口。
 * 具体实现类必须实现所有纯虚方法。
 */
class IUserService {
public:
    virtual ~IUserService() = default;

    /**
     * @brief 获取所有用户列表
     */
    virtual std::vector<models::User> getAllUsers() = 0;

    /**
     * @brief 根据 ID 查询用户
     * @param id 用户 ID
     * @return 找到返回用户，未找到返回 nullopt
     */
    virtual std::optional<models::User> getUserById(int id) = 0;

    /**
     * @brief 根据用户名查询用户
     * @param username 用户名
     * @return 找到返回用户，未找到返回 nullopt
     */
    virtual std::optional<models::User> getUserByUsername(std::string_view username) = 0;

    /**
     * @brief 用户登录验证
     * @param username 用户名
     * @param password 密码（明文，内部做哈希比对）
     * @return 验证成功返回用户信息，失败返回 nullopt
     */
    virtual std::optional<models::User> authenticate(std::string_view username,
                                                      std::string_view password) = 0;

    /**
     * @brief 创建新用户
     * @param user 用户信息
     * @return 新用户 ID，失败返回 -1
     */
    virtual int createUser(const models::User& user) = 0;

    /**
     * @brief 更新用户信息
     * @param user 用户信息（需包含 id）
     * @return 是否成功
     */
    virtual bool updateUser(const models::User& user) = 0;

    /**
     * @brief 删除用户
     * @param id 用户 ID
     * @return 是否成功
     */
    virtual bool deleteUser(int id) = 0;

    /**
     * @brief 分页查询用户
     * @param page     页码（从 1 开始）
     * @param pageSize 每页条数
     * @param total    输出参数，总记录数
     * @return 用户列表
     */
    virtual std::vector<models::User> getUsersByPage(int page, int pageSize, int& total) = 0;
};

}  // namespace services
}  // namespace community
