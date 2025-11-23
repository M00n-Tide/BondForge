#!/usr/bin/env python3
"""
Installation script for BondForge V2.0
This script checks dependencies and prepares the environment
"""

import os
import sys
import subprocess
import platform
import json

def print_info(message):
    """Print info message with color"""
    print(f"\033[0;34m[INFO]\033[0m {message}")

def print_success(message):
    """Print success message with color"""
    print(f"\033[0;32m[SUCCESS]\033[0m {message}")

def print_error(message):
    """Print error message with color"""
    print(f"\033[0;31m[ERROR]\033[0m {message}")

def print_warning(message):
    """Print warning message with color"""
    print(f"\033[1;33m[WARNING]\033[0m {message}")

def check_system():
    """Check system compatibility"""
    print_info("Checking system compatibility...")
    
    system = platform.system()
    print(f"Operating System: {system}")
    
    # Check architecture
    arch = platform.machine()
    print(f"Architecture: {arch}")
    
    # Check Python version
    py_version = platform.python_version_tuple()
    if int(py_version[0]) < 3 or (int(py_version[0]) == 3 and int(py_version[1]) < 8):
        print_error(f"Python {py_version[0]}.{py_version[1]} is not supported. Please use Python 3.8 or later.")
        return False
    
    print(f"Python Version: {py_version[0]}.{py_version[1]}.{py_version[2]}")
    
    print_success("System compatibility check passed")
    return True

def install_dependencies():
    """Install required and optional dependencies"""
    print_info("Installing dependencies...")
    
    system = platform.system()
    
    # Install Python packages
    print_info("Installing Python packages...")
    
    packages = [
        "numpy", 
        "pandas", 
        "matplotlib", 
        "scikit-learn", 
        "requests"
    ]
    
    # Platform-specific packages
    if system == "Windows":
        packages.extend(["PyQt6", "PyQtWebEngine"])
    else:
        packages.extend(["PyQt6", "PyQtWebEngine"])
    
    try:
        subprocess.check_call([sys.executable, "-m", "pip", "install", "-q"] + packages)
        print_success("Python packages installed")
    except subprocess.CalledProcessError as e:
        print_error(f"Failed to install Python packages: {e}")
        return False
    
    # Install optional packages
    optional_packages = ["rdkit-pypi", "mlpack", "torch", "seaborn", "plotly"]
    
    print_warning("Installing optional packages...")
    for package in optional_packages:
        try:
            subprocess.check_call([sys.executable, "-m", "pip", "install", "-q", package])
            print(f"✓ {package}")
        except subprocess.CalledProcessError:
            print(f"✗ {package} (optional)")
    
    # System-specific dependencies
    if system == "Linux":
        print_info("Checking for system dependencies...")
        
        # Try to install using apt if available
        try:
            subprocess.check_call(["apt", "--version"], stdout=subprocess.DEVNULL)
            
            deps = [
                "build-essential", 
                "cmake", 
                "qt6-base-dev", 
                "qt6-tools-dev"
            ]
            
            print_info("Installing system packages with apt...")
            for dep in deps:
                try:
                    subprocess.check_call(["sudo", "apt-get", "install", "-y", dep])
                    print(f"✓ {dep}")
                except subprocess.CalledProcessError:
                    print_warning(f"Could not install {dep}")
            
        except (subprocess.CalledProcessError, FileNotFoundError):
            print_warning("apt not available. Please install system dependencies manually.")
    
    print_success("Dependency installation completed")
    return True

def create_directories():
    """Create necessary directories"""
    print_info("Creating necessary directories...")
    
    directories = [
        "data",
        "data/backup",
        "cache",
        "logs",
        "models"
    ]
    
    for directory in directories:
        os.makedirs(directory, exist_ok=True)
        print(f"✓ Created directory: {directory}")
    
    print_success("Directories created")
    return True

def create_config():
    """Create default configuration if it doesn't exist"""
    config_path = "config/bondforge.json"
    
    if os.path.exists(config_path):
        print_info("Configuration file already exists")
        return True
    
    print_info("Creating default configuration...")
    
    # Create config directory if it doesn't exist
    os.makedirs("config", exist_ok=True)
    
    try:
        with open(config_path, 'w') as f:
            # Default configuration
            default_config = {
                "app": {
                    "name": "BondForge",
                    "version": "2.0.0",
                    "language": "en",
                    "theme": "light",
                    "auto_save_interval": 300
                },
                "data": {
                    "default_format": "JSON",
                    "auto_save": True,
                    "backup_enabled": True,
                    "backup_interval": 24
                },
                "update": {
                    "autoUpdateEnabled": True,
                    "scheduler": {
                        "enabled": True,
                        "intervalHours": 24
                    }
                }
            }
            json.dump(default_config, f, indent=4)
        
        print_success("Default configuration created")
        return True
    except Exception as e:
        print_error(f"Failed to create configuration: {e}")
        return False

def run_build():
    """Run the build script"""
    print_info("Running build script...")
    
    try:
        result = subprocess.run([sys.executable, "build.py"], check=True)
        print_success("Build completed")
        return True
    except subprocess.CalledProcessError:
        print_error("Build failed")
        return False

def main():
    print_info("Starting BondForge V2.0 installation...")
    
    # Check system compatibility
    if not check_system():
        return 1
    
    # Install dependencies
    if not install_dependencies():
        return 1
    
    # Create necessary directories
    if not create_directories():
        return 1
    
    # Create default configuration
    if not create_config():
        return 1
    
    # Run build
    if not run_build():
        return 1
    
    print_success("BondForge V2.0 installation completed successfully!")
    print_info("You can now run BondForge V2.0 with 'python run.py'")
    return 0

if __name__ == '__main__':
    sys.exit(main())