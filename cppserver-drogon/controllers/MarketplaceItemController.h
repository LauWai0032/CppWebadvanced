/**
 * @file MarketplaceItemController.h
 * @brief MarketplaceItem控制器（Drogon HttpController）
 *
 * 继承 drogon::HttpController，使用 ADD_METHOD_TO 宏自动注册路由。
 *
 * 路由映射：
 *   GET    /api/marketplace/items       — 列表（分页，需认证）
 *   GET    /api/marketplace/items/{id}  — 详情（需认证）
 *   POST   /api/marketplace/items       — 新增（需认证）
 *   PUT    /api/marketplace/items/{id}  — 修改（需认证）
 *   DELETE /api/marketplace/items/{id}  — 删除（需认证）
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpController.h>
#include "services/MarketplaceItemService.h"
#include "common/ResponseUtil.h"
#include "common/Logger.h"
#include "filters/AuthFilter.h"

namespace community {
namespace controllers {

/**
 * @class MarketplaceItemController
 * @brief MarketplaceItem控制器
 */
class MarketplaceItemController : public drogon::HttpController<MarketplaceItemController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MarketplaceItemController::getList,    "/api/marketplace/items", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(MarketplaceItemController::getById,    "/api/marketplace/items/{1}", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(MarketplaceItemController::create,     "/api/marketplace/items", drogon::Post, "AuthFilter");
    ADD_METHOD_TO(MarketplaceItemController::update,     "/api/marketplace/items/{1}", drogon::Put, "AuthFilter");
    ADD_METHOD_TO(MarketplaceItemController::remove,     "/api/marketplace/items/{1}", drogon::Delete, "AuthFilter");

    METHOD_LIST_END

    MarketplaceItemController();

    std::shared_ptr<services::MarketplaceItemService> getService() const;

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
