@echo off
echo BondForge V1.1 构建工具
echo ========================
echo.

REM 检查Qt安装
where qmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo 错误: 未找到Qt。请安装Qt并添加到PATH环境变量。
    echo 下载地址: https://www.qt.io/download
    pause
    exit /b 1
)

REM 解析命令行参数
set RUN_MODE=GUI
if "%1"=="cli" set RUN_MODE=CLI
if "%1"=="--cli" set RUN_MODE=CLI
if "%1"=="-c" set RUN_MODE=CLI

echo 运行模式: %RUN_MODE%
echo.

REM 使用CMake构建
if not exist build mkdir build
cd build
cmake ..
if %ERRORLEVEL% NEQ 0 (
    echo CMake配置失败
    cd ..
    pause
    exit /b 1
)

cmake --build .
if %ERRORLEVEL% NEQ 0 (
    echo 构建失败
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo 构建成功!
if "%RUN_MODE%"=="GUI" (
    echo 启动GUI应用...
    start "" build\bin\BondForge.exe --gui
) else (
    echo 启动命令行版本...
    build\bin\BondForge.exe --cli
)

pause