/**
 * @file stores/index.ts
 * @brief Pinia Store 入口文件
 *
 * 统一导出所有 Store，方便其他模块集中引入。
 * 同时也用于集中管理 Store 的创建和注册逻辑。
 *
 * 目前包含的 Store：
 *   - useUserStore: 用户登录信息和权限状态
 *
 * 后续可按业务模块逐步扩展：
 *   - useAppStore: 应用全局状态（主题、侧边栏折叠等）
 *   - usePermissionStore: 权限管理
 *   - useTagStore: 标签页管理
 *   ...
 *
 * 使用方式（推荐）：
 *   // 直接从各模块文件引入，获得更好的 Tree Shaking
 *   import { useUserStore } from '@/stores/user'
 *
 *   // 或者从 index 统一引入
 *   import { useUserStore } from '@/stores'
 */

// 统一导出各业务 Store
export { useUserStore } from './user'

// 可在此添加其他 Store 的导出...
