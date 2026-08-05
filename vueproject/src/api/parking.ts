/**
 * @file api/parking.ts
 * @brief 车位管理 API 接口封装
 *
 * 封装所有 车位管理相关的 HTTP 请求，与后端 ParkingSpotController 一一对应。
 *
 * 后端接口对应关系：
 *   GET    /api/parking-spots       → getParkingSpotList    获取列表（分页）
 *   GET    /api/parking-spots/{id}  → getParkingSpotById    获取详情
 *   POST   /api/parking-spots       → createParkingSpot     新增
 *   PUT    /api/parking-spots/{id}  → updateParkingSpot     修改
 *   DELETE /api/parking-spots/{id}  → deleteParkingSpot     删除
 *
 * @date 2025
 */

import service from '@/utils/request'
import type {
  ParkingSpot,
  ParkingSpotForm,
  ParkingSpotQueryParams,
  PageResult,
} from '@/types'

/**
 * @brief 获取列表（分页）
 *
 * 对应后端接口：GET /api/parking-spots
 *
 * 请求参数（Query）：
 *   - spot_number: 车位编号（可选）
 *   - zone: 区域（可选）
 *   - type: 类型（可选）
 *   - is_occupied: 是否占用（可选）
 *   - page:     页码
 *   - pageSize: 每页条数
 *
 * 需要认证：是
 *
 * @param params 查询参数
 * @returns Promise<PageResult<ParkingSpot>> 分页列表
 */
export function getParkingSpotList(params: ParkingSpotQueryParams) {
  return service.get<PageResult<ParkingSpot>>('/api/parking-spots', { params })
}

/**
 * @brief 根据 ID 获取详情
 *
 * 对应后端接口：GET /api/parking-spots/{id}
 *
 * 需要认证：是
 *
 * @param id  ID
 * @returns Promise<ParkingSpot> 详情
 */
export function getParkingSpotById(id: number) {
  return service.get<ParkingSpot>(`/api/parking-spots/${id}`)
}

/**
 * @brief 创建新记录
 *
 * 对应后端接口：POST /api/parking-spots
 *
 * 需要认证：是
 *
 * @param data 表单数据
 * @returns Promise<{ id: number }> 新记录 ID
 */
export function createParkingSpot(data: ParkingSpotForm) {
  return service.post<{ id: number }>('/api/parking-spots', data)
}

/**
 * @brief 更新记录
 *
 * 对应后端接口：PUT /api/parking-spots/{id}
 *
 * 需要认证：是
 *
 * @param id   ID
 * @param data 更新的数据
 * @returns Promise<void>
 */
export function updateParkingSpot(id: number, data: Partial<ParkingSpotForm>) {
  return service.put<void>(`/api/parking-spots/${id}`, data)
}

/**
 * @brief 删除记录
 *
 * 对应后端接口：DELETE /api/parking-spots/{id}
 *
 * 需要认证：是
 *
 * @param id ID
 * @returns Promise<void>
 */
export function deleteParkingSpot(id: number) {
  return service.delete<void>(`/api/parking-spots/${id}`)
}

/**
 * @brief 车辆入场
 * @param id 车位ID
 * @param vehicleId 车辆ID
 */
export function parkVehicle(id: number, vehicleId: number) {
  return service.put<void>(`/api/parking-spots/${id}/park`, { vehicle_id: vehicleId })
}

/**
 * @brief 车辆离场
 * @param id 车位ID
 */
export function leaveVehicle(id: number) {
  return service.put<void>(`/api/parking-spots/${id}/leave`)
}

