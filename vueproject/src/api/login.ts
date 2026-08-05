/**
 * @file api/login.ts
 * @brief 登录相关 API 接口封装
 *
 * 封装用户登录接口，与后端 UserController 的 /api/login 对应。
 * 登录接口是少数不需要 Token 认证的公开接口之一。
 *
 * 后端接口对应关系：
 *   POST /api/login  →  loginApi  用户登录
 *
 * 登录成功后前端应：
 *   1. 将 Token 存入 localStorage（request 拦截器会自动读取）
 *   2. 将用户信息存入 Pinia store
 *   3. 跳转到首页或之前访问的页面
 */

import service from '@/utils/request'
import type { LoginForm, LoginResult } from '@/types'

/**
 * @brief 用户登录
 *
 * 对应后端接口：POST /api/login
 *
 * 请求体（JSON）：
 *   - username: 用户名（必填）
 *   - password: 密码（明文，必填）
 *
 * 返回数据（data 部分）：
 *   - token:     JWT Token 字符串（用于后续接口认证）
 *   - id:        用户 ID
 *   - username:  用户名
 *   - real_name: 真实姓名
 *   - role:      用户角色（admin/property/owner）
 *
 * 需要认证：否（公开接口）
 *
 * 错误情况：
 *   - 用户名或密码为空 → code=400, msg="用户名和密码不能为空"
 *   - 用户名或密码错误 → code=400, msg="用户名或密码错误"
 *
 * @param data 登录表单数据
 * @returns Promise<LoginResult> 登录成功后的用户信息和 Token
 */
export function loginApi(data: LoginForm) {
  return service.post<LoginResult>('/api/login', data)
}
