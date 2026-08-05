/**
 * @file utils/request.ts
 * @brief Axios 请求封装（拦截器 + 统一错误处理）
 *
 * 基于 axios 创建统一的请求实例，提供：
 *   - 请求拦截器：自动注入 JWT Token 到 Authorization 头
 *   - 响应拦截器：统一处理响应格式和错误
 *   - 类型定义：后端统一响应结构的 TypeScript 泛型
 *
 * 与后端约定：
 *   - 后端统一响应格式：{ code: number, msg: string, data: T }
 *   - 成功时 code = 200，data 为业务数据
 *   - 失败时 code != 200，msg 为错误描述
 *   - Token 过期/无效时 code = 401，前端需清除登录状态并跳转登录页
 *
 * 设计要点：
 *   - 响应拦截器中直接返回 data 部分，简化组件调用
 *   - 请求错误统一通过 ElMessage 提示，组件可专注于业务逻辑
 *   - Token 从 localStorage 读取（与 Pinia user store 保持一致）
 */

import axios from 'axios'
import type { AxiosInstance, InternalAxiosRequestConfig, AxiosResponse } from 'axios'
import { ElMessage } from 'element-plus'
import { useUserStore } from '@/stores/user'

// ==================== 类型定义 ====================

/**
 * @interface BaseResponse
 * @brief 后端统一响应结构
 *
 * 与后端 ResponseUtil::buildBody 返回的 JSON 完全一致：
 *   { code, msg, data }
 *
 * @template T data 字段的类型
 */
interface BaseResponse<T = unknown> {
  /** 业务状态码：200 成功，其他为错误码 */
  code: number
  /** 响应消息：成功时为 "success"，失败时为错误描述 */
  msg: string
  /** 响应数据：具体业务数据 */
  data: T
}

// ==================== 创建 axios 实例 ====================

const service: AxiosInstance = axios.create({
  baseURL: 'http://127.0.0.1:8080', // 后端服务地址
  timeout: 10000, // 请求超时时间：10 秒
})

// ==================== 请求拦截器 ====================
/**
 * @brief 请求拦截器
 *
 * 在每个请求发送之前执行：
 *   1. 从 localStorage 读取 Token
 *   2. 如果有 Token，添加到 Authorization 请求头（Bearer 格式）
 *   3. 设置默认 Content-Type 为 application/json
 *
 * 注意：
 *   - Token 存储在 localStorage 中，与 Pinia user store 共享
 *   - 此处直接读 localStorage 而不用 store，是为了避免循环依赖
 *     （request 在 store 之前被导入）
 */
service.interceptors.request.use(
  (config: InternalAxiosRequestConfig) => {
    // 从 localStorage 获取 Token
    const token = localStorage.getItem('token')

    // 如果有 token，添加到 Authorization 请求头
    // 格式：Authorization: Bearer <token>
    if (token) {
      config.headers.Authorization = `Bearer ${token}`
    }

    // 设置默认内容类型
    config.headers['Content-Type'] = 'application/json'

    // 必须返回 config，否则请求不会被发送
    return config
  },
  (error) => {
    // 请求发送失败（如请求被取消、网络不可达等）
    console.error('Request Error:', error)
    return Promise.reject(error)
  },
)

// ==================== 响应拦截器 ====================
/**
 * @brief 响应拦截器
 *
 * 处理后端返回的响应数据：
 *
 * 成功回调（HTTP 状态码 2xx）：
 *   - 检查业务状态码 code
 *   - code == 200：返回 data 部分（剥离外层包装）
 *   - code != 200：业务错误，弹出提示，reject
 *   - code == 401：Token 无效/过期，清除登录状态，跳转登录页
 *
 * 失败回调（HTTP 状态码非 2xx，如 404、500、断网等）：
 *   - 弹出网络错误提示
 *   - reject 错误
 *
 * 注意：
 *   - 拦截器返回 data，所以组件中调用 API 拿到的直接是业务数据
 *   - 例如：const user = await getUserById(1) 拿到的就是 User 对象
 */
service.interceptors.response.use(
  (response: AxiosResponse<BaseResponse>) => {
    const res = response.data

    // 业务成功（code == 200）：返回 data 部分
    // 使用 == 而非 ===，以兼容可能的字符串 "200"
    if (res.code == 200) {
      // 直接返回 data，组件调用时拿到的就是业务数据
      return res.data as unknown as AxiosResponse['data']
    } else {
      // 业务错误：弹出错误提示
      ElMessage.error(res.msg || '请求失败')

      // 特殊处理：401 未授权
      // 可能原因：Token 过期、Token 无效、未登录
      if (res.code === 401) {
        // 清除用户登录状态
        const userStore = useUserStore()
        userStore.logout()
        // 跳转到登录页
        window.location.href = '/login'
      }

      // reject 错误，让调用方的 catch 能捕获
      return Promise.reject(new Error(res.msg || '请求失败'))
    }
  },
  (error) => {
    // HTTP 请求失败（如 404、500、断网、超时等）
    // 从 error 对象中提取错误信息
    let errMsg = '网络请求异常'
    if (error.response) {
      // 有响应，但 HTTP 状态码非 2xx
      errMsg = error.response.data?.msg || `请求失败 (${error.response.status})`
    } else if (error.message) {
      // 无响应（如断网）
      errMsg = error.message
    }

    ElMessage.error(errMsg)
    console.error('Response Error:', error)

    return Promise.reject(error)
  },
)

export default service
