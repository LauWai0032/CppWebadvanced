/**
 * @file test_main.cc
 * @brief 测试入口文件
 * @date 2025
 */

#include <drogon/drogon.h>
#include <drogon/drogon_test.h>

// 简单的连接测试
DROGON_TEST(BasicTest) {
    // 验证框架基本功能
    auto req = drogon::HttpRequest::newHttpRequest();
    CHECK(req != nullptr);
    req->setPath("/api/users");
    CHECK(req->path() == "/api/users");
}
