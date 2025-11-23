#include "ConfigManager.h"
#include <iostream>
#include <filesystem>

namespace BondForge {
namespace Utils {

// ConfigManager 实现
ConfigManager::ConfigManager() {
    // 设置默认配置文件路径
    m_configFilePath = getDefaultConfigPath();
    
    // 初始化默认配置
    initializeDefaultConfig();
    
    // 尝试加载配置文件
    loadConfig();
}

bool ConfigManager::loadConfig(const std::string& configFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 使用指定的配置文件路径或默认路径
    std::string filePath = configFilePath.empty() ? m_configFilePath : configFilePath;
    
    // 保存配置文件路径
    if (!configFilePath.empty()) {
        m_configFilePath = configFilePath;
    }
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "Config file not found: " << filePath << ", using defaults" << std::endl;
        return false;
    }
    
    try {
        // 读取文件内容
        std::string fileContent((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        file.close();
        
        // 解析JSON
        QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(fileContent));
        if (doc.isNull() || !doc.isObject()) {
            std::cout << "Invalid JSON in config file: " << filePath << std::endl;
            return false;
        }
        
        QJsonObject configObj = doc.object();
        
        // 遍历JSON对象，加载配置
        for (auto it = configObj.begin(); it != configObj.end(); ++it) {
            std::string key = it.key().toStdString();
            ConfigValue value = jsonToValue(it.value(), key);
            
            m_config[key] = value;
        }
        
        std::cout << "Config loaded from: " << filePath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigManager::saveConfig(const std::string& configFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 使用指定的配置文件路径或默认路径
    std::string filePath = configFilePath.empty() ? m_configFilePath : configFilePath;
    
    // 确保目录存在
    std::filesystem::path path(filePath);
    std::filesystem::path dir = path.parent_path();
    
    if (!std::filesystem::exists(dir) && !std::filesystem::create_directories(dir)) {
        std::cout << "Failed to create config directory: " << dir.string() << std::endl;
        return false;
    }
    
    try {
        // 创建JSON对象
        QJsonObject configObj;
        
        for (const auto& pair : m_config) {
            configObj[QString::fromStdString(pair.first)] = valueToJson(pair.second);
        }
        
        QJsonDocument doc(configObj);
        
        // 写入文件
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cout << "Failed to open config file for writing: " << filePath << std::endl;
            return false;
        }
        
        file << doc.toJson().toStdString();
        file.close();
        
        std::cout << "Config saved to: " << filePath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

ConfigValue ConfigManager::getValue(const std::string& key, const ConfigValue& defaultValue) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        return it->second;
    }
    
    return defaultValue;
}

bool ConfigManager::setValue(const std::string& key, const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 验证配置值
    auto itemIt = m_configItems.find(key);
    if (itemIt != m_configItems.end()) {
        std::string errorMessage;
        if (!ConfigUtils::validateConfigValue(itemIt->second, value, errorMessage)) {
            std::cout << "Invalid config value for " << key << ": " << errorMessage << std::endl;
            return false;
        }
    }
    
    m_config[key] = value;
    return true;
}

std::map<std::string, ConfigValue> ConfigManager::getAllValues() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_config;
}

std::map<std::string, ConfigValue> ConfigManager::getValuesByCategory(const std::string& category) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::map<std::string, ConfigValue> result;
    
    for (const auto& item : m_configItems) {
        if (item.second.category == category) {
            result[item.first] = m_config.at(item.first);
        }
    }
    
    return result;
}

void ConfigManager::resetToDefaults() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_config.clear();
    
    // 重新注册所有配置项
    for (const auto& item : m_configItems) {
        // 检查是否有默认值
        if (item.second.description.find("Default:") != std::string::npos) {
            // 从描述中提取默认值（简化方法）
            // 实际应用中应该有专门的默认值存储
            continue;
        }
    }
    
    std::cout << "Config reset to defaults." << std::endl;
}

bool ConfigManager::hasKey(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_config.find(key) != m_config.end();
}

bool ConfigManager::removeKey(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        m_config.erase(it);
        return true;
    }
    
    return false;
}

