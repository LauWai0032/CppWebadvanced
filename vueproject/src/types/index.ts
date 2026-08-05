/**
 * @file types/index.ts
 * @brief 全局 TypeScript 类型定义
 *
 * 集中存放项目中所有公共类型定义，方便各模块统一引用。
 * 包含：
 *   - 后端统一响应格式（BaseResponse、PageResult）
 *   - 业务实体类型（User 等）
 *   - 通用工具类型（可选、可空等）
 *
 * 设计原则：
 *   - 所有 API 响应和请求数据均有明确类型，避免使用 any
 *   - 类型命名采用 PascalCase，字段命名与后端 JSON 字段保持一致（蛇形或驼峰按需对齐）
 *   - 分页类型独立封装，便于各列表页复用
 *
 * 与后端的字段对齐约定：
 *   - 后端返回字段为蛇形（如 real_name、created_at），前端类型定义与之保持一致
 *   - 若需要在组件中使用驼峰，可在转换层处理
 */

// ==================== 基础响应类型 ====================

/**
 * @interface BaseResponse
 * @brief 后端统一响应格式
 *
 * 与后端 ResponseUtil 返回的 JSON 结构完全一致：
 *   { code: 200, msg: "success", data: T }
 *
 * 注意：request.ts 中的响应拦截器已将 res.data 直接返回，
 * 因此在组件中调用 API 时拿到的就是这里的 data 字段对应的类型，
 * 而非整个 BaseResponse。
 *
 * @template T data 字段的具体类型
 */
export interface BaseResponse<T = unknown> {
  /** 业务状态码，200 表示成功 */
  code: number
  /** 响应消息，成功时通常为 "success"，失败时为错误描述 */
  msg: string
  /** 响应数据，泛型 T 表示具体类型 */
  data: T
}

/**
 * @interface PageResult
 * @brief 分页查询结果
 *
 * 对应后端 ResponseUtil::successPage 返回的 data 结构：
 *   { list: T[], total: number, page: number, pageSize: number }
 *
 * 所有列表分页接口的返回数据统一使用此类型。
 *
 * @template T 列表项类型
 */
export interface PageResult<T> {
  /** 数据列表 */
  list: T[]
  /** 总记录数 */
  total: number
  /** 当前页码（从 1 开始） */
  page: number
  /** 每页条数 */
  pageSize: number
}

// ==================== 用户相关类型 ====================

/**
 * @type UserRole
 * @brief 用户角色类型（字面量联合类型）
 *
 * 与数据库 ENUM('admin', 'property', 'owner') 保持一致。
 * 使用字面量类型而非 string，可获得更好的类型提示和检查。
 */
export type UserRole = 'admin' | 'property' | 'owner'

/**
 * @interface User
 * @brief 用户信息类型
 *
 * 对应后端 models::User 结构体和数据库 users 表。
 * 字段命名与后端 JSON 响应保持一致（蛇形命名）。
 *
 * 注意：密码字段（password_hash）不会在接口响应中返回，
 * 此类型不包含密码信息。
 */
export interface User {
  /** 用户 ID（主键） */
  id: number
  /** 登录用户名 */
  username: string
  /** 真实姓名 */
  real_name: string
  /** 手机号 */
  phone: string
  /** 角色：管理员 / 物业 / 业主 */
  role: UserRole
  /** 状态：1-正常，0-禁用 */
  status: number
  /** 创建时间（ISO 格式字符串） */
  created_at: string
}

/**
 * @interface UserForm
 * @brief 用户表单数据类型（新增/编辑用户时的表单字段）
 *
 * 与 User 的区别：
 *   - 新增时没有 id
 *   - 包含 password 字段（明文，仅提交时使用）
 *   - 编辑时密码可为空（表示不修改密码）
 *
 * 用于 Dialog 弹窗中的表单绑定。
 */
