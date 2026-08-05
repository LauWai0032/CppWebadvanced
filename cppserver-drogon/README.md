# CommunitySystem - Drogon 版

基于 **Drogon 框架** 的社区管理系统后端，作为 `cppserver`（cpp-httplib 版）的对比参考项目。

## 📋 项目简介

本项目使用 Drogon 高性能 Web 框架重新实现了社区管理系统的后端服务，保持与 `cppserver` 版本完全一致的 API 接口和业务逻辑，但在架构层面充分利用了 Drogon 框架的能力。

### 技术栈

| 组件       | 版本/说明                    |
|-----------|-----------------------------|
| 框架       | Drogon 1.9.13              |
| C++ 标准   | C++20                       |
| 数据库     | MySQL 8.0（原生 C API）      |
| JSON 处理  | JsonCpp（Drogon 内置）        |
| JWT 认证   | 自定义实现（OpenSSL HMAC-SHA256） |
| 密码加密   | SHA256 + 随机盐（OpenSSL）   |
| 构建工具   | CMake 3.16+                 |
| 编译器     | g++ 11.4.0                  |

## 🔄 与 cppserver（cpp-httplib 版）的架构对比

| 对比维度         | cppserver (cpp-httplib)           | cppserver-drogon (Drogon)              |
|-----------------|----------------------------------|----------------------------------------|
| **HTTP 框架**   | cpp-httplib（header-only 单文件）  | Drogon（编译安装，完整框架）              |
| **路由注册**     | 手动 lambda 绑定                   | 宏声明式注册（PATH_ADD）                 |
| **数据库访问**   | 手写连接池 + MySQL C API            | 手写连接池 + MySQL C API *               |
| **JSON 处理**   | nlohmann/json（header-only）       | JsonCpp（Drogon 内置）                   |
| **日志系统**     | spdlog（header-only）              | Trantor（Drogon 内置）                   |
| **中间件/过滤器** | 手动实现（lambda 包装）             | HttpFilter 基类（框架原生支持）           |
| **配置管理**     | 自行解析 JSON 配置                  | 内置配置加载（config.json）               |
| **异步模型**     | 同步阻塞                           | 异步回调 + 多 IO 线程                    |
| **Controller**  | 手动继承基类，手动注册路由            | 继承 HttpController，宏自动注册           |
| **代码量**       | 较多（需手写大量基础代码）           | 较少（框架提供大量基础功能）               |
| **性能**         | 良好（单线程/线程池）               | 优秀（epoll + 多 IO 线程）               |
| **学习曲线**     | 低（header-only，概念简单）          | 中等（框架概念较多，功能丰富）             |
| **部署复杂度**   | 低（无需额外安装框架）               | 中（需预装 Drogon）                      |

> *注：Drogon 1.9.13 编译时未启用 MySQL ORM，因此本版本使用原生 MySQL C API。

## ✅ Drogon 版的优势

### 1. 声明式路由注册
```cpp
// Drogon 版：清晰的宏声明
ADD_METHOD_TO(UserController::getUserList, "/api/users", drogon::Get);

// cpp-httplib 版：手动绑定 lambda
svr.Get("/api/user/list", [this](const auto& req, auto& res) { ... });
```

### 2. 内置高性能异步 I/O
- 基于 epoll 的多路复用
- 多 IO 线程并行处理
- 连接池自动管理

### 3. 原生中间件/过滤器支持
```cpp
class AuthFilter : public drogon::HttpFilter<AuthFilter> {
    void doFilter(const HttpRequestPtr& req, ...) override;
};
```

### 4. 统一配置管理
- 一个 `config.json` 管理所有配置
- 支持数据库连接、日志、会话等
- 热加载支持

### 5. 生产级特性
- 内置 HTTPS 支持
- 内置 Session 管理
- 内置静态文件服务
- 内置 WebSocket 支持
- 支持 gzip/brotli 压缩

## 🏗️ 目录结构