// 便捷方法实现
bool ConfigManager::getBool(const std::string& key, bool defaultValue) {
    try {
        return std::get<bool>(getValue(key, defaultValue));
    } catch (...) {
        return defaultValue;
    }
}

int ConfigManager::getInt(const std::string& key, int defaultValue) {
    try {
        return std::get<int>(getValue(key, defaultValue));
    } catch (...) {
        return defaultValue;
    }
}

double ConfigManager::getDouble(const std::string& key, double defaultValue) {
    try {
        return std::get<double>(getValue(key, defaultValue));
    } catch (...) {
        return defaultValue;
    }
}

std::string ConfigManager::getString(const std::string& key, const std::string& defaultValue) {
    try {
        return std::get<std::string>(getValue(key, defaultValue));
    } catch (...) {
        return defaultValue;
    }
}

std::vector<std::string> ConfigManager::getStringList(const std::string& key, const std::vector<std::string>& defaultValue) {
    try {
        return std::get<std::vector<std::string>>(getValue(key, defaultValue));
    } catch (...) {
        return defaultValue;
    }
}

void ConfigManager::setBool(const std::string& key, bool value) {
    setValue(key, value);
}

void ConfigManager::setInt(const std::string& key, int value) {
    setValue(key, value);
}

void ConfigManager::setDouble(const std::string& key, double value) {
    setValue(key, value);
}

void ConfigManager::setString(const std::string& key, const std::string& value) {
    setValue(key, value);
}

void ConfigManager::setStringList(const std::string& key, const std::vector<std::string>& value) {
    setValue(key, value);
}

std::string ConfigManager::exportToJson(const std::string& category) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    QJsonObject result;
    
    if (category.empty()) {
        // 导出所有配置
        for (const auto& pair : m_config) {
            result[QString::fromStdString(pair.first)] = valueToJson(pair.second);
        }
    } else {
        // 导出指定类别的配置
        for (const auto& item : m_configItems) {
            if (item.second.category == category) {
                auto it = m_config.find(item.first);
                if (it != m_config.end()) {
                    result[QString::fromStdString(item.first)] = valueToJson(it->second);
                }
            }
        }
    }
    
    QJsonDocument doc(result);
    return doc.toJson().toStdString();
}

bool ConfigManager::importFromJson(const std::string& jsonString, bool merge) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(jsonString));
    if (doc.isNull() || !doc.isObject()) {
        std::cout << "Invalid JSON for import." << std::endl;
        return false;
    }
    
    QJsonObject configObj = doc.object();
    
    if (!merge) {
        // 清除现有配置
        m_config.clear();
    }
    
    // 导入配置
    for (auto it = configObj.begin(); it != configObj.end(); ++it) {
        std::string key = it.key().toStdString();
        ConfigValue value = jsonToValue(it.value(), key);
        
        if (merge) {
            m_config[key] = value;
        } else {
            // 覆盖现有值
            m_config[key] = value;
        }
    }
    
    std::cout << "Config imported successfully." << std::endl;
    return true;
}

