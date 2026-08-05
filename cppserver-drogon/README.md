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
│   └── DbPool.h/.cc            # MySQL 连接池
├── filters/
│   └── AuthFilter.h/.cc        # 认证过滤器（中间件）
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

所有接口保持与 cppserver 版本完全一致的格式：

| 方法    | 路径              | 说明         |
|--------|-------------------|-------------|
| GET    | /api/users        | 用户列表（分页） |
| GET    | /api/users/{id}   | 用户详情      |
| POST   | /api/login        | 用户登录      |
| POST   | /api/users        | 创建用户      |
| PUT    | /api/users/{id}   | 更新用户      |
| DELETE | /api/users/{id}   | 删除用户      |

### 响应格式
```json
{
    "code": 200,
    "message": "success",
    "data": {}
}
```

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
| 中间件模式 | `AuthFilter`                 | 请求拦截/日志/鉴权       |
| 依赖注入  | `UserService::setInstance()`  | 服务层全局注入            |
| RAII     | `DbConnection`               | 数据库连接自动归还       |
| 接口隔离  | `IUserService`               | Controller 依赖抽象接口   |

## 📝 许可证

本项目为教学/对比参考用途。
