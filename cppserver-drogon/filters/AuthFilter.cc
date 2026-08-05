/**
 * @file AuthFilter.cc
 * @brief 认证过滤器实现
 * @date 2025
 */

#include "AuthFilter.h"

namespace community {
namespace filters {

void AuthFilter::doFilter(
    const drogon::HttpRequestPtr& req,
    drogon::FilterCallback&& /*fcb*/,
    drogon::FilterChainCallback&& fccb)
{
    // 记录请求日志
    LOG_INFO << "[AuthFilter] " 
             << req->methodString() << " " 
             << req->path();

    // 当前版本：直接放行（实际项目中可添加 Token 验证等逻辑）
    // 示例：检查 Authorization 头
    // auto authHeader = req->getHeader("Authorization");
    // if (authHeader.empty()) {
    //     auto resp = ResponseUtil::error(RespCode::UNAUTHORIZED, "请先登录");
    //     fcb(resp);  // 拦截请求
    //     return;
    // }

    // 放行到下一个过滤器或 Controller
    fccb();
}

}  // namespace filters
}  // namespace community
