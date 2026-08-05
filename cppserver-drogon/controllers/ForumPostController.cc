/**
 * @file ForumPostController.cc
 * @brief ForumPost控制器实现
 *
 * 利用 Drogon 框架的异步回调模式处理请求。
 *
 * @date 2025
 */

#include "ForumPostController.h"

namespace community {
namespace controllers {

// ========== 构造函数 ==========
ForumPostController::ForumPostController() {
    LOG_INFO << "ForumPostController 已创建（Drogon 自动实例化）";
}

// ========== 获取服务实例 ==========
std::shared_ptr<services::ForumPostService> ForumPostController::getService() const {
    return services::ForumPostService::getInstance();
}

// ========== GET /api/forum/posts ==========
void ForumPostController::getList(
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
        auto s = req->getParameter("title");
        if (!s.empty()) keyword = std::string(s);
    } catch (...) {}
    std::string filter1;
    try {
        auto s = req->getParameter("category");
        if (!s.empty()) filter1 = std::string(s);
    } catch (...) {}

    int total = 0;
    auto dataList = svc->getForumPostsByPage(page, pageSize, total, keyword, filter1);

    Json::Value list(Json::arrayValue);
    for (const auto& item : dataList) {
        list.append(item.toJson());
    }

    callback(ResponseUtil::successPage(list, total, page, pageSize));
}

// ========== GET /api/forum/posts/{id} ==========
void ForumPostController::getById(
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

    auto item = svc->getForumPostById(id);
    if (!item) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "记录不存在"));
        return;
    }

    callback(ResponseUtil::success(item->toJson()));
}

// ========== POST /api/forum/posts ==========
void ForumPostController::create(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonBody = ResponseUtil::parseJsonBody(req);
    if (!jsonBody) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "请求体为空或格式错误"));
        return;
    }

    auto obj = models::ForumPost::fromJson(*jsonBody);

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    int newId = svc->createForumPost(obj);
    if (newId <= 0) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "创建失败"));
        return;
    }

    Json::Value data;
    data["id"] = newId;
    callback(ResponseUtil::success(data, "创建成功"));
}

// ========== PUT /api/forum/posts/{id} ==========
void ForumPostController::update(
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

    auto obj = models::ForumPost::fromJson(*jsonBody);
    obj.id = id;

    auto svc = getService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    bool ok = svc->updateForumPost(obj);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "更新失败或记录不存在"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("更新成功")));
}

// ========== DELETE /api/forum/posts/{id} ==========
void ForumPostController::remove(
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

    bool ok = svc->deleteForumPost(id);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "记录不存在"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("删除成功")));
}

}  // namespace controllers
}  // namespace community
