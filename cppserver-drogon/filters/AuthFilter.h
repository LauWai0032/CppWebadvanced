/**
 * @file AuthFilter.h
 * @brief 认证过滤器（JWT Token 验证）
 *
 * 继承 Drogon 的 HttpFilter，实现 JWT 认证中间件。
 * 从请求头提取 Authorization Bearer Token，调用 JwtUtil 验证有效性。
 *
 * 工作流程：
 *   1. 提取 Authorization 请求头
 *   2. 解析 Bearer Token
 *   3. 调用 JwtUtil 验证签名和过期时间
 *   4. 验证通过：将用户信息存入 request attributes，放行到 Controller
 *   5. 验证失败：返回 401 未授权响应，拦截请求
 *
 * @date 2025
 */

#pragma once

#include <drogon/HttpFilter.h>
#include "common/Logger.h"

namespace community {
namespace filters {

/**
 * @class AuthFilter
 * @brief JWT 认证过滤器
 *
 * 在请求到达 Controller 之前执行 Token 验证。
 * 验证通过后，以下信息会被写入 request 的 attributes：
 *   - "userId"   (int)         用户 ID
 *   - "username" (std::string) 用户名
 *   - "role"     (std::string) 用户角色
 *
 * 使用方式（在 Controller 中）：
 *   int userId = req->getAttributes()->get<int>("userId");
 */
class AuthFilter : public drogon::HttpFilter<AuthFilter> {
public:
    /**
     * @brief 过滤器执行方法
     * @param req   HTTP 请求
     * @param fcb   过滤器回调（调用则拦截请求，直接返回响应）
     * @param fccb  过滤器链回调（调用则放行到下一级）
     */
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fccb) override;
};

}  // namespace filters
}  // namespace community
