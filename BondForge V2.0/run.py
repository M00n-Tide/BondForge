#!/usr/bin/env python3
"""
Run script for BondForge V2.0
This script starts the application with proper environment
"""

import os
import sys
import subprocess
import platform

def print_info(message):
    """Print info message with color"""
    print(f"\033[0;34m[INFO]\033[0m {message}")

def print_error(message):
    """Print error message with color"""
    print(f"\033[0;31m[ERROR]\033[0m {message}")

def setup_environment():
    """Setup environment variables"""
    # Add Qt plugins to path if needed
    if platform.system() == "Linux":
        qt_plugin_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "plugins")
        if "QT_PLUGIN_PATH" in os.environ:
            os.environ["QT_PLUGIN_PATH"] = f"{qt_plugin_path}:{os.environ['QT_PLUGIN_PATH']}"
        else:
            os.environ["QT_PLUGIN_PATH"] = qt_plugin_path
    
    # Set data directories
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.environ["BONDFORGE_DATA_DIR"] = os.path.join(script_dir, "data")
    os.environ["BONDFORGE_CACHE_DIR"] = os.path.join(script_dir, "cache")
    os.environ["BONDFORGE_LOG_DIR"] = os.path.join(script_dir, "logs")

def check_build():
    """Check if the application has been built"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    if platform.system() == "Windows":
        exe_path = os.path.join(script_dir, "build", "Release", "BondForge.exe")
    elif platform.system() == "Darwin":  # macOS
        exe_path = os.path.join(script_dir, "build", "BondForge.app", "Contents", "MacOS", "BondForge")
    else:  # Linux and others
        exe_path = os.path.join(script_dir, "build", "BondForge")
    
    if not os.path.exists(exe_path):
        print_error("Application not found. Please run 'python install.py' first.")
        return False, exe_path
    
    return True, exe_path

def main():
    print_info("Starting BondForge V2.0...")
    
    # Setup environment
    setup_environment()
    
    # Check if built
    built, exe_path = check_build()
    if not built:
        return 1
    
    try:
        # Start the application
        subprocess.run([exe_path])
        print_info("BondForge V2.0 closed")
        return 0
    except Exception as e:
        print_error(f"Failed to start application: {e}")
        return 1

if __name__ == '__main__':
    sys.exit(main())