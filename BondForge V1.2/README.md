# BondForge V1.1 - 化学机器学习系统

## 概述
BondForge V1.1 是一个化学机器学习系统，支持数据管理、上传和分析。系统提供命令行界面和图形用户界面，并支持中英文双语切换。

## 功能特性
- **数据管理**: 上传、编辑、删除化学数据
- **多格式支持**: CSV、JSON、SDF等格式
- **权限管理**: 基于角色的访问控制
- **国际化**: 支持中文和英文界面切换
- **图形界面**: 直观的Qt图形用户界面
- **命令行工具**: 适用于自动化脚本
- **单文件应用**: 所有功能集成在一个源文件中

## 系统要求

### 开发环境
- Qt 6.0 或更高版本
- CMake 3.16.0 或更高版本
- C++17 兼容的编译器
- Make 或 Ninja (可选)

### 运行环境
- Windows 10/11, macOS 10.15+, 或 Linux
- Qt 6.0 运行时库

## 构建和运行

### Windows 用户
1. 确保已安装Qt并添加到PATH环境变量
2. 运行 `build.bat` 构建并启动GUI应用
3. 或运行 `build.bat cli` 构建命令行版本

### Linux/macOS 用户
1. 安装依赖：`sudo apt-get install qt6-base-dev cmake build-essential`
2. 运行 `./build.sh` 构建并启动GUI应用
3. 或运行 `./build.sh cli` 构建命令行版本

### 使用 CMake 手动构建
```bash
mkdir build && cd build
cmake ..
cmake --build .
./bin/BondForge --gui  # GUI版本
./bin/BondForge --cli  # 命令行版本
```

## 界面使用说明

### 语言切换
- 在界面顶部的语言下拉框中选择"中文"或"English"
- 或通过菜单栏的"语言"菜单选择
- 界面将立即切换到选择的语言，无需重启

### 数据上传
1. 切换到"数据上传"标签页
2. 填写表单中的必填字段 (ID, Category, Content)
3. 选择数据格式 (CSV, JSON, SDF)
4. 添加标签 (多个标签用逗号分隔)
5. 点击"Submit"按钮上传数据

### 数据管理
1. 在"数据管理"标签页查看所有数据
2. 点击表格行选择数据
3. 使用按钮执行删除或编辑操作
4. 点击"Refresh"按钮刷新数据列表

## 命令行使用

```bash
# 启动GUI界面
./BondForge --gui

# 启动命令行界面
./BondForge --cli

# 显示帮助信息
./BondForge --help
```

## 项目结构

```
BondForge V1.1/
├── bondforge.cpp          # 单文件应用（包含所有功能）
├── CMakeLists.txt         # CMake构建文件
├── Makefile              # Make构建文件
├── build.bat             # Windows构建脚本
├── build.sh             # Linux/macOS构建脚本
├── README.md            # 项目说明文档
└── resources/           # 语言资源文件夹（现在已内置到源码中）
    ├── zh-CN.json      # 中文语言资源（参考）
    └── en-US.json      # 英文语言资源（参考）
```

## 技术实现

### 架构
- **前端**: Qt6 GUI框架
- **后端**: 原生C++实现，包含所有核心功能
- **通信**: 直接调用API，无网络通信

### 国际化实现
- 使用自定义的I18nManager类管理语言资源
- 语言资源内嵌在源代码中，无需外部文件
- 支持动态语言切换，无需重启应用

## 故障排除

### 常见问题
1. **找不到Qt库**: 确保Qt已正确安装并添加到PATH
2. **编译错误**: 检查编译器是否支持C++17标准
3. **运行时错误**: 检查Qt运行时库是否可用