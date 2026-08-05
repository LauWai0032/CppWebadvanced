/**
 * @file Database.hpp
 * @brief 数据库模块兼容头文件（已迁移到 ConnectionPool）
 *
 * 注意：本文件为历史兼容保留，实际数据库连接管理已迁移到
 *       ConnectionPool 类（include/common/ConnectionPool.hpp）。
 *
 * 请使用 ConnectionPool::instance() 替代旧的 Database 单例。
 *
 * @date 2025
 */

#pragma once

#include "ConnectionPool.hpp"

namespace community_system {
namespace common {

// 兼容别名：旧代码中的 Database 名称可继续使用
using Database = ConnectionPool;

}  // namespace common
}  // namespace community_system
