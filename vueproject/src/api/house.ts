/**
 * @file api/house.ts
 * @brief 房屋管理 API 接口封装
 *
 * 封装所有 房屋管理相关的 HTTP 请求，与后端 HouseController 一一对应。
 *
 * 后端接口对应关系：
 *   GET    /api/houses       → getHouseList    获取列表（分页）
 *   GET    /api/houses/{id}  → getHouseById    获取详情
 *   POST   /api/houses       → createHouse     新增
 *   PUT    /api/houses/{id}  → updateHouse     修改
 *   DELETE /api/houses/{id}  → deleteHouse     删除
 *
 * @date 2025
 */

import service from '@/utils/request'
import type {
  House,
  HouseForm,
  HouseQueryParams,
  PageResult,
} from '@/types'

/**
 * @brief 获取列表（分页）
 *
 * 对应后端接口：GET /api/houses
 *
 * 请求参数（Query）：
 *   - building_no: 楼栋号（可选）
 *   - room_no: 门牌号（可选）
 *   - page:     页码
 *   - pageSize: 每页条数
 *
 * 需要认证：是
 *
 * @param params 查询参数
 * @returns Promise<PageResult<House>> 分页列表
 */
export function getHouseList(params: HouseQueryParams) {
  return service.get<PageResult<House>>('/api/houses', { params })
}

/**
 * @brief 根据 ID 获取详情
 *
 * 对应后端接口：GET /api/houses/{id}
 *
 * 需要认证：是
 *
 * @param id  ID
 * @returns Promise<House> 详情
 */
export function getHouseById(id: number) {
  return service.get<House>(`/api/houses/${id}`)
}

/**
 * @brief 创建新记录
 *
 * 对应后端接口：POST /api/houses
 *
 * 需要认证：是
 *
 * @param data 表单数据
 * @returns Promise<{ id: number }> 新记录 ID
 */
export function createHouse(data: HouseForm) {
  return service.post<{ id: number }>('/api/houses', data)
}

/**
 * @brief 更新记录
 *
 * 对应后端接口：PUT /api/houses/{id}
 *
 * 需要认证：是
 *
 * @param id   ID
 * @param data 更新的数据
 * @returns Promise<void>
 */
export function updateHouse(id: number, data: Partial<HouseForm>) {
  return service.put<void>(`/api/houses/${id}`, data)
}

/**
 * @brief 删除记录
 *
 * 对应后端接口：DELETE /api/houses/{id}
 *
 * 需要认证：是
 *
 * @param id ID
 * @returns Promise<void>
 */
export function deleteHouse(id: number) {
  return service.delete<void>(`/api/houses/${id}`)
}

