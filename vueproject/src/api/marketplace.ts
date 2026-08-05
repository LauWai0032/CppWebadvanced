/**
 * @file api/marketplace.ts
 * @brief 二手交易 API 接口封装
 *
 * 封装所有 二手交易相关的 HTTP 请求，与后端 MarketplaceItemController 一一对应。
 *
 * 后端接口对应关系：
 *   GET    /api/marketplace/items       → getMarketplaceItemList    获取列表（分页）
 *   GET    /api/marketplace/items/{id}  → getMarketplaceItemById    获取详情
 *   POST   /api/marketplace/items       → createMarketplaceItem     新增
 *   PUT    /api/marketplace/items/{id}  → updateMarketplaceItem     修改
 *   DELETE /api/marketplace/items/{id}  → deleteMarketplaceItem     删除
 *
 * @date 2025
 */

import service from '@/utils/request'
import type {
  MarketplaceItem,
  MarketplaceItemForm,
  MarketplaceItemQueryParams,
  PageResult,
} from '@/types'

/**
 * @brief 获取列表（分页）
 *
 * 对应后端接口：GET /api/marketplace/items
 *
 * 请求参数（Query）：
 *   - title: 标题（可选）
 *   - exchange_type: 交易方式（可选）
 *   - status: 状态（可选）
 *   - page:     页码
 *   - pageSize: 每页条数
 *
 * 需要认证：是
 *
 * @param params 查询参数
 * @returns Promise<PageResult<MarketplaceItem>> 分页列表
 */
export function getMarketplaceItemList(params: MarketplaceItemQueryParams) {
  return service.get<PageResult<MarketplaceItem>>('/api/marketplace/items', { params })
}

/**
 * @brief 根据 ID 获取详情
 *
 * 对应后端接口：GET /api/marketplace/items/{id}
 *
 * 需要认证：是
 *
 * @param id  ID
 * @returns Promise<MarketplaceItem> 详情
 */
export function getMarketplaceItemById(id: number) {
  return service.get<MarketplaceItem>(`/api/marketplace/items/${id}`)
}

/**
 * @brief 创建新记录
 *
 * 对应后端接口：POST /api/marketplace/items
 *
 * 需要认证：是
 *
 * @param data 表单数据
 * @returns Promise<{ id: number }> 新记录 ID
 */
export function createMarketplaceItem(data: MarketplaceItemForm) {
  return service.post<{ id: number }>('/api/marketplace/items', data)
}

/**
 * @brief 更新记录
 *
 * 对应后端接口：PUT /api/marketplace/items/{id}
 *
 * 需要认证：是
 *
 * @param id   ID
 * @param data 更新的数据
 * @returns Promise<void>
 */
export function updateMarketplaceItem(id: number, data: Partial<MarketplaceItemForm>) {
  return service.put<void>(`/api/marketplace/items/${id}`, data)
}

/**
 * @brief 删除记录
 *
 * 对应后端接口：DELETE /api/marketplace/items/{id}
 *
 * 需要认证：是
 *
 * @param id ID
 * @returns Promise<void>
 */
export function deleteMarketplaceItem(id: number) {
  return service.delete<void>(`/api/marketplace/items/${id}`)
}