export interface UserForm {
  /** 用户 ID（编辑时有值，新增时为 undefined） */
  id?: number
  /** 登录用户名 */
  username: string
  /** 密码（明文，仅创建或修改密码时填写） */
  password: string
  /** 真实姓名 */
  real_name: string
  /** 手机号 */
  phone: string
  /** 用户角色 */
  role: UserRole
  /** 状态：1-正常，0-禁用 */
  status: number
}

/**
 * @interface UserQueryParams
 * @brief 用户列表查询参数
 *
 * 对应 GET /api/users 的 query 参数。
 * 用于搜索框、分页等查询条件的类型化封装。
 */
export interface UserQueryParams {
  /** 页码（从 1 开始） */
  page: number
  /** 每页条数 */
  pageSize: number
  /** 用户名搜索（可选，模糊匹配） */
  username?: string
  /** 角色筛选（可选） */
  role?: UserRole | string
  /** 状态筛选（可选） */
  status?: number
}

// ==================== 登录相关类型 ====================

/**
 * @interface LoginForm
 * @brief 登录表单数据
 */
export interface LoginForm {
  /** 用户名 */
  username: string
  /** 密码 */
  password: string
}

/**
 * @interface LoginResult
 * @brief 登录成功后返回的数据
 *
 * 对应 POST /api/login 返回的 data 部分。
 * 包含用户基本信息和 JWT Token。
 */
export interface LoginResult {
  /** JWT Token（Bearer Token，存 localStorage） */
  token: string
  /** 用户 ID */
  id: number
  /** 用户名 */
  username: string
  /** 真实姓名 */
  real_name: string
  /** 用户角色 */
  role: UserRole
}

// ==================== 用户信息 store 类型 ====================

/**
 * @interface UserState
 * @brief Pinia 用户状态
 *
 * 存储当前登录用户的基本信息。
 * 初始化时从 localStorage 读取，支持持久化。
 */
export interface UserState {
  /** JWT Token */
  token: string
  /** 用户 ID */
  userId: number
  /** 用户名 */
  username: string
  /** 真实姓名 */
  realName: string
  /** 用户角色 */
  role: UserRole | string
}

// ==================== 房屋管理相关类型 ====================

/**
 * @interface House
 * @brief 房屋管理信息类型
 *
 * 对应后端 models::House 结构体和数据库 houses 表。
 */
export interface House {
  /** ID */
  id: number
  /** 楼栋号 */
  building_no: string
  /** 单元号 */
  unit_no: string
  /** 门牌号 */
  room_no: string
  /** 面积 */
  area: number
  /** 业主ID */
  owner_id: number
}

/**
 * @interface HouseForm
 * @brief 房屋管理表单数据类型（新增/编辑时的表单字段）
 */
export interface HouseForm {
  /** ID（编辑时有值，新增时为 undefined） */
  id?: number
  /** 楼栋号 */
  building_no: string
  /** 单元号 */
  unit_no: string
  /** 门牌号 */
  room_no: string
  /** 面积(平米) */
  area: number
  /** 业主ID */
  owner_id: number
}

/**
 * @interface HouseQueryParams
 * @brief 房屋管理列表查询参数
 */
export interface HouseQueryParams {
  /** 页码（从 1 开始） */
  page: number
  /** 每页条数 */
  pageSize: number
  /** 楼栋号（可选） */
  building_no?: string
  /** 门牌号（可选） */
  room_no?: string
}

// ==================== 车辆管理相关类型 ====================

/**
 * @interface Vehicle
 * @brief 车辆管理信息类型
 *
 * 对应后端 models::Vehicle 结构体和数据库 vehicles 表。
 */
export interface Vehicle {
  /** ID */
  id: number
  /** 车主ID */
  user_id: number
  /** 车牌号 */
  plate_number: string
  /** 品牌 */
  brand: string
  /** 颜色 */
  color: string
  /** 状态 */
  status: number
  /** 创建时间 */
  created_at: string
}

/**
 * @interface VehicleForm
 * @brief 车辆管理表单数据类型（新增/编辑时的表单字段）
 */
