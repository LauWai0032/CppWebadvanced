/**
 * @file UserController.hpp
 * @brief 用户控制器
 *
 * 处理 /api/user 前缀的所有 HTTP 请求，包括：
 *   - 用户登录
 *   - 用户列表（分页）
 *   - 用户详情
 *   - 用户创建 / 更新 / 删除
 *
 * 使用到的 C++ 特性：
 *   - C++11: std::shared_ptr, lambda, std::string
 *   - C++14: std::make_unique / make_shared
 *   - C++17: std::optional, std::string_view, if constexpr
 *   - C++20: 三路比较等（预留）
 *
 * 设计模式：
 *   - 依赖注入：UserService 通过构造函数注入（接口 IUserService）
 *   - 组合：Controller 持有 Service 的智能指针
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <memory>

#include "../common/BaseController.hpp"
#include "../services/IUserService.hpp"

namespace community_system {
namespace controllers {

/**
 * @class UserController
 * @brief 用户控制器
 *
 * 负责用户相关的 HTTP 请求处理，将请求参数转发给 UserService，
 * 并将结果封装为统一的 Response 返回。
 *
 * 依赖注入：
 *   - 通过构造函数注入 IUserService 的实现
 *   - 使用 shared_ptr 管理 Service 生命周期（可能被多个 Controller 共享）
 *
 * 路由前缀：/api/user
 */
class UserController : public BaseController {
public:
    /**
     * @brief 构造函数
     * @param userService 用户服务实例（通过接口注入，便于替换和测试）
     */
    explicit UserController(std::shared_ptr<services::IUserService> userService);

    /**
     * @brief 析构函数
     */
    ~UserController() override = default;

    /**
     * @brief 注册所有用户相关路由
     * @param svr httplib 服务器引用
     *
     * 路由列表：
     *   POST   /api/user/login       - 用户登录
     *   GET    /api/user/list        - 用户列表（分页）
     *   GET    /api/user/:id         - 用户详情
     *   POST   /api/user             - 创建用户
     *   PUT    /api/user/:id         - 更新用户
     *   DELETE /api/user/:id         - 删除用户
     */
    void registerRoutes(httplib::Server& svr) override;

private:
    /**
     * @brief 登录接口处理
     * POST /api/user/login
     */
    void handleLogin(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 用户列表接口处理
     * GET /api/user/list?page=1&pageSize=10
     */
    void handleList(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 用户详情接口处理
     * GET /api/user/:id
     */
    void handleDetail(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 创建用户接口处理
     * POST /api/user
     */
    void handleCreate(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 更新用户接口处理
     * PUT /api/user/:id
     */
    void handleUpdate(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 删除用户接口处理
     * DELETE /api/user/:id
     */
    void handleDelete(const httplib::Request& req, httplib::Response& res);

private:
    std::shared_ptr<services::IUserService> m_userService;  ///< 用户服务实例
};

}  // namespace controllers
}  // namespace community_system
