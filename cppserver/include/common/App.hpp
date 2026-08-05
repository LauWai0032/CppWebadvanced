/**
 * @file App.hpp
 * @brief 应用启动类（应用装配器）
 *
 * App 类是整个后端服务的入口装配器，负责：
 *   1. 加载配置文件
 *   2. 初始化日志系统
 *   3. 初始化数据库连接池
 *   4. 创建并注册所有 Controller
 *   5. 启动 HTTP 服务器
 *
 * 使用到的 C++ 特性：
 *   - C++11: std::unique_ptr, std::vector, std::string, 智能指针
 *   - C++14: std::make_unique
 *   - C++17: std::string_view, std::optional
 *   - C++20: 概念（概念约束用于模板函数）
 *
 * 设计模式：
 *   - 门面模式（Facade）：对外提供简单的 run() 接口，内部封装复杂初始化流程
 *   - 组合模式：管理一组 Controller，统一注册路由
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <stdexcept>

#include "../../json.hpp"
#include "../../httplib.h"

#include "Logger.hpp"
#include "ConnectionPool.hpp"
#include "BaseController.hpp"

namespace community_system {
namespace common {

/**
 * @struct ServerConfig
 * @brief HTTP 服务器配置
 */
struct ServerConfig {
    std::string host = "0.0.0.0";
    int         port = 8080;
};

/**
 * @struct AppConfig
 * @brief 应用总配置
 */
struct AppConfig {
    ServerConfig server;
    DBConfig     database;
    std::string  logLevel = "info";
    std::string  logFile  = "";
    bool         logConsole = true;
};

/**
 * @class App
 * @brief 应用启动类
 *
 * 设计思路：
 *  - 单一职责：只负责装配和启动，不包含业务逻辑
 *  - 依赖注入：Controller 由 App 创建并注入 Server
 *  - 生命周期管理：App 管理所有资源（连接池、Controller、Server）的生命周期
 *
 * 使用方式：
 *   int main() {
 *       App app;
 *       app.loadConfig("config.json");
 *       return app.run();
 *   }
 */
class App {
public:
    /**
     * @brief 构造函数
     */
    App();

    /**
     * @brief 析构函数（确保资源正确释放）
     */
    ~App();

    // 禁止拷贝
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    /**
     * @brief 加载配置文件
     * @param configPath 配置文件路径
     * @return bool 是否加载成功
     *
     * 配置文件为 JSON 格式，包含 server、database、log 三部分。
     */
    bool loadConfig(std::string_view configPath);

    /**
     * @brief 注册一个 Controller
     * @tparam ControllerT Controller 类型，必须继承自 BaseController
     * @tparam Args 构造参数类型
     * @param args 构造参数
     *
     * 使用可变参数模板 + 完美转发，支持任意构造函数签名。
     * Controller 由 unique_ptr 管理，自动生命周期。
     */
    template <typename ControllerT, typename... Args>
    void registerController(Args&&... args) {
        static_assert(std::is_base_of_v<controllers::BaseController, ControllerT>,
                      "Controller must inherit from BaseController");

        auto ctrl = std::make_unique<ControllerT>(std::forward<Args>(args)...);
        m_controllers.push_back(std::move(ctrl));
    }

    /**
     * @brief 启动应用
     * @return int 退出码（0 正常，非 0 异常）
     *
     * 执行流程：
     *   1. 初始化日志
     *   2. 初始化连接池
     *   3. 注册所有 Controller 的路由
     *   4. 启动 HTTP 服务器（阻塞）
     */
    int run();

    /**
     * @brief 获取配置（只读）
     * @return const AppConfig&
     */
    const AppConfig& getConfig() const { return m_config; }

    /**
     * @brief 获取服务器实例指针（测试用）
     * @return httplib::Server*
     */
    httplib::Server* getServer() { return m_server.get(); }

private:
    /**
     * @brief 初始化日志系统
     */
    void initLogger();

    /**
     * @brief 初始化数据库连接池
     * @return bool 是否成功
     */
    bool initDatabase();

    /**
     * @brief 注册所有 Controller 的路由
     */
    void registerAllRoutes();

    /**
     * @brief 注册全局中间件（跨域、日志等）
     */
    void registerGlobalMiddleware();

    /**
     * @brief 将字符串日志级别转换为 spdlog 级别
     */
    static spdlog::level::level_enum parseLogLevel(std::string_view levelStr);

private:
    AppConfig                                              m_config;         ///< 应用配置
    std::unique_ptr<httplib::Server>                       m_server;         ///< HTTP 服务器
    std::vector<std::unique_ptr<controllers::BaseController>> m_controllers; ///< Controller 列表
    bool                                                   m_configLoaded = false;  ///< 配置是否已加载
    std::string                                            m_configPath;     ///< 配置文件路径
};

}  // namespace common
}  // namespace community_system
