#!/usr/bin/env python3
"""
BondForge V1.2 依赖检查脚本
检查系统上是否安装了所有必需和可选的依赖库
"""
import sys
import os
import subprocess
import platform
import json
from typing import Dict, List, Tuple, Optional

# 依赖配置文件
DEPENDENCIES_CONFIG = {
    "required": {
        "python": {
            "name": "Python",
            "min_version": "3.8",
            "check_command": ["python", "--version"],
            "install_instructions": {
                "Windows": "请从 https://www.python.org 下载并安装Python 3.8或更高版本",
                "Linux": "在Ubuntu/Debian上运行: sudo apt-get install python3 python3-pip\n在CentOS/RHEL上运行: sudo yum install python3 python3-pip",
                "macOS": "使用Homebrew安装: brew install python3\n或从 https://www.python.org 下载安装程序"
            }
        },
        "qt": {
            "name": "Qt",
            "min_version": "5.15",
            "check_command": ["qmake", "-version"],
            "alternative_check": ["pkg-config", "--modversion", "Qt5Core"],
            "install_instructions": {
                "Windows": "请从 https://www.qt.io 下载并安装Qt 5.15或更高版本",
                "Linux": "在Ubuntu/Debian上运行: sudo apt-get install qt5-default qtbase5-dev qttools5-dev\n在CentOS/RHEL上运行: sudo yum install qt5-qtbase-devel qt5-qttools-devel",
                "macOS": "使用Homebrew安装: brew install qt5"
            }
        },
        "cmake": {
            "name": "CMake",
            "min_version": "3.10",
            "check_command": ["cmake", "--version"],
            "install_instructions": {
                "Windows": "请从 https://cmake.org 下载并安装CMake",
                "Linux": "在Ubuntu/Debian上运行: sudo apt-get install cmake\n在CentOS/RHEL上运行: sudo yum install cmake",
                "macOS": "使用Homebrew安装: brew install cmake"
            }
        }
    },
    "python_packages": {
        "required": [
            {"name": "numpy", "min_version": "1.19", "pip_name": "numpy"},
            {"name": "pandas", "min_version": "1.3", "pip_name": "pandas"},
            {"name": "matplotlib", "min_version": "3.3", "pip_name": "matplotlib"},
            {"name": "scikit-learn", "min_version": "0.24", "pip_name": "scikit-learn"},
            {"name": "requests", "min_version": "2.25", "pip_name": "requests"},
            {"name": "pyqt5" if platform.system() != "Darwin" else "PyQt6", 
             "min_version": "5.15" if platform.system() != "Darwin" else "6.0",
             "pip_name": "PyQt5" if platform.system() != "Darwin" else "PyQt6"},
            {"name": "pyqtwebengine" if platform.system() != "Darwin" else "PyQt6-WebEngine", 
             "min_version": "5.15" if platform.system() != "Darwin" else "6.0",
             "pip_name": "PyQtWebEngine" if platform.system() != "Darwin" else "PyQt6-WebEngine"}
        ],
        "optional": [
            {"name": "rdkit", "min_version": "2021.09", "pip_name": "rdkit-pypi", "conda_name": "rdkit"},
            {"name": "mlpack", "min_version": "4.0", "pip_name": "mlpack", "conda_name": "mlpack"},
            {"name": "tensorflow", "min_version": "2.6", "pip_name": "tensorflow"},
            {"name": "torch", "min_version": "1.9", "pip_name": "torch"},
            {"name": "torchvision", "min_version": "0.10", "pip_name": "torchvision"},
            {"name": "jupyter", "min_version": "1.0", "pip_name": "jupyter"},
            {"name": "seaborn", "min_version": "0.11", "pip_name": "seaborn"},
            {"name": "plotly", "min_version": "5.0", "pip_name": "plotly"},
            {"name": "openbabel", "min_version": "3.0", "pip_name": "openbabel", "conda_name": "openbabel"},
            {"name": "biopython", "min_version": "1.79", "pip_name": "biopython"},
            {"name": "simpy", "min_version": "4.0", "pip_name": "simpy"},
            {"name": "networkx", "min_version": "2.6", "pip_name": "networkx"},
            {"name": "joblib", "min_version": "1.1", "pip_name": "joblib"}
        ]
    },
    "compilers": {
        "cpp": {
            "name": "C++ Compiler",
            "check_commands": {
                "Windows": ["cl"],
                "Linux": ["g++", "--version"],
                "macOS": ["clang++", "--version"]
            },
            "install_instructions": {
                "Windows": "请安装Visual Studio 2019或更高版本，包含C++开发工具",
                "Linux": "在Ubuntu/Debian上运行: sudo apt-get install build-essential\n在CentOS/RHEL上运行: sudo yum groupinstall \"Development Tools\"",
                "macOS": "安装Xcode命令行工具: xcode-select --install"
            }
        }
    }
}

