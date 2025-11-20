cmake_minimum_required(VERSION 3.16.0)

project(BondForge VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 启用Qt的MOC
set(CMAKE_AUTOMOC ON)

# 创建可执行文件
add_executable(BondForge bondforge.cpp)

# 链接Qt库
target_link_libraries(BondForge Qt6::Core Qt6::Widgets)

# 设置Windows子系统
if(WIN32)
    set_target_properties(BondForge PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 安装规则
install(TARGETS BondForge
    RUNTIME DESTINATION bin
)