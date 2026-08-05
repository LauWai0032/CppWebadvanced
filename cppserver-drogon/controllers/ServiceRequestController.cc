/**
 * @file ServiceRequestController.cc
 * @brief ServiceRequest控制器实现
 *
 * 利用 Drogon 框架的异步回调模式处理请求。
 *
 * @date 2025
 */

#include "ServiceRequestController.h"

namespace community {
namespace controllers {

// ========== 构造函数 ==========
ServiceRequestController::ServiceRequestController() {
    LOG_INFO << "ServiceRequestController 已创建（Drogon 自动实例化）";
}

// ========== 获取服务实例 ==========
std::shared_ptr<services::ServiceRequestService> ServiceRequestController::getService() const {
    return services::ServiceRequestService::getInstance();
}

// ========== GET /api/service-requests ==========
void ServiceRequestController::getList(
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
        auto s = req->getParameter("type");
        if (!s.empty()) keyword = std::string(s);
    } catch (...) {}
    std::string filter1;
    try {
        auto s = req->getParameter("status");
        if (!s.empty()) filter1 = std::string(s);
    } catch (...) {}

    int total = 0;
    auto dataList = svc->getServiceRequestsByPage(page, pageSize, total, keyword, filter1);

    Json::Value list(Json::arrayValue);
    for (const auto& item : dataList) {
        list.append(item.toJson());
    }

    callback(ResponseUtil::successPage(list, total, page, pageSize));
}

// ========== GET /api/service-requests/{id} ==========
void ServiceRequestController::getById(
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

    auto item = svc->getServiceRequestById(id);
    if (!item) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "记录不存在"));
        return;
    }

    callback(ResponseUtil::success(item->toJson()));
}

// ========== POST /api/service-requests ==========
void ServiceRequestController::create(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonBody = ResponseUtil::parseJsonBody(req);
    if (!jsonBody) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "请求体为空或格式错误"));
        return;
    }

    auto obj = models::ServiceRequest::fromJson(*jsonBody);

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    int newId = svc->createServiceRequest(obj);
    if (newId <= 0) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "创建失败"));
        return;
    }

    Json::Value data;
    data["id"] = newId;
    callback(ResponseUtil::success(data, "创建成功"));
}

// ========== PUT /api/service-requests/{id} ==========
void ServiceRequestController::update(
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

    auto obj = models::ServiceRequest::fromJson(*jsonBody);
    obj.id = id;

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    bool ok = svc->updateServiceRequest(obj);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "更新失败或记录不存在"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("更新成功")));
}

// ========== DELETE /api/service-requests/{id} ==========
void ServiceRequestController::remove(
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

    bool ok = svc->deleteServiceRequest(id);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "记录不存在"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("删除成功")));
}

// ========== PUT /api/service-requests/{id}/status ==========
void ServiceRequestController::updateStatus(
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

    std::string status = jsonBody->get("status", "").asString();
    if (status.empty()) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "状态不能为空"));
        return;
    }

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    bool ok = svc->updateStatus(id, status);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "状态修改失败"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("状态修改成功")));
}

}  // namespace controllers
}  // namespace community
