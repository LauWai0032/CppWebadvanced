/**
 * @file ForumPostController.h
 * @brief ForumPost控制器（Drogon HttpController）
 *
 * 继承 drogon::HttpController，使用 ADD_METHOD_TO 宏自动注册路由。
 *
 * 路由映射：
 *   GET    /api/forum/posts       — 列表（分页，需认证）
 *   GET    /api/forum/posts/{id}  — 详情（需认证）
 *   POST   /api/forum/posts       — 新增（需认证）
 *   PUT    /api/forum/posts/{id}  — 修改（需认证）
 *   DELETE /api/forum/posts/{id}  — 删除（需认证）
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpController.h>
#include "services/ForumPostService.h"
#include "common/ResponseUtil.h"
#include "common/Logger.h"
#include "filters/AuthFilter.h"

namespace community {
namespace controllers {

/**
 * @class ForumPostController
 * @brief ForumPost控制器
 */
class ForumPostController : public drogon::HttpController<ForumPostController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ForumPostController::getList,    "/api/forum/posts", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(ForumPostController::getById,    "/api/forum/posts/{1}", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(ForumPostController::create,     "/api/forum/posts", drogon::Post, "AuthFilter");
    ADD_METHOD_TO(ForumPostController::update,     "/api/forum/posts/{1}", drogon::Put, "AuthFilter");
    ADD_METHOD_TO(ForumPostController::remove,     "/api/forum/posts/{1}", drogon::Delete, "AuthFilter");

    METHOD_LIST_END

    ForumPostController();

    std::shared_ptr<services::ForumPostService> getService() const;

private:
    void getList(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getById(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                 int id);

    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void update(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                int id);

    void remove(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                int id);

};

}  // namespace controllers
}  // namespace community
