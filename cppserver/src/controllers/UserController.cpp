/**
 * @file UserController.cpp
 * @brief 用户控制器实现
 * @date 2025
 */

#include "controllers/UserController.hpp"
#include "common/Logger.hpp"
#include "common/Response.hpp"

namespace community_system {
namespace controllers {

// ============== 构造函数 ==============

UserController::UserController(std::shared_ptr<services::IUserService> userService)
    : m_userService(std::move(userService)) {
    if (!m_userService) {
        throw std::invalid_argument("UserService cannot be null");
    }
}

// ============== 路由注册 ==============

void UserController::registerRoutes(httplib::Server& svr) {
    // 登录接口
    svr.Post("/api/user/login", [this](const httplib::Request& req, httplib::Response& res) {
        handleExceptions(res, [&] { handleLogin(req, res); });
    });

    // 用户列表（分页）
    svr.Get("/api/user/list", [this](const httplib::Request& req, httplib::Response& res) {
        handleExceptions(res, [&] { handleList(req, res); });
    });

    // 用户详情
    svr.Get(R"(/api/user/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleExceptions(res, [&] { handleDetail(req, res); });
    });

    // 创建用户
    svr.Post("/api/user", [this](const httplib::Request& req, httplib::Response& res) {
        handleExceptions(res, [&] { handleCreate(req, res); });
    });

    // 更新用户
    svr.Put(R"(/api/user/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleExceptions(res, [&] { handleUpdate(req, res); });
    });

    // 删除用户
    svr.Delete(R"(/api/user/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleExceptions(res, [&] { handleDelete(req, res); });
    });

    common::Logger::info("UserController routes registered");
}

// ============== 登录 ==============

void UserController::handleLogin(const httplib::Request& req, httplib::Response& res) {
    auto body = parseJsonBody(req);
    if (!body) {
        sendError(res, common::ErrorCode::BAD_REQUEST, "请求体为空或格式错误");
        return;
    }

    std::string username = body->value("username", "");
    std::string password = body->value("password", "");

    if (username.empty() || password.empty()) {
        sendError(res, common::ErrorCode::BAD_REQUEST, "用户名和密码不能为空");
        return;
    }

    auto user = m_userService->authenticate(username, password);
    if (!user) {
        sendError(res, common::ErrorCode::BAD_REQUEST, "用户名或密码错误");
        return;
    }

    // 构建返回数据（不含密码哈希）
    nlohmann::json data = user->toJson(false);
    // 模拟生成 token（实际应使用 JWT 等）
    data["token"] = "mock-token-" + std::to_string(user->id);

    sendSuccess(res, data);
}

// ============== 用户列表 ==============

void UserController::handleList(const httplib::Request& req, httplib::Response& res) {
    int page = 1;
    int pageSize = 10;

    auto pageParam = getIntParam(req, "page");
    if (pageParam) page = *pageParam;

    auto sizeParam = getIntParam(req, "pageSize");
    if (sizeParam) pageSize = *sizeParam;

    int total = 0;
    auto users = m_userService->getUsersByPage(page, pageSize, total);

    // 转换为 JSON 数组
    nlohmann::json list = nlohmann::json::array();
    for (const auto& u : users) {
        list.push_back(u.toJson(false));
    }

    nlohmann::json data = {
        {"list",     list},
        {"total",    total},
        {"page",     page},
        {"pageSize", pageSize}
    };

    sendSuccess(res, data);
}

// ============== 用户详情 ==============

void UserController::handleDetail(const httplib::Request& req, httplib::Response& res) {
    // 从路径参数获取 ID（httplib 正则匹配存储在 req.matches 中）
    // matches[0] 是整个匹配字符串，matches[1] 是第一个捕获组
    int id = 0;
    if (req.matches.size() > 1) {
        try {
            id = std::stoi(req.matches[1]);
        } catch (...) {
            sendError(res, common::ErrorCode::BAD_REQUEST, "无效的用户ID");
            return;
        }
    }

    if (id <= 0) {
        sendError(res, common::ErrorCode::BAD_REQUEST, "无效的用户ID");
        return;
    }

    auto user = m_userService->getUserById(id);
    if (!user) {
        sendError(res, common::ErrorCode::NOT_FOUND, "用户不存在");
        return;
    }

    sendSuccess(res, user->toJson(false));
}

// ============== 创建用户 ==============

void UserController::handleCreate(const httplib::Request& req, httplib::Response& res) {
    auto body = parseJsonBody(req);
    if (!body) {
        sendError(res, common::ErrorCode::BAD_REQUEST, "请求体为空或格式错误");
        return;
    }

    auto user = models::User::fromJson(*body);

    if (user.username.empty()) {
        sendError(res, common::ErrorCode::BAD_REQUEST, "用户名不能为空");
        return;
    }

    int newId = m_userService->createUser(user);
    if (newId <= 0) {
        sendError(res, common::ErrorCode::INTERNAL_ERROR, "创建用户失败");
        return;
    }

    nlohmann::json data = {{"id", newId}};
    sendSuccess(res, data);
}

// ============== 更新用户 ==============

void UserController::handleUpdate(const httplib::Request& req, httplib::Response& res) {
    int id = 0;
    if (req.matches.size() > 1) {
        try {
            id = std::stoi(req.matches[1]);
        } catch (...) {
            sendError(res, common::ErrorCode::BAD_REQUEST, "无效的用户ID");
            return;
        }
    }

    if (id <= 0) {
        sendError(res, common::ErrorCode::BAD_REQUEST, "无效的用户ID");
        return;
    }

    auto body = parseJsonBody(req);
    if (!body) {
        sendError(res, common::ErrorCode::BAD_REQUEST, "请求体为空或格式错误");
        return;
    }

    auto user = models::User::fromJson(*body);
    user.id = id;  // 以路径参数中的 ID 为准

    bool ok = m_userService->updateUser(user);
    if (!ok) {
        sendError(res, common::ErrorCode::NOT_FOUND, "用户不存在或更新失败");
        return;
    }

    sendSuccess(res, "更新成功");
}

// ============== 删除用户 ==============

void UserController::handleDelete(const httplib::Request& req, httplib::Response& res) {
    int id = 0;
    if (req.matches.size() > 1) {
        try {
            id = std::stoi(req.matches[1]);
        } catch (...) {
            sendError(res, common::ErrorCode::BAD_REQUEST, "无效的用户ID");
            return;
        }
    }

    if (id <= 0) {
        sendError(res, common::ErrorCode::BAD_REQUEST, "无效的用户ID");
        return;
    }

    bool ok = m_userService->deleteUser(id);
    if (!ok) {
        sendError(res, common::ErrorCode::NOT_FOUND, "用户不存在");
        return;
    }

    sendSuccess(res, "删除成功");
}

}  // namespace controllers
}  // namespace community_system
