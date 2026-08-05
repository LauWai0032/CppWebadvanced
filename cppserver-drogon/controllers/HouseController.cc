/**
 * @file HouseController.cc
 * @brief House控制器实现
 *
 * 利用 Drogon 框架的异步回调模式处理请求。
 *
 * @date 2025
 */

#include "HouseController.h"

namespace community {
namespace controllers {

// ========== 构造函数 ==========
HouseController::HouseController() {
    LOG_INFO << "HouseController 已创建（Drogon 自动实例化）";
}

// ========== 获取服务实例 ==========
std::shared_ptr<services::HouseService> HouseController::getService() const {
    return services::HouseService::getInstance();
}

// ========== GET /api/houses ==========
void HouseController::getList(
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
        auto s = req->getParameter("building_no");
        if (!s.empty()) keyword = std::string(s);
    } catch (...) {}
    std::string filter1;
    try {
        auto s = req->getParameter("owner_id");
        if (!s.empty()) filter1 = std::string(s);
    } catch (...) {}

    int total = 0;
    auto dataList = svc->getHousesByPage(page, pageSize, total, keyword, filter1);

    Json::Value list(Json::arrayValue);
    for (const auto& item : dataList) {
        list.append(item.toJson());
    }

    callback(ResponseUtil::successPage(list, total, page, pageSize));
}

// ========== GET /api/houses/{id} ==========
void HouseController::getById(
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

    auto item = svc->getHouseById(id);
    if (!item) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "记录不存在"));
        return;
    }

    callback(ResponseUtil::success(item->toJson()));
}

// ========== POST /api/houses ==========
void HouseController::create(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonBody = ResponseUtil::parseJsonBody(req);
    if (!jsonBody) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "请求体为空或格式错误"));
        return;
    }

    auto obj = models::House::fromJson(*jsonBody);

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    int newId = svc->createHouse(obj);
    if (newId <= 0) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "创建失败"));
        return;
    }

    Json::Value data;
    data["id"] = newId;
    callback(ResponseUtil::success(data, "创建成功"));
}

// ========== PUT /api/houses/{id} ==========
void HouseController::update(
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

    auto obj = models::House::fromJson(*jsonBody);
    obj.id = id;

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    bool ok = svc->updateHouse(obj);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "更新失败或记录不存在"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("更新成功")));
}

// ========== DELETE /api/houses/{id} ==========
void HouseController::remove(
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

    bool ok = svc->deleteHouse(id);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "记录不存在"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("删除成功")));
}

}  // namespace controllers
}  // namespace community
