/**
 * @file UserController.cc
 * @brief 用户控制器实现
 *
 * 利用 Drogon 框架的异步回调模式处理请求。
 * 所有方法均使用 std::shared_ptr<HttpResponse> 作为回调参数。
 *
 * 登录成功后返回真实 JWT Token（由 JwtUtil 生成），
 * 前端需在后续请求的 Authorization 头中携带此 Token。
 *
 * @date 2025
 */

#include "UserController.h"
#include "common/JwtUtil.h"

namespace community {
namespace controllers {

// ========== 构造函数 ==========
UserController::UserController() {
    LOG_INFO << "UserController 已创建（Drogon 自动实例化）";
}

// ========== 获取服务实例 ==========
std::shared_ptr<services::UserService> UserController::getUserService() const {
    return services::UserService::getInstance();
}

// ========== GET /api/users ==========
void UserController::getUserList(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) 
{
    auto svc = getUserService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    // 获取分页参数（带默认值）
    int page = 1;
    int pageSize = 10;
    try {
        auto pageStr = req->getParameter("page");
        if (!pageStr.empty()) page = std::stoi(std::string(pageStr));
        auto sizeStr = req->getParameter("pageSize");
        if (!sizeStr.empty()) pageSize = std::stoi(std::string(sizeStr));
    } catch (...) {
        // 参数解析失败，使用默认值
    }
    if (page <= 0) page = 1;
    if (pageSize <= 0 || pageSize > 100) pageSize = 10;

    int total = 0;
    auto users = svc->getUsersByPage(page, pageSize, total);

    // 转换为 JSON 数组（不包含密码哈希）
    Json::Value list(Json::arrayValue);
    for (const auto& u : users) {
        list.append(u.toJson(false));
    }

    callback(ResponseUtil::successPage(list, total, page, pageSize));
}

// ========== GET /api/users/{id} ==========
void UserController::getUserById(
    const drogon::HttpRequestPtr& /*req*/,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    int id)
{
    if (id <= 0) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "无效的用户ID"));
        return;
    }

    auto svc = getUserService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    auto user = svc->getUserById(id);
    if (!user) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "用户不存在"));
        return;
    }

    callback(ResponseUtil::success(user->toJson(false)));
}

// ========== POST /api/login ==========
void UserController::login(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonBody = ResponseUtil::parseJsonBody(req);
    if (!jsonBody) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "请求体为空或格式错误"));
        return;
    }

    auto username = jsonBody->get("username", "").asString();
    auto password = jsonBody->get("password", "").asString();

    if (username.empty() || password.empty()) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "用户名和密码不能为空"));
        return;
    }

    auto svc = getUserService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    auto user = svc->authenticate(username, password);
    if (!user) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "用户名或密码错误"));
        return;
    }

    // 构建返回数据
    auto data = user->toJson(false);

    // 生成真实 JWT Token（HMAC-SHA256）
    std::string token = JwtUtil::generateToken(user->id, user->username, user->role);
    data["token"] = token;

    LOG_INFO << "用户登录成功: " << user->username << " (" << user->role << ")";
    callback(ResponseUtil::success(data));
}

// ========== POST /api/users ==========
void UserController::createUser(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonBody = ResponseUtil::parseJsonBody(req);
    if (!jsonBody) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "请求体为空或格式错误"));
        return;
    }

    auto user = models::User::fromJson(*jsonBody);

    if (user.username.empty()) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "用户名不能为空"));
        return;
    }

    // 支持前端传 password 字段（明文字段名）
    if (user.passwordHash.empty() && !jsonBody->get("password", "").asString().empty()) {
        user.passwordHash = jsonBody->get("password", "").asString();
    }

    auto svc = getUserService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    int newId = svc->createUser(user);
    if (newId <= 0) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "创建用户失败（用户名可能已存在）"));
        return;
    }

    Json::Value data;
    data["id"] = newId;
    callback(ResponseUtil::success(data, "创建成功"));
}

// ========== PUT /api/users/{id} ==========
void UserController::updateUser(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    int id)
{
    if (id <= 0) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "无效的用户ID"));
        return;
    }

    auto jsonBody = ResponseUtil::parseJsonBody(req);
    if (!jsonBody) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "请求体为空或格式错误"));
        return;
    }

    auto user = models::User::fromJson(*jsonBody);
    user.id = id;  // 以路径参数中的 ID 为准

    // 支持前端传 password 字段（明文字段名）
    if (user.passwordHash.empty() && !jsonBody->get("password", "").asString().empty()) {
        user.passwordHash = jsonBody->get("password", "").asString();
    }

    auto svc = getUserService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    bool ok = svc->updateUser(user);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "用户不存在或更新失败"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("更新成功")));
}

// ========== DELETE /api/users/{id} ==========
void UserController::deleteUser(
    const drogon::HttpRequestPtr& /*req*/,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    int id)
{
    if (id <= 0) {
        callback(ResponseUtil::error(RespCode::BAD_REQUEST, "无效的用户ID"));
        return;
    }

    auto svc = getUserService();
    if (!svc) {
        callback(ResponseUtil::error(RespCode::INTERNAL_ERROR, "服务未初始化"));
        return;
    }

    bool ok = svc->deleteUser(id);
    if (!ok) {
        callback(ResponseUtil::error(RespCode::NOT_FOUND, "用户不存在"));
        return;
    }

    callback(ResponseUtil::success(std::string_view("删除成功")));
}

}  // namespace controllers
}  // namespace community
