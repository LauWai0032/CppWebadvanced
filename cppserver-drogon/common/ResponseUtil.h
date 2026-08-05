/**
 * @file ResponseUtil.h
 * @brief 统一 JSON 响应封装工具
 *
 * 所有接口统一返回以下格式（与前端 request.ts 完全对齐）：
 *   {
 *     "code": 200,
 *     "msg": "success",
 *     "data": {}
 *   }
 *
 * 注意事项：
 *   - 字段名为 `msg` 而非 `message`，与前端 axios 封装保持一致
 *   - data 字段永远是 JSON 对象（即使为空，也返回 {} 而非 null）
 *   - 分页数据使用统一的 successPage 格式，便于前端通用处理
 *
 * 设计模式：
 *   - 静态工厂方法模式：通过 success/error 静态方法创建响应对象
 *   - 利用 Drogon 的 HttpResponse::newHttpJsonResponse 直接返回 JSON
 *   - 业务错误码 RespCode 枚举与 HTTP 状态码一一对应
 *
 * 使用方式：
 *   callback(ResponseUtil::success(data));          // 成功响应（带数据）
 *   callback(ResponseUtil::error(RespCode::NOT_FOUND, "用户不存在"));  // 错误响应
 *   callback(ResponseUtil::successPage(list, total, page, pageSize)); // 分页响应
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
 * 业务状态码与 HTTP 状态码保持一致，便于理解和使用。
 * 常见状态码：
 *   - 200: 成功
 *   - 400: 请求参数错误
 *   - 401: 未授权（未登录或 Token 无效）
 *   - 403: 禁止访问（权限不足）
 *   - 404: 资源不存在
 *   - 500: 服务器内部错误
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
 *
 * 核心方法：
 *   - success(data, msg): 成功响应（带数据）
 *   - success(msg):       成功响应（仅消息）
 *   - error(code, msg):   错误响应
 *   - successPage(...):   分页成功响应
 *   - parseJsonBody(req): 从请求体解析 JSON
 *
 * 内部辅助：
 *   - buildBody():    构建统一格式的 JSON 响应体
 *   - makeJsonResponse(): 创建 JSON HTTP 响应对象
 *   - defaultMessage(): 获取错误码对应的默认消息
 */
class ResponseUtil {
public:
    /**
     * @brief 构造成功响应（带数据）
     * @param data JSON 数据对象
     * @param msg  消息文本，默认 "success"
     * @return std::shared_ptr<drogon::HttpResponse> Drogon 响应对象
     */
    static std::shared_ptr<drogon::HttpResponse> 
    success(const Json::Value& data, std::string_view msg = "success");

    /**
     * @brief 构造成功响应（仅消息）
     * @param msg 消息文本，默认 "success"
     * @return std::shared_ptr<drogon::HttpResponse> Drogon 响应对象
     */
    static std::shared_ptr<drogon::HttpResponse>
    success(std::string_view msg = "success");

    /**
     * @brief 构造错误响应（自定义消息）
     * @param code 响应码（业务错误码）
     * @param msg  错误消息文本
     * @return std::shared_ptr<drogon::HttpResponse> Drogon 响应对象
     *
     * 注意：HTTP 状态码与业务码保持一致（如 RespCode::NOT_FOUND → HTTP 404）
     */
    static std::shared_ptr<drogon::HttpResponse>
    error(RespCode code, std::string_view msg);

    /**
     * @brief 构造错误响应（使用默认消息）
     * @param code 响应码
     * @return std::shared_ptr<drogon::HttpResponse> Drogon 响应对象
     */
    static std::shared_ptr<drogon::HttpResponse>
    error(RespCode code);

    /**
     * @brief 构造带分页的成功响应
     *
     * 分页响应的 data 结构固定为：
     *   { list: [...], total: N, page: 1, pageSize: 10 }
     *
     * 前端 PageResult 类型与此完全对应。
     *
     * @param list     数据列表（JSON 数组）
     * @param total    总记录数
     * @param page     当前页码
     * @param pageSize 每页大小
     * @return std::shared_ptr<drogon::HttpResponse> Drogon 响应对象
     */
    static std::shared_ptr<drogon::HttpResponse>
    successPage(const Json::Value& list, int total, int page, int pageSize);

    /**
     * @brief 从 Drogon 请求中解析 JSON 请求体
     *
     * 封装 Drogon 的 getJsonObject() 方法，添加异常处理。
     * 解析失败返回 std::nullopt，调用者可根据返回值判断。
     *
     * @param req HTTP 请求对象
     * @return std::optional<Json::Value> 解析成功返回 JSON 对象，失败返回 nullopt
     */
    static std::optional<Json::Value> 
    parseJsonBody(const drogon::HttpRequestPtr& req);

private:
    ResponseUtil() = delete;  // 禁止实例化（工具类）

    /**
     * @brief 构建统一格式的 JSON 响应体
     *
     * 内部方法，所有公共方法最终都调用此方法构建 body。
     * 格式：{ code, msg, data }
     *
     * @param code 业务状态码
     * @param msg  消息文本
     * @param data 数据对象
     * @return Json::Value 构建好的 JSON 对象
     */
    static Json::Value buildBody(RespCode code, std::string_view msg, const Json::Value& data);

    /**
     * @brief 创建 JSON HTTP 响应
     *
     * 将 JSON body 包装为 Drogon 的 HttpResponse 对象。
     *
     * @param body       JSON 响应体
     * @param httpStatus HTTP 状态码（默认 200 OK）
     * @return std::shared_ptr<drogon::HttpResponse> Drogon 响应对象
     */
    static std::shared_ptr<drogon::HttpResponse> 
    makeJsonResponse(const Json::Value& body, drogon::HttpStatusCode httpStatus = drogon::k200OK);

    /**
     * @brief 获取错误码对应的默认消息
     * @param code 响应码
     * @return std::string_view 默认消息字符串
     */
    static std::string_view defaultMessage(RespCode code);
};

}  // namespace community