export interface VehicleForm {
  /** ID（编辑时有值，新增时为 undefined） */
  id?: number
  /** 车主ID */
  user_id: number
  /** 车牌号 */
  plate_number: string
  /** 品牌 */
  brand: string
  /** 颜色 */
  color: string
  /** 状态 */
  status: string
}

/**
 * @interface VehicleQueryParams
 * @brief 车辆管理列表查询参数
 */
export interface VehicleQueryParams {
  /** 页码（从 1 开始） */
  page: number
  /** 每页条数 */
  pageSize: number
  /** 车牌号（可选） */
  plate_number?: string
  /** 车主ID（可选） */
  user_id?: string
}

// ==================== 车位管理相关类型 ====================

/**
 * @interface ParkingSpot
 * @brief 车位管理信息类型
 *
 * 对应后端 models::ParkingSpot 结构体和数据库 parkingspots 表。
 */
export interface ParkingSpot {
  /** ID */
  id: number
  /** 车位编号 */
  spot_number: string
  /** 区域 */
  zone: string
  /** 类型 */
  type: string
  /** 当前车辆ID */
  current_vehicle_id: number
  /** 是否占用 */
  is_occupied: number
  /** 更新时间 */
  updated_at: string
}

/**
 * @interface ParkingSpotForm
 * @brief 车位管理表单数据类型（新增/编辑时的表单字段）
 */
export interface ParkingSpotForm {
  /** ID（编辑时有值，新增时为 undefined） */
  id?: number
  /** 车位编号 */
  spot_number: string
  /** 区域 */
  zone: string
  /** 类型 */
  type: string
  /** 当前车辆ID */
  current_vehicle_id: number
  /** 是否占用 */
  is_occupied: string
}

/**
 * @interface ParkingSpotQueryParams
 * @brief 车位管理列表查询参数
 */
export interface ParkingSpotQueryParams {
  /** 页码（从 1 开始） */
  page: number
  /** 每页条数 */
  pageSize: number
  /** 车位编号（可选） */
  spot_number?: string
  /** 区域（可选） */
  zone?: string
  /** 类型（可选） */
  type?: string
  /** 是否占用（可选） */
  is_occupied?: string
}

// ==================== 论坛帖子相关类型 ====================

/**
 * @interface ForumPost
 * @brief 论坛帖子信息类型
 *
 * 对应后端 models::ForumPost 结构体和数据库 forumposts 表。
 */
export interface ForumPost {
  /** ID */
  id: number
  /** 发帖人ID */
  user_id: number
  /** 标题 */
  title: string
  /** 正文 */
  content: string
  /** 分类 */
  category: string
  /** 标签 */
  tags: string
  /** 附件 */
  attachments: string
  /** 状态 */
  status: number
  /** 创建时间 */
  created_at: string
}

/**
 * @interface ForumPostForm
 * @brief 论坛帖子表单数据类型（新增/编辑时的表单字段）
 */
export interface ForumPostForm {
  /** ID（编辑时有值，新增时为 undefined） */
  id?: number
  /** 标题 */
  title: string
  /** 发帖人ID */
  user_id: number
  /** 分类 */
  category: string
  /** 标签(逗号分隔) */
  tags: string
  /** 正文 */
  content: string
  /** 状态 */
  status: string
}

/**
 * @interface ForumPostQueryParams
 * @brief 论坛帖子列表查询参数
 */
export interface ForumPostQueryParams {
  /** 页码（从 1 开始） */
  page: number
  /** 每页条数 */
  pageSize: number
  /** 标题（可选） */
  title?: string
  /** 分类（可选） */
  category?: string
}

// ==================== 二手交易相关类型 ====================

/**
 * @interface MarketplaceItem
 * @brief 二手交易信息类型
 *
 * 对应后端 models::MarketplaceItem 结构体和数据库 marketplaceitems 表。
 */
export interface MarketplaceItem {
  /** ID */
  id: number
  /** 发布者ID */
  seller_id: number
  /** 物品标题 */
  title: string
  /** 物品描述 */
  description: string
  /** 交易方式 */
  exchange_type: string
  /** 售价 */
  price: number
  /** 期望交换物品 */
  expect_item: string
  /** 状态 */
  status: string
  /** 创建时间 */
  created_at: string
}

