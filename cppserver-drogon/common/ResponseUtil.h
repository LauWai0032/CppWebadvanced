/**
 * @file ResponseUtil.h
 * @brief 统一 JSON 响应封装工具
 *
 * 保持与 cppserver（cpp-httplib 版）完全一致的响应格式：
 *   {
 *     "code": 200,
 *     "message": "success",
 *     "data": {}
 *   }
 *
 * 设计模式：
 *   - 静态工厂方法模式：通过 success/error 静态方法创建响应
 *   - 利用 Drogon 的 HttpResponse 直接返回 JSON 响应
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpResponse.h>
#include <json/json.h>
#include <string>
#include <string_view>
#include <optional>
#include <memory>

namespace community {

/**
 * @enum RespCode
 * @brief 业务响应码枚举
 *
 * 与 cppserver 版本保持一致的错误码定义。
 */
enum class RespCode : int {
    OK             = 200,  ///< 成功
    BAD_REQUEST    = 400,  ///< 请求参数错误
    UNAUTHORIZED   = 401,  ///< 未授权
    FORBIDDEN      = 403,  ///< 禁止访问
    NOT_FOUND      = 404,  ///< 资源不存在
    INTERNAL_ERROR = 500   ///< 服务器内部错误
};

/**
 * @class ResponseUtil
 * @brief 统一响应工具类
 *
 * 所有方法均为静态方法，提供便捷的 JSON 响应构造能力。
 * 底层使用 Drogon 的 HttpResponse 和 JsonCpp。
 */
class ResponseUtil {
public:
    /**
     * @brief 构造成功响应（带数据）
     * @param data JSON 数据对象
     * @param msg  消息文本
     * @return std::shared_ptr<drogon::HttpResponse>
     */
    static std::shared_ptr<drogon::HttpResponse> 
    success(const Json::Value& data, std::string_view msg = "success");

    /**
     * @brief 构造成功响应（仅消息）
     * @param msg 消息文本
     * @return std::shared_ptr<drogon::HttpResponse>
     */
    static std::shared_ptr<drogon::HttpResponse>
    success(std::string_view msg = "success");

    /**
     * @brief 构造错误响应
     * @param code 响应码
     * @param msg  错误消息
     * @return std::shared_ptr<drogon::HttpResponse>
     */
    static std::shared_ptr<drogon::HttpResponse>
    error(RespCode code, std::string_view msg);

    /**
     * @brief 构造错误响应（使用默认消息）
     * @param code 响应码
     * @return std::shared_ptr<drogon::HttpResponse>
     */
    static std::shared_ptr<drogon::HttpResponse>
    error(RespCode code);

    /**
     * @brief 构造带分页的成功响应
     * @param list     数据列表
     * @param total    总记录数
     * @param page     当前页码
     * @param pageSize 每页大小
     * @return std::shared_ptr<drogon::HttpResponse>
     */
    static std::shared_ptr<drogon::HttpResponse>
    successPage(const Json::Value& list, int total, int page, int pageSize);

    /**
     * @brief 从 Drogon 请求中解析 JSON 请求体
     * @param req HTTP 请求
     * @return std::optional<Json::Value> 解析成功返回 JSON 对象，失败返回 nullopt
     */
    static std::optional<Json::Value> 
    parseJsonBody(const drogon::HttpRequestPtr& req);

private:
    ResponseUtil() = delete;  // 禁止实例化

    /**
     * @brief 构建统一格式的 JSON 响应体
     */
    static Json::Value buildBody(RespCode code, std::string_view msg, const Json::Value& data);

    /**
     * @brief 创建 JSON HTTP 响应
     */
    static std::shared_ptr<drogon::HttpResponse> 
    makeJsonResponse(const Json::Value& body, drogon::HttpStatusCode httpStatus = drogon::k200OK);

    /**
     * @brief 获取错误码对应的默认消息
     */
    static std::string_view defaultMessage(RespCode code);
};

}  // namespace community
