#!/usr/bin/env python3
"""
Simplified build script for BondForge V2.0
This script checks dependencies and builds project with available features
"""

import os
import sys
import subprocess
import argparse
import json
import platform
import shutil

def print_info(message):
    """Print info message with color"""
    print(f"\033[0;34m[INFO]\033[0m {message}")

def print_success(message):
    """Print success message with color"""
    print(f"\033[0;32m[SUCCESS]\033[0m {message}")

def print_error(message):
    """Print error message with color"""
    print(f"\033[0;31m[ERROR]\033[0m {message}")

def check_dependencies():
    """Check if required dependencies are available"""
    print_info("Checking dependencies...")
    
    # Check CMake
    try:
        result = subprocess.run(['cmake', '--version'], capture_output=True, text=True)
        cmake_version = result.stdout.splitlines()[0].split()[2]
        print(f"✓ CMake {cmake_version}")
    except FileNotFoundError:
        print_error("CMake not found. Please install CMake 3.16 or later.")
        return False
    
    # Check Qt6
    try:
        result = subprocess.run(['qmake', '-version'], capture_output=True, text=True)
        qt_version = result.stdout.splitlines()[2].split()[1]
        print(f"✓ Qt {qt_version}")
    except (FileNotFoundError, IndexError):
        print_error("Qt not found. Please install Qt6.")
        return False
    
    # Check optional dependencies
    has_rdkit = False
    has_mlpack = False
    
    # Check for RDKit
    try:
        import rdkit
        print("✓ RDKit (Python)")
        has_rdkit = True
    except ImportError:
        print("⚠ RDKit not found (optional)")
    
    # Check for mlpack
    try:
        result = subprocess.run(['mlpack_test', '--help'], capture_output=True, text=True)
        if result.returncode == 0:
            print("✓ mlpack")
            has_mlpack = True
    except FileNotFoundError:
        print("⚠ mlpack not found (optional)")
    
    return True, has_rdkit, has_mlpack

def clean_build(build_dir):
    """Clean build directory"""
    if os.path.exists(build_dir):
        print_info(f"Removing build directory: {build_dir}")
        shutil.rmtree(build_dir)
    print_success("Build directory cleaned")

def configure_cmake(build_dir, build_type, enable_rdkit, enable_mlpack):
    """Configure CMake"""
    os.makedirs(build_dir, exist_ok=True)
    os.chdir(build_dir)
    
    cmake_args = [
        'cmake',
        '..',
        f'-DCMAKE_BUILD_TYPE={build_type}'
    ]
    
    if enable_rdkit:
        cmake_args.append('-DENABLE_RDKIT=ON')
    
    if enable_mlpack:
        cmake_args.append('-DENABLE_MLPACK=ON')
    
    print_info(f"Configuring with: {' '.join(cmake_args)}")
    
    result = subprocess.run(cmake_args)
    if result.returncode != 0:
        print_error("CMake configuration failed")
        return False
    
    print_success("CMake configuration successful")
    return True

def build_project(build_dir, parallel_jobs):
    """Build the project"""
    os.chdir(build_dir)
    
    # Determine number of parallel jobs
    if parallel_jobs == 'auto':
        try:
            parallel_jobs = os.cpu_count() or 1
        except:
            parallel_jobs = 1
    
    build_args = ['cmake', '--build', '.', '--config', 'Release', f'-j{parallel_jobs}']
    
    print_info(f"Building with: {' '.join(build_args)}")
    
    result = subprocess.run(build_args)
    if result.returncode != 0:
        print_error("Build failed")
        return False
    
    print_success("Build completed successfully")
    return True

def run_tests(build_dir):
    """Run tests"""
    os.chdir(build_dir)
    
    print_info("Running tests...")
    
    # Try to find and run tests
    if os.path.exists('tests'):
        test_args = ['ctest', '--output-on-failure']
        result = subprocess.run(test_args)
        
        if result.returncode != 0:
            print_error("Some tests failed")
            return False
    
    print_success("All tests passed")
    return True

def main():
    parser = argparse.ArgumentParser(description='Build script for BondForge V2.0')
    parser.add_argument('--clean', action='store_true', help='Clean build directory')
    parser.add_argument('--type', default='Release', choices=['Debug', 'Release'], 
                        help='Build type')
    parser.add_argument('--enable-rdkit', action='store_true', help='Enable RDKit features')
    parser.add_argument('--enable-mlpack', action='store_true', help='Enable mlpack features')
    parser.add_argument('--enable-all', action='store_true', 
                        help='Enable all optional features')
    parser.add_argument('--no-tests', action='store_true', help='Skip running tests')
    parser.add_argument('--jobs', default='auto', 
                        help='Number of parallel build jobs')
    
    args = parser.parse_args()
    
    # Get the script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    
    # Clean build if requested
    build_dir = 'build'
    if args.clean:
        clean_build(build_dir)
        return 0
    
    # Check dependencies
    deps_ok, has_rdkit, has_mlpack = check_dependencies()
    if not deps_ok:
        return 1
    
    # Configure optional features
    enable_rdkit = args.enable_rdkit or (args.enable_all and has_rdkit)
    enable_mlpack = args.enable_mlpack or (args.enable_all and has_mlpack)
    
    if args.enable_all:
        print_info("Auto-enabling available optional features")
    
    # Configure CMake
    if not configure_cmake(build_dir, args.type, enable_rdkit, enable_mlpack):
        return 1
    
    # Build project
    if not build_project(build_dir, args.jobs):
        return 1
    
    # Run tests if not disabled
    if not args.no_tests:
        run_tests(build_dir)
    
    print_success("BondForge V2.0 build completed successfully")
    return 0

if __name__ == '__main__':
    sys.exit(main())