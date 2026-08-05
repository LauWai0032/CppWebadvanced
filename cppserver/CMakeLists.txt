# ========================================
# CMakeLists.txt
# 小区管理系统 C++ 后端 - 构建配置
# ========================================
#
# 功能：
#   - C++20 标准
#   - 支持 Debug / Release 两种构建模式
#   - 集成 MySQL 客户端库
#   - 自动收集 src/ 下所有 .cpp 源文件
#   - 头文件搜索路径：include/ + 项目根目录（httplib.h, json.hpp）
#
# 使用方式：
#   mkdir -p build && cd build
#   cmake .. -DCMAKE_BUILD_TYPE=Release
#   make -j$(nproc)
#
# @date 2025
# ========================================

cmake_minimum_required(VERSION 3.14)

# 项目信息
project(cppserver
    VERSION 1.0.0
    DESCRIPTION "Community Management System - C++ Backend"
    LANGUAGES CXX
)

# ========================================
# C++ 标准设置
# ========================================
set(CMAKE_CXX_STANDARD 20)              # 使用 C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # 强制要求 C++20
set(CMAKE_CXX_EXTENSIONS OFF)           # 不使用编译器扩展，保持标准 C++

# ========================================
# 构建类型（默认 Release）
# ========================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ========================================
# 编译选项
# ========================================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # 通用警告选项
    add_compile_options(
        -Wall                   # 开启大部分警告
        -Wextra                 # 额外警告
        -Wpedantic              # 严格遵循标准
        -Wno-unused-parameter   # 忽略未使用参数警告（接口设计中常见）
        -Wno-unused-function    # 忽略未使用函数警告
    )

    # Debug 模式：调试信息 + 无优化
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

    # Release 模式：优化 + 去除调试信息
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

    # 可选：开启 AddressSanitizer（调试内存问题时使用）
    # set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")

elseif(MSVC)
    add_compile_options(/W4)
endif()

# ========================================
# 线程库（pthread）
# ========================================
find_package(Threads REQUIRED)

# ========================================
# MySQL 客户端库
# ========================================
# 尝试通过 pkg-config 查找 MySQL
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(MYSQL QUIET mysqlclient)
endif()

# 如果 pkg-config 没找到，尝试手动查找
if(NOT MYSQL_FOUND)
    find_path(MYSQL_INCLUDE_DIR
        NAMES mysql/mysql.h
        PATHS
            /usr/include
            /usr/local/include
            /usr/include/mysql
        DOC "MySQL include directory"
    )

    find_library(MYSQL_LIBRARY
        NAMES mysqlclient mysqlclient_r
        PATHS
            /usr/lib
            /usr/lib64
            /usr/local/lib
            /usr/local/lib64
        DOC "MySQL client library"
    )

    if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
        set(MYSQL_FOUND TRUE)
        set(MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR})
        set(MYSQL_LIBRARIES ${MYSQL_LIBRARY})
    endif()
endif()

# 处理 MySQL 查找结果
if(MYSQL_FOUND)
    message(STATUS "MySQL client library found: ${MYSQL_LIBRARIES}")
    message(STATUS "MySQL include dir: ${MYSQL_INCLUDE_DIRS}")
    add_compile_definitions(HAS_MYSQL=1)
else()
    message(WARNING "MySQL client library NOT found!")
    message(WARNING "  ConnectionPool module will fail to compile.")
    message(WARNING "  Install with: sudo apt install libmysqlclient-dev")
    message(WARNING "  Continuing anyway (cmake will succeed, build may fail)...")
    # 仍然设置空变量，让 CMake 配置通过
    set(MYSQL_INCLUDE_DIRS "")
    set(MYSQL_LIBRARIES "")
endif()

# ========================================
# 源文件收集
# ========================================
# 自动收集 src/ 下所有 .cpp 文件
file(GLOB_RECURSE CPP_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# 主入口文件
set(MAIN_SOURCE "${CMAKE_SOURCE_DIR}/main.cpp")

# ========================================
# 可执行目标
# ========================================
add_executable(cppserver
    ${MAIN_SOURCE}
    ${CPP_SOURCES}
)

# ========================================
# 头文件搜索路径
# ========================================
target_include_directories(cppserver
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include           # 项目头文件
        ${CMAKE_SOURCE_DIR}                   # 根目录（httplib.h, json.hpp）
        ${MYSQL_INCLUDE_DIRS}                 # MySQL 头文件
)

# ========================================
# 链接库
# ========================================
target_link_libraries(cppserver
    PRIVATE
        Threads::Threads                      # 线程库
        ${MYSQL_LIBRARIES}                    # MySQL 客户端库
        # 如果需要其他库，在这里添加
)

# ========================================
# 输出目录设置
# ========================================
set_target_properties(cppserver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ========================================
# 复制配置文件到输出目录（可选）
# ========================================
add_custom_command(TARGET cppserver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/config"
        "$<TARGET_FILE_DIR:cppserver>/config"
    COMMENT "Copying config files to output directory"
)

# ========================================
# 安装规则（可选）
# ========================================
install(TARGETS cppserver
    RUNTIME DESTINATION bin
)

install(DIRECTORY config/
    DESTINATION etc/cppserver
    OPTIONAL
)

# ========================================
# 构建摘要
# ========================================
message(STATUS "")
message(STATUS "==========================================")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  MySQL: ${MYSQL_FOUND}")
message(STATUS "==========================================")
message(STATUS "")
