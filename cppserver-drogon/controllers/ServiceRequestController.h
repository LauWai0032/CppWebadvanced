/**
 * @file ServiceRequestController.h
 * @brief ServiceRequest控制器（Drogon HttpController）
 *
 * 继承 drogon::HttpController，使用 ADD_METHOD_TO 宏自动注册路由。
 *
 * 路由映射：
 *   GET    /api/service-requests       — 列表（分页，需认证）
 *   GET    /api/service-requests/{id}  — 详情（需认证）
 *   POST   /api/service-requests       — 新增（需认证）
 *   PUT    /api/service-requests/{id}  — 修改（需认证）
 *   DELETE /api/service-requests/{id}  — 删除（需认证）
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpController.h>
#include "services/ServiceRequestService.h"
#include "common/ResponseUtil.h"
#include "common/Logger.h"
#include "filters/AuthFilter.h"

namespace community {
namespace controllers {

/**
 * @class ServiceRequestController
 * @brief ServiceRequest控制器
 */
class ServiceRequestController : public drogon::HttpController<ServiceRequestController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ServiceRequestController::getList,    "/api/service-requests", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(ServiceRequestController::getById,    "/api/service-requests/{1}", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(ServiceRequestController::create,     "/api/service-requests", drogon::Post, "AuthFilter");
    ADD_METHOD_TO(ServiceRequestController::update,     "/api/service-requests/{1}", drogon::Put, "AuthFilter");
    ADD_METHOD_TO(ServiceRequestController::remove,     "/api/service-requests/{1}", drogon::Delete, "AuthFilter");

    ADD_METHOD_TO(ServiceRequestController::updateStatus, "/api/service-requests/{1}/status", drogon::Put, "AuthFilter");

    METHOD_LIST_END

    ServiceRequestController();

    std::shared_ptr<services::ServiceRequestService> getService() const;

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

    /**
     * @brief 修改状态
     */
    void updateStatus(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                      int id);

};

}  // namespace controllers
}  // namespace community
