/**
 * @file ResponseUtil.cc
 * @brief 统一 JSON 响应封装实现
 * @date 2025
 */

#include "ResponseUtil.h"
#include "Logger.h"

namespace community {

std::shared_ptr<drogon::HttpResponse>
ResponseUtil::success(const Json::Value& data, std::string_view msg) {
    auto body = buildBody(RespCode::OK, msg, data);
    return makeJsonResponse(body);
}

std::shared_ptr<drogon::HttpResponse>
ResponseUtil::success(std::string_view msg) {
    auto body = buildBody(RespCode::OK, msg, Json::Value(Json::nullValue));
    return makeJsonResponse(body);
}

std::shared_ptr<drogon::HttpResponse>
ResponseUtil::error(RespCode code, std::string_view msg) {
    auto body = buildBody(code, msg, Json::Value(Json::nullValue));
    auto httpStatus = static_cast<drogon::HttpStatusCode>(static_cast<int>(code));
    return makeJsonResponse(body, httpStatus);
}

std::shared_ptr<drogon::HttpResponse>
ResponseUtil::error(RespCode code) {
    return error(code, defaultMessage(code));
}

std::shared_ptr<drogon::HttpResponse>
ResponseUtil::successPage(const Json::Value& list, int total, int page, int pageSize) {
    Json::Value data;
    data["list"] = list;
    data["total"] = total;
    data["page"] = page;
    data["pageSize"] = pageSize;
    return success(data);
}

std::optional<Json::Value>
ResponseUtil::parseJsonBody(const drogon::HttpRequestPtr& req) {
    try {
        auto json = req->getJsonObject();
        if (!json) {
            return std::nullopt;
        }
        return *json;
    } catch (const std::exception& e) {
        LOG_ERROR << "解析请求体 JSON 失败: " << e.what();
        return std::nullopt;
    }
}

Json::Value ResponseUtil::buildBody(RespCode code, std::string_view msg, const Json::Value& data) {
    Json::Value body;
    body["code"] = static_cast<int>(code);
    body["message"] = std::string(msg);
    
    if (data.isNull()) {
        body["data"] = Json::Value(Json::objectValue);
    } else {
        body["data"] = data;
    }
    
    return body;
}

std::shared_ptr<drogon::HttpResponse>
ResponseUtil::makeJsonResponse(const Json::Value& body, drogon::HttpStatusCode httpStatus) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(httpStatus);
    return resp;
}

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