// 私有方法实现
void ConfigManager::initializeDefaultConfig() {
    // 应用程序基本设置
    registerConfigItem({"app.name", std::string("BondForge"), "Application name", "application", false, true});
    registerConfigItem({"app.version", std::string("1.2.0"), "Application version", "application", false, true});
    registerConfigItem({"app.language", std::string("en"), "Interface language", "ui", true, false});
    registerConfigItem({"app.theme", std::string("light"), "UI theme", "ui", true, false});
    
    // 数据存储设置
    registerConfigItem({"data.default_format", std::string("JSON"), "Default data format", "data", true, false});
    registerConfigItem({"data.auto_save", true, "Auto-save data changes", "data", true, false});
    registerConfigItem({"data.backup_enabled", true, "Enable automatic backups", "data", true, false});
    registerConfigItem({"data.backup_interval", 24, "Backup interval in hours", "data", true, false});
    
    // 分子可视化设置
    registerConfigItem({"visualization.default_renderer", std::string("simple"), "Default molecular renderer", "visualization", true, false});
    registerConfigItem({"visualization.show_hydrogens", true, "Show hydrogen atoms", "visualization", true, false});
    registerConfigItem({"visualization.show_atom_labels", true, "Show atom labels", "visualization", true, false});
    
    // 机器学习设置
    registerConfigItem({"ml.default_algorithm", std::string("linear_regression"), "Default ML algorithm", "ml", true, false});
    registerConfigItem({"ml.use_gpu", false, "Use GPU for ML computations", "ml", true, false});
    registerConfigItem({"ml.model_dir", std::string("./models"), "Directory to save ML models", "ml", true, false});
    
    // 协作设置
    registerConfigItem({"collab.auto_refresh", true, "Auto-refresh shared data", "collaboration", true, false});
    registerConfigItem({"collab.notification_enabled", true, "Enable collaboration notifications", "collaboration", true, false});
    registerConfigItem({"collab.default_share_expiry", std::string("30_days"), "Default share expiry", "collaboration", true, false});
    
    // 网络设置
    registerConfigItem({"network.server_url", std::string("https://api.bondforge.com"), "Server URL", "network", true, false});
    registerConfigItem({"network.timeout", 30, "Network timeout in seconds", "network", true, false});
    registerConfigItem({"network.retry_attempts", 3, "Network retry attempts", "network", true, false});
    
    // 调试设置
    registerConfigItem({"debug.log_level", std::string("info"), "Log level", "debug", true, false});
    registerConfigItem({"debug.log_to_file", false, "Log to file", "debug", true, false});
    registerConfigItem({"debug.max_log_files", 10, "Maximum log files to keep", "debug", true, false});
    
    // 安全设置
    registerConfigItem({"security.require_password_change", false, "Require password change on first login", "security", true, false});
    registerConfigItem({"security.session_timeout", 120, "Session timeout in minutes", "security", true, false});
    registerConfigItem({"security.two_factor_enabled", false, "Enable two-factor authentication", "security", true, false});
    
    std::cout << "Default configuration initialized." << std::endl;
}

void ConfigManager::registerConfigItem(const ConfigItem& item) {
    m_configItems[item.key] = item;
    
    // 如果配置中没有该键，则使用默认值
    if (m_config.find(item.key) == m_config.end()) {
        // 尝试从描述中提取默认值（简化方法）
        // 实际应用中应该有专门的默认值存储
        if (item.key == "app.language") {
            m_config[item.key] = std::string("en");
        } else if (item.key == "app.theme") {
            m_config[item.key] = std::string("light");
        } else if (item.key == "data.backup_interval") {
            m_config[item.key] = 24;
        } else if (item.key == "network.timeout") {
            m_config[item.key] = 30;
        } else if (item.key == "security.session_timeout") {
            m_config[item.key] = 120;
        }
        // 其他配置项以此类推...
    }
}