/**
 * @interface MarketplaceItemForm
 * @brief 二手交易表单数据类型（新增/编辑时的表单字段）
 */
export interface MarketplaceItemForm {
  /** ID（编辑时有值，新增时为 undefined） */
  id?: number
  /** 物品标题 */
  title: string
  /** 发布者ID */
  seller_id: number
  /** 交易方式 */
  exchange_type: string
  /** 售价 */
  price: number
  /** 期望交换物品 */
  expect_item: string
  /** 物品描述 */
  description: string
  /** 状态 */
  status: string
}

/**
 * @interface MarketplaceItemQueryParams
 * @brief 二手交易列表查询参数
 */
export interface MarketplaceItemQueryParams {
  /** 页码（从 1 开始） */
  page: number
  /** 每页条数 */
  pageSize: number
  /** 标题（可选） */
  title?: string
  /** 交易方式（可选） */
  exchange_type?: string
  /** 状态（可选） */
  status?: string
}

// ==================== 物业账单相关类型 ====================

/**
 * @interface PropertyFee
 * @brief 物业账单信息类型
 *
 * 对应后端 models::PropertyFee 结构体和数据库 propertyfees 表。
 */
export interface PropertyFee {
  /** ID */
  id: number
  /** 房屋ID */
  house_id: number
  /** 费用类型 */
  fee_type: string
  /** 应缴金额 */
  amount: number
  /** 账单月份 */
  bill_month: string
  /** 状态 */
  status: string
  /** 支付方式 */
  payment_method: string
  /** 支付时间 */
  paid_at: string
  /** 创建时间 */
  created_at: string
}

/**
 * @interface PropertyFeeForm
 * @brief 物业账单表单数据类型（新增/编辑时的表单字段）
 */
export interface PropertyFeeForm {
  /** ID（编辑时有值，新增时为 undefined） */
  id?: number
  /** 房屋ID */
  house_id: number
  /** 费用类型 */
  fee_type: string
  /** 应缴金额 */
  amount: number
  /** 账单月份 */
  bill_month: string
  /** 状态 */
  status: string
  /** 支付方式 */
  payment_method: string
}

/**
 * @interface PropertyFeeQueryParams
 * @brief 物业账单列表查询参数
 */
export interface PropertyFeeQueryParams {
  /** 页码（从 1 开始） */
  page: number
  /** 每页条数 */
  pageSize: number
  /** 费用类型（可选） */
  fee_type?: string
  /** 状态（可选） */
  status?: string
  /** 账单月份（可选） */
  bill_month?: string
}

// ==================== 报修投诉相关类型 ====================

/**
 * @interface ServiceRequest
 * @brief 报修投诉信息类型
 *
 * 对应后端 models::ServiceRequest 结构体和数据库 servicerequests 表。
 */
export interface ServiceRequest {
  /** ID */
  id: number
  /** 发起人ID */
  user_id: number
  /** 类型 */
  type: string
  /** 标题 */
  title: string
  /** 详细描述 */
  description: string
  /** 状态 */
  status: string
  /** 创建时间 */
  created_at: string
}

/**
 * @interface ServiceRequestForm
 * @brief 报修投诉表单数据类型（新增/编辑时的表单字段）
 */
export interface ServiceRequestForm {
  /** ID（编辑时有值，新增时为 undefined） */
  id?: number
  /** 标题 */
  title: string
  /** 发起人ID */
  user_id: number
  /** 类型 */
  type: string
  /** 详细描述 */
  description: string
  /** 状态 */
  status: string
}

/**
 * @interface ServiceRequestQueryParams
 * @brief 报修投诉列表查询参数
 */
export interface ServiceRequestQueryParams {
  /** 页码（从 1 开始） */
  page: number
  /** 每页条数 */
  pageSize: number
  /** 类型（可选） */
  type?: string
  /** 状态（可选） */
  status?: string
}
