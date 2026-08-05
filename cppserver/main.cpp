/**
 * @file main.cpp
 * @brief 程序入口
 *
 * 只做最简单的事情：
 *   1. 创建 App 实例
 *   2. 加载配置
 *   3. 注册业务 Controller
 *   4. 启动服务
 *
 * 所有复杂的初始化逻辑都封装在 App 类中，
 * 遵循"单一职责原则"和"依赖倒置原则"。
 *
 * @date 2025
 */

#include <iostream>
#include <memory>
#include <string>

#include "include/common/App.hpp"
#include "include/common/Logger.hpp"
#include "include/services/UserService.hpp"
#include "include/controllers/UserController.hpp"

int main(int argc, char* argv[]) {
    using namespace community_system;

    // 配置文件路径（可通过命令行参数指定）
    std::string configPath = "config/config.json";
    if (argc > 1) {
        configPath = argv[1];
    }

    try {
        // 创建应用实例
        common::App app;

        // 加载配置
        if (!app.loadConfig(configPath)) {
            std::cerr << "[Main] Failed to load config: " << configPath << std::endl;
            std::cerr << "[Main] Using default config... actually will fail but let's try" << std::endl;
            // 注意：配置加载失败时也可以继续使用默认配置启动，这里选择退出
            return 1;
        }

        // ========== 注册业务 Service ==========
        auto userService = std::make_shared<services::UserService>();

        // ========== 注册 Controller ==========
        // （通过依赖注入方式将 Service 注入 Controller）
        app.registerController<controllers::UserController>(userService);

        // ========== 启动服务 ==========
        return app.run();

    } catch (const std::exception& e) {
        std::cerr << "[Main] Fatal exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[Main] Unknown fatal exception" << std::endl;
        return 1;
    }
}
