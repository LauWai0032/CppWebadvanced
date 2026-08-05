/**
 * @file api/vehicle.ts
 * @brief 车辆管理 API 接口封装
 *
 * 封装所有 车辆管理相关的 HTTP 请求，与后端 VehicleController 一一对应。
 *
 * 后端接口对应关系：
 *   GET    /api/vehicles       → getVehicleList    获取列表（分页）
 *   GET    /api/vehicles/{id}  → getVehicleById    获取详情
 *   POST   /api/vehicles       → createVehicle     新增
 *   PUT    /api/vehicles/{id}  → updateVehicle     修改
 *   DELETE /api/vehicles/{id}  → deleteVehicle     删除
 *
 * @date 2025
 */

import service from '@/utils/request'
import type {
  Vehicle,
  VehicleForm,
  VehicleQueryParams,
  PageResult,
} from '@/types'

/**
 * @brief 获取列表（分页）
 *
 * 对应后端接口：GET /api/vehicles
 *
 * 请求参数（Query）：
 *   - plate_number: 车牌号（可选）
 *   - user_id: 车主ID（可选）
 *   - page:     页码
 *   - pageSize: 每页条数
 *
 * 需要认证：是
 *
 * @param params 查询参数
 * @returns Promise<PageResult<Vehicle>> 分页列表
 */
export function getVehicleList(params: VehicleQueryParams) {
  return service.get<PageResult<Vehicle>>('/api/vehicles', { params })
}

/**
 * @brief 根据 ID 获取详情
 *
 * 对应后端接口：GET /api/vehicles/{id}
 *
 * 需要认证：是
 *
 * @param id  ID
 * @returns Promise<Vehicle> 详情
 */
export function getVehicleById(id: number) {
  return service.get<Vehicle>(`/api/vehicles/${id}`)
}

/**
 * @brief 创建新记录
 *
 * 对应后端接口：POST /api/vehicles
 *
 * 需要认证：是
 *
 * @param data 表单数据
 * @returns Promise<{ id: number }> 新记录 ID
 */
export function createVehicle(data: VehicleForm) {
  return service.post<{ id: number }>('/api/vehicles', data)
}

/**
 * @brief 更新记录
 *
 * 对应后端接口：PUT /api/vehicles/{id}
 *
 * 需要认证：是
 *
 * @param id   ID
 * @param data 更新的数据
 * @returns Promise<void>
 */
export function updateVehicle(id: number, data: Partial<VehicleForm>) {
  return service.put<void>(`/api/vehicles/${id}`, data)
}

/**
 * @brief 删除记录
 *
 * 对应后端接口：DELETE /api/vehicles/{id}
 *
 * 需要认证：是
 *
 * @param id ID
 * @returns Promise<void>
 */
export function deleteVehicle(id: number) {
  return service.delete<void>(`/api/vehicles/${id}`)
}

