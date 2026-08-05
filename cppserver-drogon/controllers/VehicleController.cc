/**
 * @file VehicleController.cc
 * @brief Vehicle控制器实现
 *
 * 利用 Drogon 框架的异步回调模式处理请求。
 *
 * @date 2025
 */

#include "VehicleController.h"

namespace community {
namespace controllers {

// ========== 构造函数 ==========
VehicleController::VehicleController() {
    LOG_INFO << "VehicleController 已创建（Drogon 自动实例化）";
}

// ========== 获取服务实例 ==========
std::shared_ptr<services::VehicleService> VehicleController::getService() const {
    return services::VehicleService::getInstance();
}

// ========== GET /api/vehicles ==========
void VehicleController::getList(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    int page = 1;
    int pageSize = 10;
    try {
        auto pageStr = req->getParameter("page");
        if (!pageStr.empty()) page = std::stoi(std::string(pageStr));
        auto sizeStr = req->getParameter("pageSize");
        if (!sizeStr.empty()) pageSize = std::stoi(std::string(sizeStr));
    } catch (...) {}
    if (page <= 0) page = 1;
    if (pageSize <= 0 || pageSize > 100) pageSize = 10;

    std::string keyword;
    try {
        auto s = req->getParameter("plate_number");
        if (!s.empty()) keyword = std::string(s);
    } catch (...) {}
    std::string filter1;
    try {
        auto s = req->getParameter("user_id");
        if (!s.empty()) filter1 = std::string(s);
    } catch (...) {}

    int total = 0;
    auto dataList = svc->getVehiclesByPage(page, pageSize, total, keyword, filter1);

    Json::Value list(Json::arrayValue);
    for (const auto& item : dataList) {
        list.append(item.toJson());
    }

    callback(ResponseUtil::successPage(list, total, page, pageSize));
}

// ========== GET /api/vehicles/{id} ==========
void VehicleController::getById(
    const drogon::HttpRequestPtr& /*req*/,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    int id)
{
    if (id <= 0) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "无效的ID"));
        return;
    }

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    auto item = svc->getVehicleById(id);
    if (!item) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "记录不存在"));
        return;
    }

    callback(ResponseUtil::success(item->toJson()));
}

// ========== POST /api/vehicles ==========
void VehicleController::create(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonBody = ResponseUtil::parseJsonBody(req);
    if (!jsonBody) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "请求体为空或格式错误"));
        return;
    }

    auto obj = models::Vehicle::fromJson(*jsonBody);

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    int newId = svc->createVehicle(obj);
    if (newId <= 0) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "创建失败"));
        return;
    }

    Json::Value data;
    data["id"] = newId;
    callback(ResponseUtil::success(data, "创建成功"));
}

// ========== PUT /api/vehicles/{id} ==========
void VehicleController::update(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    int id)
{
    if (id <= 0) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "无效的ID"));
        return;
    }

    auto jsonBody = ResponseUtil::parseJsonBody(req);
    if (!jsonBody) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "请求体为空或格式错误"));
        return;
    }

    auto obj = models::Vehicle::fromJson(*jsonBody);
    obj.id = id;

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    bool ok = svc->updateVehicle(obj);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "更新失败或记录不存在"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("更新成功")));
}

// ========== DELETE /api/vehicles/{id} ==========
void VehicleController::remove(
    const drogon::HttpRequestPtr& /*req*/,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    int id)
{
    if (id <= 0) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "无效的ID"));
        return;
    }

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    bool ok = svc->deleteVehicle(id);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "记录不存在"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("删除成功")));
}

}  // namespace controllers
}  // namespace community
