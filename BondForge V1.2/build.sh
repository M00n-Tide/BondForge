#!/bin/bash

echo "BondForge V1.1 构建工具"
echo "========================"
echo

# 检查Qt安装
if ! command -v qmake &> /dev/null; then
    echo "错误: 未找到Qt。请安装Qt并添加到PATH环境变量。"
    echo "Ubuntu/Debian: sudo apt-get install qt6-base-dev"
    echo "macOS: brew install qt@6"
    exit 1
fi

# 解析命令行参数
RUN_MODE="GUI"
if [ "$1" == "cli" ] || [ "$1" == "--cli" ] || [ "$1" == "-c" ]; then
    RUN_MODE="CLI"
fi

echo "运行模式: $RUN_MODE"
echo

# 使用CMake构建
if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake ..
if [ $? -ne 0 ]; then
    echo "CMake配置失败"
    cd ..
    exit 1
fi

cmake --build .
if [ $? -ne 0 ]; then
    echo "构建失败"
    cd ..
    exit 1
fi

cd ..

echo
echo "构建成功!"
if [ "$RUN_MODE" == "GUI" ]; then
    echo "启动GUI应用..."
    ./build/bin/BondForge --gui &
else
    echo "启动命令行版本..."
    ./build/bin/BondForge --cli
fi