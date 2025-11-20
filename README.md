# Chemistry-machine-learning-application-software
This is a powerful big data software, it uses advanced machine learning algorithm and high-performance computing, integrated model training, data set management, model evaluation and analysis, prediction results, log management and other functions. Will improve the efficiency and accuracy of scientific research.
# BondForge V1.2 - Chemical Machine Learning System

## Overview
BondForge V1.1 is a chemical machine learning system that supports data management, uploading, and analysis. The system provides both command-line interface and graphical user interface, with support for Chinese and English language switching.

## Features
- **Data Management**: Upload, edit, and delete chemical data
- **Multiple Format Support**: CSV, JSON, SDF and other formats
- **Access Control**: Role-based access control
- **Internationalization**: Support for Chinese and English interface switching
- **Graphical Interface**: Intuitive Qt GUI
- **Command Line Tool**: Suitable for automation scripts
- **Single File Application**: All functionality integrated in one source file

## System Requirements

### Development Environment
- Qt 6.0 or higher
- CMake 3.16.0 or higher
- C++17 compatible compiler
- Make or Ninja (optional)

### Runtime Environment
- Windows 10/11, macOS 10.15+, or Linux
- Qt 6.0 runtime libraries

## Building and Running

### For Windows Users
1. Ensure Qt is installed and added to PATH environment variable
2. Run `build.bat` to build and start the GUI application
3. Or run `build.bat cli` to build the command-line version

### For Linux/macOS Users
1. Install dependencies: `sudo apt-get install qt6-base-dev cmake build-essential`
2. Run `./build.sh` to build and start the GUI application
3. Or run `./build.sh cli` to build the command-line version

### Manual Build with CMake
```bash
mkdir build && cd build
cmake ..
cmake --build .
./bin/BondForge --gui  # GUI version
./bin/BondForge --cli  # Command-line version
```

## Interface Usage Guide

### Language Switching
- Select "中文" or "English" from the language dropdown at the top of the interface
- Or select from the "Language" menu in the menu bar
- The interface will immediately switch to the selected language without restart

### Data Upload
1. Switch to the "Data Upload" tab
2. Fill in the required fields in the form (ID, Category, Content)
3. Select data format (CSV, JSON, SDF)
4. Add tags (separate multiple tags with commas)
5. Click the "Submit" button to upload data

### Data Management
1. View all data in the "Data Management" tab
2. Click on a table row to select data
3. Use buttons to perform delete or edit operations
4. Click the "Refresh" button to refresh the data list

## Command Line Usage

```bash
# Start GUI interface
./BondForge --gui

# Start command-line interface
./BondForge --cli

# Display help information
./BondForge --help
```

## Project Structure

```
BondForge V1.1/
├── bondforge.cpp          # Single-file application (contains all features)
├── CMakeLists.txt         # CMake build file
├── Makefile              # Make build file
├── build.bat             # Windows build script
├── build.sh             # Linux/macOS build script
├── README.md            # Project documentation (Chinese)
├── README_EN.md         # Project documentation (English)
└── resources/           # Language resource folder (now embedded in source)
    ├── zh-CN.json      # Chinese language resource (reference)
    └── en-US.json      # English language resource (reference)
```

## Technical Implementation

### Architecture
- **Frontend**: Qt6 GUI framework
- **Backend**: Native C++ implementation with all core functionality
- **Communication**: Direct API calls, no network communication

### Internationalization Implementation
- Uses custom I18nManager class to manage language resources
- Language resources are embedded in source code, no external files needed
- Supports dynamic language switching without application restart

## Troubleshooting

### Common Issues
1. **Qt libraries not found**: Ensure Qt is properly installed and added to PATH
2. **Compilation errors**: Check if compiler supports C++17 standard
3. **Runtime errors**: Check if Qt runtime libraries are available