```
cppserver-drogon/
├── CMakeLists.txt              # 构建配置
├── config.json                 # Drogon 配置（端口、数据库等）
├── main.cc                     # 程序入口
├── controllers/
│   └── UserController.h/.cc    # 用户控制器（Drogon HttpController）
├── services/
│   ├── IUserService.h          # 用户服务接口
│   └── UserService.h/.cc       # 用户服务实现（MySQL）
├── models/
│   └── User.h                  # 用户模型
├── common/
│   ├── ResponseUtil.h/.cc      # 统一 JSON 响应封装
│   ├── Logger.h/.cc            # 日志工具（Trantor 封装）
│   ├── DbPool.h/.cc            # MySQL 连接池
│   ├── JwtUtil.h/.cc           # JWT 工具（HMAC-SHA256 + OpenSSL）
│   └── PasswordUtil.h/.cc      # 密码加密工具（SHA256 + salt）
├── filters/
│   └── AuthFilter.h/.cc        # 认证过滤器（JWT 验证中间件）
├── views/                      # 视图模板（暂无）
├── plugins/                    # 插件目录（暂无）
├── test/                       # 测试目录
│   ├── CMakeLists.txt
│   └── test_main.cc
└── README.md                   # 本文档
```

## 🔨 构建与运行

### 前置条件
- Drogon 1.9.13 已安装（/usr/local/）
- MySQL 客户端开发库（libmysqlclient-dev）
- OpenSSL 开发库（libssl-dev）
- CMake 3.16+
- g++ 11+（支持 C++20）

### 构建步骤

