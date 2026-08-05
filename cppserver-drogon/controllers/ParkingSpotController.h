/**
 * @file ParkingSpotController.h
 * @brief ParkingSpot控制器（Drogon HttpController）
 *
 * 继承 drogon::HttpController，使用 ADD_METHOD_TO 宏自动注册路由。
 *
 * 路由映射：
 *   GET    /api/parking-spots       — 列表（分页，需认证）
 *   GET    /api/parking-spots/{id}  — 详情（需认证）
 *   POST   /api/parking-spots       — 新增（需认证）
 *   PUT    /api/parking-spots/{id}  — 修改（需认证）
 *   DELETE /api/parking-spots/{id}  — 删除（需认证）
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpController.h>
#include "services/ParkingSpotService.h"
#include "common/ResponseUtil.h"
#include "common/Logger.h"
#include "filters/AuthFilter.h"

namespace community {
namespace controllers {

/**
 * @class ParkingSpotController
 * @brief ParkingSpot控制器
 */
class ParkingSpotController : public drogon::HttpController<ParkingSpotController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ParkingSpotController::getList,    "/api/parking-spots", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(ParkingSpotController::getById,    "/api/parking-spots/{1}", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(ParkingSpotController::create,     "/api/parking-spots", drogon::Post, "AuthFilter");
    ADD_METHOD_TO(ParkingSpotController::update,     "/api/parking-spots/{1}", drogon::Put, "AuthFilter");
    ADD_METHOD_TO(ParkingSpotController::remove,     "/api/parking-spots/{1}", drogon::Delete, "AuthFilter");

    ADD_METHOD_TO(ParkingSpotController::park,        "/api/parking-spots/{1}/park", drogon::Put, "AuthFilter");
    ADD_METHOD_TO(ParkingSpotController::leave,       "/api/parking-spots/{1}/leave", drogon::Put, "AuthFilter");

    METHOD_LIST_END

    ParkingSpotController();

    std::shared_ptr<services::ParkingSpotService> getService() const;

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
     * @brief 车辆入场
     */
    void park(const drogon::HttpRequestPtr& req,
              std::function<void(const drogon::HttpResponsePtr&)>&& callback,
              int id);

    /**
     * @brief 车辆离场
     */
    void leave(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback,
               int id);

};

}  // namespace controllers
}  // namespace community
