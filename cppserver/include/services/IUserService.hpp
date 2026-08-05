/**
 * @file IUserService.hpp
 * @brief 用户服务接口（抽象基类）
 *
 * 定义用户相关业务操作的接口，采用依赖倒置原则：
 * Controller 依赖接口而非具体实现，便于替换和单元测试。
 *
 * 使用到的 C++ 特性：
 *   - C++11: 虚析构函数 =default, 纯虚函数, override
 *   - C++17: std::optional, std::string_view
 *
 * 设计模式：
 *   - 接口隔离原则：用户相关操作集中在一个接口中
 *   - 依赖倒置：高层模块（Controller）依赖抽象接口，而非具体实现
 *
 * @date 2025
 */

#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <memory>

#include "../models/User.hpp"

namespace community_system {
namespace services {

/**
 * @class IUserService
 * @brief 用户服务接口
 *
 * 所有用户业务操作的抽象接口。
 * 具体实现类（如 UserService）必须实现所有纯虚方法。
 *
 * 使用 std::optional 返回值表达"可能不存在"的语义（如根据ID查询），
 * 比抛出异常或返回 bool + out 参数更现代、更安全。
 */
class IUserService {
public:
    /**
     * @brief 虚析构函数（确保通过基类指针删除子类时正确析构）
     */
    virtual ~IUserService() = default;

    /**
     * @brief 获取所有用户列表
     * @return std::vector<models::User> 用户列表
     */
    virtual std::vector<models::User> getAllUsers() = 0;

    /**
     * @brief 根据 ID 查询用户
     * @param id 用户 ID
     * @return std::optional<models::User> 找到返回用户，未找到返回 nullopt
     */
    virtual std::optional<models::User> getUserById(int id) = 0;

    /**
     * @brief 根据用户名查询用户
     * @param username 用户名
     * @return std::optional<models::User>
     */
    virtual std::optional<models::User> getUserByUsername(std::string_view username) = 0;

    /**
     * @brief 用户登录验证
     * @param username 用户名
     * @param password 密码（明文，内部会做哈希比对）
     * @return std::optional<models::User>
     *         验证成功返回用户信息，失败返回 nullopt
     */
    virtual std::optional<models::User> authenticate(std::string_view username,
                                                     std::string_view password) = 0;

    /**
     * @brief 创建新用户
     * @param user 用户信息
     * @return int 新用户 ID，失败返回 -1
     */
    virtual int createUser(const models::User& user) = 0;

    /**
     * @brief 更新用户信息
     * @param user 用户信息（需包含 id）
     * @return bool 是否成功
     */
    virtual bool updateUser(const models::User& user) = 0;

    /**
     * @brief 删除用户
     * @param id 用户 ID
     * @return bool 是否成功
     */
    virtual bool deleteUser(int id) = 0;

    /**
     * @brief 分页查询用户
     * @param page 页码（从 1 开始）
     * @param pageSize 每页条数
     * @param total 输出参数，总记录数
     * @return std::vector<models::User>
     */
    virtual std::vector<models::User> getUsersByPage(int page, int pageSize, int& total) = 0;
};

}  // namespace services
}  // namespace community_system
