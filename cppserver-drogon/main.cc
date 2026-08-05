/**
 * @file main.cc
 * @brief Drogon 社区系统后端 - 程序入口
 *
 * 使用 Drogon 框架构建的高性能社区管理系统后端服务。
 * 相比 cpp-httplib 版本（cppserver），本版本利用了 Drogon 框架的：
 *   - 内置高性能 HTTP 服务器（基于 epoll 多路复用）
 *   - 自动路由注册机制（通过宏注解）
 *   - 内置日志系统（Trantor Logger）
 *   - 过滤器/中间件机制（AuthFilter 实现 JWT 认证）
 *   - 灵活的配置管理（config.json / config.yaml）
 *
 * 数据库层使用原生 MySQL C API（因 Drogon 编译时未启用 MySQL ORM）。
 *
 * 初始化流程（按顺序）：
 *   1. 初始化日志系统
 *   2. 初始化数据库连接池
 *   3. 注册服务实例（依赖注入）
 *   4. 初始化 JWT 配置（密钥、过期时间）
 *   5. 加载 Drogon 配置并启动 HTTP 服务
 *
 * 使用到的 C++ 特性：
 *   - C++20: concepts, auto 推导
 *   - C++17: std::optional, std::string_view, if constexpr
 *   - C++14: std::make_unique
 *   - 智能指针管理资源（shared_ptr / unique_ptr）
 *   - RAII 资源管理（DbConnection 自动归还连接）
 *
 * @date 2025
 */

#include <drogon/drogon.h>
#include <iostream>
#include <memory>
#include <csignal>

#include "common/Logger.h"
#include "common/DbPool.h"
#include "common/JwtUtil.h"
#include "services/UserService.h"

using namespace community;

/**
 * @brief 程序入口函数
 *
 * 按顺序完成各项初始化，最后启动 Drogon 事件循环（阻塞调用）。
 * 使用 try-catch 包裹整个启动过程，捕获任何异常并输出错误信息。
 *
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组（argv[1] 可指定配置文件路径）
 * @return 退出码（0 正常退出，1 启动失败）
 */
int main(int argc, char* argv[]) {
    try {
        // ========== 1. 初始化日志 ==========
        // 将 Drogon/Trantor 的日志级别设置为 INFO
        // 生产环境可设置为 WARN 减少日志量
        Logger::init(trantor::Logger::kInfo);
        LOG_INFO << "=== CommunitySystem (Drogon Edition) 启动中 ===";

        // ========== 2. 初始化数据库连接池 ==========
        // 数据库连接配置
        // TODO: 生产环境应从配置文件读取，而不是硬编码
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
        // 此处使用默认值，启动后可从 custom_config 读取并覆盖
        auto& dbPool = DbPool::instance();
        dbPool.init(dbCfg);
        LOG_INFO << "数据库连接池初始化完成";

        // ========== 3. 注册服务实例（依赖注入） ==========
        // UserService 作为全局单例服务注册
        // 设计思路：通过 setInstance/getInstance 实现依赖注入，
        // Controller 层不直接依赖具体实现类的创建，便于单元测试和替换。
        auto userService = std::make_shared<services::UserService>();
        // 将服务实例注册到全局服务管理
        services::UserService::setInstance(userService);
        LOG_INFO << "服务层初始化完成";

        // ========== 4. 初始化 JWT 配置 ==========
        // 设置 JWT 签名密钥和默认过期时间。
        //
        // 【重要】生产环境注意事项：
        //   - 密钥必须从环境变量或配置文件读取，严禁硬编码
        //   - 密钥长度建议至少 32 字节（256 位）
        //   - 一旦密钥泄露，所有 Token 都可被伪造
        //   - 可考虑定期轮换密钥
        //
        // 默认过期时间 24 小时，可根据安全需求调整
        JwtUtil::setSecret("community_system_jwt_secret_key_2025_change_in_production");
        JwtUtil::setDefaultExpireSeconds(86400);  // 24 小时
        LOG_INFO << "JWT 配置初始化完成";

        // ========== 5. 配置 Drogon 并启动 ==========
        // 加载配置文件
        // 配置文件路径默认为当前目录下的 config.json
        // 也可通过命令行参数指定：./CommunitySystem /path/to/config.json
        std::string configPath = "config.json";
        if (argc > 1) {
            configPath = argv[1];
        }

        // 加载 Drogon 配置文件（JSON 或 YAML 格式均可）
        drogon::app().loadConfigFile(configPath);

        // 设置日志级别（与 Logger::init 保持一致）
        drogon::app().setLogLevel(trantor::Logger::kInfo);

        LOG_INFO << "HTTP 服务器启动中，监听端口 8080...";
        LOG_INFO << "=== CommunitySystem (Drogon Edition) 已就绪 ===";

        // 启动 Drogon 事件循环（阻塞调用）
        // 此方法会一直运行，直到收到 SIGINT / SIGTERM 信号或调用 app().quit()
        drogon::app().run();

    } catch (const std::exception& e) {
        // 捕获标准异常，打印错误信息并退出
        std::cerr << "[致命错误] " << e.what() << std::endl;
        LOG_FATAL << "启动失败: " << e.what();
        return 1;
    } catch (...) {
        // 捕获未知异常（防止程序直接崩溃无日志）
        std::cerr << "[致命错误] 未知异常" << std::endl;
        LOG_FATAL << "启动失败: 未知异常";
        return 1;
    }

    return 0;
}
