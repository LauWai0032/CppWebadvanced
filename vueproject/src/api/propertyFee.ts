/**
 * @file api/propertyFee.ts
 * @brief 物业账单 API 接口封装
 *
 * 封装所有 物业账单相关的 HTTP 请求，与后端 PropertyFeeController 一一对应。
 *
 * 后端接口对应关系：
 *   GET    /api/property-fees       → getPropertyFeeList    获取列表（分页）
 *   GET    /api/property-fees/{id}  → getPropertyFeeById    获取详情
 *   POST   /api/property-fees       → createPropertyFee     新增
 *   PUT    /api/property-fees/{id}  → updatePropertyFee     修改
 *   DELETE /api/property-fees/{id}  → deletePropertyFee     删除
 *
 * @date 2025
 */

import service from '@/utils/request'
import type {
  PropertyFee,
  PropertyFeeForm,
  PropertyFeeQueryParams,
  PageResult,
} from '@/types'

/**
 * @brief 获取列表（分页）
 *
 * 对应后端接口：GET /api/property-fees
 *
 * 请求参数（Query）：
 *   - fee_type: 费用类型（可选）
 *   - status: 状态（可选）
 *   - bill_month: 账单月份（可选）
 *   - page:     页码
 *   - pageSize: 每页条数
 *
 * 需要认证：是
 *
 * @param params 查询参数
 * @returns Promise<PageResult<PropertyFee>> 分页列表
 */
export function getPropertyFeeList(params: PropertyFeeQueryParams) {
  return service.get<PageResult<PropertyFee>>('/api/property-fees', { params })
}

/**
 * @brief 根据 ID 获取详情
 *
 * 对应后端接口：GET /api/property-fees/{id}
 *
 * 需要认证：是
 *
 * @param id  ID
 * @returns Promise<PropertyFee> 详情
 */
export function getPropertyFeeById(id: number) {
  return service.get<PropertyFee>(`/api/property-fees/${id}`)
}

/**
 * @brief 创建新记录
 *
 * 对应后端接口：POST /api/property-fees
 *
 * 需要认证：是
 *
 * @param data 表单数据
 * @returns Promise<{ id: number }> 新记录 ID
 */
export function createPropertyFee(data: PropertyFeeForm) {
  return service.post<{ id: number }>('/api/property-fees', data)
}

/**
 * @brief 更新记录
 *
 * 对应后端接口：PUT /api/property-fees/{id}
 *
 * 需要认证：是
 *
 * @param id   ID
 * @param data 更新的数据
 * @returns Promise<void>
 */
export function updatePropertyFee(id: number, data: Partial<PropertyFeeForm>) {
  return service.put<void>(`/api/property-fees/${id}`, data)
}

/**
 * @brief 删除记录
 *
 * 对应后端接口：DELETE /api/property-fees/{id}
 *
 * 需要认证：是
 *
 * @param id ID
 * @returns Promise<void>
 */
export function deletePropertyFee(id: number) {
  return service.delete<void>(`/api/property-fees/${id}`)
}

/**
 * @brief 缴费
 * @param id 账单ID
 * @param paymentMethod 支付方式
 */
export function payFee(id: number, paymentMethod: string) {
  return service.put<void>(`/api/property-fees/${id}/pay`, { payment_method: paymentMethod })
}