class DependencyChecker:
    def __init__(self):
        self.platform = platform.system()
        self.results = {
            "required": {},
            "optional": {},
            "missing_required": [],
            "missing_optional": []
        }
        
    def check_command(self, command: List[str]) -> Tuple[bool, Optional[str]]:
        """检查命令是否存在并获取版本信息"""
        try:
            result = subprocess.run(
                command, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE,
                text=True,
                timeout=10
            )
            return True, result.stdout.strip()
        except (subprocess.TimeoutExpired, subprocess.CalledProcessError, FileNotFoundError):
            return False, None
    
    def check_python_version(self) -> Tuple[bool, Optional[str]]:
        """检查Python版本"""
        try:
            version = sys.version.split()[0]
            major, minor = version.split(".")[:2]
            major, minor = int(major), int(minor)
            
            min_major, min_minor = DEPENDENCIES_CONFIG["required"]["python"]["min_version"].split(".")
            min_major, min_minor = int(min_major), int(min_minor)
            
            if major > min_major or (major == min_major and minor >= min_minor):
                return True, version
            return False, version
        except:
            return False, None
    
    def check_python_package(self, package: Dict) -> Tuple[bool, Optional[str]]:
        """检查Python包是否存在并获取版本"""
        try:
            import importlib
            module = importlib.import_module(package["name"])
            version = getattr(module, "__version__", None)
            if version is None:
                version = getattr(module, "version", None)
            if version is None:
                # 尝试其他常见属性
                for attr in ["VERSION", "version_string", "__VERSION__"]:
                    version = getattr(module, attr, None)
                    if version:
                        break
            
            if version and self._compare_versions(version, package["min_version"]):
                return True, version
            return False, version
        except ImportError:
            return False, None
    
    def _compare_versions(self, current: str, minimum: str) -> bool:
        """比较版本号"""
        try:
            # 简单的版本比较，实际应用中可能需要更复杂的版本解析
            current_parts = [int(part) for part in current.split(".")]
            minimum_parts = [int(part) for part in minimum.split(".")]
            
            # 补齐版本号长度
            max_len = max(len(current_parts), len(minimum_parts))
            current_parts.extend([0] * (max_len - len(current_parts)))
            minimum_parts.extend([0] * (max_len - len(minimum_parts)))
            
            return current_parts >= minimum_parts
        except:
            return True  # 无法解析版本时，假设版本合适
    
    def check_system_dependency(self, name: str, config: Dict) -> Tuple[bool, Optional[str]]:
        """检查系统依赖"""
        if name == "python":
            return self.check_python_version()
        
        check_command = config.get("check_command")
        if check_command:
            success, output = self.check_command(check_command)
            if success:
                # 尝试从输出中提取版本号
                version = self._extract_version(output)
                if version and self._compare_versions(version, config["min_version"]):
                    return True, version
                return False, version if version else "未知版本"
        
        # 尝试替代检查命令
        alternative_check = config.get("alternative_check")
        if alternative_check:
            success, output = self.check_command(alternative_check)
            if success:
                version = self._extract_version(output)
                if version and self._compare_versions(version, config["min_version"]):
                    return True, version
                return False, version if version else "未知版本"
        
        return False, None
    
    def _extract_version(self, output: str) -> Optional[str]:
        """从命令输出中提取版本号"""
        import re
        # 尝试匹配常见的版本号模式
        version_patterns = [
            r"(\d+\.\d+\.\d+)",  # 1.2.3
            r"(\d+\.\d+)",       # 1.2
            r"Qt\s*(\d+\.\d+\.\d+)",  # Qt 5.15.2
        ]
        
        for pattern in version_patterns:
            match = re.search(pattern, output)
            if match:
                return match.group(1)
        
        return None
    
    def check_compiler(self) -> Tuple[bool, Optional[str]]:
        """检查C++编译器"""
        config = DEPENDENCIES_CONFIG["compilers"]["cpp"]
        check_commands = config["check_commands"].get(self.platform)
        
        if not check_commands:
            return False, f"不支持的平台: {self.platform}"
        
        if isinstance(check_commands, str):
            check_commands = [check_commands]
        
        for command in check_commands:
            success, output = self.check_command(command)
            if success:
                version = self._extract_version(output)
                return True, version if version else "可用"
        
        return False, None
    
    def check_all_dependencies(self):
        """检查所有依赖"""
        print(f"正在检查 BondForge V1.2 依赖...")
        print(f"平台: {self.platform}")
        print("=" * 60)
        
        # 检查必需的系统依赖
        print("检查必需的系统依赖...")
        for name, config in DEPENDENCIES_CONFIG["required"].items():
            success, version = self.check_system_dependency(name, config)
            self.results["required"][name] = {
                "success": success,
                "version": version,
                "name": config["name"]
            }
            
            if success:
                print(f"✓ {config['name']}: {version}")
            else:
                print(f"✗ {config['name']}: 未安装或版本过低")
                self.results["missing_required"].append(name)
        
        # 检查编译器
        print("\n检查C++编译器...")
        compiler_success, compiler_version = self.check_compiler()
        self.results["required"]["compiler"] = {
            "success": compiler_success,
            "version": compiler_version,
            "name": "C++ Compiler"
        }
        
        if compiler_success:
            print(f"✓ C++ Compiler: {compiler_version}")
        else:
            print(f"✗ C++ Compiler: 未安装")
            self.results["missing_required"].append("compiler")
        
        # 检查必需的Python包
        print("\n检查必需的Python包...")
        for package in DEPENDENCIES_CONFIG["python_packages"]["required"]:
            success, version = self.check_python_package(package)
            self.results["required"][package["name"]] = {
                "success": success,
                "version": version,
                "name": package["name"]
            }
            
            if success:
                print(f"✓ {package['name']}: {version}")
            else:
                print(f"✗ {package['name']}: 未安装或版本过低")
                self.results["missing_required"].append(package["name"])
        
        # 检查可选的Python包
        print("\n检查可选的Python包...")
        for package in DEPENDENCIES_CONFIG["python_packages"]["optional"]:
            success, version = self.check_python_package(package)
            self.results["optional"][package["name"]] = {
                "success": success,
                "version": version,
                "name": package["name"]
            }
            
            if success:
                print(f"✓ {package['name']}: {version}")
            else:
                print(f"- {package['name']}: 未安装 (可选)")
                self.results["missing_optional"].append(package["name"])
        
        print("\n" + "=" * 60)
        
        # 输出检查结果
        if not self.results["missing_required"]:
            print("✓ 所有必需依赖都已满足，可以编译运行 BondForge V1.2")
        else:
            print("✗ 以下必需依赖缺失，请安装后重试:")
            for dep in self.results["missing_required"]:
                config = self._get_dependency_config(dep)
                if config:
                    print(f"  - {config['name']}")
        
        if self.results["missing_optional"]:
            print("\n- 以下可选依赖缺失，某些高级功能可能不可用:")
            for dep in self.results["missing_optional"]:
                package = self._get_package_config(dep)
                if package:
                    print(f"  - {package['name']}")
        
        # 生成依赖报告
        self.generate_dependency_report()
    
    def _get_dependency_config(self, name: str) -> Optional[Dict]:
        """获取依赖配置"""
        if name in DEPENDENCIES_CONFIG["required"]:
            return DEPENDENCIES_CONFIG["required"][name]
        elif name == "compiler":
            return DEPENDENCIES_CONFIG["compilers"]["cpp"]
        return None
    
    def _get_package_config(self, name: str) -> Optional[Dict]:
        """获取Python包配置"""
        for package in DEPENDENCIES_CONFIG["python_packages"]["required"] + DEPENDENCIES_CONFIG["python_packages"]["optional"]:
            if package["name"] == name:
                return package
        return None
    
    def generate_dependency_report(self):
        """生成依赖报告"""
        report = {
            "platform": platform.system(),
            "check_time": subprocess.check_output(["date"], text=True).strip() if platform.system() != "Windows" else "Unknown",
            "dependencies": {
                "required": self.results["required"],
                "optional": self.results["optional"]
            },
            "missing": {
                "required": self.results["missing_required"],
                "optional": self.results["missing_optional"]
            }
        }
        
        try:
            with open("dependency_report.json", "w") as f:
                json.dump(report, f, indent=2)
            print(f"\n依赖报告已保存到: dependency_report.json")
        except:
            print("\n无法保存依赖报告")
    
    def print_installation_help(self):
        """打印安装帮助"""
        print("\n" + "=" * 60)
        print("安装帮助")
        print("=" * 60)
        
        for dep in self.results["missing_required"]:
            config = self._get_dependency_config(dep)
            if not config:
                continue
                
            print(f"\n{config['name']}:")
            instructions = config.get("install_instructions", {}).get(self.platform, "暂无安装说明")
            print(f"  {instructions}")
        
        # Python包安装帮助
        missing_python_packages = [pkg for pkg in self.results["missing_required"] 
                                 if pkg in [p["name"] for p in DEPENDENCIES_CONFIG["python_packages"]["required"]]]
        
        if missing_python_packages:
            print(f"\nPython包安装:")
            print("  运行以下命令安装缺失的Python包:")
            print(f"  pip install {' '.join(missing_python_packages)}")
        
        # 可选包安装帮助
        if self.results["missing_optional"]:
            print(f"\n可选Python包安装:")
            print("  运行以下命令安装可选的Python包:")
            missing_opt_packages = [p["pip_name"] for pkg in self.results["missing_optional"] 
                                 for p in DEPENDENCIES_CONFIG["python_packages"]["optional"] 
                                 if p["name"] == pkg]
            
            # 对于RDKit和OpenBabel，建议使用conda
            conda_packages = ["rdkit", "openbabel"]
            conda_missing = [pkg for pkg in missing_opt_packages if pkg in conda_packages]
            pip_missing = [pkg for pkg in missing_opt_packages if pkg not in conda_packages]
            
            if conda_missing:
                print(f"  conda install -c conda-forge {' '.join(conda_missing)}")
            if pip_missing:
                print(f"  pip install {' '.join(pip_missing)}")
        
        # 一键安装脚本提示
        print(f"\n一键安装:")
        print("  您可以运行以下脚本自动安装所有依赖:")
        print("  - Windows: scripts\\install_dependencies.bat")
        print("  - Linux/macOS: ./scripts/install_dependencies.sh")

def main():
    if len(sys.argv) > 1 and sys.argv[1] in ["-h", "--help"]:
        print("BondForge V1.2 依赖检查脚本")
        print("\n用法:")
        print("  python check_dependencies.py     # 检查所有依赖")
        print("  python check_dependencies.py -h  # 显示帮助")
        print("\n功能:")
        print("  - 检查所有必需的系统依赖和Python包")
        print("  - 检查可选的Python包")
        print("  - 生成依赖报告文件")
        print("  - 提供安装帮助")
        return
    
    checker = DependencyChecker()
    checker.check_all_dependencies()
    
    if checker.results["missing_required"]:
        checker.print_installation_help()
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())