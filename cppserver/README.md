# 小区管理系统 C++ 后端

基于 cpp-httplib + nlohmann/json + MySQL 的现代化 C++ 后端服务。

## 项目结构

```
cppserver/
├── CMakeLists.txt              # CMake 构建配置（C++20）
├── main.cpp                    # 程序入口
├── httplib.h                   # cpp-httplib 单头文件 HTTP 库
├── json.hpp                    # nlohmann/json 单头文件 JSON 库
├── config/
│   └── config.json             # 应用配置（数据库、端口、日志等）
├── include/
│   ├── common/                 # 公共基础设施
│   │   ├── App.hpp             # 应用启动类（装配器）
│   │   ├── BaseController.hpp  # Controller 基类
│   │   ├── BaseDAO.hpp         # DAO 模板基类（模板方法模式 + C++20 concepts）
│   │   ├── ConnectionPool.hpp  # MySQL 连接池（单例 + RAII）
│   │   ├── Database.hpp        # 兼容别名（已迁移到 ConnectionPool）
│   │   ├── Logger.hpp          # 日志封装（基于 spdlog 的单例）
│   │   └── Response.hpp        # 统一响应封装
│   ├── controllers/            # 控制器层
│   │   └── UserController.hpp  # 用户控制器
│   ├── models/                 # 数据模型
│   │   └── User.hpp            # 用户模型
│   ├── services/               # 业务服务层
│   │   ├── IUserService.hpp    # 用户服务接口
│   │   └── UserService.hpp     # 用户服务实现
│   └── spdlog/                 # spdlog 日志库（第三方）
└── src/
    ├── common/
    │   ├── App.cpp
    │   ├── ConnectionPool.cpp
    │   ├── Database.cpp
    │   └── Response.cpp
    ├── controllers/
    │   └── UserController.cpp
    └── services/
        └── UserService.cpp
```

## 架构分层

```
HTTP Request
    │
    ▼
┌─────────────────┐
│   Controller    │  请求处理、参数校验、响应封装
│  (BaseController)│
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│    Service      │  业务逻辑（IUserService 接口）
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│      DAO        │  数据访问（BaseDAO 模板基类）
│   (BaseDAO<T>)   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  ConnectionPool │  MySQL 连接池（单例 + RAII）
└─────────────────┘
```

## 设计模式 & C++ 特性

| 模块 | 设计模式 | C++ 特性 |
|------|---------|----------|
| ConnectionPool | 单例模式 + 对象池 + RAII | shared_ptr 自定义 deleter, mutex, condition_variable, optional, string_view |
| BaseDAO | 模板方法模式 | 模板元编程, C++20 concepts (HasId), std::optional |
| BaseController | 策略模式 + 模板方法 | std::optional, SFINAE, lambda, std::string_view |
| Response | 静态工厂方法 | enum class, nlohmann/json |
| Logger | 单例模式（Meyers'） | 可变参数模板, 完美转发 |
| App | 门面模式 + 依赖注入 | unique_ptr, vector, 可变参数模板 |
| UserService | 接口隔离 + 依赖倒置 | 虚函数, override, std::optional |

## 构建

```bash
# 安装依赖
sudo apt install cmake g++ libmysqlclient-dev

# 构建
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 运行
./bin/cppserver
```

## 配置文件

修改 `config/config.json` 调整数据库连接、服务端口、日志级别等。

## 开发说明

- 当前 UserService 使用内存模拟数据，待 DAO 层完善后可替换为真实数据库操作
- 新增 Controller：继承 BaseController，实现 registerRoutes，在 App 中注册
- 新增 DAO：继承 BaseDAO<Model>，实现 getTableName() 和 rowToModel()
- 所有代码使用 UTF-8 编码，中文注释说明设计思路
