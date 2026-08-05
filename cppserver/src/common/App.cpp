/**
 * @file App.cpp
 * @brief 应用启动类实现
 * @date 2025
 */

#include "common/App.hpp"

#include <fstream>
#include <sstream>

namespace community_system {
namespace common {

// ============== 构造与析构 ==============

App::App() {
    // 创建 HTTP 服务器实例
    m_server = std::make_unique<httplib::Server>();
}

App::~App() {
    // 显式停止服务器（如果在运行）
    if (m_server) {
        m_server->stop();
    }
    // 连接池由单例管理，其析构会自动释放
}

// ============== 配置加载 ==============

bool App::loadConfig(std::string_view configPath) {
    m_configPath = std::string(configPath);

    try {
        std::ifstream file(m_configPath);
        if (!file.is_open()) {
            std::cerr << "[App] Failed to open config file: " << m_configPath << std::endl;
            return false;
        }

        nlohmann::json j;
        file >> j;
        file.close();

        // 解析 server 配置
        if (j.contains("server")) {
            auto& s = j["server"];
            m_config.server.host = s.value("host", "0.0.0.0");
            m_config.server.port = s.value("port", 8080);
        }

        // 解析 database 配置
        if (j.contains("database")) {
            auto& d = j["database"];
            m_config.database.host     = d.value("host", "127.0.0.1");
            m_config.database.port     = d.value("port", 3306);
            m_config.database.user     = d.value("user", "root");
            m_config.database.password = d.value("password", "");
            m_config.database.database = d.value("database", "community_system");
            m_config.database.charset  = d.value("charset", "utf8mb4");
            m_config.database.maxSize  = d.value("maxSize", 10);
            m_config.database.minSize  = d.value("minSize", 2);
            m_config.database.timeout  = d.value("timeout", 5);
        }

        // 解析 log 配置
        if (j.contains("log")) {
            auto& l = j["log"];
            m_config.logLevel   = l.value("level", "info");
            m_config.logFile    = l.value("file", "");
            m_config.logConsole = l.value("console", true);
        }

        m_configLoaded = true;
        std::cout << "[App] Config loaded from " << m_configPath << std::endl;
        return true;

    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[App] Config parse error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[App] Load config error: " << e.what() << std::endl;
        return false;
    }
}

// ============== 日志初始化 ==============

void App::initLogger() {
    auto level = parseLogLevel(m_config.logLevel);

    // 如果指定了文件，同时输出到控制台和文件
    std::string logFile = m_config.logConsole ? "" : ""; // 默认仅控制台
    if (!m_config.logFile.empty()) {
        logFile = m_config.logFile;
    }

    Logger::instance().init(logFile, level);
    Logger::info("Logger initialized, level: {}", m_config.logLevel);
}

spdlog::level::level_enum App::parseLogLevel(std::string_view levelStr) {
    if (levelStr == "trace")    return spdlog::level::trace;
    if (levelStr == "debug")    return spdlog::level::debug;
    if (levelStr == "info")     return spdlog::level::info;
    if (levelStr == "warn")     return spdlog::level::warn;
    if (levelStr == "error")    return spdlog::level::err;
    if (levelStr == "critical") return spdlog::level::critical;
    if (levelStr == "off")      return spdlog::level::off;
    return spdlog::level::info;  // 默认 info
}

// ============== 数据库初始化 ==============

bool App::initDatabase() {
    try {
        ConnectionPool::instance().init(m_config.database);
        Logger::info("Database connection pool initialized");
        return true;
    } catch (const std::exception& e) {
        Logger::error("Database init failed: {}", e.what());
        // 数据库失败不一定要终止服务，视业务需求而定
        // 这里打印警告并继续（因为可能只是开发阶段未配置数据库）
        Logger::warn("Continuing without database (development mode)");
        return false;
    }
}

// ============== 路由注册 ==============

void App::registerAllRoutes() {
    for (auto& ctrl : m_controllers) {
        ctrl->registerRoutes(*m_server);
    }
    Logger::info("{} controllers registered", m_controllers.size());
}

// ============== 全局中间件 ==============

void App::registerGlobalMiddleware() {
    // 全局 OPTIONS 预检请求处理（CORS）
    m_server->Options("/.*", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type,Authorization");
        res.status = 204;
    });

    // 请求日志中间件（仅在 debug 级别记录，避免影响性能）
    m_server->set_pre_routing_handler([](const httplib::Request& req, httplib::Response&) {
        Logger::debug("{} {}", req.method, req.path);
        // 返回 Unhandled 表示继续后续路由处理
        return httplib::Server::HandlerResponse::Unhandled;
    });

    Logger::info("Global middleware registered");
}

// ============== 启动 ==============

int App::run() {
    if (!m_configLoaded) {
        std::cerr << "[App] Config not loaded, call loadConfig() first" << std::endl;
        return 1;
    }

    // 1. 初始化日志
    initLogger();

    Logger::info("========================================");
    Logger::info("  Community System Backend Starting...");
    Logger::info("========================================");
    Logger::info("Server: {}:{}", m_config.server.host, m_config.server.port);

    // 2. 初始化数据库连接池
    initDatabase();

    // 3. 注册全局中间件
    registerGlobalMiddleware();

    // 4. 注册所有 Controller 路由
    registerAllRoutes();

    // 5. 启动 HTTP 服务器（阻塞调用）
    Logger::info("HTTP server starting on http://{}:{}",
                 m_config.server.host, m_config.server.port);

    bool ok = m_server->listen(m_config.server.host, m_config.server.port);

    if (!ok) {
        Logger::error("Failed to start HTTP server on port {}", m_config.server.port);
        return 1;
    }

    Logger::info("Server stopped gracefully");
    return 0;
}

}  // namespace common
}  // namespace community_system
