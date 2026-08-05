/**
 * @file AuthFilter.h
 * @brief 认证过滤器（中间件模式）
 *
 * 演示 Drogon 的过滤器/中间件机制。
 * 在实际项目中，可用于：
 *   - Token 验证
 *   - 权限检查
 *   - 请求日志记录
 *
 * 当前版本为示例实现，仅做日志记录。
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
 * @brief 认证过滤器
 *
 * 继承 drogon::HttpFilter，实现 doFilter 方法。
 * 通过 REGISTER_FILTER 宏自动注册到 Drogon 框架。
 */
class AuthFilter : public drogon::HttpFilter<AuthFilter> {
public:
    /**
     * @brief 过滤器执行方法
     * @param req       HTTP 请求
     * @param callback  回调函数（调用则继续处理，不调用则拦截请求）
     *
     * 设计思路（中间件模式）：
     *   - 在请求到达 Controller 之前执行
     *   - 可修改请求、拦截请求、或直接返回响应
     *   - 调用 callback(resp) 表示放行，调用 callback(nullptr) 表示继续
     */
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fccb) override;
};

}  // namespace filters
}  // namespace community
