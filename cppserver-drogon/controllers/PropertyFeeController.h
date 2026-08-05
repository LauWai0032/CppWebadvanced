/**
 * @file PropertyFeeController.h
 * @brief PropertyFee控制器（Drogon HttpController）
 *
 * 继承 drogon::HttpController，使用 ADD_METHOD_TO 宏自动注册路由。
 *
 * 路由映射：
 *   GET    /api/property-fees       — 列表（分页，需认证）
 *   GET    /api/property-fees/{id}  — 详情（需认证）
 *   POST   /api/property-fees       — 新增（需认证）
 *   PUT    /api/property-fees/{id}  — 修改（需认证）
 *   DELETE /api/property-fees/{id}  — 删除（需认证）
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpController.h>
#include "services/PropertyFeeService.h"
#include "common/ResponseUtil.h"
#include "common/Logger.h"
#include "filters/AuthFilter.h"

namespace community {
namespace controllers {

/**
 * @class PropertyFeeController
 * @brief PropertyFee控制器
 */
class PropertyFeeController : public drogon::HttpController<PropertyFeeController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(PropertyFeeController::getList,    "/api/property-fees", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(PropertyFeeController::getById,    "/api/property-fees/{1}", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(PropertyFeeController::create,     "/api/property-fees", drogon::Post, "AuthFilter");
    ADD_METHOD_TO(PropertyFeeController::update,     "/api/property-fees/{1}", drogon::Put, "AuthFilter");
    ADD_METHOD_TO(PropertyFeeController::remove,     "/api/property-fees/{1}", drogon::Delete, "AuthFilter");

    ADD_METHOD_TO(PropertyFeeController::pay,         "/api/property-fees/{1}/pay", drogon::Put, "AuthFilter");

    METHOD_LIST_END

    PropertyFeeController();

    std::shared_ptr<services::PropertyFeeService> getService() const;

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
     * @brief 缴费
     */
    void pay(const drogon::HttpRequestPtr& req,
             std::function<void(const drogon::HttpResponsePtr&)>&& callback,
             int id);

};

}  // namespace controllers
}  // namespace community
