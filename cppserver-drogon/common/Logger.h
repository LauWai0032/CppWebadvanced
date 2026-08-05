/**
 * @file Logger.h
 * @brief 日志工具封装
 *
 * 基于 Drogon/Trantor 内置日志系统的封装。
 * 提供便捷的日志初始化和输出接口。
 *
 * 设计思路：
 *   - 使用 Drogon 内置日志（trantor::Logger）
 *   - 静态工具类，无需实例化
 *   - 支持日志级别设置
 *
 * 使用方式：
 *   LOG_INFO << "这是一条日志";
 *   LOG_ERROR << "这是一条错误日志";
 *   LOG_DEBUG << "这是一条调试日志";
 *
 * @date 2025
 */

#pragma once

#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <string>
#include <string_view>

namespace community {

/**
 * @class Logger
 * @brief 日志工具类（静态方法）
 *
 * 封装 Drogon 内置日志功能，提供：
 *   - 日志级别设置
 *   - 便捷的日志输出方法
 *   - 与 Drogon 框架日志系统完全集成
 */
class Logger {
public:
    /**
     * @brief 初始化日志系统
     * @param level 日志级别（trantor::Logger::LogLevel）
     */
    static void init(trantor::Logger::LogLevel level = trantor::Logger::kInfo);

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    static void setLevel(trantor::Logger::LogLevel level);

    /**
     * @brief 输出 INFO 级别日志
     * @param msg 日志消息
     */
    static void info(std::string_view msg);

    /**
     * @brief 输出 WARN 级别日志
     * @param msg 日志消息
     */
    static void warn(std::string_view msg);

    /**
     * @brief 输出 ERROR 级别日志
     * @param msg 日志消息
     */
    static void error(std::string_view msg);

    /**
     * @brief 输出 DEBUG 级别日志
     * @param msg 日志消息
     */
    static void debug(std::string_view msg);

private:
    Logger() = default;  // 禁止实例化
};

}  // namespace community
