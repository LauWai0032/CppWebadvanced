/**
 * @file Response.cpp
 * @brief 统一响应封装实现
 * @date 2025
 */

#include "common/Response.hpp"

namespace community_system {
namespace common {

// ============== 构造函数 ==============

Response::Response(ErrorCode code, std::string msg, nlohmann::json data)
    : m_code(code), m_msg(std::move(msg)), m_data(std::move(data)) {
}

// ============== 静态工厂方法 ==============

Response Response::successWithData(const nlohmann::json& data) {
    return Response(ErrorCode::OK, "success", data);
}

Response Response::success(const std::string& msg) {
    return Response(ErrorCode::OK, msg, nullptr);
}

Response Response::error(ErrorCode code, const std::string& msg) {
    return Response(code, msg, nullptr);
}

Response Response::error(ErrorCode code) {
    return Response(code, defaultMessage(code), nullptr);
}

// ============== 序列化 ==============

nlohmann::json Response::toJson() const {
    return {
        {"code", static_cast<int>(m_code)},
        {"msg",  m_msg},
        {"data", m_data}
    };
}

// ============== 辅助函数 ==============

const char* Response::defaultMessage(ErrorCode code) {
    switch (code) {
        case ErrorCode::OK:             return "success";
        case ErrorCode::BAD_REQUEST:    return "请求参数错误";
        case ErrorCode::UNAUTHORIZED:   return "未授权，请先登录";
        case ErrorCode::FORBIDDEN:      return "无权限访问";
        case ErrorCode::NOT_FOUND:      return "资源不存在";
        case ErrorCode::INTERNAL_ERROR: return "服务器内部错误";
        default:                        return "未知错误";
    }
}

}  // namespace common
}  // namespace community_system
