/**
 * @file BaseController.hpp
 * @brief HTTP Controller 基类
 *
 * 所有业务 Controller 的基类，定义路由注册接口和通用工具方法。
 * 子类通过继承并实现 registerRoutes() 来注册自身的 HTTP 路由。
 *
 * 使用到的 C++ 特性：
 *   - C++11: 纯虚函数, std::optional（通过 C++17 引入）
 *   - C++17: std::optional, std::string_view
 *
 * 设计模式：
 *   - 策略模式：每个 Controller 是一个路由策略，由 App 统一装配
 *   - 模板方法思想：基类提供通用工具，子类专注业务逻辑
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

// httplib 和 nlohmann/json 来自项目根目录
#include "../../httplib.h"
#include "../../json.hpp"

#include "Response.hpp"
#include "Logger.hpp"

namespace community_system {
namespace controllers {

/**
 * @class BaseController
 * @brief Controller 基类（抽象基类）
 *
 * 设计思路：
 *  - 纯虚方法 registerRoutes() 强制子类实现路由注册
 *  - 提供受保护的工具方法，减少子类重复代码
 *  - 所有返回给客户端的响应都通过 Response 类统一格式
 *
 * 子类示例：
 *   class UserController : public BaseController {
 *   public:
 *       void registerRoutes(httplib::Server& svr) override {
 *           svr.Get("/api/user/list", [this](const auto& req, auto& res) {
 *               handleList(req, res);
 *           });
 *       }
 *   };
 */
class BaseController {
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~BaseController() = default;

    /**
     * @brief 注册路由（纯虚函数，子类必须实现）
     * @param svr httplib 服务器引用
     *
     * 子类在此方法中调用 svr.Get/Post/Put/Delete 等注册 HTTP 路由。
     */
    virtual void registerRoutes(httplib::Server& svr) = 0;

protected:
    /**
     * @brief 发送成功响应（带数据 JSON）
     * @tparam T 可转换为 nlohmann::json 的类型（但排除字符串类型）
     * @param res HTTP 响应对象
     * @param data 业务数据
     *
     * 使用 C++17 的 std::enable_if_t + is_convertible 做 SFINAE，
     * 避免 const char* 同时匹配 json 和 string 导致二义性。
     */
    template <typename T,
              typename = std::enable_if_t<
                  std::is_convertible_v<T, nlohmann::json> &&
                  !std::is_convertible_v<T, std::string> &&
                  !std::is_same_v<std::decay_t<T>, const char*> &&
                  !std::is_same_v<std::decay_t<T>, char*>>>
    void sendSuccess(httplib::Response& res, T&& data) const {
        auto resp = common::Response::successWithData(nlohmann::json(std::forward<T>(data)));
        writeJsonResponse(res, resp);
    }

    /**
     * @brief 发送成功响应（带消息字符串）
     * @param res HTTP 响应对象
     * @param msg 成功消息
     */
    void sendSuccess(httplib::Response& res, const std::string& msg) const {
        auto resp = common::Response::success(msg);
        writeJsonResponse(res, resp);
    }

    /**
     * @brief 发送成功响应（C 字符串消息的明确重载，避免二义性）
     * @param res HTTP 响应对象
     * @param msg 成功消息（C 字符串）
     */
    void sendSuccess(httplib::Response& res, const char* msg) const {
        auto resp = common::Response::success(std::string(msg));
        writeJsonResponse(res, resp);
    }

    /**
     * @brief 发送成功响应（仅状态码，无数据无消息）
     * @param res HTTP 响应对象
     */
    void sendSuccess(httplib::Response& res) const {
        auto resp = common::Response::success("success");
        writeJsonResponse(res, resp);
    }

    /**
     * @brief 发送错误响应
     * @param res HTTP 响应对象
     * @param code 错误码
     * @param msg 错误消息
     */
    void sendError(httplib::Response& res, common::ErrorCode code,
                   const std::string& msg) const {
        auto resp = common::Response::error(code, msg);
        writeJsonResponse(res, resp);
    }

    /**
     * @brief 发送错误响应（使用默认消息）
     * @param res HTTP 响应对象
     * @param code 错误码
     */
    void sendError(httplib::Response& res, common::ErrorCode code) const {
        auto resp = common::Response::error(code);
        writeJsonResponse(res, resp);
    }

    /**
     * @brief 解析请求体中的 JSON
     * @param req HTTP 请求对象
     * @return std::optional<nlohmann::json>
     *         解析成功返回 JSON 对象，失败返回 nullopt
     *
     * 使用 std::optional 表达"解析可能失败"的语义，
     * 调用方可通过 has_value() 或 value_or() 安全处理。
     */
    std::optional<nlohmann::json> parseJsonBody(const httplib::Request& req) const {
        if (req.body.empty()) {
            return std::nullopt;
        }
        try {
            return nlohmann::json::parse(req.body);
        } catch (const nlohmann::json::parse_error& e) {
            common::Logger::warn("parseJsonBody failed: {}", e.what());
            return std::nullopt;
        }
    }

    /**
     * @brief 从 URL query 参数中获取指定 key 的值
     * @param req HTTP 请求对象
     * @param key 参数名
     * @return std::optional<std::string>
     *         存在返回值，不存在返回 nullopt
     */
    std::optional<std::string> getParam(const httplib::Request& req,
                                        const std::string& key) const {
        auto it = req.params.find(key);
        if (it != req.params.end()) {
            return it->second;
        }
        // 再尝试从 query string 获取
        if (req.has_param(key.c_str())) {
            return req.get_param_value(key.c_str());
        }
        return std::nullopt;
    }

    /**
     * @brief 从 URL 路径/query 参数中获取整数
     * @param req HTTP 请求对象
     * @param key 参数名
     * @return std::optional<int>
     */
    std::optional<int> getIntParam(const httplib::Request& req,
                                   const std::string& key) const {
        auto val = getParam(req, key);
        if (!val) return std::nullopt;
        try {
            return std::stoi(*val);
        } catch (...) {
            return std::nullopt;
        }
    }

    /**
     * @brief 设置统一的跨域响应头
     * @param res HTTP 响应对象
     *
     * 所有接口共用的 CORS 头，统一在此处理。
     */
    void setCorsHeader(httplib::Response& res) const {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type,Authorization");
        res.set_header("Access-Control-Expose-Headers", "*");
    }

    /**
     * @brief 通用异常捕获包装器
     *
     * 子类的路由处理函数可以用此包装，统一处理异常。
     * 使用示例：
     *   svr.Get("/api/xxx", [this](const auto& req, auto& res) {
     *       handleExceptions(res, [&] {
     *           // 业务逻辑
     *       });
     *   });
     */
    template <typename Func>
    void handleExceptions(httplib::Response& res, Func&& func) const {
        try {
            setCorsHeader(res);
            func();
        } catch (const std::exception& e) {
            common::Logger::error("Exception in controller: {}", e.what());
            sendError(res, common::ErrorCode::INTERNAL_ERROR, e.what());
        } catch (...) {
            common::Logger::error("Unknown exception in controller");
            sendError(res, common::ErrorCode::INTERNAL_ERROR);
        }
    }

private:
    /**
     * @brief 写入 JSON 响应
     * @param res HTTP 响应对象
     * @param resp 统一响应对象
     */
    void writeJsonResponse(httplib::Response& res,
                           const common::Response& resp) const {
        setCorsHeader(res);
        res.status = resp.getCode();
        res.set_content(resp.toJson().dump(), "application/json");
    }
};

}  // namespace controllers
}  // namespace community_system
