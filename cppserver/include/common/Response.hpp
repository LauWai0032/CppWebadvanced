/**
 * @file Response.hpp
 * @brief 统一 HTTP 响应封装
 *
 * 定义统一的 API 响应格式和错误码枚举，提供便捷的静态方法
 * 构造成功/失败响应。所有 Controller 的接口返回都应使用此类。
 *
 * 使用到的 C++ 特性：
 *   - C++11: enum class（强类型枚举）, std::string
 *   - C++17: std::string_view
 *
 * 响应格式：
 *   {
 *     "code": 200,
 *     "msg": "success",
 *     "data": {...}
 *   }
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include "../json.hpp"  // nlohmann/json

namespace community_system {
namespace common {

/**
 * @enum ErrorCode
 * @brief HTTP 业务错误码枚举
 *
 * 采用强类型枚举（enum class），避免命名空间污染。
 * 数值与 HTTP 状态码保持一致，便于映射。
 */
enum class ErrorCode {
    OK             = 200,  ///< 成功
    BAD_REQUEST    = 400,  ///< 请求参数错误
    UNAUTHORIZED   = 401,  ///< 未授权
    FORBIDDEN      = 403,  ///< 禁止访问
    NOT_FOUND      = 404,  ///< 资源不存在
    INTERNAL_ERROR = 500   ///< 服务器内部错误
};

/**
 * @class Response
 * @brief 统一响应封装类
 *
 * 设计思路：
 *   - 静态工厂方法模式：通过 success/error 静态方法创建对象
 *   - 支持链式调用（暂未实现，预留扩展空间）
 *   - 最终转换为 nlohmann::json 输出到 HTTP 响应
 */
class Response {
public:
    /**
     * @brief 构造函数
     * @param code 错误码
     * @param msg  消息文本
     * @param data 数据 JSON 对象
     */
    explicit Response(ErrorCode code = ErrorCode::OK,
                      std::string msg = "success",
                      nlohmann::json data = nullptr);

    /**
     * @brief 构造成功响应（带数据 JSON）
     * @param data 业务数据 JSON
     * @return Response 成功响应对象
     */
    static Response successWithData(const nlohmann::json& data);

    /**
     * @brief 构造成功响应（仅消息，无数据）
     * @param msg 成功消息
     * @return Response 成功响应对象
     */
    static Response success(const std::string& msg = "success");

    /**
     * @brief 构造错误响应
     * @param code 错误码
     * @param msg  错误消息
     * @return Response 错误响应对象
     */
    static Response error(ErrorCode code, const std::string& msg);

    /**
     * @brief 构造错误响应（使用默认错误消息）
     * @param code 错误码
     * @return Response 错误响应对象
     */
    static Response error(ErrorCode code);

    /**
     * @brief 转换为 nlohmann::json 对象
     * @return nlohmann::json
     */
    nlohmann::json toJson() const;

    /**
     * @brief 获取错误码数值
     * @return int 错误码整型值
     */
    int getCode() const { return static_cast<int>(m_code); }

    /**
     * @brief 获取消息
     * @return const std::string&
     */
    const std::string& getMsg() const { return m_msg; }

    /**
     * @brief 获取数据
     * @return const nlohmann::json&
     */
    const nlohmann::json& getData() const { return m_data; }

private:
    ErrorCode      m_code;  ///< 响应码
    std::string    m_msg;   ///< 响应消息
    nlohmann::json m_data;  ///< 响应数据

    /**
     * @brief 根据错误码返回默认错误消息
     * @param code 错误码
     * @return const char* 默认消息
     */
    static const char* defaultMessage(ErrorCode code);
};

}  // namespace common
}  // namespace community_system
