/**
 * @file ResponseUtil.cc
 * @brief 统一 JSON 响应封装实现
 *
 * 实现 ResponseUtil 类的所有静态方法。
 * 核心职责：将业务数据包装为统一格式的 JSON 响应，
 * 确保前后端交互的数据结构一致。
 *
 * 实现要点：
 *   - data 字段永远是 JSON 对象（null 时转为空对象 {}），
 *     避免前端遇到 null 时报错
 *   - 错误响应的 HTTP 状态码与业务码一致（如 401 → HTTP 401），
 *     便于网络层面的错误识别
 *   - 解析请求体时捕获异常，防止恶意格式的 JSON 导致服务崩溃
 *
 * @date 2025
 */

#include "ResponseUtil.h"
#include "Logger.h"

namespace community {

// ==================== 成功响应（带数据） ====================
/**
 * @brief 构造带数据的成功响应
 *
 * 最常用的成功响应形式：业务处理成功，并返回数据。
 * 调用示例：
 *   callback(ResponseUtil::success(user.toJson()));
 *
 * @param data 业务数据（JSON 对象或数组）
 * @param msg  成功消息，默认 "success"
 * @return 封装好的 HTTP 响应对象
 */
std::shared_ptr<drogon::HttpResponse>
ResponseUtil::success(const Json::Value& data, std::string_view msg) {
    // 构建响应体：code=200 + msg + data
    auto body = buildBody(RespCode::OK, msg, data);
    return makeJsonResponse(body);
}

// ==================== 成功响应（仅消息） ====================
/**
 * @brief 构造仅含消息的成功响应
 *
 * 适用于删除、更新等不需要返回数据的操作。
 * 调用示例：
 *   callback(ResponseUtil::success("删除成功"));
 *
 * @param msg 成功消息
 * @return 封装好的 HTTP 响应对象
 */
std::shared_ptr<drogon::HttpResponse>
ResponseUtil::success(std::string_view msg) {
    // 传 nullValue，buildBody 会自动转为空对象 {}
    auto body = buildBody(RespCode::OK, msg, Json::Value(Json::nullValue));
    return makeJsonResponse(body);
}

// ==================== 错误响应（自定义消息） ====================
/**
 * @brief 构造错误响应
 *
 * 将业务错误码同时用作 HTTP 状态码，
 * 这样即使不看响应体，仅看 HTTP 状态码也能大致判断错误类型。
 *
 * 例如：
 *   - 用户不存在 → RespCode::NOT_FOUND → HTTP 404
 *   - 未登录     → RespCode::UNAUTHORIZED → HTTP 401
 *
 * @param code 业务错误码（同时也是 HTTP 状态码）
 * @param msg  错误描述信息
 * @return 封装好的 HTTP 响应对象
 */
std::shared_ptr<drogon::HttpResponse>
ResponseUtil::error(RespCode code, std::string_view msg) {
    auto body = buildBody(code, msg, Json::Value(Json::nullValue));
    // 业务码转 HTTP 状态码：强转 int 再转枚举
    auto httpStatus = static_cast<drogon::HttpStatusCode>(static_cast<int>(code));
    return makeJsonResponse(body, httpStatus);
}

// ==================== 错误响应（默认消息） ====================
/**
 * @brief 构造错误响应（使用默认消息）
 *
 * 适用于不需要自定义错误消息的通用场景。
 *
 * @param code 业务错误码
 * @return 封装好的 HTTP 响应对象
 */
std::shared_ptr<drogon::HttpResponse>
ResponseUtil::error(RespCode code) {
    return error(code, defaultMessage(code));
}

// ==================== 分页成功响应 ====================
/**
 * @brief 构造分页数据的成功响应
 *
 * 将列表数据和分页信息组装为统一的分页结构：
 *   data = { list: [...], total, page, pageSize }
 *
 * 前端的 PageResult<T> 类型与此结构完全对应。
 *
 * @param list     当前页的数据列表（JSON 数组）
 * @param total    总记录数
 * @param page     当前页码（从 1 开始）
 * @param pageSize 每页条数
 * @return 封装好的 HTTP 响应对象
 */
std::shared_ptr<drogon::HttpResponse>
ResponseUtil::successPage(const Json::Value& list, int total, int page, int pageSize) {
    // 组装分页 data 对象
    Json::Value data;
    data["list"] = list;     // 数据列表
    data["total"] = total;   // 总记录数
    data["page"] = page;     // 当前页码
    data["pageSize"] = pageSize;  // 每页大小
    return success(data);
}

// ==================== 解析请求体 JSON ====================
/**
 * @brief 从 HTTP 请求中解析 JSON 请求体
 *
 * 封装 Drogon 内置的 getJsonObject() 方法，
 * 增加异常捕获，确保即使请求体格式非法也不会导致服务崩溃。
 *
 * 使用 optional 作为返回值，调用方通过判断是否有值来决定后续逻辑，
 * 比返回 bool + 出参更符合现代 C++ 风格。
 *
 * @param req HTTP 请求对象
 * @return 解析成功返回 JSON 对象，失败返回 std::nullopt
 */
std::optional<Json::Value>
ResponseUtil::parseJsonBody(const drogon::HttpRequestPtr& req) {
    try {
        // Drogon 内置方法：获取并解析 JSON 请求体
        auto json = req->getJsonObject();
        if (!json) {
            // 请求体为空或不是合法 JSON
            return std::nullopt;
        }
        return *json;
    } catch (const std::exception& e) {
        // 解析过程中发生异常（如 JSON 格式严重错误）
        LOG_ERROR << "解析请求体 JSON 失败: " << e.what();
        return std::nullopt;
    }
}

// ==================== 构建响应体 ====================
/**
 * @brief 构建统一格式的 JSON 响应体
 *
 * 所有响应的最终格式由本方法决定：
 *   { code: <int>, msg: <string>, data: <object> }
 *
 * 关键细节：
 *   - 字段名必须与前端约定一致（msg 不是 message）
 *   - data 为 null 时转成空对象 {}，避免前端访问 data.xxx 时报错
 *   - 这是一个纯函数，无副作用，便于测试
 *
 * @param code 业务状态码
 * @param msg  消息文本
 * @param data 业务数据
 * @return 构建好的 JSON 对象
 */
Json::Value ResponseUtil::buildBody(RespCode code, std::string_view msg, const Json::Value& data) {
    Json::Value body;
    // 业务状态码（int 类型）
    body["code"] = static_cast<int>(code);
    // 消息文本（字段名 msg，与前端对齐）
    body["msg"] = std::string(msg);
    
    // data 字段：如果为 null，转为空对象 {}
    // 这样前端访问 res.data.xxx 时永远不会因 null 而报错
    if (data.isNull()) {
        body["data"] = Json::Value(Json::objectValue);
    } else {
        body["data"] = data;
    }
    
    return body;
}

// ==================== 创建 JSON HTTP 响应 ====================
/**
 * @brief 将 JSON 对象包装为 Drogon 的 HTTP 响应对象
 *
 * 使用 Drogon 内置的 newHttpJsonResponse 工厂方法，
 * 自动设置 Content-Type 为 application/json。
 *
 * @param body       JSON 响应体
 * @param httpStatus HTTP 状态码（默认 200）
 * @return Drogon HTTP 响应对象
 */
std::shared_ptr<drogon::HttpResponse>
ResponseUtil::makeJsonResponse(const Json::Value& body, drogon::HttpStatusCode httpStatus) {
    // Drogon 内置方法：创建 JSON 格式的响应
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    // 设置 HTTP 状态码（200, 400, 401, 404, 500 等）
    resp->setStatusCode(httpStatus);
    return resp;
}

// ==================== 默认错误消息 ====================
/**
 * @brief 根据错误码返回默认的中文错误消息
 *
 * 当调用 error(code) 不带自定义消息时，从此方法获取默认消息。
 * 使用 switch-case 保证 O(1) 查找效率。
 *
 * @param code 业务错误码
 * @return 对应的中文默认消息
 */
std::string_view ResponseUtil::defaultMessage(RespCode code) {
    switch (code) {
        case RespCode::OK:             return "success";
        case RespCode::BAD_REQUEST:    return "请求参数错误";
        case RespCode::UNAUTHORIZED:   return "未授权，请先登录";
        case RespCode::FORBIDDEN:      return "禁止访问";
        case RespCode::NOT_FOUND:      return "资源不存在";
        case RespCode::INTERNAL_ERROR: return "服务器内部错误";
        default:                       return "未知错误";
    }
}

}  // namespace community
