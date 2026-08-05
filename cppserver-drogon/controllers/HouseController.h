/**
 * @file HouseController.h
 * @brief House控制器（Drogon HttpController）
 *
 * 继承 drogon::HttpController，使用 ADD_METHOD_TO 宏自动注册路由。
 *
 * 路由映射：
 *   GET    /api/houses       — 列表（分页，需认证）
 *   GET    /api/houses/{id}  — 详情（需认证）
 *   POST   /api/houses       — 新增（需认证）
 *   PUT    /api/houses/{id}  — 修改（需认证）
 *   DELETE /api/houses/{id}  — 删除（需认证）
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpController.h>
#include "services/HouseService.h"
#include "common/ResponseUtil.h"
#include "common/Logger.h"
#include "filters/AuthFilter.h"

namespace community {
namespace controllers {

/**
 * @class HouseController
 * @brief House控制器
 */
class HouseController : public drogon::HttpController<HouseController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HouseController::getList,    "/api/houses", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(HouseController::getById,    "/api/houses/{1}", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(HouseController::create,     "/api/houses", drogon::Post, "AuthFilter");
    ADD_METHOD_TO(HouseController::update,     "/api/houses/{1}", drogon::Put, "AuthFilter");
    ADD_METHOD_TO(HouseController::remove,     "/api/houses/{1}", drogon::Delete, "AuthFilter");

    METHOD_LIST_END

    HouseController();

    std::shared_ptr<services::HouseService> getService() const;

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
