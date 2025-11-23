#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <QPluginLoader>

namespace BondForge {
namespace Core {
namespace Plugins {

/**
 * @brief 插件接口
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    /**
     * @brief 插件名称
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief 插件版本
     */
    virtual std::string getVersion() const = 0;
    
    /**
     * @brief 插件描述
     */
    virtual std::string getDescription() const = 0;
    
    /**
     * @brief 插件作者
     */
    virtual std::string getAuthor() const = 0;
    
    /**
     * @brief 初始化插件
     * 
     * @return 是否成功
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief 关闭插件
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief 获取插件支持的扩展点
     * 
     * @return 扩展点列表
     */
    virtual std::vector<std::string> getExtensionPoints() const = 0;
};

/**
 * @brief 插件信息
 */
struct PluginInfo {
    std::string filePath;                 // 插件文件路径
    std::string name;                    // 插件名称
    std::string version;                 // 插件版本
    std::string description;              // 插件描述
    std::string author;                   // 插件作者
    std::vector<std::string> dependencies; // 依赖
    bool isLoaded;                       // 是否已加载
    std::shared_ptr<IPlugin> instance;   // 插件实例
};

/**
 * @brief 扩展点管理器
 */
class ExtensionPoint {
private:
    std::string m_name;
    std::map<std::string, std::shared_ptr<IPlugin>> m_plugins;
    
public:
    explicit ExtensionPoint(const std::string& name) : m_name(name) {}
    
    /**
     * @brief 获取扩展点名称
     */
    std::string getName() const { return m_name; }
    
    /**
     * @brief 注册插件
     * 
     * @param plugin 插件实例
     * @param pluginId 插件ID
     */
    void registerPlugin(const std::string& pluginId, std::shared_ptr<IPlugin> plugin) {
        m_plugins[pluginId] = plugin;
    }
    
    /**
     * @brief 取消注册插件
     * 
     * @param pluginId 插件ID
     */
    void unregisterPlugin(const std::string& pluginId) {
        m_plugins.erase(pluginId);
    }
    
    /**
     * @brief 获取所有注册的插件
     */
    std::map<std::string, std::shared_ptr<IPlugin>> getPlugins() const {
        return m_plugins;
    }
};

/**
 * @brief 插件管理器接口
 */
class IPluginManager {
public:
    virtual ~IPluginManager() = default;
    
    /**
     * @brief 扫描并加载插件
     * 
     * @param pluginDirectory 插件目录
     * @return 加载的插件数量
     */
    virtual int loadPlugins(const std::string& pluginDirectory) = 0;
    
    /**
     * @brief 卸载所有插件
     */
    virtual void unloadAllPlugins() = 0;
    
    /**
     * @brief 获取所有插件信息
     * 
     * @return 插件信息列表
     */
    virtual std::vector<PluginInfo> getAllPlugins() const = 0;
    
    /**
     * @brief 获取已加载的插件信息
     * 
     * @return 已加载的插件信息列表
     */
    virtual std::vector<PluginInfo> getLoadedPlugins() const = 0;
    
    /**
     * @brief 获取指定名称的插件
     * 
     * @param name 插件名称
     * @return 插件信息（如果存在）
     */
    virtual std::unique_ptr<PluginInfo> getPlugin(const std::string& name) const = 0;
    
    /**
     * @brief 获取指定扩展点的所有插件
     * 
     * @param extensionPoint 扩展点名称
     * @return 插件实例列表
     */
    virtual std::vector<std::shared_ptr<IPlugin>> getPluginsForExtension(const std::string& extensionPoint) = 0;
    
    /**
     * @brief 检查插件依赖
     * 
     * @param pluginInfo 插件信息
     * @return 是否满足依赖
     */
    virtual bool checkDependencies(const PluginInfo& pluginInfo) = 0;
};

/**
 * @brief 插件管理器实现类
 */
class PluginManager : public IPluginManager {
private:
    std::vector<PluginInfo> m_plugins;
    std::map<std::string, std::unique_ptr<ExtensionPoint>> m_extensionPoints;
    std::map<std::string, std::unique_ptr<QPluginLoader>> m_loaders;
    
    /**
     * @brief 扫描插件目录
     * 
     * @param pluginDirectory 插件目录
     * @return 插件文件路径列表
     */
    std::vector<std::string> scanPluginDirectory(const std::string& pluginDirectory);
    
    /**
     * @brief 加载单个插件
     * 
     * @param pluginPath 插件文件路径
     * @return 是否成功
     */
    bool loadSinglePlugin(const std::string& pluginPath);
    
    /**
     * @brief 注册标准扩展点
     */
    void registerStandardExtensionPoints();
    
    /**
     * @brief 初始化插件的扩展点
     * 
     * @param plugin 插件实例
     */
    void initializePluginExtensions(std::shared_ptr<IPlugin> plugin);
    
public:
    PluginManager();
    ~PluginManager();
    
    int loadPlugins(const std::string& pluginDirectory) override;
    void unloadAllPlugins() override;
    std::vector<PluginInfo> getAllPlugins() const override;
    std::vector<PluginInfo> getLoadedPlugins() const override;
    std::unique_ptr<PluginInfo> getPlugin(const std::string& name) const override;
    std::vector<std::shared_ptr<IPlugin>> getPluginsForExtension(const std::string& extensionPoint) override;
    bool checkDependencies(const PluginInfo& pluginInfo) override;
    
    // 扩展点管理
    std::shared_ptr<ExtensionPoint> getExtensionPoint(const std::string& name);
    void registerExtensionPoint(const std::string& name);
};

/**
 * @brief 插件事件
 */
enum class PluginEvent {
    Loaded,         // 插件已加载
    Unloaded,       // 插件已卸载
    Initialized,     // 插件已初始化
    Shutdown,        // 插件已关闭
    Error           // 插件发生错误
};

/**
 * @brief 事件监听器类型
 */
using PluginEventListener = std::function<void(const std::string& pluginName, PluginEvent event, const std::string& message)>;

/**
 * @brief 事件管理器
 */
class PluginEventManager {
private:
    std::vector<PluginEventListener> m_listeners;
    
public:
    /**
     * @brief 添加事件监听器
     * 
     * @param listener 监听器
     */
    void addListener(const PluginEventListener& listener) {
        m_listeners.push_back(listener);
    }
    
    /**
     * @brief 移除事件监听器
     * 
     * @param listener 监听器
     */
    void removeListener(const PluginEventListener& listener) {
        m_listeners.erase(
            std::remove(m_listeners.begin(), m_listeners.end(), listener),
            m_listeners.end()
        );
    }
    
    /**
     * @brief 触发事件
     * 
     * @param pluginName 插件名称
     * @param event 事件
     * @param message 消息
     */
    void fireEvent(
        const std::string& pluginName,
        PluginEvent event,
        const std::string& message) {
        
        for (const auto& listener : m_listeners) {
            listener(pluginName, event, message);
        }
    }
};

/**
 * @brief 插件工具
 */
class PluginUtils {
public:
    /**
     * @brief 检查插件兼容性
     * 
     * @param pluginVersion 插件版本
     * @param requiredVersion 需要的版本
     * @return 是否兼容
     */
    static bool isVersionCompatible(
        const std::string& pluginVersion,
        const std::string& requiredVersion);
    
    /**
     * @brief 格式化插件信息为字符串
     * 
     * @param pluginInfo 插件信息
     * @return 格式化字符串
     */
    static std::string formatPluginInfo(const PluginInfo& pluginInfo);
};

} // namespace Plugins
} // namespace Core
} // namespace BondForge