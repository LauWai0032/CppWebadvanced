/**
 * @file api/user.ts
 * @brief 用户管理 API 接口封装
 *
 * 封装所有用户管理相关的 HTTP 请求，与后端 UserController 一一对应。
 * 每个接口都有完整的 TypeScript 类型定义（请求参数和返回数据）。
 *
 * 后端接口对应关系（Drogon 版 UserController）：
 *   GET    /api/users       → getUserList    获取用户列表（分页）
 *   GET    /api/users/{id}  → getUserById    获取单个用户详情
 *   POST   /api/users       → createUser     创建用户
 *   PUT    /api/users/{id}  → updateUser     更新用户
 *   DELETE /api/users/{id}  → deleteUser     删除用户
 *
 * 注意事项：
 *   - 所有接口（除登录外）都需要在请求头携带 JWT Token
 *   - Token 注入由 request.ts 的请求拦截器自动处理
 *   - 响应拦截器已剥离外层包装，直接返回 data 部分
 *
 * 使用方式：
 *   import { getUserList, createUser } from '@/api/user'
 *   const data = await getUserList({ page: 1, pageSize: 10 })
 *   // data 的类型为 PageResult<User>
 */

import service from '@/utils/request'
import type {
  User,
  UserForm,
  UserQueryParams,
  PageResult,
} from '@/types'

/**
 * @brief 获取用户列表（分页）
 *
 * 对应后端接口：GET /api/users
 *
 * 请求参数（Query）：
 *   - page:     页码（从 1 开始，默认 1）
 *   - pageSize: 每页条数（默认 10，最大 100）
 *   - username: 用户名搜索（可选，模糊匹配）
 *   - role:     角色筛选（可选）
 *   - status:   状态筛选（可选）
 *
 * 返回数据：PageResult<User>
 *   - list:     用户信息数组
 *   - total:    总记录数
 *   - page:     当前页码
 *   - pageSize: 每页条数
 *
 * 需要认证：是（请求头需携带 Bearer Token）
 *
 * @param params 查询参数
 * @returns Promise<PageResult<User>> 分页用户列表
 */
export function getUserList(params: UserQueryParams) {
  return service.get<PageResult<User>>('/api/users', { params })
}

/**
 * @brief 根据 ID 获取单个用户详情
 *
 * 对应后端接口：GET /api/users/{id}
 *
 * 路径参数：
 *   - id: 用户 ID（正整数）
 *
 * 返回数据：User 用户完整信息（不包含密码）
 *
 * 需要认证：是
 *
 * @param id 用户 ID
 * @returns Promise<User> 用户详情
 */
export function getUserById(id: number) {
  return service.get<User>(`/api/users/${id}`)
}

/**
 * @brief 创建新用户
 *
 * 对应后端接口：POST /api/users
 *
 * 请求体（JSON）：
 *   - username:  用户名（必填，唯一）
 *   - password:  密码（明文，后端会自动加密存储）
 *   - real_name: 真实姓名（可选）
 *   - phone:     手机号（可选）
 *   - role:      角色（admin/property/owner，默认 owner）
 *   - status:    状态（1-正常 0-禁用，默认 1）
 *
 * 返回数据：{ id: number } 新创建用户的 ID
 *
 * 需要认证：是
 *
 * @param data 用户表单数据
 * @returns Promise<{ id: number }> 新用户 ID
 */
export function createUser(data: UserForm) {
  return service.post<{ id: number }>('/api/users', data)
}

/**
 * @brief 更新用户信息
 *
 * 对应后端接口：PUT /api/users/{id}
 *
 * 路径参数：
 *   - id: 用户 ID
 *
 * 请求体（JSON）：
 *   - username:  用户名（可选，不修改则不传）
 *   - password:  密码（可选，为空表示不修改密码）
 *   - real_name: 真实姓名（可选）
 *   - phone:     手机号（可选）
 *   - role:      角色（可选）
 *   - status:    状态（必传）
 *
 * 返回数据：操作结果（成功时 code=200）
 *
 * 需要认证：是
 *
 * @param id   用户 ID
 * @param data 更新的用户数据
 * @returns Promise<void> 成功时无返回数据（仅 code=200）
 */
export function updateUser(id: number, data: Partial<UserForm>) {
  return service.put<void>(`/api/users/${id}`, data)
}

/**
 * @brief 删除用户
 *
 * 对应后端接口：DELETE /api/users/{id}
 *
 * 路径参数：
 *   - id: 用户 ID
 *
 * 返回数据：操作结果（成功时 code=200, msg="删除成功"）
 *
 * 需要认证：是
 *
 * 注意：删除操作不可恢复，调用前应让用户二次确认。
 *
 * @param id 用户 ID
 * @returns Promise<void> 成功时无返回数据
 */
export function deleteUser(id: number) {
  return service.delete<void>(`/api/users/${id}`)
}