```bash
cd cppserver-drogon
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 运行

```bash
# 确保 config.json 在运行目录下
cd cppserver-drogon
./build/CommunitySystem config.json
```

### 服务端口
- HTTP: 8080（与 cppserver 版本保持一致）

## 📡 API 接口

### 统一响应格式
```json
{
    "code": 200,
    "msg": "success",
    "data": {}
}
```

> **注意**：响应字段名为 `msg`（与前端 axios 封装保持一致），不是 `message`。

### 接口清单

#### 公开接口（无需认证）

| 方法 | 路径        | 说明   | 请求体示例                           | 响应 data 示例                                   |
|------|-------------|--------|------------------------------------|------------------------------------------------|
| POST | /api/login  | 用户登录 | `{"username":"admin","password":"123456"}` | `{"token":"eyJhbGciOi...","id":1,"username":"admin","real_name":"管理员","role":"admin"}` |

#### 需要认证的接口（需携带 JWT Token）

所有 `/api/users/*` 接口均需在请求头中携带 Bearer Token：
```
Authorization: Bearer <token>
```

| 方法   | 路径              | 说明         | 查询参数                              | 响应 data 示例                                                                 |
|--------|-------------------|-------------|-------------------------------------|------------------------------------------------------------------------------|
| GET    | /api/users        | 用户列表（分页） | `page=1&pageSize=10&username=xxx&role=admin&status=1` | `{"list": [...], "total": 100, "page": 1, "pageSize": 10}`                    |
| GET    | /api/users/{id}   | 用户详情      | 无                                   | `{"id":1,"username":"admin","real_name":"管理员","phone":"","role":"admin","status":1,"created_at":"2025-01-01 00:00:00"}` |
| POST   | /api/users        | 创建用户      | 请求体 JSON（见下方）                      | `{"id": 10}`                                                                  |
| PUT    | /api/users/{id}   | 更新用户      | 请求体 JSON（见下方）                      | `{}`（code=200, msg="更新成功"）                                                |
| DELETE | /api/users/{id}   | 删除用户      | 无                                   | `{}`（code=200, msg="删除成功"）                                                |

#### 创建/更新用户请求体格式
```json
{
    "username": "testuser",
    "password": "123456",
    "real_name": "测试用户",
    "phone": "13800138000",
    "role": "owner",
    "status": 1
}
```
- `password` 字段：创建时必填，更新时可空（表示不修改密码）
- `role` 可选值：`admin`（管理员）、`property`（物业）、`owner`（业主）
- `status`：1 正常，0 禁用

### 与前端的对接方式

前端使用 axios 发送请求，request.ts 拦截器自动处理：
1. **请求拦截器**：自动从 localStorage 读取 Token，添加到 `Authorization: Bearer <token>` 头
2. **响应拦截器**：判断 code==200 则返回 data；否则弹出错误提示
3. **401 处理**：Token 过期或无效时，自动清除登录状态并跳转登录页

前端 API 调用示例（已剥离外层包装）：
```typescript
import { getUserList } from '@/api/user'
const data = await getUserList({ page: 1, pageSize: 10 })
// data 类型为 PageResult<User>，直接是 data 部分
```

## 🔐 JWT 认证说明

### 认证流程

```
客户端                                 服务器
  |                                     |
  | 1. POST /api/login (用户名+密码)    |
  |------------------------------------>|
  |                                     | 2. 查询用户，验证密码（SHA256+salt）
  |                                     | 3. 生成 JWT Token（HMAC-SHA256）
  | 4. 返回 { token, 用户信息 }          |
  |<------------------------------------|
  |                                     |
  | 5. GET /api/users (带 Token)        |
  |------------------------------------>|
  |                                     | 6. AuthFilter 提取并验证 Token
  |                                     | 7. 验证通过，写入 request attributes
  |                                     | 8. Controller 处理业务
  | 9. 返回业务数据                      |
  |<------------------------------------|
```

### JWT 载荷结构

Token 的 Payload 包含以下声明：

| 字段       | 类型   | 说明                        |
|-----------|--------|---------------------------|
| `sub`     | number | 主题 = 用户 ID               |
| `username`| string | 用户名                      |
| `role`    | string | 用户角色（admin/property/owner） |
| `iat`     | number | 签发时间（Unix 时间戳）        |
| `exp`     | number | 过期时间（Unix 时间戳）        |

### Token 默认配置

- **签名算法**：HS256（HMAC-SHA256，基于 OpenSSL 实现）
- **默认过期时间**：24 小时（86400 秒）
- **密钥**：默认密钥（生产环境请务必修改，通过 `JwtUtil::setSecret()` 设置）

### AuthFilter 工作方式

1. 从 `Authorization` 请求头提取 Bearer Token
2. 调用 `JwtUtil::verifyToken()` 验证签名和过期
3. 验证通过：将 `userId`、`username`、`role` 写入 request attributes
4. 验证失败：返回 401 JSON 响应，拦截请求

Controller 中使用用户信息：
```cpp
auto attrs = req->getAttributes();
int userId = attrs->get<int>("userId");
std::string username = attrs->get<std::string>("username");
std::string role = attrs->get<std::string>("role");
```

### 密码存储说明

- 加密方式：SHA256 + 随机盐值（16 字节）
- 存储格式：`salt$hash`（盐值 + `$` + SHA256(salt+password) 的十六进制表示）
- 实现：`common/PasswordUtil`（基于 OpenSSL）
- 注意：比明文存储安全，但生产环境建议升级为 bcrypt 或 argon2

## 📚 第二阶段进度

> **第二阶段目标**：用户管理模块端到端跑通，作为后续 7 个模块的可复制样板。

### ✅ 已完成

#### 后端（Drogon）
- [x] **统一响应字段**：`message` → `msg`，与前端保持一致
- [x] **密码加密**：SHA256 + 随机盐（PasswordUtil），替代明文比对
- [x] **JWT 工具类**：`common/JwtUtil.h/.cc`，HMAC-SHA256 签名
- [x] **认证过滤器**：`AuthFilter` 真正接入 JWT 验证
  - 从 Authorization 头提取 Bearer Token
  - 验证签名和过期时间
  - 验证通过将用户信息写入 request attributes
  - 验证失败返回 401
- [x] **用户模块 6 个接口 + 登录接口完整实现**
  - GET /api/users（分页列表，需认证）
  - GET /api/users/{id}（详情，需认证）
  - POST /api/login（登录，公开）
  - POST /api/users（创建，需认证）
  - PUT /api/users/{id}（更新，需认证）
  - DELETE /api/users/{id}（删除，需认证）
- [x] **登录返回真实 JWT Token**（替代 mock-token）
- [x] **CMakeLists.txt 更新**：新增 OpenSSL 链接
- [x] **编译验证**：零错误零警告通过 ✅

#### 前端（Vue 3 + TypeScript）
- [x] **Pinia 状态管理**：`stores/user.ts` + `stores/index.ts`
- [x] **Element Plus 图标库**：`@element-plus/icons-vue`
- [x] **API 层完善**：`api/user.ts`（5 个接口 + 完整类型定义）
- [x] **类型定义**：`types/index.ts`（User、PageResult、LoginForm 等）
- [x] **用户列表页**：`views/user/UserList.vue`
  - 搜索栏（用户名/角色/状态筛选）
  - 数据表格（Element Plus Table）
  - 分页器
  - 新增/编辑弹窗（Form 校验）
  - 删除确认（MessageBox）
- [x] **路由配置**：`/users` 路由 + 嵌套路由结构
- [x] **侧边栏菜单**：用户列表菜单项指向 /users
- [x] **登录页修复**：catch 块 res.msg 错误修正，改用 Pinia
- [x] **Axios 封装完善**：401 自动登出、错误统一提示

### 📋 后续模块参考

用户管理模块作为样板模块，后续 7 个模块（房屋、车位、车辆、论坛、二手交易、缴费、报修）均可按以下模式复制：

1. **后端文件结构**（每个模块 5 个文件）：
   ```
   controllers/XxxController.h/.cc   — 控制器（路由注册 + 参数校验）
   services/IXxxService.h            — 服务接口
   services/XxxService.h/.cc         — 服务实现（数据库操作）
   models/Xxx.h                      — 数据模型（toJson/fromDbRow）
   ```

2. **前端文件结构**（每个模块 2 个文件）：
   ```
   api/xxx.ts          — API 封装 + 类型
   views/xxx/XxxList.vue — 列表页（搜索 + 表格 + 分页 + CRUD 弹窗）
   ```

3. **认证策略**：所有业务接口均挂接 AuthFilter，需 JWT Token

4. **响应格式**：统一使用 ResponseUtil，code/msg/data 三段式

## 🧪 C++ 特性使用

本项目有意识地使用现代 C++ 特性：

| 特性            | C++ 版本 | 使用场景                        |
|----------------|---------|-------------------------------|
| `std::optional` | C++17   | 可能为空的返回值（如查询用户）     |
| `std::string_view` | C++17 | 避免字符串拷贝（参数传递）        |
| `auto` 类型推导  | C++14   | 简化类型声明                    |
| 结构化绑定       | C++17   | 简化多返回值处理                 |
| `[[nodiscard]]` | C++17   | 防止忽略返回值                   |
| 智能指针         | C++11   | 自动内存管理（shared_ptr 等）    |
| `enum class`    | C++11   | 强类型枚举                      |
| RAII            | C++11   | 连接自动归还（DbConnection）     |
| `= default/delete` | C++11 | 显式默认/禁止特殊成员函数        |
| 概念 concepts   | C++20   | 类型约束（预留）                 |

## 📐 设计模式

| 模式      | 实现位置                      | 说明                    |
|----------|------------------------------|------------------------|
| 单例模式  | `DbPool::instance()`         | 全局唯一数据库连接池     |
| 工厂模式  | Drogon HttpController 宏     | 框架自动实例化 Controller |
| 中间件模式 | `AuthFilter`                 | JWT 认证拦截            |
| 依赖注入  | `UserService::setInstance()`  | 服务层全局注入            |
| RAII     | `DbConnection`               | 数据库连接自动归还       |
| 接口隔离  | `IUserService`               | Controller 依赖抽象接口   |
| 静态工具类 | `ResponseUtil` / `JwtUtil`    | 无状态工具函数集合        |

## 📝 许可证

本项目为教学/对比参考用途。

## 📚 第三阶段进度

> **第三阶段目标**：批量扩展剩余 7 个业务模块，以 User 模块为样板，
> 把房屋、车辆、车位、论坛、二手交易、物业账单、报修投诉全部实现。

### ✅ 已完成

#### 后端（Drogon）— 7 个模块 × 5 文件 = 35 个文件
- [x] **房屋管理**（houses 表）
  - Model: `models/House.h`
  - Service: `services/IHouseService.h` + `services/HouseService.h/.cc`
  - Controller: `controllers/HouseController.h/.cc`
  - 接口：列表(分页+楼栋号/业主ID筛选) / 详情 / 新增 / 修改 / 删除
- [x] **车辆管理**（vehicles 表）
  - Model: `models/Vehicle.h`
  - Service: `services/IVehicleService.h` + `services/VehicleService.h/.cc`
  - Controller: `controllers/VehicleController.h/.cc`
  - 接口：列表(分页+车牌号/车主ID筛选) / 详情 / 新增 / 修改 / 删除
- [x] **车位管理**（parking_spots 表）
  - Model: `models/ParkingSpot.h`
  - Service: `services/IParkingSpotService.h` + `services/ParkingSpotService.h/.cc`
  - Controller: `controllers/ParkingSpotController.h/.cc`
  - 接口：列表(分页+区域/类型/占用筛选) / 详情 / 新增 / 修改 / 删除
  - 额外接口：车辆入场 PUT /parking-spots/{id}/park、车辆离场 PUT /parking-spots/{id}/leave
- [x] **论坛帖子**（forum_posts 表）
  - Model: `models/ForumPost.h`（tags/attachments 用 std::string 存储 JSON）
  - Service: `services/IForumPostService.h` + `services/ForumPostService.h/.cc`
  - Controller: `controllers/ForumPostController.h/.cc`
  - 接口：列表(分页+分类/标题搜索) / 详情 / 发布 / 修改 / 删除
- [x] **二手交易**（marketplace_items 表）
  - Model: `models/MarketplaceItem.h`
  - Service: `services/IMarketplaceItemService.h` + `services/MarketplaceItemService.h/.cc`
  - Controller: `controllers/MarketplaceItemController.h/.cc`
  - 接口：列表(分页+交易方式/状态/标题筛选) / 详情 / 发布 / 修改 / 删除
- [x] **物业账单**（property_fees 表）
  - Model: `models/PropertyFee.h`
  - Service: `services/IPropertyFeeService.h` + `services/PropertyFeeService.h/.cc`
  - Controller: `controllers/PropertyFeeController.h/.cc`
  - 接口：列表(分页+费用类型/状态/月份/房屋ID筛选) / 详情 / 生成 / 修改 / 删除
  - 额外接口：缴费 PUT /property-fees/{id}/pay
- [x] **服务请求/报修投诉**（service_requests 表）
  - Model: `models/ServiceRequest.h`
  - Service: `services/IServiceRequestService.h` + `services/ServiceRequestService.h/.cc`
  - Controller: `controllers/ServiceRequestController.h/.cc`
  - 接口：列表(分页+类型/状态筛选) / 详情 / 提交 / 修改 / 删除
  - 额外接口：状态流转 PUT /service-requests/{id}/status
- [x] **main.cc 更新**：注册全部 8 个服务（User + 7 个新模块）
- [x] **编译验证**：零错误零警告通过 ✅（`cmake .. && make -j$(nproc)`）

#### 前端（Vue 3 + TypeScript）— 7 个模块
- [x] **API 层**：每个模块一个 `api/xxx.ts`，完整类型定义
  - `api/house.ts`、`api/vehicle.ts`、`api/parking.ts`
  - `api/forum.ts`、`api/marketplace.ts`
  - `api/propertyFee.ts`、`api/serviceRequest.ts`
- [x] **类型定义**：`types/index.ts` 追加 7 个模块的类型
  - House/Vehicle/ParkingSpot/ForumPost/MarketplaceItem/PropertyFee/ServiceRequest
  - 每个类型配套 Form 和 QueryParams 类型
- [x] **页面组件**：每个模块一个列表页（搜索 + 表格 + 分页 + CRUD 弹窗）
  - `views/house/HouseList.vue`
  - `views/vehicle/VehicleList.vue`
  - `views/parking/ParkingSpotList.vue`（含入场/离场按钮）
  - `views/forum/ForumPostList.vue`
  - `views/marketplace/MarketItemList.vue`
  - `views/property-fee/PropertyFeeList.vue`（含缴费按钮）
  - `views/service-request/ServiceRequestList.vue`（含状态流转按钮）
- [x] **路由配置**：`router/index.ts` 追加 7 个路由（嵌套在 Home 下）
  - /houses、/vehicles、/parking、/forum、/market、/bills、/repair
- [x] **菜单配置**：`Home.vue` 侧边栏启用所有菜单项，指向正确路由
- [x] **详细中文注释**：所有文件均有清晰的中文注释

#### 架构一致性
- [x] 命名空间统一：`community::models` / `community::services` / `community::controllers`
- [x] 单例模式：每个 Service 都有 `getInstance()` / `setInstance()`
- [x] AuthFilter：所有接口均挂接认证过滤器
- [x] 分页查询：统一模式（vector + total 引用传参）
- [x] 响应格式：统一 `{ code, msg, data }`，使用 ResponseUtil
- [x] 数据库操作：DbPool + 原生 MySQL C API + RAII 连接管理
- [x] 前端 Element Plus 风格统一，与 UserList 保持一致
