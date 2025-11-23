#include "PluginManager.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <iostream>

namespace BondForge {
namespace Core {
namespace Plugins {

// PluginManager 实现
PluginManager::PluginManager() {
    registerStandardExtensionPoints();
}

PluginManager::~PluginManager() {
    unloadAllPlugins();
}

int PluginManager::loadPlugins(const std::string& pluginDirectory) {
    int loadedCount = 0;
    
    std::cout << "Loading plugins from: " << pluginDirectory << std::endl;
    
    // 扫描插件目录
    std::vector<std::string> pluginPaths = scanPluginDirectory(pluginDirectory);
    
    for (const auto& pluginPath : pluginPaths) {
        if (loadSinglePlugin(pluginPath)) {
            loadedCount++;
            std::cout << "Successfully loaded plugin: " << pluginPath << std::endl;
        } else {
            std::cout << "Failed to load plugin: " << pluginPath << std::endl;
        }
    }
    
    std::cout << "Loaded " << loadedCount << " out of " << pluginPaths.size() << " plugins." << std::endl;
    return loadedCount;
}

void PluginManager::unloadAllPlugins() {
    std::cout << "Unloading all plugins..." << std::endl;
    
    // 卸载所有插件
    for (auto& pluginInfo : m_plugins) {
        if (pluginInfo.isLoaded && pluginInfo.instance) {
            pluginInfo.instance->shutdown();
            pluginInfo.isLoaded = false;
            pluginInfo.instance.reset();
            
            std::cout << "Unloaded plugin: " << pluginInfo.name << std::endl;
        }
    }
    
    // 清理加载器
    m_loaders.clear();
    
    std::cout << "All plugins unloaded." << std::endl;
}

std::vector<PluginInfo> PluginManager::getAllPlugins() const {
    return m_plugins;
}

std::vector<PluginInfo> PluginManager::getLoadedPlugins() const {
    std::vector<PluginInfo> loadedPlugins;
    
    for (const auto& pluginInfo : m_plugins) {
        if (pluginInfo.isLoaded) {
            loadedPlugins.push_back(pluginInfo);
        }
    }
    
    return loadedPlugins;
}

std::unique_ptr<PluginInfo> PluginManager::getPlugin(const std::string& name) const {
    for (const auto& pluginInfo : m_plugins) {
        if (pluginInfo.name == name) {
            return std::make_unique<PluginInfo>(pluginInfo);
        }
    }
    
    return nullptr;
}

std::vector<std::shared_ptr<IPlugin>> PluginManager::getPluginsForExtension(const std::string& extensionPoint) {
    auto it = m_extensionPoints.find(extensionPoint);
    if (it != m_extensionPoints.end()) {
        std::vector<std::shared_ptr<IPlugin>> plugins;
        
        for (const auto& pair : it->second->getPlugins()) {
            plugins.push_back(pair.second);
        }
        
        return plugins;
    }
    
    return {};
}

bool PluginManager::checkDependencies(const PluginInfo& pluginInfo) {
    // 检查所有依赖是否已加载
    for (const auto& dependency : pluginInfo.dependencies) {
        bool dependencyFound = false;
        
        for (const auto& otherPlugin : m_plugins) {
            if (otherPlugin.name == dependency && otherPlugin.isLoaded) {
                dependencyFound = true;
                break;
            }
        }
        
        if (!dependencyFound) {
            std::cout << "Missing dependency: " << dependency << " for plugin: " << pluginInfo.name << std::endl;
            return false;
        }
    }
    
    return true;
}

std::shared_ptr<ExtensionPoint> PluginManager::getExtensionPoint(const std::string& name) {
    auto it = m_extensionPoints.find(name);
    if (it != m_extensionPoints.end()) {
        return it->second;
    }
    
    return nullptr;
}

void PluginManager::registerExtensionPoint(const std::string& name) {
    if (m_extensionPoints.find(name) == m_extensionPoints.end()) {
        m_extensionPoints[name] = std::make_unique<ExtensionPoint>(name);
    }
}

// 私有方法实现
std::vector<std::string> PluginManager::scanPluginDirectory(const std::string& pluginDirectory) {
    std::vector<std::string> pluginPaths;
    
    QDir dir(QString::fromStdString(pluginDirectory));
    if (!dir.exists()) {
        std::cout << "Plugin directory does not exist: " << pluginDirectory << std::endl;
        return pluginPaths;
    }
    
    QStringList filters;
    filters << "*.dll" << "*.so" << "*.dylib"; // Windows, Linux, macOS
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const auto& fileInfo : files) {
        pluginPaths.push_back(fileInfo.absoluteFilePath().toStdString());
    }
    
