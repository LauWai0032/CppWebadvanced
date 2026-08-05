/**
 * @file UserController.h
 * @brief 用户控制器（Drogon HttpController）
 *
 * 继承 drogon::HttpController，使用 PATH_ADD 宏自动注册路由。
 * 与 cppserver 版本的 UserController 功能完全一致，
 * 但利用了 Drogon 框架的路由注册机制，无需手动绑定路由。
 *
 * 路由映射：
 *   GET    /api/users          — 获取用户列表
 *   GET    /api/users/{id}     — 获取单个用户
 *   POST   /api/login          — 登录接口
 *   POST   /api/users          — 创建用户
 *   PUT    /api/users/{id}     — 更新用户
 *   DELETE /api/users/{id}     — 删除用户
 *
 * 设计模式：
 *   - 工厂模式：Drogon 自动实例化 Controller
 *   - 依赖注入：通过 UserService 全局实例获取服务
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpController.h>
#include "services/UserService.h"
#include "common/ResponseUtil.h"
#include "common/Logger.h"

namespace community {
namespace controllers {

/**
 * @class UserController
 * @brief 用户控制器
 *
 * 使用 Drogon 的 HttpController 基类，通过 PATH_LIST_BEGIN / PATH_ADD 宏
 * 声明式注册路由，比 httplib 手动注册更清晰。
 */
class UserController : public drogon::HttpController<UserController> {
public:
    // ========== Drogon 路由注册（声明式） ==========
    METHOD_LIST_BEGIN
    // GET    /api/users       — 用户列表（分页）
    ADD_METHOD_TO(UserController::getUserList,   "/api/users", drogon::Get);
    // GET    /api/users/{1}   — 用户详情
    ADD_METHOD_TO(UserController::getUserById,   "/api/users/{1}", drogon::Get);
    // POST   /api/login       — 用户登录
    ADD_METHOD_TO(UserController::login,         "/api/login",   drogon::Post);
    // POST   /api/users       — 创建用户
    ADD_METHOD_TO(UserController::createUser,    "/api/users",   drogon::Post);
    // PUT    /api/users/{1}   — 更新用户
    ADD_METHOD_TO(UserController::updateUser,    "/api/users/{1}", drogon::Put);
    // DELETE /api/users/{1}   — 删除用户
    ADD_METHOD_TO(UserController::deleteUser,    "/api/users/{1}", drogon::Delete);
    METHOD_LIST_END

    /**
     * @brief 构造函数
     */
    UserController();

    /**
     * @brief 获取用户服务实例
     */
    std::shared_ptr<services::UserService> getUserService() const;

private:
    /**
     * @brief 获取用户列表（分页）
     * GET /api/users?page=1&pageSize=10
     */
    void getUserList(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /**
     * @brief 获取单个用户
     * GET /api/users/{id}
     */
    void getUserById(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                     int id);

    /**
     * @brief 用户登录
     * POST /api/login
     */
    void login(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /**
     * @brief 创建用户
     * POST /api/users
     */
    void createUser(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /**
     * @brief 更新用户
     * PUT /api/users/{id}
     */
    void updateUser(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    int id);

    /**
     * @brief 删除用户
     * DELETE /api/users/{id}
     */
    void deleteUser(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    int id);
};

}  // namespace controllers
}  // namespace community
