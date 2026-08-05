/**
 * @file VehicleController.h
 * @brief Vehicle控制器（Drogon HttpController）
 *
 * 继承 drogon::HttpController，使用 ADD_METHOD_TO 宏自动注册路由。
 *
 * 路由映射：
 *   GET    /api/vehicles       — 列表（分页，需认证）
 *   GET    /api/vehicles/{id}  — 详情（需认证）
 *   POST   /api/vehicles       — 新增（需认证）
 *   PUT    /api/vehicles/{id}  — 修改（需认证）
 *   DELETE /api/vehicles/{id}  — 删除（需认证）
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpController.h>
#include "services/VehicleService.h"
#include "common/ResponseUtil.h"
#include "common/Logger.h"
#include "filters/AuthFilter.h"

namespace community {
namespace controllers {

/**
 * @class VehicleController
 * @brief Vehicle控制器
 */
class VehicleController : public drogon::HttpController<VehicleController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(VehicleController::getList,    "/api/vehicles", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(VehicleController::getById,    "/api/vehicles/{1}", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(VehicleController::create,     "/api/vehicles", drogon::Post, "AuthFilter");
    ADD_METHOD_TO(VehicleController::update,     "/api/vehicles/{1}", drogon::Put, "AuthFilter");
    ADD_METHOD_TO(VehicleController::remove,     "/api/vehicles/{1}", drogon::Delete, "AuthFilter");

    METHOD_LIST_END

    VehicleController();

    std::shared_ptr<services::VehicleService> getService() const;

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
