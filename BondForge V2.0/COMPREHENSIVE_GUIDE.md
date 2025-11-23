# BondForge V2.0 综合指南 | Comprehensive Guide

---

## 目录 | Table of Contents

1. [概述 | Overview](#概述--overview)
2. [系统要求 | System Requirements](#系统要求--system-requirements)
3. [快速安装 | Quick Installation](#快速安装--quick-installation)
4. [手动安装 | Manual Installation](#手动安装--manual-installation)
5. [验证安装 | Verifying Installation](#验证安装--verifying-installation)
6. [基本使用 | Basic Usage](#基本使用--basic-usage)
7. [核心功能详解 | Core Features](#核心功能详解--core-features)
8. [自动更新功能 | Auto Update Features](#自动更新功能--auto-update-features)
9. [项目结构 | Project Structure](#项目结构--project-structure)
10. [开发者指南 | Developer Guide](#开发者指南--developer-guide)
11. [故障排除 | Troubleshooting](#故障排除--troubleshooting)
12. [最佳实践 | Best Practices](#最佳实践--best-practices)

---

## 概述 | Overview

### 中文

BondForge V2.0 是一个专业级化学机器学习平台，已从演示版本完全升级为生产就绪的软件。该平台提供了完整的化学数据管理、分子可视化、机器学习分析和协作功能，适用于研究机构和工业应用。

### English

BondForge V2.0 is a professional-grade chemical machine learning platform, fully upgraded from a demonstration version to production-ready software. The platform provides complete chemical data management, molecular visualization, machine learning analysis, and collaboration features, suitable for research institutions and industrial applications.

---

## 系统要求 | System Requirements

### 中文

#### 最低系统要求：
- **操作系统**: Windows 10, macOS 10.14, 或 Linux (Ubuntu 18.04+)
- **处理器**: 64位双核CPU (2.0 GHz+)
- **内存**: 4 GB RAM (8 GB 推荐)
- **存储空间**: 2 GB 可用空间
- **显卡**: 支持OpenGL 3.3+的显卡

#### 可选依赖：
- **RDKit**: 专业化学信息学功能
- **mlpack**: 传统机器学习算法
- **TensorFlow/PyTorch**: 深度学习功能

### English

#### Minimum System Requirements:
- **Operating System**: Windows 10, macOS 10.14, or Linux (Ubuntu 18.04+)
- **Processor**: 64-bit dual-core CPU (2.0 GHz+)
- **Memory**: 4 GB RAM (8 GB recommended)
- **Storage**: 2 GB available space
- **Graphics**: OpenGL 3.3+ compatible graphics card

#### Optional Dependencies:
- **RDKit**: Professional cheminformatics functionality
- **mlpack**: Traditional machine learning algorithms
- **TensorFlow/PyTorch**: Deep learning capabilities

---

## 快速安装 | Quick Installation

### 中文

我们提供了自动化脚本来简化安装过程。这些脚本将自动检测系统、安装依赖并构建BondForge。

#### Windows

1. **下载并运行依赖检查脚本**:
   ```cmd
   python scripts\check_dependencies.py
   ```

2. **运行自动安装脚本**:
   ```cmd
   install.bat
   ```

3. **启动应用程序**:
   ```cmd
   run.bat
   ```

#### Linux/macOS

1. **下载并运行依赖检查脚本**:
   ```bash
   python scripts/check_dependencies.py
   ```

2. **运行自动安装脚本**:
   ```bash
   chmod +x install.sh
   ./install.sh
   ```

3. **启动应用程序**:
   ```bash
   ./run.sh
   ```

### English

We provide automated scripts to simplify the installation process. These scripts will automatically detect the system, install dependencies, and build BondForge.

#### Windows

1. **Download and run the dependency check script**:
   ```cmd
   python scripts\check_dependencies.py
   ```

2. **Run the automatic installation script**:
   ```cmd
   install.bat
   ```

3. **Launch the application**:
   ```cmd
   run.bat
   ```

#### Linux/macOS

1. **Download and run the dependency check script**:
   ```bash
   python scripts/check_dependencies.py
   ```

2. **Run the automatic installation script**:
   ```bash
   chmod +x install.sh
   ./install.sh
   ```

3. **Launch the application**:
   ```bash
   ./run.sh
   ```

---

## 手动安装 | Manual Installation

### 中文

如果您需要更多控制或自动脚本无法正常工作，可以按照以下步骤手动安装。

#### 步骤1: 安装系统依赖

**Windows**:
1. 安装Visual Studio 2019或更高版本，确保包含"C++桌面开发工具"工作负载
2. 从https://cmake.org下载并安装CMake 3.10或更高版本
3. 从https://www.qt.io下载并安装Qt 6.0或更高版本

**macOS** (使用Homebrew):
```bash
brew install cmake qt6
```

**Linux** (Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install build-essential cmake qt6-base-dev qt6-tools-dev
```

**Linux** (CentOS/RHEL):
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake qt6-qtbase-devel qt6-qttools-devel
```

#### 步骤2: 安装Python依赖

创建并激活虚拟环境（推荐）:
```bash
# Linux/macOS
python3 -m venv bondforge_env
source bondforge_env/bin/activate

# Windows
python -m venv bondforge_env
bondforge_env\Scripts\activate
```

安装基本Python包:
```bash
pip install --upgrade pip
pip install numpy pandas matplotlib scikit-learn requests PyQt6 PyQtWebEngine
```

安装可选依赖（用于高级功能）:
```bash
# 使用conda安装RDKit（推荐）
conda install -c conda-forge rdkit

# 或使用pip安装
pip install rdkit-pypi

# 安装其他可选包
pip install mlpack torch torchvision seaborn plotly jupyter
```

#### 步骤3: 构建项目

1. 克隆或下载源代码
2. 创建构建目录:
   ```bash
   mkdir build
   cd build
   ```

3. 配置CMake:
   ```bash
   # 基本配置
   cmake ..
   
   # 启用可选功能的配置
   cmake .. -DENABLE_RDKIT=ON -DENABLE_MLPACK=ON
   ```

4. 构建项目:
   ```bash
   # Linux/macOS
   make -j$(nproc)
   
   # Windows
   cmake --build . --config Release
   ```

5. （可选）安装项目:
   ```bash
   # Linux/macOS
   sudo make install
   
   # Windows
   cmake --install . --config Release
   ```

### English

If you need more control or the automatic scripts don't work properly, you can follow these steps to install manually.

#### Step 1: Install System Dependencies

**Windows**:
1. Install Visual Studio 2019 or higher, ensuring the "Desktop development with C++" workload is included
2. Download and install CMake 3.10 or higher from https://cmake.org
3. Download and install Qt 6.0 or higher from https://www.qt.io

**macOS** (using Homebrew):
```bash
brew install cmake qt6
```

**Linux** (Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install build-essential cmake qt6-base-dev qt6-tools-dev
```

**Linux** (CentOS/RHEL):
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake qt6-qtbase-devel qt6-qttools-devel
```

#### Step 2: Install Python Dependencies

Create and activate a virtual environment (recommended):
```bash
# Linux/macOS
python3 -m venv bondforge_env
source bondforge_env/bin/activate

# Windows
python -m venv bondforge_env
bondforge_env\Scripts\activate
```

Install basic Python packages:
```bash
pip install --upgrade pip
pip install numpy pandas matplotlib scikit-learn requests PyQt6 PyQtWebEngine
```

Install optional dependencies (for advanced features):
```bash
# Install RDKit using conda (recommended)
conda install -c conda-forge rdkit

# Or install using pip
pip install rdkit-pypi

# Install other optional packages
pip install mlpack torch torchvision seaborn plotly jupyter
```

#### Step 3: Build the Project

1. Clone or download the source code
2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Configure CMake:
   ```bash
   # Basic configuration
   cmake ..
   
   # Configuration with optional features
   cmake .. -DENABLE_RDKIT=ON -DENABLE_MLPACK=ON
   ```

4. Build the project:
   ```bash
   # Linux/macOS
   make -j$(nproc)
   
   # Windows
   cmake --build . --config Release
   ```

5. (Optional) Install the project:
   ```bash
   # Linux/macOS
   sudo make install
   
   # Windows
   cmake --install . --config Release
   ```

---

## 验证安装 | Verifying Installation

### 中文

#### 步骤1: 运行BondForge

**Windows**:
```cmd
build\Release\BondForge.exe
```

**Linux/macOS**:
```bash
./build/bin/BondForge
```

#### 步骤2: 验证功能

1. **打开示例分子**:
   - 文件 → 打开 → 选择 `examples/molecules/benzene.sdf`
   - 确认分子正确显示

2. **导入示例数据**:
   - 数据管理 → 导入数据 → 选择 `examples/data/molecular_properties.csv`
   - 确认数据正确加载

3. **测试机器学习功能**:
   - 切换到"机器学习分析"选项卡
   - 导入示例数据并尝试训练一个简单的模型

4. **检查可选功能**:
   - 如果安装了RDKit，测试高级分子可视化功能
   - 如果安装了mlpack，测试高级机器学习算法

### English

#### Step 1: Run BondForge

**Windows**:
```cmd
build\Release\BondForge.exe
```

**Linux/macOS**:
```bash
./build/bin/BondForge
```

#### Step 2: Verify Functionality

1. **Open a sample molecule**:
   - File → Open → Select `examples/molecules/benzene.sdf`
   - Verify the molecule displays correctly

2. **Import sample data**:
   - Data Management → Import Data → Select `examples/data/molecular_properties.csv`
   - Verify the data loads correctly

3. **Test machine learning functionality**:
   - Switch to the "Machine Learning Analysis" tab
   - Import sample data and try to train a simple model

4. **Check optional features**:
   - If RDKit is installed, test advanced molecular visualization features
   - If mlpack is installed, test advanced machine learning algorithms

---

## 基本使用 | Basic Usage

### 中文

#### 首次启动
1. 启动应用程序后，您将看到主界面
2. 系统会自动创建配置文件和数据库
3. 选择您偏好的语言（中文或英文）

#### 基本工作流程
1. **导入数据**：支持CSV、JSON、SDF等多种格式
2. **分子可视化**：选择分子记录，在可视化面板中查看
3. **数据分析**：使用内置的统计分析和机器学习工具
4. **保存和导出**：保存您的工作并导出结果

### English

#### First Launch
1. After starting the application, you will see the main interface
2. The system will automatically create configuration files and database
3. Select your preferred language (Chinese or English)

#### Basic Workflow
1. **Import Data**: Support for CSV, JSON, SDF and other formats
2. **Molecular Visualization**: Select molecular records and view them in the visualization panel
3. **Data Analysis**: Use built-in statistical analysis and machine learning tools
4. **Save and Export**: Save your work and export results

---

## 核心功能详解 | Core Features

### 中文

#### 1. 数据管理
- 支持多种化学数据格式导入
- 提供完整的CRUD操作
- 高级搜索和过滤功能
- 数据分类和标签系统

#### 2. 分子可视化
- 2D/3D分子结构渲染
- 多种显示模式和风格
- 交互式缩放和旋转
- 多格式导出支持

#### 3. 机器学习分析
- 多种算法支持（回归、分类、聚类）
- 完整的数据预处理流程
- 模型训练、评估和部署
- 可视化结果展示

#### 4. 协作功能
- 多用户管理和权限控制
- 数据共享和协作编辑
- 实时更新和通知系统
- 版本控制和历史记录

### English

#### 1. Data Management
- Import various chemical data formats
- Complete CRUD operations
- Advanced search and filtering
- Data categorization and tagging system

#### 2. Molecular Visualization
- 2D/3D molecular structure rendering
- Multiple display modes and styles
- Interactive zoom and rotation
- Multi-format export support

#### 3. Machine Learning Analysis
- Support for multiple algorithms (regression, classification, clustering)
- Complete data preprocessing pipeline
- Model training, evaluation, and deployment
- Visualized result presentation

#### 4. Collaboration Features
- Multi-user management and access control
- Data sharing and collaborative editing
- Real-time updates and notification system
- Version control and history tracking

---

## 自动更新功能 | Auto Update Features

### 中文

BondForge V2.0 内置了全面的自动更新系统，可以定期检查并更新内置数据，确保数据始终保持最新状态。

#### 功能特性
- **智能更新调度**：根据数据源类型和使用频率智能安排更新时间
- **增量更新**：只下载变更部分，减少网络流量和更新时间
- **备份与恢复**：更新前自动备份，支持一键回滚
- **完整性验证**：使用校验和验证下载数据的完整性
- **并发控制**：支持多个数据源并发更新，提高效率
- **网络优化**：支持断点续传和网络状况自适应
- **通知系统**：提供详细的更新进度和结果通知
- **灵活配置**：支持全局和单个数据源的独立配置

#### 使用方法
1. **通过图形界面**:
   - 启动 BondForge V2.0
   - 在主菜单中选择"工具" > "更新管理"
   - 在更新管理界面中查看和配置自动更新设置

2. **通过命令行**:
   ```bash
   # 检查更新
   BondForge --check-updates
   
   # 显示更新管理器
   BondForge --update-manager
   
   # 启动时跳过自动更新检查
   BondForge --no-update-check
   ```

### English

BondForge V2.0 includes a comprehensive automatic update system that can regularly check and update built-in data, ensuring that data always remains up-to-date.

#### Features
- **Smart Update Scheduling**: Intelligently schedules update times based on data source type and usage frequency
- **Incremental Updates**: Downloads only changed portions to reduce network traffic and update time
- **Backup and Recovery**: Automatically backs up before updates, supports one-click rollback
- **Integrity Verification**: Uses checksums to verify the integrity of downloaded data
- **Concurrency Control**: Supports concurrent updates of multiple sources to improve efficiency
- **Network Optimization**: Supports resumable downloads and network condition adaptation
- **Notification System**: Provides detailed update progress and result notifications
- **Flexible Configuration**: Supports independent configuration for global and individual data sources

#### Usage
1. **Through the GUI**:
   - Launch BondForge V2.0
   - Select "Tools" > "Update Manager" from the main menu
   - View and configure automatic update settings in the update manager interface

2. **Through Command Line**:
   ```bash
   # Check for updates
   BondForge --check-updates
   
   # Show update manager
   BondForge --update-manager
   
   # Skip automatic update check on startup
   BondForge --no-update-check
   ```

---

## 项目结构 | Project Structure

### 中文

```
BondForge V2.0/
├── main.cpp                    # 主程序入口
├── CMakeLists.txt              # CMake构建配置
├── config/                     # 配置文件
│   ├── bondforge.json          # 主配置
│   └── i18n/                   # 国际化
├── core/                       # 核心模块
│   ├── data/                   # 数据管理
│   ├── chemistry/              # 化学信息学
│   ├── ml/                     # 机器学习
│   ├── collaboration/          # 协作功能
│   ├── permissions/            # 权限管理
│   └── plugins/                # 插件系统
├── ui/                         # 用户界面
│   ├── MainWindow.h/cpp        # 主窗口
│   ├── DataManagementWidget.h/cpp  # 数据管理
│   ├── VisualizationWidget.h/cpp   # 分子可视化
│   ├── MLAnalysisWidget.h/cpp      # 机器学习分析
│   ├── CollaborationWidget.h/cpp    # 协作功能
│   └── SettingsWidget.h/cpp         # 设置管理
├── services/                   # 服务层
│   ├── DatabaseService.h/cpp   # 数据库服务
│   ├── NetworkService.h/cpp    # 网络服务
│   └── UpdateService.h/cpp     # 更新服务
├── utils/                      # 工具类
│   ├── ConfigManager.h/cpp     # 配置管理
│   └── Logger.h/cpp           # 日志系统
├── scripts/                    # 脚本文件
│   ├── build.py               # Python构建工具
│   ├── install_dependencies.sh # 依赖安装脚本
│   └── check_dependencies.py  # 依赖检查工具
├── examples/                   # 示例文件
│   ├── molecules/             # 分子结构示例
│   ├── data/                  # 数据集示例
│   └── models/                # 模型示例
└── plugins/                    # 插件目录
```

### English

```
BondForge V2.0/
├── main.cpp                    # Main program entry
├── CMakeLists.txt              # CMake build configuration
├── config/                     # Configuration files
│   ├── bondforge.json          # Main configuration
│   └── i18n/                   # Internationalization
├── core/                       # Core modules
│   ├── data/                   # Data management
│   ├── chemistry/              # Cheminformatics
│   ├── ml/                     # Machine learning
│   ├── collaboration/          # Collaboration features
│   ├── permissions/            # Permission management
│   └── plugins/                # Plugin system
├── ui/                         # User interface
│   ├── MainWindow.h/cpp        # Main window
│   ├── DataManagementWidget.h/cpp  # Data management
│   ├── VisualizationWidget.h/cpp   # Molecular visualization
│   ├── MLAnalysisWidget.h/cpp      # ML analysis
│   ├── CollaborationWidget.h/cpp    # Collaboration
│   └── SettingsWidget.h/cpp         # Settings
├── services/                   # Service layer
│   ├── DatabaseService.h/cpp   # Database service
│   ├── NetworkService.h/cpp    # Network service
│   └── UpdateService.h/cpp     # Update service
├── utils/                      # Utility classes
│   ├── ConfigManager.h/cpp     # Configuration management
│   └── Logger.h/cpp           # Logging system
├── scripts/                    # Script files
│   ├── build.py               # Python build tool
│   ├── install_dependencies.sh # Dependency installation
│   └── check_dependencies.py  # Dependency checker
├── examples/                   # Example files
│   ├── molecules/             # Molecular structure examples
│   ├── data/                  # Dataset examples
│   └── models/                # Model examples
└── plugins/                    # Plugin directory
```

---

## 开发者指南 | Developer Guide

### 中文

#### 构建系统
- 使用CMake作为跨平台构建工具
- 支持可选依赖的自动检测和配置
- 提供Python构建脚本实现一键构建

#### 扩展开发
- 插件系统支持功能扩展
- 提供完整的API文档和示例
- 模块化架构便于代码维护

#### 贡献指南
1. Fork项目仓库
2. 创建功能分支
3. 编写测试用例
4. 提交Pull Request

#### 插件开发
请参考 [plugins/example_plugin](plugins/example_plugin) 目录中的示例插件。

### English

#### Build System
- Uses CMake as cross-platform build tool
- Supports automatic detection and configuration of optional dependencies
- Provides Python build scripts for one-click building

#### Extension Development
- Plugin system supports feature extension
- Provides complete API documentation and examples
- Modular architecture facilitates code maintenance

#### Contributing Guidelines
1. Fork the project repository
2. Create a feature branch
3. Write test cases
4. Submit a Pull Request

#### Plugin Development
Please refer to the example plugin in the [plugins/example_plugin](plugins/example_plugin) directory.

---

## 故障排除 | Troubleshooting

### 中文

#### 常见问题和解决方案

**问题1: CMake配置失败**
- **原因**: 缺少依赖或路径不正确
- **解决方案**: 运行`python scripts/check_dependencies.py`检查依赖，确保所有必需软件已安装

**问题2: Python模块导入错误**
- **原因**: 虚拟环境未激活或Python包未正确安装
- **解决方案**: 确保已激活虚拟环境，重新安装Python包

**问题3: 分子可视化功能不可用**
- **原因**: Qt未正确安装或路径不正确
- **解决方案**: 重新安装Qt并确保已添加到PATH

**问题4: 高级化学功能不可用**
- **原因**: RDKit未安装或版本过低
- **解决方案**: 使用conda安装RDKit: `conda install -c conda-forge rdkit`

**问题5: 机器学习算法不可用**
- **原因**: mlpack未正确安装
- **解决方案**: 使用pip安装mlpack: `pip install mlpack`

#### 更新失败故障排除

1. **检查网络连接**
   ```bash
   ping api.bondforge.org
   ```

2. **检查磁盘空间**
   - 确保有足够空间下载和存储更新
   - 清理不必要的文件和旧备份

3. **检查权限**
   - 确保应用程序有写入数据目录的权限
   - 在 Windows 上可能需要管理员权限

4. **查看日志**
   - 检查更新日志了解详细错误信息
   - 日志位置：`logs/update.log`

#### 获取帮助

如果遇到问题，请通过以下方式获取帮助：
- 运行依赖检查脚本：`python scripts/check_dependencies.py`
- 查看构建日志了解详细错误信息
- 访问GitHub仓库的Issues页面
- 发送邮件至：support@bondforge.org

### English

#### Common Issues and Solutions

**Issue 1: CMake configuration fails**
- **Cause**: Missing dependencies or incorrect paths
- **Solution**: Run `python scripts/check_dependencies.py` to check dependencies, ensure all required software is installed

**Issue 2: Python module import errors**
- **Cause**: Virtual environment not activated or Python packages not properly installed
- **Solution**: Ensure virtual environment is activated, reinstall Python packages

**Issue 3: Molecular visualization features unavailable**
- **Cause**: Qt not properly installed or path incorrect
- **Solution**: Reinstall Qt and ensure it's added to PATH

**Issue 4: Advanced chemistry features unavailable**
- **Cause**: RDKit not installed or version too low
- **Solution**: Install RDKit using conda: `conda install -c conda-forge rdkit`

**Issue 5: Machine learning algorithms unavailable**
- **Cause**: mlpack not properly installed
- **Solution**: Install mlpack using pip: `pip install mlpack`

#### Update Failure Troubleshooting

1. **Check Network Connection**
   ```bash
   ping api.bondforge.org
   ```

2. **Check Disk Space**
   - Ensure there is enough space to download and store updates
   - Clean up unnecessary files and old backups

3. **Check Permissions**
   - Ensure the application has permission to write to data directories
   - May require administrator privileges on Windows

4. **View Logs**
   - Check update logs for detailed error information
   - Log location: `logs/update.log`

#### Getting Help

If you encounter problems, please get help through the following methods:
- Run the dependency check script: `python scripts/check_dependencies.py`
- Check build logs for detailed error information
- Visit the Issues page on the GitHub repository
- Send email to: support@bondforge.org

---

## 最佳实践 | Best Practices

### 中文

#### 1. 定期检查更新
- 建议启用自动更新
- 对于关键数据源，缩短检查间隔

#### 2. 保持备份
- 启用自动备份功能
- 定期手动备份重要数据

#### 3. 监控更新
- 关注更新通知
- 定期查看更新日志

#### 4. 网络优化
- 在网络状况良好时进行更新
- 对于大型更新，考虑在非高峰时段进行

#### 5. 测试更新
- 在测试环境中先应用更新
- 验证功能正常后再在生产环境使用

### English

#### 1. Regularly Check for Updates
- Recommended to enable automatic updates
- For critical data sources, shorten check intervals

#### 2. Maintain Backups
- Enable automatic backup functionality
- Regularly manually backup important data

#### 3. Monitor Updates
- Pay attention to update notifications
- Regularly review update logs

#### 4. Network Optimization
- Perform updates when network conditions are good
- For large updates, consider doing so during off-peak hours

#### 5. Test Updates
- Apply updates in a test environment first
- Verify functionality before using in production environment