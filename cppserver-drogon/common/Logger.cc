/**
 * @file Logger.cc
 * @brief 日志工具实现
 * @date 2025
 */

#include "Logger.h"

namespace community {

void Logger::init(trantor::Logger::LogLevel level) {
    // 设置 Drogon/Trantor 全局日志级别
    trantor::Logger::setLogLevel(level);
    
    // 设置日志输出到控制台（同时也会输出到文件，由 config.json 配置）
    trantor::Logger::setLogLevel(level);
    
    LOG_INFO << "日志系统初始化完成，级别: " 
             << (level == trantor::Logger::kDebug ? "DEBUG" :
                 level == trantor::Logger::kInfo  ? "INFO"  :
                 level == trantor::Logger::kWarn  ? "WARN"  : "ERROR");
}

void Logger::setLevel(trantor::Logger::LogLevel level) {
    trantor::Logger::setLogLevel(level);
}

void Logger::info(std::string_view msg) {
    LOG_INFO << std::string(msg);
}

void Logger::warn(std::string_view msg) {
    LOG_WARN << std::string(msg);
}

void Logger::error(std::string_view msg) {
    LOG_ERROR << std::string(msg);
}

void Logger::debug(std::string_view msg) {
    LOG_DEBUG << std::string(msg);
}

}  // namespace community
