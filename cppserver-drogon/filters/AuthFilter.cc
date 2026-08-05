/**
 * @file AuthFilter.cc
 * @brief 认证过滤器实现（JWT Token 验证）
 * @date 2025
 */

#include "AuthFilter.h"
#include "common/JwtUtil.h"
#include "common/ResponseUtil.h"

#include <string>

namespace community {
namespace filters {

void AuthFilter::doFilter(
    const drogon::HttpRequestPtr& req,
    drogon::FilterCallback&& fcb,
    drogon::FilterChainCallback&& fccb)
{
    // 1. 提取 Authorization 请求头
    std::string authHeader = req->getHeader("Authorization");

    LOG_INFO << "[AuthFilter] " << req->methodString() << " " << req->path();

    if (authHeader.empty()) {
        LOG_WARN << "[AuthFilter] 请求缺少 Authorization 头";
        auto resp = ResponseUtil::error(RespCode::UNAUTHORIZED, "请先登录");
        fcb(resp);  // 拦截请求，返回 401
        return;
    }

    // 2. 提取 Bearer Token
    auto tokenOpt = JwtUtil::extractBearerToken(authHeader);
    if (!tokenOpt) {
        LOG_WARN << "[AuthFilter] Authorization 头格式错误（需要 Bearer 前缀）";
        auto resp = ResponseUtil::error(RespCode::UNAUTHORIZED, "Token 格式错误");
        fcb(resp);
        return;
    }

    // 3. 验证 Token
    auto payloadOpt = JwtUtil::verifyToken(*tokenOpt);
    if (!payloadOpt) {
        LOG_WARN << "[AuthFilter] Token 验证失败或已过期";
        auto resp = ResponseUtil::error(RespCode::UNAUTHORIZED, "Token 无效或已过期，请重新登录");
        fcb(resp);
        return;
    }

    // 4. 验证通过，将用户信息存入 request attributes
    //    Controller 中可通过 req->getAttributes()->get<T>("key") 获取
    auto attrs = req->getAttributes();
    attrs->insert("userId", payloadOpt->userId);
    attrs->insert("username", payloadOpt->username);
    attrs->insert("role", payloadOpt->role);

    LOG_INFO << "[AuthFilter] 认证通过，用户: " << payloadOpt->username
             << "，角色: " << payloadOpt->role;

    // 5. 放行到下一个过滤器或 Controller
    fccb();
}

}  // namespace filters
}  // namespace community
