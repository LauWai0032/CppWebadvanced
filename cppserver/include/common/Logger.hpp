/**
 * @file Logger.hpp
 * @brief 日志系统单例封装
 *
 * 基于 spdlog 库封装的日志工具，提供统一的日志接口。
 * 采用单例模式（Meyers' Singleton），支持控制台输出和文件输出。
 *
 * 使用到的 C++ 特性：
 *   - C++11: std::mutex, static local 线程安全（Meyers Singleton）
 *   - C++17: 内联静态成员, std::string_view
 *
 * @date 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <mutex>
#include <iostream>

// 引入 spdlog 头文件（第三方 header-only 库）
#include "../spdlog/spdlog.h"
#include "../spdlog/sinks/stdout_color_sinks.h"
#include "../spdlog/sinks/basic_file_sink.h"

namespace community_system {
namespace common {

/**
 * @class Logger
 * @brief 日志单例类，封装 spdlog 提供统一日志接口
 *
 * 设计思路：
 *   - 单例模式：全局唯一日志实例，通过 instance() 获取
 *   - 组合模式：内部持有 spdlog::logger 的多个 sink（控制台 + 文件）
 *   - RAII：程序结束时自动 flush 并关闭日志
 *
 * 使用方式：
 *   Logger::info("Server started on port {}", 8080);
 *   Logger::error("Connection failed: {}", error_msg);
 */
class Logger {
public:
    /**
     * @brief 获取单例实例（Meyers' Singleton，C++11 起线程安全）
     * @return Logger& 单例引用
     */
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    // 禁止拷贝和移动
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    /**
     * @brief 初始化日志系统
     * @param log_file 日志文件路径，为空则仅输出到控制台
     * @param level 日志级别，默认 info
     *
     * 该方法不是线程安全的，应在程序启动时调用一次
     */
    void init(std::string_view log_file = "",
              spdlog::level::level_enum level = spdlog::level::info) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_initialized) {
            return;
        }

        // 创建控制台 sink（带颜色）
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(level);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");

        std::vector<spdlog::sink_ptr> sinks{console_sink};

        // 如果指定了日志文件，添加文件 sink
        if (!log_file.empty()) {
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                std::string(log_file), true);
            file_sink->set_level(level);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");
            sinks.push_back(file_sink);
        }

        // 创建组合 logger
        m_logger = std::make_shared<spdlog::logger>("community_system",
                                                    sinks.begin(), sinks.end());
        m_logger->set_level(level);
        m_logger->flush_on(spdlog::level::err);  // error 级别立即刷新

        // 注册为全局 logger，方便使用
        spdlog::register_logger(m_logger);
        spdlog::set_default_logger(m_logger);

        m_initialized = true;
    }

    /**
     * @brief 获取底层 spdlog logger 指针
     * @return std::shared_ptr<spdlog::logger>
     */
    std::shared_ptr<spdlog::logger> getLogger() const {
        return m_logger;
    }

    // ============== 便捷静态方法 ==============

    template <typename... Args>
    static void debug(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        spdlog::debug(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        spdlog::info(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        spdlog::warn(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        spdlog::error(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void critical(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        spdlog::critical(fmt, std::forward<Args>(args)...);
    }

private:
    /**
     * @brief 私有构造函数（单例）
     *
     * 默认初始化控制台输出，确保在 init() 未调用时也能输出日志
     */
    Logger() : m_initialized(false) {
        // 默认控制台 logger，防止 init 前使用崩溃
        try {
            m_logger = spdlog::stdout_color_mt("community_system_default");
            m_logger->set_level(spdlog::level::info);
            spdlog::set_default_logger(m_logger);
            m_initialized = true;
        } catch (const spdlog::spdlog_ex& ex) {
            // 极端情况：spdlog 初始化失败，向 stderr 输出
            std::cerr << "[Logger] Init failed: " << ex.what() << std::endl;
        }
    }

    /**
     * @brief 析构函数，确保日志 flush
     */
    ~Logger() {
        if (m_logger) {
            m_logger->flush();
        }
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;  ///< 底层 spdlog 实例
    std::mutex m_mutex;                         ///< 初始化互斥锁
    bool m_initialized;                         ///< 是否已初始化
};

}  // namespace common
}  // namespace community_system
