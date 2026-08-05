/**
 * @file stores/user.ts
 * @brief 用户信息 Pinia Store
 *
 * 管理当前登录用户的状态信息，包括 Token、用户基本信息等。
 * 状态持久化到 localStorage，刷新页面后自动恢复。
 *
 * 设计思路：
 *   - 初始化时从 localStorage 读取已登录用户信息（SSR 友好）
 *   - 提供登录、登出、更新信息等 action
 *   - 使用 Composition API 风格的 defineStore（Pinia 推荐写法）
 *   - 与 request.ts 中的 Token 注入逻辑配合使用
 *
 * 使用方式：
 *   import { useUserStore } from '@/stores/user'
 *   const userStore = useUserStore()
 *   console.log(userStore.username)
 *   userStore.login(loginResult)
 *   userStore.logout()
 */

import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import type { LoginResult, UserRole } from '@/types'

/**
 * @brief localStorage 中存储 Token 的键名
 *
 * 与 request.ts 中读取的键名保持一致，
 * 确保 axios 拦截器能正确读取并注入请求头。
 */
const TOKEN_KEY = 'token'

/**
 * @brief localStorage 中存储用户信息的键名
 */
const USER_INFO_KEY = 'userInfo'

/**
 * @function useUserStore
 * @brief 用户状态 Store
 *
 * 采用 Pinia Composition API 风格（setup 写法），
 * 与 Vue 3 的组合式 API 保持一致，更灵活。
 *
 * State（ref 变量）：
 *   - token: JWT 令牌
 *   - userId: 用户 ID
 *   - username: 用户名
 *   - realName: 真实姓名
 *   - role: 用户角色
 *
 * Getters（computed 变量）：
 *   - isLoggedIn: 是否已登录
 *   - isAdmin: 是否为管理员
 *   - isProperty: 是否为物业人员
 *
 * Actions（函数）：
 *   - login(data): 登录成功后保存用户信息
 *   - logout(): 登出，清除所有状态和本地存储
 *   - loadFromStorage(): 从 localStorage 恢复状态
 */
export const useUserStore = defineStore('user', () => {
  // ==================== State ====================

  /** JWT Token，从 localStorage 初始化（如果有的话） */
  const token = ref<string>('')

  /** 用户 ID */
  const userId = ref<number>(0)

  /** 登录用户名 */
  const username = ref<string>('')

  /** 用户真实姓名 */
  const realName = ref<string>('')

  /** 用户角色（admin/property/owner） */
  const role = ref<UserRole | string>('')

  // ==================== Getters ====================

  /**
   * @brief 是否已登录
   *
   * 简单判断 Token 是否存在。
   * 注意：Token 可能已过期，过期判断在后端 AuthFilter 中进行，
   * 前端仅做快速判断，遇到 401 时再清除状态。
   */
  const isLoggedIn = computed(() => !!token.value)

  /**
   * @brief 是否为管理员
   */
  const isAdmin = computed(() => role.value === 'admin')

  /**
   * @brief 是否为物业人员
   */
  const isProperty = computed(() => role.value === 'property')

  /**
   * @brief 是否为业主
   */
  const isOwner = computed(() => role.value === 'owner')

  // ==================== Actions ====================

  /**
   * @brief 登录成功 - 保存用户信息到 store 和 localStorage
   *
   * 登录接口调用成功后调用此方法。
   * 将后端返回的用户信息存入响应式状态，
   * 同时持久化到 localStorage，实现刷新页面保持登录状态。
   *
   * @param data 登录接口返回的用户数据（包含 token 和用户信息）
   */
  function login(data: LoginResult) {
    // 更新响应式状态
    token.value = data.token
    userId.value = data.id
    username.value = data.username
    realName.value = data.real_name
    role.value = data.role

    // 持久化到 localStorage
    // 注意：localStorage 只能存字符串，用户信息需要 JSON.stringify
    localStorage.setItem(TOKEN_KEY, data.token)
    localStorage.setItem(
      USER_INFO_KEY,
      JSON.stringify({
        id: data.id,
        username: data.username,
        real_name: data.real_name,
        role: data.role,
      }),
    )
  }

  /**
   * @brief 登出 - 清除所有用户状态
   *
   * 清除 store 中的状态和 localStorage 中的持久化数据。
   * 通常在以下场景调用：
   *   - 用户主动点击"退出登录"
   *   - 请求返回 401（Token 过期/无效）
   *   - 账号被禁用等安全场景
   */
  function logout() {
    // 清除响应式状态
    token.value = ''
    userId.value = 0
    username.value = ''
    realName.value = ''
    role.value = ''

    // 清除本地存储
    localStorage.removeItem(TOKEN_KEY)
    localStorage.removeItem(USER_INFO_KEY)
  }

  /**
   * @brief 从 localStorage 加载用户信息
   *
   * 页面刷新或应用初始化时调用，
   * 从 localStorage 中恢复用户状态，保持登录态。
   *
   * 注意：
   *   - 此方法在 Store 实例化时自动调用一次
   *   - 如果 localStorage 中没有数据，则状态保持为空
   *   - Token 有效性由后端验证，前端不做校验
   */
  function loadFromStorage() {
    // 读取 Token
    const storedToken = localStorage.getItem(TOKEN_KEY)
    if (storedToken) {
      token.value = storedToken
    }

    // 读取用户信息
    const storedUserInfo = localStorage.getItem(USER_INFO_KEY)
    if (storedUserInfo) {
      try {
        const info = JSON.parse(storedUserInfo)
        userId.value = info.id || 0
        username.value = info.username || ''
        realName.value = info.real_name || ''
        role.value = info.role || ''
      } catch (e) {
        // JSON 解析失败，说明存储的数据已损坏，清除之
        console.warn('用户信息存储损坏，已清除', e)
        localStorage.removeItem(USER_INFO_KEY)
      }
    }
  }

  // ==================== 初始化 ====================

  /**
   * Store 创建时自动从 localStorage 加载状态。
   * 这样无论在哪个组件首次 useUserStore()，状态都已经恢复好了。
   */
  loadFromStorage()

  // ==================== 返回值 ====================
  // 返回所有需要暴露给外部的 state / getters / actions
  return {
    // state
    token,
    userId,
    username,
    realName,
    role,
    // getters
    isLoggedIn,
    isAdmin,
    isProperty,
    isOwner,
    // actions
    login,
    logout,
    loadFromStorage,
  }
})
