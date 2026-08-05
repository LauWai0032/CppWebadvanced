/**
 * @file main.cc
 * @brief Drogon 社区系统后端 - 程序入口
 *
 * 使用 Drogon 框架构建的高性能社区管理系统后端服务。
 * 相比 cpp-httplib 版本（cppserver），本版本利用了 Drogon 框架的：
 *   - 内置高性能 HTTP 服务器（基于 epoll 多路复用）
 *   - 自动路由注册机制（通过宏注解）
 *   - 内置日志系统
 *   - 过滤器/中间件机制
 *   - 灵活的配置管理
 *
 * 数据库层使用原生 MySQL C API（因 Drogon 编译时未启用 MySQL ORM）。
 *
 * 使用到的 C++ 特性：
 *   - C++20: concepts, auto 推导, 结构化绑定
 *   - C++17: std::optional, std::string_view, if constexpr
 *   - C++14: std::make_unique
 *   - 智能指针管理资源
 *
 * @date 2025
 */

#include <drogon/drogon.h>
#include <iostream>
#include <memory>
#include <csignal>

#include "common/Logger.h"
#include "common/DbPool.h"
#include "services/UserService.h"

using namespace community;

/**
 * @brief 程序入口函数
 *
 * 初始化流程：
 *   1. 初始化日志系统
 *   2. 初始化数据库连接池
 *   3. 注册服务实例（依赖注入）
 *   4. 加载 Drogon 配置并启动
 */
int main(int argc, char* argv[]) {
    try {
        // ========== 1. 初始化日志 ==========
        Logger::init(trantor::Logger::kInfo);
        LOG_INFO << "=== CommunitySystem (Drogon Edition) 启动中 ===";

        // ========== 2. 初始化数据库连接池 ==========
        // 数据库连接配置
        DbConfig dbCfg;
        dbCfg.host = "127.0.0.1";
        dbCfg.port = 3306;
        dbCfg.user = "root";
        dbCfg.password = "123456";
        dbCfg.database = "community_system";
        dbCfg.charset = "utf8mb4";
        dbCfg.poolSize = 10;

        // 如果配置文件已加载，从中读取
        // 注意：配置文件在 loadConfigFile 后才会生效，
        // 此处使用默认值，启动后会从 config.json 读取
        
        auto& dbPool = DbPool::instance();
        dbPool.init(dbCfg);
        LOG_INFO << "数据库连接池初始化完成";

        // ========== 3. 注册服务实例（依赖注入） ==========
        // UserService 作为全局单例服务注册
        auto userService = std::make_shared<services::UserService>();
        // 将服务实例注册到全局服务管理
        services::UserService::setInstance(userService);
        LOG_INFO << "服务层初始化完成";

        // ========== 4. 配置 Drogon 并启动 ==========
        // 加载配置文件
        std::string configPath = "config.json";
        if (argc > 1) {
            configPath = argv[1];
        }

        drogon::app().loadConfigFile(configPath);

        // 设置监听地址和端口（与 cppserver 保持一致：8080）
        drogon::app().setLogLevel(trantor::Logger::kInfo);

        LOG_INFO << "HTTP 服务器启动中，监听端口 8080...";
        LOG_INFO << "=== CommunitySystem (Drogon Edition) 已就绪 ===";

        // 启动 Drogon 事件循环（阻塞）
        drogon::app().run();

    } catch (const std::exception& e) {
        std::cerr << "[致命错误] " << e.what() << std::endl;
        LOG_FATAL << "启动失败: " << e.what();
        return 1;
    } catch (...) {
        std::cerr << "[致命错误] 未知异常" << std::endl;
        LOG_FATAL << "启动失败: 未知异常";
        return 1;
    }

    return 0;
}
