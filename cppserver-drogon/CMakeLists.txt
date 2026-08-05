cmake_minimum_required(VERSION 3.16)
project(CommunitySystem CXX)

# ==================== C++20 标准 ====================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

# ==================== 查找 Drogon 框架 ====================
find_package(Drogon CONFIG REQUIRED)
message(STATUS "Drogon found: ${Drogon_INCLUDE_DIRS}")

# ==================== 查找 MySQL 客户端库 ====================
# Drogon 1.9.13 编译时未启用 MySQL ORM，因此使用原生 MySQL C API
find_path(MYSQL_INCLUDE_DIR mysql.h
    PATHS /usr/include/mysql /usr/local/include/mysql
)
find_library(MYSQL_LIBRARY
    NAMES mysqlclient mysqlclient_r
    PATHS /usr/lib/x86_64-linux-gnu /usr/local/lib /usr/lib/mysql
)

if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL client library not found! Install libmysqlclient-dev.")
endif()
message(STATUS "MySQL include: ${MYSQL_INCLUDE_DIR}")
message(STATUS "MySQL library: ${MYSQL_LIBRARY}")

# ==================== 查找 OpenSSL ====================
# 用于 JWT 签名（HMAC-SHA256）和密码加密（SHA256 + salt）
find_package(OpenSSL REQUIRED)
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")

# ==================== 收集源文件 ====================
# 控制器源文件
file(GLOB_RECURSE CTL_SRC controllers/*.cc)
# 过滤器源文件
file(GLOB_RECURSE FILTER_SRC filters/*.cc)
# 服务层源文件
file(GLOB_RECURSE SVC_SRC services/*.cc)
# 公共工具源文件
file(GLOB_RECURSE COMMON_SRC common/*.cc)
# 主入口
set(MAIN_SRC main.cc)

# ==================== 创建可执行文件 ====================
add_executable(${PROJECT_NAME}
    ${MAIN_SRC}
    ${CTL_SRC}
    ${FILTER_SRC}
    ${SVC_SRC}
    ${COMMON_SRC}
)

# ==================== 包含目录 ====================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${MYSQL_INCLUDE_DIR}
)

# ==================== 链接库 ====================
target_link_libraries(${PROJECT_NAME} PRIVATE
    Drogon::Drogon
    ${MYSQL_LIBRARY}
    OpenSSL::SSL
    OpenSSL::Crypto
)

# ==================== Drogon 视图处理 ====================
drogon_create_views(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/views
    ${CMAKE_CURRENT_BINARY_DIR})

# ==================== 编译选项 ====================
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Project: CommunitySystem (Drogon Edition)")
