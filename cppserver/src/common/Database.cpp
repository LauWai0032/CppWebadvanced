/**
 * @file Database.cpp
 * @brief 数据库兼容实现文件（已迁移到 ConnectionPool）
 *
 * 本文件为历史兼容保留，实际实现请参考 ConnectionPool.cpp。
 *
 * @date 2025
 */

#include "common/Database.hpp"

// 兼容实现：所有功能已迁移到 ConnectionPool，
// 本文件仅保证链接时不会因为空文件而产生警告。
namespace community_system {
namespace common {

// 故意留空：Database 是 ConnectionPool 的别名，所有实现在 ConnectionPool.cpp 中

}  // namespace common
}  // namespace community_system
