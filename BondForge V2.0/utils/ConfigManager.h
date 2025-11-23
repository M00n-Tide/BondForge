#pragma once

#include <string>
#include <map>
#include <variant>
#include <fstream>
#include <mutex>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace BondForge {
namespace Utils {

/**
 * @brief 配置值类型
 */
using ConfigValue = std::variant<
    bool,
    int,
    double,
    std::string,
    std::vector<std::string>,
    std::map<std::string, std::string>
>;

/**
 * @brief 配置项
 */
struct ConfigItem {
    std::string key;                    // 配置键
    ConfigValue value;                  // 配置值
    std::string description;             // 描述
    std::string category;               // 类别
    bool isUserConfig;                  // 是否用户配置
    bool isRequired;                    // 是否必需
};

/**
 * @brief 配置管理器接口
 */
class IConfigManager {
public:
    virtual ~IConfigManager() = default;
    
    /**
     * @brief 加载配置
     * 
     * @param configFilePath 配置文件路径
     * @return 是否成功
     */
    virtual bool loadConfig(const std::string& configFilePath = "") = 0;
    
    /**
     * @brief 保存配置
     * 
     * @param configFilePath 配置文件路径
     * @return 是否成功
     */
    virtual bool saveConfig(const std::string& configFilePath = "") = 0;
    
    /**
     * @brief 获取配置值
     * 
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    virtual ConfigValue getValue(const std::string& key, const ConfigValue& defaultValue = {}) = 0;
    
    /**
     * @brief 设置配置值
     * 
     * @param key 配置键
     * @param value 配置值
     * @return 是否成功
     */
    virtual bool setValue(const std::string& key, const ConfigValue& value) = 0;
    
    /**
     * @brief 获取所有配置
     * 
     * @return 配置映射
     */
    virtual std::map<std::string, ConfigValue> getAllValues() = 0;
    
    /**
     * @brief 获取指定类别的配置
     * 
     * @param category 类别
     * @return 配置映射
     */
    virtual std::map<std::string, ConfigValue> getValuesByCategory(const std::string& category) = 0;
    
    /**
     * @brief 重置为默认配置
     */
    virtual void resetToDefaults() = 0;
    
    /**
     * @brief 检查配置项是否存在
     * 
     * @param key 配置键
     * @return 是否存在
     */
    virtual bool hasKey(const std::string& key) = 0;
    
    /**
     * @brief 删除配置项
     * 
     * @param key 配置键
     * @return 是否成功
     */
    virtual bool removeKey(const std::string& key) = 0;
};

/**
 * @brief 配置管理器实现类
 */
class ConfigManager : public IConfigManager {
private:
    std::map<std::string, ConfigValue> m_config;
    std::map<std::string, ConfigItem> m_configItems;
    std::string m_configFilePath;
    mutable std::mutex m_mutex;
    
    /**
     * @brief 初始化默认配置
     */
    void initializeDefaultConfig();
    
    /**
     * @brief 注册配置项
     * 
     * @param item 配置项
     */
    void registerConfigItem(const ConfigItem& item);
    
    /**
     * @brief 将配置值转换为JSON值
     * 
     * @param value 配置值
     * @return JSON值
     */
    QJsonValue valueToJson(const ConfigValue& value);
    
    /**
     * @brief 将JSON值转换为配置值
     * 
     * @param jsonValue JSON值
     * @return 配置值
     */
    ConfigValue jsonToValue(const QJsonValue& jsonValue, const std::string& key);
    
    /**
     * @brief 获取默认配置文件路径
     * 
     * @return 配置文件路径
     */
    std::string getDefaultConfigPath();
    
public:
    ConfigManager();
    
    bool loadConfig(const std::string& configFilePath = "") override;
    bool saveConfig(const std::string& configFilePath = "") override;
    ConfigValue getValue(const std::string& key, const ConfigValue& defaultValue = {}) override;
    bool setValue(const std::string& key, const ConfigValue& value) override;
    std::map<std::string, ConfigValue> getAllValues() override;
    std::map<std::string, ConfigValue> getValuesByCategory(const std::string& category) override;
    void resetToDefaults() override;
    bool hasKey(const std::string& key) override;
    bool removeKey(const std::string& key) override;
    
    /**
     * @brief 便捷方法：获取布尔值
     */
    bool getBool(const std::string& key, bool defaultValue = false);
    
    /**
     * @brief 便捷方法：获取整数值
     */
    int getInt(const std::string& key, int defaultValue = 0);
    
    /**
     * @brief 便捷方法：获取浮点数值
     */
    double getDouble(const std::string& key, double defaultValue = 0.0);
    
    /**
     * @brief 便捷方法：获取字符串值
     */
    std::string getString(const std::string& key, const std::string& defaultValue = "");
    
    /**
     * @brief 便捷方法：获取字符串列表
     */
    std::vector<std::string> getStringList(const std::string& key, const std::vector<std::string>& defaultValue = {});
    
    /**
     * @brief 便捷方法：设置布尔值
     */
    void setBool(const std::string& key, bool value);
    
    /**
     * @brief 便捷方法：设置整数值
     */
    void setInt(const std::string& key, int value);
    
    /**
     * @brief 便捷方法：设置浮点数值
     */
    void setDouble(const std::string& key, double value);
    
    /**
     * @brief 便捷方法：设置字符串值
     */
    void setString(const std::string& key, const std::string& value);
    
    /**
     * @brief 便捷方法：设置字符串列表
     */
    void setStringList(const std::string& key, const std::vector<std::string>& value);
    
    /**
     * @brief 导出配置为JSON字符串
     * 
     * @param category 类别过滤（空表示导出所有）
     * @return JSON字符串
     */
    std::string exportToJson(const std::string& category = "");
    
    /**
     * @brief 从JSON字符串导入配置
     * 
     * @param jsonString JSON字符串
     * @param merge 是否与现有配置合并
     * @return 是否成功
     */
    bool importFromJson(const std::string& jsonString, bool merge = false);
};

/**
 * @brief 配置工具
 */
class ConfigUtils {
public:
    /**
     * @brief 将配置值转换为字符串
     * 
     * @param value 配置值
     * @return 字符串表示
     */
    static std::string valueToString(const ConfigValue& value);
    
    /**
     * @brief 从字符串创建配置值
     * 
     * @param str 字符串
     * @param type 值类型提示
     * @return 配置值
     */
    static ConfigValue stringToValue(const std::string& str, const std::string& type = "auto");
    
    /**
     * @brief 验证配置值
     * 
     * @param item 配置项
     * @param value 配置值
     * @param errorMessage 错误消息
     * @return 是否有效
     */
    static bool validateConfigValue(
        const ConfigItem& item,
        const ConfigValue& value,
        std::string& errorMessage);
};

} // namespace Utils
} // namespace BondForge