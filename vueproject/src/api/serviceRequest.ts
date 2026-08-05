/**
 * @file api/serviceRequest.ts
 * @brief 报修投诉 API 接口封装
 *
 * 封装所有 报修投诉相关的 HTTP 请求，与后端 ServiceRequestController 一一对应。
 *
 * 后端接口对应关系：
 *   GET    /api/service-requests       → getServiceRequestList    获取列表（分页）
 *   GET    /api/service-requests/{id}  → getServiceRequestById    获取详情
 *   POST   /api/service-requests       → createServiceRequest     新增
 *   PUT    /api/service-requests/{id}  → updateServiceRequest     修改
 *   DELETE /api/service-requests/{id}  → deleteServiceRequest     删除
 *
 * @date 2025
 */

import service from '@/utils/request'
import type {
  ServiceRequest,
  ServiceRequestForm,
  ServiceRequestQueryParams,
  PageResult,
} from '@/types'

/**
 * @brief 获取列表（分页）
 *
 * 对应后端接口：GET /api/service-requests
 *
 * 请求参数（Query）：
 *   - type: 类型（可选）
 *   - status: 状态（可选）
 *   - page:     页码
 *   - pageSize: 每页条数
 *
 * 需要认证：是
 *
 * @param params 查询参数
 * @returns Promise<PageResult<ServiceRequest>> 分页列表
 */
export function getServiceRequestList(params: ServiceRequestQueryParams) {
  return service.get<PageResult<ServiceRequest>>('/api/service-requests', { params })
}

/**
 * @brief 根据 ID 获取详情
 *
 * 对应后端接口：GET /api/service-requests/{id}
 *
 * 需要认证：是
 *
 * @param id  ID
 * @returns Promise<ServiceRequest> 详情
 */
export function getServiceRequestById(id: number) {
  return service.get<ServiceRequest>(`/api/service-requests/${id}`)
}

/**
 * @brief 创建新记录
 *
 * 对应后端接口：POST /api/service-requests
 *
 * 需要认证：是
 *
 * @param data 表单数据
 * @returns Promise<{ id: number }> 新记录 ID
 */
export function createServiceRequest(data: ServiceRequestForm) {
  return service.post<{ id: number }>('/api/service-requests', data)
}

/**
 * @brief 更新记录
 *
 * 对应后端接口：PUT /api/service-requests/{id}
 *
 * 需要认证：是
 *
 * @param id   ID
 * @param data 更新的数据
 * @returns Promise<void>
 */
export function updateServiceRequest(id: number, data: Partial<ServiceRequestForm>) {
  return service.put<void>(`/api/service-requests/${id}`, data)
}

/**
 * @brief 删除记录
 *
 * 对应后端接口：DELETE /api/service-requests/{id}
 *
 * 需要认证：是
 *
 * @param id ID
 * @returns Promise<void>
 */
export function deleteServiceRequest(id: number) {
  return service.delete<void>(`/api/service-requests/${id}`)
}

/**
 * @brief 修改状态
 * @param id 请求ID
 * @param status 新状态
 */
export function updateRequestStatus(id: number, status: string) {
  return service.put<void>(`/api/service-requests/${id}/status`, { status })
}