    return pluginPaths;
}

bool PluginManager::loadSinglePlugin(const std::string& pluginPath) {
    // 创建插件加载器
    auto loader = std::make_unique<QPluginLoader>(QString::fromStdString(pluginPath));
    
    // 检查是否是有效的插件
    if (loader->metaData().isEmpty()) {
        std::cout << "Invalid plugin metadata: " << pluginPath << std::endl;
        return false;
    }
    
    // 获取插件元数据
    QJsonObject metaData = loader->metaData().toObject();
    
    // 创建插件信息
    PluginInfo pluginInfo;
    pluginInfo.filePath = pluginPath;
    pluginInfo.name = metaData["name"].toString().toStdString();
    pluginInfo.version = metaData["version"].toString().toStdString();
    pluginInfo.description = metaData["description"].toString().toStdString();
    pluginInfo.author = metaData["author"].toString().toStdString();
    pluginInfo.isLoaded = false;
    
    // 获取依赖
    if (metaData.contains("dependencies")) {
        QJsonArray dependencies = metaData["dependencies"].toArray();
        for (const auto& dependency : dependencies) {
            pluginInfo.dependencies.push_back(dependency.toString().toStdString());
        }
    }
    
    // 检查插件是否已存在
    for (const auto& existingPlugin : m_plugins) {
        if (existingPlugin.name == pluginInfo.name) {
            std::cout << "Plugin already exists: " << pluginInfo.name << std::endl;
            return false;
        }
    }
    
    // 尝试加载插件
    QObject* pluginObject = loader->instance();
    if (!pluginObject) {
        std::cout << "Failed to create plugin instance: " << pluginPath << std::endl;
        return false;
    }
    
    // 转换为插件接口
    IPlugin* pluginInterface = qobject_cast<IPlugin*>(pluginObject);
    if (!pluginInterface) {
        std::cout << "Plugin does not implement IPlugin interface: " << pluginPath << std::endl;
        delete pluginObject;
        return false;
    }
    
    // 检查依赖
    if (!checkDependencies(pluginInfo)) {
        delete pluginObject;
        return false;
    }
    
    // 初始化插件
    if (!pluginInterface->initialize()) {
        std::cout << "Failed to initialize plugin: " << pluginPath << std::endl;
        delete pluginObject;
        return false;
    }
    
    // 设置插件信息
    pluginInfo.isLoaded = true;
    pluginInfo.instance = std::shared_ptr<IPlugin>(pluginInterface);
    
    // 添加到插件列表
    m_plugins.push_back(pluginInfo);
    m_loaders[pluginPath] = std::move(loader);
    
    // 初始化插件的扩展点
    initializePluginExtensions(pluginInfo.instance);
    
    std::cout << "Successfully loaded plugin: " << pluginInfo.name 
              << " v" << pluginInfo.version << std::endl;
    
    return true;
}

void PluginManager::registerStandardExtensionPoints() {
    // 注册标准扩展点
    registerExtensionPoint("chemistry.visualizer");
    registerExtensionPoint("ml.algorithm");
    registerExtensionPoint("data.importer");
    registerExtensionPoint("data.exporter");
    registerExtensionPoint("ui.component");
    registerExtensionPoint("report.generator");
    
    std::cout << "Registered standard extension points." << std::endl;
}

void PluginManager::initializePluginExtensions(std::shared_ptr<IPlugin> plugin) {
    // 获取插件支持的扩展点
    std::vector<std::string> extensionPoints = plugin->getExtensionPoints();
    
    // 将插件注册到相应的扩展点
    for (const auto& extensionPoint : extensionPoints) {
        auto extPoint = getExtensionPoint(extensionPoint);
        if (extPoint) {
            extPoint->registerPlugin(plugin->getName(), plugin);
            std::cout << "Registered plugin " << plugin->getName() 
                      << " to extension point " << extensionPoint << std::endl;
        }
    }
}

// PluginUtils 实现
bool PluginUtils::isVersionCompatible(
    const std::string& pluginVersion,
    const std::string& requiredVersion) {
    
    // 简化版本比较，实际应用中应使用更健壮的版本比较算法
    // 格式假设为 "major.minor.patch"
    std::vector<int> pluginVer;
    std::vector<int> requiredVer;
    
    std::stringstream ss1(pluginVersion);
    std::string token;
    
    while (std::getline(ss1, token, '.')) {
        try {
            pluginVer.push_back(std::stoi(token));
        } catch (...) {
            pluginVer.push_back(0);
        }
    }
    
    std::stringstream ss2(requiredVersion);
    while (std::getline(ss2, token, '.')) {
        try {
            requiredVer.push_back(std::stoi(token));
        } catch (...) {
            requiredVer.push_back(0);
        }
    }
    
    // 补齐版本号
    while (pluginVer.size() < 3) pluginVer.push_back(0);
    while (requiredVer.size() < 3) requiredVer.push_back(0);
    
    // 比较主版本号
    if (pluginVer[0] > requiredVer[0]) return true;
    if (pluginVer[0] < requiredVer[0]) return false;
    
    // 比较次版本号
    if (pluginVer[1] > requiredVer[1]) return true;
    if (pluginVer[1] < requiredVer[1]) return false;
    
    // 比较修订号
    if (pluginVer[2] >= requiredVer[2]) return true;
    return false;
}

std::string PluginUtils::formatPluginInfo(const PluginInfo& pluginInfo) {
    std::stringstream ss;
    ss << "Name: " << pluginInfo.name;
    ss << ", Version: " << pluginInfo.version;
    ss << ", Author: " << pluginInfo.author;
    ss << ", Status: " << (pluginInfo.isLoaded ? "Loaded" : "Not Loaded");
    
    if (!pluginInfo.dependencies.empty()) {
        ss << ", Dependencies: ";
        for (size_t i = 0; i < pluginInfo.dependencies.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << pluginInfo.dependencies[i];
        }
    }
    
    return ss.str();
}

} // namespace Plugins
} // namespace Core
} // namespace BondForge