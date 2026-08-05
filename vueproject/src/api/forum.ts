/**
 * @file api/forum.ts
 * @brief 论坛帖子 API 接口封装
 *
 * 封装所有 论坛帖子相关的 HTTP 请求，与后端 ForumPostController 一一对应。
 *
 * 后端接口对应关系：
 *   GET    /api/forum/posts       → getForumPostList    获取列表（分页）
 *   GET    /api/forum/posts/{id}  → getForumPostById    获取详情
 *   POST   /api/forum/posts       → createForumPost     新增
 *   PUT    /api/forum/posts/{id}  → updateForumPost     修改
 *   DELETE /api/forum/posts/{id}  → deleteForumPost     删除
 *
 * @date 2025
 */

import service from '@/utils/request'
import type {
  ForumPost,
  ForumPostForm,
  ForumPostQueryParams,
  PageResult,
} from '@/types'

/**
 * @brief 获取列表（分页）
 *
 * 对应后端接口：GET /api/forum/posts
 *
 * 请求参数（Query）：
 *   - title: 标题（可选）
 *   - category: 分类（可选）
 *   - page:     页码
 *   - pageSize: 每页条数
 *
 * 需要认证：是
 *
 * @param params 查询参数
 * @returns Promise<PageResult<ForumPost>> 分页列表
 */
export function getForumPostList(params: ForumPostQueryParams) {
  return service.get<PageResult<ForumPost>>('/api/forum/posts', { params })
}

/**
 * @brief 根据 ID 获取详情
 *
 * 对应后端接口：GET /api/forum/posts/{id}
 *
 * 需要认证：是
 *
 * @param id  ID
 * @returns Promise<ForumPost> 详情
 */
export function getForumPostById(id: number) {
  return service.get<ForumPost>(`/api/forum/posts/${id}`)
}

/**
 * @brief 创建新记录
 *
 * 对应后端接口：POST /api/forum/posts
 *
 * 需要认证：是
 *
 * @param data 表单数据
 * @returns Promise<{ id: number }> 新记录 ID
 */
export function createForumPost(data: ForumPostForm) {
  return service.post<{ id: number }>('/api/forum/posts', data)
}

/**
 * @brief 更新记录
 *
 * 对应后端接口：PUT /api/forum/posts/{id}
 *
 * 需要认证：是
 *
 * @param id   ID
 * @param data 更新的数据
 * @returns Promise<void>
 */
export function updateForumPost(id: number, data: Partial<ForumPostForm>) {
  return service.put<void>(`/api/forum/posts/${id}`, data)
}

/**
 * @brief 删除记录
 *
 * 对应后端接口：DELETE /api/forum/posts/{id}
 *
 * 需要认证：是
 *
 * @param id ID
 * @returns Promise<void>
 */
export function deleteForumPost(id: number) {
  return service.delete<void>(`/api/forum/posts/${id}`)
}