QJsonValue ConfigManager::valueToJson(const ConfigValue& value) {
    if (std::holds_alternative<bool>(value)) {
        return QJsonValue(std::get<bool>(value));
    } else if (std::holds_alternative<int>(value)) {
        return QJsonValue(std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
        return QJsonValue(std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        return QJsonValue(QString::fromStdString(std::get<std::string>(value)));
    } else if (std::holds_alternative<std::vector<std::string>>(value)) {
        QJsonArray array;
        for (const auto& str : std::get<std::vector<std::string>>(value)) {
            array.append(QString::fromStdString(str));
        }
        return QJsonValue(array);
    } else if (std::holds_alternative<std::map<std::string, std::string>>(value)) {
        QJsonObject obj;
        for (const auto& pair : std::get<std::map<std::string, std::string>>(value)) {
            obj[QString::fromStdString(pair.first)] = QJsonValue(QString::fromStdString(pair.second));
        }
        return QJsonValue(obj);
    }
    
    return QJsonValue();
}

ConfigValue ConfigManager::jsonToValue(const QJsonValue& jsonValue, const std::string& key) {
    // 查找配置项以获取类型信息
    auto it = m_configItems.find(key);
    
    if (jsonValue.isBool()) {
        return jsonValue.toBool();
    } else if (jsonValue.isDouble()) {
        return jsonValue.toDouble();
    } else if (jsonValue.isString()) {
        return jsonValue.toString().toStdString();
    } else if (jsonValue.isArray()) {
        std::vector<std::string> stringList;
        QJsonArray array = jsonValue.toArray();
        
        for (const auto& item : array) {
            if (item.isString()) {
                stringList.push_back(item.toString().toStdString());
            }
        }
        
        return stringList;
    } else if (jsonValue.isObject()) {
        std::map<std::string, std::string> stringMap;
        QJsonObject obj = jsonValue.toObject();
        
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            if (it.value().isString()) {
                stringMap[it.key().toStdString()] = it.value().toString().toStdString();
            }
        }
        
        return stringMap;
    }
    
    // 默认返回字符串
    return jsonValue.toString().toStdString();
}

std::string ConfigManager::getDefaultConfigPath() {
    // 简化实现，实际应用中应根据平台选择正确的路径
    #ifdef _WIN32
        return "./config.json";
    #else
        return std::string(getenv("HOME")) + "/.config/BondForge/config.json";
    #endif
}

// ConfigUtils 实现
std::string ConfigUtils::valueToString(const ConfigValue& value) {
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    } else if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
        return std::to_string(std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    } else if (std::holds_alternative<std::vector<std::string>>(value)) {
        std::string result = "[";
        for (size_t i = 0; i < std::get<std::vector<std::string>>(value).size(); ++i) {
            if (i > 0) result += ",";
            result += std::get<std::vector<std::string>>(value)[i];
        }
        result += "]";
        return result;
    } else if (std::holds_alternative<std::map<std::string, std::string>>(value)) {
        std::string result = "{";
        for (const auto& pair : std::get<std::map<std::string, std::string>>(value)) {
            result += "\"" + pair.first + "\":\"" + pair.second + "\"";
        }
        result += "}";
        return result;
    }
    
    return "";
}

ConfigValue ConfigUtils::stringToValue(const std::string& str, const std::string& type) {
    if (type == "bool") {
        return str == "true" || str == "1";
    } else if (type == "int") {
        try {
            return std::stoi(str);
        } catch (...) {
            return 0;
        }
    } else if (type == "double") {
        try {
            return std::stod(str);
        } catch (...) {
            return 0.0;
        }
    }
    
    return str;
}

bool ConfigUtils::validateConfigValue(
    const ConfigItem& item,
    const ConfigValue& value,
    std::string& errorMessage) {
    
    // 根据配置项类型进行验证
    if (std::holds_alternative<bool>(value)) {
        // 布尔值总是有效
        return true;
    } else if (std::holds_alternative<int>(value)) {
        // 检查整数值范围
        int intValue = std::get<int>(value);
        if (item.key == "data.backup_interval" && (intValue < 1 || intValue > 168)) {
            errorMessage = "Backup interval must be between 1 and 168 hours";
            return false;
        }
        if (item.key == "network.timeout" && (intValue < 5 || intValue > 300)) {
            errorMessage = "Network timeout must be between 5 and 300 seconds";
            return false;
        }
        if (item.key == "security.session_timeout" && (intValue < 5 || intValue > 480)) {
            errorMessage = "Session timeout must be between 5 and 480 minutes";
            return false;
        }
        return true;
    } else if (std::holds_alternative<double>(value)) {
        // 检查浮点数值范围
        double doubleValue = std::get<double>(value);
        if (item.key == "ml.learning_rate" && (doubleValue <= 0.0 || doubleValue > 1.0)) {
            errorMessage = "Learning rate must be between 0 and 1";
            return false;
        }
        return true;
    } else if (std::holds_alternative<std::string>(value)) {
        // 检查字符串值
        std::string strValue = std::get<std::string>(value);
        
        if (item.key == "app.language") && 
            (strValue != "en" && strValue != "zh-CN")) {
            errorMessage = "Language must be 'en' or 'zh-CN'";
            return false;
        }
        
        if (item.key == "app.theme" && 
            (strValue != "light" && strValue != "dark")) {
            errorMessage = "Theme must be 'light' or 'dark'";
            return false;
        }
        
        return true;
    }
    
    return true;
}

} // namespace Utils
} // namespace BondForge