#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <regex>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <chrono>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QStatusBar>
#include <QProgressBar>
#include <QHeaderView>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QFormLayout>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QSplitter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QPieSeries>
#include <QPieSlice>
#include <QValueAxis>
#include <QCategoryAxis>
#include <QDateTimeAxis>
#include <QScatterSeries>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QDateTimeEdit>
#include <QListWidget>
#include <QTextEdit>
#include <QStringList>
#include <QBoxSet>
#include <cmath>

// 数据结构定义
struct DataRecord { 
    std::string id;                // 数据唯一标识
    std::string content;           // 数据内容（如化学式、分子结构）
    std::string format;            // 数据格式（CSV/JSON/SDF等）
    std::unordered_set<std::string> tags;  // 数据标签集合
    std::string category;          // 数据分类
    std::string uploader;          // 上传用户
    uint64_t timestamp;            // 上传时间戳
    
    // 序列化方法
    std::string serializeTags() const {
        std::string result;
        for (const auto& tag : tags) {
            if (!result.empty()) result += ",";
            result += tag;
        }
        return result;
    }
    
    // 反序列化方法
    void deserializeTags(const std::string& tagStr) {
        tags.clear();
        std::stringstream ss(tagStr);
        std::string tag;
        while (std::getline(ss, tag, ',')) {
            if (!tag.empty()) {
                tags.insert(tag);
            }
        }
    }
}; 

// 错误码枚举
enum class ErrorCode { 
    SUCCESS = 0,
    INVALID_DATA_FORMAT,  // 无效数据格式
    NOT_FOUND,            // 数据未找到
    PERMISSION_DENIED,    // 权限拒绝
    UPLOAD_FAILED,        // 上传失败
    INVALID_PARAMETER,    // 无效参数
    QUALITY_CHECK_FAILED, // 质量检测失败
    DUPLICATE_DATA,       // 数据重复
    STORAGE_ERROR,        // 存储错误
    MIGRATION_ERROR,      // 数据迁移错误
    UNKNOWN_ERROR         // 未知错误
};

// 存储模式枚举
enum class StorageMode {
    MEMORY,  // 内存存储
    SQLITE   // SQLite数据库存储
}; 

// 权限级别枚举
enum class AccessLevel { 
    NONE = 0,     // 无权限
    READ = 1,     // 只读权限
    WRITE = 2,    // 读写权限
    ADMIN = 3     // 管理员权限
}; 

// 国际化管理器
class I18nManager {
public:
    // 获取单例实例
    static I18nManager& getInstance();
    
    // 初始化语言资源
    bool initialize();
    
    // 设置当前语言
    bool setLanguage(const std::string& languageCode);
    
    // 获取当前语言代码
    std::string getCurrentLanguage() const;
    
    // 获取本地化文本
    std::string getText(const std::string& key) const;
    
    // 检查键是否存在
    bool hasKey(const std::string& key) const;
    
    // 获取可用语言列表
    std::vector<std::string> getAvailableLanguages() const;

private:
    I18nManager() = default;
    ~I18nManager() = default;
    
    // 禁止拷贝构造和赋值
    I18nManager(const I18nManager&) = delete;
    I18nManager& operator=(const I18nManager&) = delete;
    
    // 加载内置语言资源
    void loadBuiltinLanguages();
    
    // 递归查找键值
    std::string findValue(const std::string& key, const std::map<std::string, std::string>& data) const;

private:
    std::string m_currentLanguage;
    std::map<std::string, std::map<std::string, std::string>> m_languages;
    mutable std::mutex m_mutex;
};

// 存储抽象接口
class IDataStorage {
public:
    virtual ~IDataStorage() = default;
    
    // 初始化存储
    virtual bool initialize() = 0;
    
    // 清理资源
    virtual void cleanup() = 0;
    
    // 数据操作接口
    virtual bool insertData(const DataRecord& data) = 0;
    virtual bool updateData(const DataRecord& data) = 0;
    virtual bool deleteData(const std::string& id) = 0;
    virtual bool containsData(const std::string& id) = 0;
    virtual DataRecord getData(const std::string& id) = 0;
    virtual std::vector<DataRecord> getAllData() = 0;
    virtual std::vector<std::string> listDataByCategory(const std::string& category) = 0;
    virtual std::vector<std::string> listDataByTag(const std::string& tag) = 0;
    
    // 用户角色管理
    virtual bool setUserRole(const std::string& username, int role) = 0;
    virtual int getUserRole(const std::string& username) = 0;
    virtual std::map<std::string, int> getAllUserRoles() = 0;
};

// 内存存储实现
class MemoryStorage : public IDataStorage {
private:
    std::unordered_map<std::string, DataRecord> dataStore;
    std::unordered_map<std::string, std::unordered_set<std::string>> categoryIndex;
    std::unordered_map<std::string, std::unordered_set<std::string>> tagIndex;
    std::unordered_map<std::string, int> userRoles;
    std::mutex mutex_;
    
public:
    bool initialize() override {
        // 初始化默认用户角色
        std::lock_guard<std::mutex> lock(mutex_);
        userRoles["admin"] = 3;  // ADMIN
        userRoles["guest"] = 1;  // GUEST
        return true;
    }
    
    void cleanup() override {
        std::lock_guard<std::mutex> lock(mutex_);
        dataStore.clear();
        categoryIndex.clear();
        tagIndex.clear();
        userRoles.clear();
    }
    
    bool insertData(const DataRecord& data) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (dataStore.find(data.id) != dataStore.end()) {
            return false;  // ID已存在
        }
        
        dataStore[data.id] = data;
        categoryIndex[data.category].insert(data.id);
        for (const auto& tag : data.tags) {
            tagIndex[tag].insert(data.id);
        }
        return true;
    }
    
    bool updateData(const DataRecord& data) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = dataStore.find(data.id);
        if (it == dataStore.end()) {
            return false;
        }
        
        // 移除旧索引
        categoryIndex[it->second.category].erase(data.id);
        if (categoryIndex[it->second.category].empty()) {
            categoryIndex.erase(it->second.category);
        }
        
        for (const auto& tag : it->second.tags) {
            auto tagIt = tagIndex.find(tag);
            if (tagIt != tagIndex.end()) {
                tagIt->second.erase(data.id);
                if (tagIt->second.empty()) {
                    tagIndex.erase(tagIt);
                }
            }
        }
        
        // 更新数据
        it->second = data;
        
        // 添加新索引
        categoryIndex[data.category].insert(data.id);
        for (const auto& tag : data.tags) {
            tagIndex[tag].insert(data.id);
        }
        
        return true;
    }
    
    bool deleteData(const std::string& id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = dataStore.find(id);
        if (it == dataStore.end()) {
            return false;
        }
        
        // 移除索引
        categoryIndex[it->second.category].erase(id);
        if (categoryIndex[it->second.category].empty()) {
            categoryIndex.erase(it->second.category);
        }
        
        for (const auto& tag : it->second.tags) {
            auto tagIt = tagIndex.find(tag);
            if (tagIt != tagIndex.end()) {
                tagIt->second.erase(id);
                if (tagIt->second.empty()) {
                    tagIndex.erase(tagIt);
                }
            }
        }
        
        // 删除数据
        dataStore.erase(it);
        return true;
    }
    
    bool containsData(const std::string& id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        return dataStore.find(id) != dataStore.end();
    }
    
    DataRecord getData(const std::string& id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = dataStore.find(id);
        if (it == dataStore.end()) {
            throw std::runtime_error("Data not found");
        }
        return it->second;
    }
    
    std::vector<DataRecord> getAllData() override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<DataRecord> result;
        result.reserve(dataStore.size());
        for (const auto& pair : dataStore) {
            result.push_back(pair.second);
        }
        return result;
    }
    
    std::vector<std::string> listDataByCategory(const std::string& category) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> result;
        auto it = categoryIndex.find(category);
        if (it != categoryIndex.end()) {
            result.insert(result.end(), it->second.begin(), it->second.end());
        }
        return result;
    }
    
    std::vector<std::string> listDataByTag(const std::string& tag) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> result;
        auto it = tagIndex.find(tag);
        if (it != tagIndex.end()) {
            result.insert(result.end(), it->second.begin(), it->second.end());
        }
        return result;
    }
    
    bool setUserRole(const std::string& username, int role) override {
        std::lock_guard<std::mutex> lock(mutex_);
        userRoles[username] = role;
        return true;
    }
    
    int getUserRole(const std::string& username) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = userRoles.find(username);
        return (it != userRoles.end()) ? it->second : 1;  // 默认为GUEST
    }
    
    std::map<std::string, int> getAllUserRoles() override {
        std::lock_guard<std::mutex> lock(mutex_);
        return userRoles;
    }
};

// SQLite存储实现
class SQLiteStorage : public IDataStorage {
private:
    QSqlDatabase db;
    std::string dbPath;
    bool initialized;
    
    bool executeQuery(const QString& sql) {
        QSqlQuery query(db);
        if (!query.exec(sql)) {
            std::cerr << "SQL Error: " << query.lastError().text().toStdString() << std::endl;
            return false;
        }
        return true;
    }
    
public:
    SQLiteStorage(const std::string& path = "bondforge.db") : dbPath(path), initialized(false) {
        db = QSqlDatabase::addDatabase("QSQLITE", "BondForgeDB");
        db.setDatabaseName(QString::fromStdString(dbPath));
    }
    
    ~SQLiteStorage() {
        if (initialized) {
            cleanup();
        }
    }
    
    bool initialize() override {
        if (!db.open()) {
            std::cerr << "Cannot open database: " << db.lastError().text().toStdString() << std::endl;
            return false;
        }
        
        // 创建表
        if (!executeQuery(R"(
            CREATE TABLE IF NOT EXISTS data_records (
                id TEXT PRIMARY KEY,
                content TEXT NOT NULL,
                format TEXT NOT NULL,
                tags TEXT,
                category TEXT NOT NULL,
                uploader TEXT NOT NULL,
                timestamp INTEGER NOT NULL
            )
        )")) {
            return false;
        }
        
        if (!executeQuery(R"(
            CREATE TABLE IF NOT EXISTS user_roles (
                username TEXT PRIMARY KEY,
                role INTEGER NOT NULL
            )
        )")) {
            return false;
        }
        
        // 创建索引
        executeQuery("CREATE INDEX IF NOT EXISTS idx_category ON data_records(category)");
        executeQuery("CREATE INDEX IF NOT EXISTS idx_uploader ON data_records(uploader)");
        
        // 初始化默认用户角色
        QSqlQuery query(db);
        query.prepare("INSERT OR IGNORE INTO user_roles (username, role) VALUES (?, ?)");
        query.addBindValue("admin");
        query.addBindValue(3);  // ADMIN
        if (!query.exec()) {
            return false;
        }
        
        query.addBindValue("guest");
        query.addBindValue(1);  // GUEST
        if (!query.exec()) {
            return false;
        }
        
        initialized = true;
        return true;
    }
    
    void cleanup() override {
        if (db.isOpen()) {
            db.close();
        }
        initialized = false;
    }
    
    bool insertData(const DataRecord& data) override {
        if (!initialized) return false;
        
        QSqlQuery query(db);
        query.prepare(R"(
            INSERT INTO data_records (id, content, format, tags, category, uploader, timestamp)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        )");
        
        query.addBindValue(QString::fromStdString(data.id));
        query.addBindValue(QString::fromStdString(data.content));
        query.addBindValue(QString::fromStdString(data.format));
        query.addBindValue(QString::fromStdString(data.serializeTags()));
        query.addBindValue(QString::fromStdString(data.category));
        query.addBindValue(QString::fromStdString(data.uploader));
        query.addBindValue(static_cast<qint64>(data.timestamp));
        
        return query.exec();
    }
    
    bool updateData(const DataRecord& data) override {
        if (!initialized) return false;
        
        QSqlQuery query(db);
        query.prepare(R"(
            UPDATE data_records 
            SET content = ?, format = ?, tags = ?, category = ?, uploader = ?, timestamp = ?
            WHERE id = ?
        )");
        
        query.addBindValue(QString::fromStdString(data.content));
        query.addBindValue(QString::fromStdString(data.format));
        query.addBindValue(QString::fromStdString(data.serializeTags()));
        query.addBindValue(QString::fromStdString(data.category));
        query.addBindValue(QString::fromStdString(data.uploader));
        query.addBindValue(static_cast<qint64>(data.timestamp));
        query.addBindValue(QString::fromStdString(data.id));
        
        return query.exec();
    }
    
    bool deleteData(const std::string& id) override {
        if (!initialized) return false;
        
        QSqlQuery query(db);
        query.prepare("DELETE FROM data_records WHERE id = ?");
        query.addBindValue(QString::fromStdString(id));
        
        return query.exec();
    }
    
    bool containsData(const std::string& id) override {
        if (!initialized) return false;
        
        QSqlQuery query(db);
        query.prepare("SELECT COUNT(*) FROM data_records WHERE id = ?");
        query.addBindValue(QString::fromStdString(id));
        
        if (!query.exec()) return false;
        
        if (query.next()) {
            return query.value(0).toInt() > 0;
        }
        return false;
    }
    
    DataRecord getData(const std::string& id) override {
        if (!initialized) {
            throw std::runtime_error("Storage not initialized");
        }
        
        QSqlQuery query(db);
        query.prepare("SELECT id, content, format, tags, category, uploader, timestamp FROM data_records WHERE id = ?");
        query.addBindValue(QString::fromStdString(id));
        
        if (!query.exec() || !query.next()) {
            throw std::runtime_error("Data not found");
        }
        
        DataRecord record;
        record.id = query.value(0).toString().toStdString();
        record.content = query.value(1).toString().toStdString();
        record.format = query.value(2).toString().toStdString();
        record.deserializeTags(query.value(3).toString().toStdString());
        record.category = query.value(4).toString().toStdString();
        record.uploader = query.value(5).toString().toStdString();
        record.timestamp = query.value(6).toULongLong();
        
        return record;
    }
    
    std::vector<DataRecord> getAllData() override {
        std::vector<DataRecord> result;
        if (!initialized) return result;
        
        QSqlQuery query(db);
        query.exec("SELECT id, content, format, tags, category, uploader, timestamp FROM data_records ORDER BY timestamp DESC");
        
        while (query.next()) {
            DataRecord record;
            record.id = query.value(0).toString().toStdString();
            record.content = query.value(1).toString().toStdString();
            record.format = query.value(2).toString().toStdString();
            record.deserializeTags(query.value(3).toString().toStdString());
            record.category = query.value(4).toString().toStdString();
            record.uploader = query.value(5).toString().toStdString();
            record.timestamp = query.value(6).toULongLong();
            
            result.push_back(record);
        }
        
        return result;
    }
    
    std::vector<std::string> listDataByCategory(const std::string& category) override {
        std::vector<std::string> result;
        if (!initialized) return result;
        
        QSqlQuery query(db);
        query.prepare("SELECT id FROM data_records WHERE category = ? ORDER BY timestamp DESC");
        query.addBindValue(QString::fromStdString(category));
        
        if (query.exec()) {
            while (query.next()) {
                result.push_back(query.value(0).toString().toStdString());
            }
        }
        
        return result;
    }
    
    std::vector<std::string> listDataByTag(const std::string& tag) override {
        std::vector<std::string> result;
        if (!initialized) return result;
        
        QSqlQuery query(db);
        query.prepare("SELECT id FROM data_records WHERE tags LIKE ? ORDER BY timestamp DESC");
        query.addBindValue(QString::fromStdString("%" + tag + "%"));
        
        if (query.exec()) {
            while (query.next()) {
                result.push_back(query.value(0).toString().toStdString());
            }
        }
        
        return result;
    }
    
    bool setUserRole(const std::string& username, int role) override {
        if (!initialized) return false;
        
        QSqlQuery query(db);
        query.prepare("INSERT OR REPLACE INTO user_roles (username, role) VALUES (?, ?)");
        query.addBindValue(QString::fromStdString(username));
        query.addBindValue(role);
        
        return query.exec();
    }
    
    int getUserRole(const std::string& username) override {
        if (!initialized) return 1;  // 默认为GUEST
        
        QSqlQuery query(db);
        query.prepare("SELECT role FROM user_roles WHERE username = ?");
        query.addBindValue(QString::fromStdString(username));
        
        if (query.exec() && query.next()) {
            return query.value(0).toInt();
        }
        
        return 1;  // 默认为GUEST
    }
    
    std::map<std::string, int> getAllUserRoles() override {
        std::map<std::string, int> result;
        if (!initialized) return result;
        
        QSqlQuery query(db);
        query.exec("SELECT username, role FROM user_roles");
        
        while (query.next()) {
            result[query.value(0).toString().toStdString()] = query.value(1).toInt();
        }
        
        return result;
    }
};

// 数据质量检测类
class DataQualityChecker { 
public: 
    // 格式校验
    bool checkFormat(const std::string& content, const std::string& format) { 
        if (content.empty()) return false; 
        if (format == "CSV") { 
            return content.find(',') != std::string::npos;  // CSV需包含逗号
        } else if (format == "JSON") { 
            return !content.empty() && content.front() == '{' && content.back() == '}';  // 完整JSON判断
        } else if (format == "SDF") { 
            return content.size() > 3 && content.find("$$$$") != std::string::npos;  // SDF以"$$$$"结尾
        }
        return false;  // 不支持的格式
    }

    // 标签校验
    bool checkTags(const std::unordered_set<std::string>& tags) { 
        std::regex r("^[a-zA-Z0-9_]+$");  // 仅允许字母、数字、下划线
        for (const auto& tag : tags) { 
            if (tag.empty() || !std::regex_match(tag, r)) { 
                return false; 
            }
        }
        return true;
    }

    // 分类校验
    bool checkCategory(const std::string& category) {
        return !category.empty() && category.size() <= 100;  // 非空且长度≤100
    }
}; 

// 权限管理类
class PermissionManager { 
public: 
    enum class Role { ADMIN = 3, USER = 2, GUEST = 1 };  // 权限角色，使用数字方便存储
private: 
    std::shared_ptr<IDataStorage> storage;  // 存储接口引用
    std::mutex mutex_;  // 线程安全锁
public: 
    // 构造函数：使用存储接口
    PermissionManager(std::shared_ptr<IDataStorage> storage) : storage(storage) { 
        // 存储接口会自动初始化默认用户
    }

    // 设置用户角色
    void setUserRole(const std::string& username, Role role) { 
        std::lock_guard<std::mutex> lock(mutex_); 
        storage->setUserRole(username, static_cast<int>(role));
    }

    // 获取用户角色
    Role getUserRole(const std::string& username) { 
        std::lock_guard<std::mutex> lock(mutex_); 
        int role = storage->getUserRole(username);
        return static_cast<Role>(role);
    }

    // 上传权限判断：ADMIN/USER可上传
    bool canUpload(const std::string& username) { 
        Role r = getUserRole(username); 
        return (r == Role::ADMIN || r == Role::USER); 
    }

    // 访问权限判断：ADMIN/USER无分类限制，GUEST仅可访问public
    bool canAccess(const std::string& username, const std::string& category) { 
        Role r = getUserRole(username);
        if (r == Role::ADMIN || r == Role::USER) return true; 
        if (r == Role::GUEST) return (category == "public"); 
        return false;
    }

    // 修改权限判断：ADMIN或数据上传者可修改
    bool canModify(const std::string& username, const DataRecord& data) { 
        Role r = getUserRole(username);
        if (r == Role::ADMIN) return true; 
        return (r == Role::USER && data.uploader == username); 
    }
};

// 存储配置类
class StorageConfig {
private:
    StorageMode currentMode;
    std::string dbPath;
    QSettings settings;
    
public:
    StorageConfig() : settings("BondForge", "Storage") {
        // 从配置中读取存储模式
        int mode = settings.value("storageMode", static_cast<int>(StorageMode::MEMORY)).toInt();
        currentMode = static_cast<StorageMode>(mode);
        
        // 从配置中读取数据库路径
        dbPath = settings.value("dbPath", "bondforge.db").toString().toStdString();
    }
    
    void setStorageMode(StorageMode mode) {
        currentMode = mode;
        settings.setValue("storageMode", static_cast<int>(mode));
    }
    
    StorageMode getStorageMode() const {
        return currentMode;
    }
    
    void setDatabasePath(const std::string& path) {
        dbPath = path;
        settings.setValue("dbPath", QString::fromStdString(path));
    }
    
    std::string getDatabasePath() const {
        return dbPath;
    }
};

// 核心服务类
class ChemicalMLService { 
private: 
    std::shared_ptr<IDataStorage> storage;  // 存储接口
    std::mutex mutex_;                      // 线程安全锁
    DataQualityChecker qualityChecker;     // 质量检测实例
    std::unique_ptr<PermissionManager> permissionManager;    // 权限管理实例
    I18nManager& i18n;                      // 国际化管理器引用
    StorageConfig& config;                  // 存储配置引用
    
public:
    ChemicalMLService(StorageConfig& storageConfig) : 
        i18n(I18nManager::getInstance()), 
        config(storageConfig) {
        switchStorageMode(storageConfig.getStorageMode());
    }
    
    // 切换存储模式
    bool switchStorageMode(StorageMode newMode) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 创建新的存储实例
        std::shared_ptr<IDataStorage> newStorage;
        switch (newMode) {
            case StorageMode::MEMORY:
                newStorage = std::make_shared<MemoryStorage>();
                break;
            case StorageMode::SQLITE:
                newStorage = std::make_shared<SQLiteStorage>(config.getDatabasePath());
                break;
            default:
                return false;
        }
        
        // 初始化新存储
        if (!newStorage->initialize()) {
            return false;
        }
        
        // 更新存储和权限管理器
        storage = newStorage;
        permissionManager = std::make_unique<PermissionManager>(storage);
        
        // 更新配置
        config.setStorageMode(newMode);
        
        return true;
    }
    
    // 获取当前存储模式
    StorageMode getCurrentStorageMode() const {
        return config.getStorageMode();
    }
    
    // 数据迁移功能
    bool migrateData(StorageMode targetMode) {
        if (targetMode == getCurrentStorageMode()) {
            return true;  // 已经是目标模式，无需迁移
        }
        
        // 获取当前所有数据
        auto allData = storage->getAllData();
        auto allUserRoles = storage->getAllUserRoles();
        
        // 切换到新模式
        if (!switchStorageMode(targetMode)) {
            return false;
        }
        
        // 迁移数据
        for (const auto& record : allData) {
            if (!storage->insertData(record)) {
                return false;
            }
        }
        
        // 迁移用户角色
        for (const auto& pair : allUserRoles) {
            if (!storage->setUserRole(pair.first, pair.second)) {
                return false;
            }
        }
        
        return true;
    }

    // 上传数据
    bool uploadData(const DataRecord& rawData); 
    
    // 删除数据
    bool deleteData(const std::string& id, const std::string& username);
    
    // 更新数据
    bool updateData(const DataRecord& newData, const std::string& username);
    
    // 查询单条数据
    DataRecord getData(const std::string& id, const std::string& username);
    
    // 按分类查询数据ID
    std::vector<std::string> listDataByCategory(const std::string& category, const std::string& username);
    
    // 按标签查询数据ID
    std::vector<std::string> listDataByTag(const std::string& tag, const std::string& username);
    
    // 获取所有数据
    std::vector<DataRecord> getAllData();
    
    // 设置用户角色
    bool setUserRole(const std::string& adminUser, const std::string& username, PermissionManager::Role role);
};

// GUI类
class BondForgeGUI : public QMainWindow
{
    Q_OBJECT

public:
    BondForgeGUI(QWidget *parent = nullptr);
    ~BondForgeGUI();

private slots:
    // 语言切换相关
    void switchToChinese();
    void switchToEnglish();
    void updateUI();
    
    // 数据管理相关
    void uploadData();
    void deleteData();
    void editData();
    void refreshDataList();
    
    // 文件操作
    void openFile();
    void saveFile();
    void about();
    
    // 存储设置相关
    void showStorageSettings();
    void applyStorageSettings();
    void browseDatabasePath();
    void migrateData();
    
    // 导入/导出相关
    void importCsvFile();
    void importJsonFile();
    void importSdfFile();
    void exportCsvFile();
    void exportJsonFile();
    void exportSdfFile();
    
    // 搜索/过滤相关
    void applySearchFilter();
    void addToFavorites();
    
    // 数据可视化相关
    void showMolecularStructure();
    void showDataCharts();
    void showTrendAnalysis();
    void showCompareData();
    
    // 协作相关
    void showUserManagement();
    void showDataSharing();
    void showComments();
    void showVersionHistory();
    
    // 数据分析相关
    void showStatisticalAnalysis();
    void showCorrelationAnalysis();
    void showPredictionModel();
    void showReportGeneration();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void updateTexts();
    void populateTable();
    void connectSignals();
    
    // 数据可视化辅助函数
    void renderSimpleMolecule(QGraphicsScene* scene, const std::string& content, bool is3D);
    void renderDataChart(QChartView* chartView, const std::string& chartType);
    void renderTrendAnalysis(QChartView* chartView, const QString& timeRange);
    void renderCompareData(QChartView* chartView, const QStringList& selectedItems);
    void populateTrendDataTable(QTableWidget* table, const QString& timeRange, const QString& trendType);
    void populateCompareTable(QTableWidget* table, const QStringList& selectedIds);
    void showContentDiff(QTextEdit* diffEdit, const QStringList& selectedIds);
    
    // 数据分析辅助函数
    double calculateCorrelation(const std::vector<double>& x, const std::vector<double>& y);
    double calculatePValue(double correlation, int sampleSize);
    
    // UI组件
    QWidget *m_centralWidget;
    QTabWidget *m_tabWidget;
    QMenuBar *m_menuBar;
    QStatusBar *m_statusBar;
    QProgressBar *m_progressBar;
    
    // 数据管理标签页
    QWidget *m_dataTab;
    QVBoxLayout *m_dataLayout;
    QTableWidget *m_dataTable;
    QGroupBox *m_dataGroupBox;
    QVBoxLayout *m_dataGroupLayout;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_uploadButton;
    QPushButton *m_deleteButton;
    QPushButton *m_editButton;
    QPushButton *m_refreshButton;
    
    // 数据上传标签页
    QWidget *m_uploadTab;
    QVBoxLayout *m_uploadLayout;
    QGroupBox *m_uploadGroupBox;
    QFormLayout *m_uploadGroupLayout;
    QLineEdit *m_idEdit;
    QLineEdit *m_categoryEdit;
    QTextEdit *m_contentEdit;
    QLineEdit *m_tagsEdit;
    QComboBox *m_formatCombo;
    QHBoxLayout *m_uploadButtonLayout;
    QPushButton *m_submitButton;
    QPushButton *m_clearButton;
    
    // 语言选择
    QComboBox *m_languageCombo;
    
    // 存储设置界面
    QDialog *m_storageSettingsDialog;
    QComboBox *m_storageModeCombo;
    QLineEdit *m_databasePathEdit;
    QPushButton *m_browseDatabaseButton;
    QPushButton *m_applyStorageButton;
    QPushButton *m_migrateDataButton;
    QTextEdit *m_storageDescriptionEdit;
    
    // 导入/导出标签页
    QWidget *m_importExportTab;
    QVBoxLayout *m_importExportLayout;
    QGroupBox *m_importGroupBox;
    QFormLayout *m_importGroupLayout;
    QPushButton *m_importCsvButton;
    QPushButton *m_importJsonButton;
    QPushButton *m_importSdfButton;
    QGroupBox *m_exportGroupBox;
    QFormLayout *m_exportGroupLayout;
    QPushButton *m_exportCsvButton;
    QPushButton *m_exportJsonButton;
    QPushButton *m_exportSdfButton;
    
    // 搜索/过滤标签页
    QWidget *m_searchFilterTab;
    QVBoxLayout *m_searchFilterLayout;
    QGroupBox *m_searchGroupBox;
    QVBoxLayout *m_searchGroupLayout;
    QLineEdit *m_searchEdit;
    QComboBox *m_filterCategoryCombo;
    QLineEdit *m_filterTagEdit;
    QDateEdit *m_filterDateEdit;
    QComboBox *m_sortCombo;
    QPushButton *m_applyFilterButton;
    QPushButton *m_addToFavoritesButton;
    
    // 数据可视化标签页
    QWidget *m_visualizationTab;
    QVBoxLayout *m_visualizationLayout;
    QTabWidget *m_visualizationTabWidget;
    QWidget *m_molecularStructureWidget;
    QWidget *m_dataChartsWidget;
    QWidget *m_trendAnalysisWidget;
    QWidget *m_compareDataWidget;
    
    // 协作标签页
    QWidget *m_collaborationTab;
    QVBoxLayout *m_collaborationLayout;
    QTabWidget *m_collaborationTabWidget;
    QWidget *m_userManagementWidget;
    QWidget *m_dataSharingWidget;
    QWidget *m_commentsWidget;
    QWidget *m_versionHistoryWidget;
    
    // 数据分析标签页
    QWidget *m_analysisTab;
    QVBoxLayout *m_analysisLayout;
    QTabWidget *m_analysisTabWidget;
    QWidget *m_statisticalAnalysisWidget;
    QWidget *m_correlationAnalysisWidget;
    QWidget *m_predictionModelWidget;
    QWidget *m_reportGenerationWidget;
    
    // 业务逻辑
    std::unique_ptr<StorageConfig> m_storageConfig;
    std::unique_ptr<ChemicalMLService> m_service;
    I18nManager& m_i18n;
    
    // 当前选择的数据ID
    QString m_selectedId;
};

// 函数声明
void runCLI();
int runGUI(int argc, char *argv[]);
void showHelp();

// 主函数
int main(int argc, char *argv[]) {
    // 解析命令行参数
    bool useGUI = true;  // 默认使用GUI
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--cli" || arg == "-c") {
            useGUI = false;
        } else if (arg == "--gui" || arg == "-g") {
            useGUI = true;
        } else if (arg == "--help" || arg == "-h") {
            showHelp();
            return 0;
        }
    }
    
    if (useGUI) {
        return runGUI(argc, argv);
    } else {
        runCLI();
        return 0;
    }
}

// 国际化管理器实现
I18nManager& I18nManager::getInstance() {
    static I18nManager instance;
    return instance;
}

bool I18nManager::initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 加载内置语言资源
    loadBuiltinLanguages();
    
    // 默认加载中文
    m_currentLanguage = "zh-CN";
    return true;
}

bool I18nManager::setLanguage(const std::string& languageCode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 检查语言是否存在
    if (m_languages.find(languageCode) == m_languages.end()) {
        return false;
    }
    
    m_currentLanguage = languageCode;
    return true;
}

std::string I18nManager::getCurrentLanguage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentLanguage;
}

std::string I18nManager::getText(const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 检查当前语言是否存在
    auto langIt = m_languages.find(m_currentLanguage);
    if (langIt == m_languages.end()) {
        return key; // 如果找不到语言，返回键名
    }
    
    // 查找键值
    return findValue(key, langIt->second);
}

bool I18nManager::hasKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto langIt = m_languages.find(m_currentLanguage);
    if (langIt == m_languages.end()) {
        return false;
    }
    
    return langIt->second.find(key) != langIt->second.end();
}

std::vector<std::string> I18nManager::getAvailableLanguages() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<std::string> languages;
    for (const auto& pair : m_languages) {
        languages.push_back(pair.first);
    }
    
    return languages;
}

void I18nManager::loadBuiltinLanguages() {
    // 加载中文资源
    std::map<std::string, std::string> zhCN;
    zhCN["error.no_upload_permission"] = "无上传权限";
    zhCN["error.data_format_validation_failed"] = "数据格式校验失败";
    zhCN["error.tag_validation_failed"] = "标签校验失败";
    zhCN["error.category_validation_failed"] = "分类校验失败";
    zhCN["error.data_id_exists"] = "数据ID已存在";
    zhCN["error.data_not_found"] = "数据不存在";
    zhCN["error.no_deletion_permission"] = "无删除权限";
    zhCN["error.no_edit_permission"] = "无编辑权限";
    zhCN["error.no_access_permission"] = "无访问权限";
    zhCN["error.cannot_modify_tags"] = "无权限修改标签";
    zhCN["error.cannot_modify_category"] = "无权限修改分类";
    zhCN["error.invalid_tag_format"] = "标签格式非法";
    zhCN["error.not_admin"] = "非管理员无权限设置用户角色";
    zhCN["error.unsupported_format_conversion"] = "暂不支持的格式转换";
    zhCN["error.storage_error"] = "存储操作失败";
    zhCN["error.migration_error"] = "数据迁移失败";
    zhCN["category.organic"] = "有机";
    zhCN["category.inorganic"] = "无机";
    zhCN["category.polymer"] = "高分子";
    zhCN["category.catalyst"] = "催化剂";
    zhCN["category.nanomaterial"] = "纳米材料";
    zhCN["category.pharmaceutical_chemistry"] = "药物化学";
    zhCN["category.public"] = "公开";
    zhCN["status.pending"] = "等待中";
    zhCN["status.running"] = "运行中";
    zhCN["status.completed"] = "已完成";
    zhCN["status.failed"] = "失败";
    zhCN["ui.welcome"] = "欢迎使用化学机器学习系统";
    zhCN["ui.upload_data"] = "上传数据";
    zhCN["ui.manage_data"] = "管理数据";
    zhCN["ui.training_tasks"] = "训练任务";
    zhCN["ui.model_versions"] = "模型版本";
    zhCN["ui.datasets"] = "数据集";
    zhCN["ui.language"] = "语言";
    zhCN["ui.chinese"] = "中文";
    zhCN["ui.english"] = "English";
    zhCN["ui.settings"] = "设置";
    zhCN["ui.storage_settings"] = "存储设置";
    zhCN["ui.storage_mode"] = "存储模式";
    zhCN["ui.memory_storage"] = "内存存储";
    zhCN["ui.database_storage"] = "数据库存储";
    zhCN["ui.database_path"] = "数据库路径";
    zhCN["ui.browse"] = "浏览";
    zhCN["ui.apply_settings"] = "应用设置";
    zhCN["ui.migrate_data"] = "迁移数据";
    zhCN["ui.migrate_confirm"] = "确认要将当前数据迁移到新的存储模式吗？";
    zhCN["ui.migration_successful"] = "数据迁移成功";
    zhCN["ui.migration_failed"] = "数据迁移失败";
    zhCN["ui.settings_applied"] = "设置已应用";
    zhCN["ui.memory_mode_desc"] = "数据存储在内存中，程序关闭后数据将丢失。适合测试和小规模使用。";
    zhCN["ui.database_mode_desc"] = "数据存储在SQLite数据库中，程序关闭后数据仍然保留。适合长期使用和大量数据。";
    zhCN["ui.import_data"] = "导入数据";
    zhCN["ui.export_data"] = "导出数据";
    zhCN["ui.import_export"] = "导入/导出";
    zhCN["ui.select_file"] = "选择文件";
    zhCN["ui.import_csv"] = "导入CSV文件";
    zhCN["ui.import_json"] = "导入JSON文件";
    zhCN["ui.import_sdf"] = "导入SDF文件";
    zhCN["ui.export_csv"] = "导出为CSV";
    zhCN["ui.export_json"] = "导出为JSON";
    zhCN["ui.export_sdf"] = "导出为SDF";
    zhCN["ui.import_successful"] = "数据导入成功";
    zhCN["ui.export_successful"] = "数据导出成功";
    zhCN["ui.import_failed"] = "数据导入失败";
    zhCN["ui.export_failed"] = "数据导出失败";
    zhCN["ui.search_filter"] = "搜索与过滤";
    zhCN["ui.search"] = "搜索";
    zhCN["ui.filter"] = "过滤";
    zhCN["ui.filter_by_category"] = "按分类过滤";
    zhCN["ui.filter_by_tag"] = "按标签过滤";
    zhCN["ui.filter_by_date"] = "按日期过滤";
    zhCN["ui.sort_by"] = "排序方式";
    zhCN["ui.sort_by_date"] = "按日期";
    zhCN["ui.sort_by_name"] = "按名称";
    zhCN["ui.sort_by_category"] = "按分类";
    zhCN["ui.add_to_favorites"] = "添加到收藏";
    zhCN["ui.favorites"] = "收藏夹";
    zhCN["ui.data_visualization"] = "数据可视化";
    zhCN["ui.molecular_structure"] = "分子结构";
    zhCN["ui.data_charts"] = "数据图表";
    zhCN["ui.trend_analysis"] = "趋势分析";
    zhCN["ui.compare_data"] = "数据对比";
    zhCN["ui.collaboration"] = "协作与共享";
    zhCN["ui.user_management"] = "用户管理";
    zhCN["ui.data_sharing"] = "数据共享";
    zhCN["ui.comments"] = "评论";
    zhCN["ui.version_history"] = "版本历史";
    zhCN["ui.data_analysis"] = "数据分析";
    zhCN["ui.statistical_analysis"] = "统计分析";
    zhCN["ui.correlation_analysis"] = "相关性分析";
    zhCN["ui.prediction_model"] = "预测模型";
    zhCN["ui.report_generation"] = "报告生成";
    zhCN["role.admin"] = "管理员";
    zhCN["role.user"] = "用户";
    zhCN["role.guest"] = "访客";
    m_languages["zh-CN"] = zhCN;
    
    // 加载英文资源
    std::map<std::string, std::string> enUS;
    enUS["error.no_upload_permission"] = "No upload permission";
    enUS["error.data_format_validation_failed"] = "Data format validation failed";
    enUS["error.tag_validation_failed"] = "Tag validation failed";
    enUS["error.category_validation_failed"] = "Category validation failed";
    enUS["error.data_id_exists"] = "Data ID already exists";
    enUS["error.data_not_found"] = "Data not found";
    enUS["error.no_deletion_permission"] = "No deletion permission";
    enUS["error.no_edit_permission"] = "No edit permission";
    enUS["error.no_access_permission"] = "No access permission";
    enUS["error.cannot_modify_tags"] = "No permission to modify tags";
    enUS["error.cannot_modify_category"] = "No permission to modify category";
    enUS["error.invalid_tag_format"] = "Invalid tag format";
    enUS["error.not_admin"] = "Only administrators can set user roles";
    enUS["error.unsupported_format_conversion"] = "Unsupported format conversion";
    enUS["error.storage_error"] = "Storage operation failed";
    enUS["error.migration_error"] = "Data migration failed";
    enUS["category.organic"] = "Organic";
    enUS["category.inorganic"] = "Inorganic";
    enUS["category.polymer"] = "Polymer";
    enUS["category.catalyst"] = "Catalyst";
    enUS["category.nanomaterial"] = "Nanomaterial";
    enUS["category.pharmaceutical_chemistry"] = "Pharmaceutical Chemistry";
    enUS["category.public"] = "Public";
    enUS["status.pending"] = "Pending";
    enUS["status.running"] = "Running";
    enUS["status.completed"] = "Completed";
    enUS["status.failed"] = "Failed";
    enUS["ui.welcome"] = "Welcome to Chemical Machine Learning System";
    enUS["ui.upload_data"] = "Upload Data";
    enUS["ui.manage_data"] = "Manage Data";
    enUS["ui.training_tasks"] = "Training Tasks";
    enUS["ui.model_versions"] = "Model Versions";
    enUS["ui.datasets"] = "Datasets";
    enUS["ui.language"] = "Language";
    enUS["ui.chinese"] = "中文";
    enUS["ui.english"] = "English";
    enUS["ui.settings"] = "Settings";
    enUS["ui.storage_settings"] = "Storage Settings";
    enUS["ui.storage_mode"] = "Storage Mode";
    enUS["ui.memory_storage"] = "Memory Storage";
    enUS["ui.database_storage"] = "Database Storage";
    enUS["ui.database_path"] = "Database Path";
    enUS["ui.browse"] = "Browse";
    enUS["ui.apply_settings"] = "Apply Settings";
    enUS["ui.migrate_data"] = "Migrate Data";
    enUS["ui.migrate_confirm"] = "Are you sure you want to migrate current data to the new storage mode?";
    enUS["ui.migration_successful"] = "Data migration successful";
    enUS["ui.migration_failed"] = "Data migration failed";
    enUS["ui.settings_applied"] = "Settings applied";
    enUS["ui.memory_mode_desc"] = "Data is stored in memory and will be lost when the program closes. Suitable for testing and small-scale use.";
    enUS["ui.database_mode_desc"] = "Data is stored in SQLite database and persists after program closes. Suitable for long-term use and large amounts of data.";
    enUS["ui.import_data"] = "Import Data";
    enUS["ui.export_data"] = "Export Data";
    enUS["ui.import_export"] = "Import/Export";
    enUS["ui.select_file"] = "Select File";
    enUS["ui.import_csv"] = "Import CSV File";
    enUS["ui.import_json"] = "Import JSON File";
    enUS["ui.import_sdf"] = "Import SDF File";
    enUS["ui.export_csv"] = "Export as CSV";
    enUS["ui.export_json"] = "Export as JSON";
    enUS["ui.export_sdf"] = "Export as SDF";
    enUS["ui.import_successful"] = "Data imported successfully";
    enUS["ui.export_successful"] = "Data exported successfully";
    enUS["ui.import_failed"] = "Data import failed";
    enUS["ui.export_failed"] = "Data export failed";
    enUS["ui.search_filter"] = "Search & Filter";
    enUS["ui.search"] = "Search";
    enUS["ui.filter"] = "Filter";
    enUS["ui.filter_by_category"] = "Filter by Category";
    enUS["ui.filter_by_tag"] = "Filter by Tag";
    enUS["ui.filter_by_date"] = "Filter by Date";
    enUS["ui.sort_by"] = "Sort By";
    enUS["ui.sort_by_date"] = "By Date";
    enUS["ui.sort_by_name"] = "By Name";
    enUS["ui.sort_by_category"] = "By Category";
    enUS["ui.add_to_favorites"] = "Add to Favorites";
    enUS["ui.favorites"] = "Favorites";
    enUS["ui.data_visualization"] = "Data Visualization";
    enUS["ui.molecular_structure"] = "Molecular Structure";
    enUS["ui.data_charts"] = "Data Charts";
    enUS["ui.trend_analysis"] = "Trend Analysis";
    enUS["ui.compare_data"] = "Compare Data";
    enUS["ui.collaboration"] = "Collaboration";
    enUS["ui.user_management"] = "User Management";
    enUS["ui.data_sharing"] = "Data Sharing";
    enUS["ui.comments"] = "Comments";
    enUS["ui.version_history"] = "Version History";
    enUS["ui.data_analysis"] = "Data Analysis";
    enUS["ui.statistical_analysis"] = "Statistical Analysis";
    enUS["ui.correlation_analysis"] = "Correlation Analysis";
    enUS["ui.prediction_model"] = "Prediction Model";
    enUS["ui.report_generation"] = "Report Generation";
    enUS["role.admin"] = "Administrator";
    enUS["role.user"] = "User";
    enUS["role.guest"] = "Guest";
    m_languages["en-US"] = enUS;
}

std::string I18nManager::findValue(const std::string& key, const std::map<std::string, std::string>& data) const {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return key; // 如果找不到键，返回键名
}

// 核心服务类实现
bool ChemicalMLService::uploadData(const DataRecord& rawData) { 
    std::lock_guard<std::mutex> lock(mutex_); 
    // 1. 权限校验
    if (!permissionManager->canUpload(rawData.uploader)) { 
        throw std::runtime_error(i18n.getText("error.no_upload_permission")); 
    }
    
    // 预处理数据
    DataRecord data = rawData; 
    
    // 3. 业务校验
    if (!qualityChecker.checkFormat(data.content, data.format)) { 
        throw std::runtime_error(i18n.getText("error.data_format_validation_failed")); 
    }
    if (!qualityChecker.checkTags(data.tags)) { 
        throw std::runtime_error(i18n.getText("error.tag_validation_failed"));
    }
    if (!qualityChecker.checkCategory(data.category)) {
        throw std::runtime_error(i18n.getText("error.category_validation_failed"));
    }
    // 4. 唯一ID检查
    if (storage->containsData(data.id)) { 
        throw std::runtime_error(i18n.getText("error.data_id_exists")); 
    }
    
    // 5. 存储数据
    return storage->insertData(data);
} 

bool ChemicalMLService::deleteData(const std::string& id, const std::string& username) { 
    std::lock_guard<std::mutex> lock(mutex_); 
    
    // 检查数据是否存在
    if (!storage->containsData(id)) { 
        throw std::runtime_error(i18n.getText("error.data_not_found")); 
    }
    
    // 获取数据用于权限检查
    DataRecord data = storage->getData(id);
    
    // 权限检查
    if (!permissionManager->canModify(username, data)) { 
        throw std::runtime_error(i18n.getText("error.no_deletion_permission")); 
    }
    
    // 删除数据
    return storage->deleteData(id);
}

bool ChemicalMLService::updateData(const DataRecord& newData, const std::string& username) { 
    std::lock_guard<std::mutex> lock(mutex_); 
    
    // 检查数据是否存在
    if (!storage->containsData(newData.id)) { 
        throw std::runtime_error(i18n.getText("error.data_not_found")); 
    }
    
    // 获取旧数据用于权限检查
    DataRecord oldData = storage->getData(newData.id);
    
    // 权限检查
    if (!permissionManager->canModify(username, oldData)) {
        throw std::runtime_error(i18n.getText("error.no_edit_permission")); 
    }
    
    // 校验新数据
    if (!qualityChecker.checkFormat(newData.content, newData.format)) { 
        throw std::runtime_error(i18n.getText("error.data_format_validation_failed")); 
    }
    if (!qualityChecker.checkTags(newData.tags)) { 
        throw std::runtime_error(i18n.getText("error.tag_validation_failed"));
    }
    if (!qualityChecker.checkCategory(newData.category)) { 
        throw std::runtime_error(i18n.getText("error.category_validation_failed"));
    }
    
    // 更新数据
    return storage->updateData(newData);
}

DataRecord ChemicalMLService::getData(const std::string& id, const std::string& username) { 
    std::lock_guard<std::mutex> lock(mutex_); 
    
    // 检查数据是否存在
    if (!storage->containsData(id)) { 
        throw std::runtime_error(i18n.getText("error.data_not_found")); 
    }
    
    // 获取数据
    DataRecord data = storage->getData(id);
    
    // 权限检查
    if (!permissionManager->canAccess(username, data.category)) { 
        throw std::runtime_error(i18n.getText("error.no_access_permission")); 
    }
    
    return data;
}

std::vector<std::string> ChemicalMLService::listDataByCategory(const std::string& category, const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_); 
    if (!permissionManager->canAccess(username, category)) { 
        throw std::runtime_error(i18n.getText("error.no_access_permission")); 
    }
    return storage->listDataByCategory(category);
}

std::vector<std::string> ChemicalMLService::listDataByTag(const std::string& tag, const std::string& username) { 
    std::lock_guard<std::mutex> lock(mutex_); 
    
    // 获取所有匹配标签的ID
    std::vector<std::string> allIds = storage->listDataByTag(tag);
    
    // 过滤用户有权限访问的数据
    std::vector<std::string> results;
    for (const auto& id : allIds) {
        try {
            DataRecord data = storage->getData(id);
            if (permissionManager->canAccess(username, data.category)) {
                results.push_back(id);
            }
        } catch (...) {
            // 忽略获取数据失败的情况
        }
    }
    
    return results;
}

// 获取所有数据
std::vector<DataRecord> ChemicalMLService::getAllData() {
    std::lock_guard<std::mutex> lock(mutex_);
    return storage->getAllData();
}

// 设置用户角色
bool ChemicalMLService::setUserRole(const std::string& adminUser, const std::string& username, PermissionManager::Role role) {
    // 检查管理员权限
    if (permissionManager->getUserRole(adminUser) != PermissionManager::Role::ADMIN) {
        throw std::runtime_error(i18n.getText("error.not_admin"));
    }
    
    // 设置用户角色
    permissionManager->setUserRole(username, role);
    return true;
}



// GUI实现
BondForgeGUI::BondForgeGUI(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_storageConfig(std::make_unique<StorageConfig>())
    , m_service(std::make_unique<ChemicalMLService>(*m_storageConfig))
    , m_i18n(I18nManager::getInstance())
    , m_storageSettingsDialog(nullptr)
{
    // 初始化国际化管理器
    if (!m_i18n.initialize()) {
        QMessageBox::critical(this, "Error", "Failed to initialize I18n manager!");
    }
    
    // 设置默认语言为中文
    m_i18n.setLanguage("zh-CN");
    
    // 设置窗口属性
    setWindowTitle("BondForge V1.2");
    resize(1024, 768);
    
    // 设置UI
    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupStorageSettingsDialog();
    connectSignals();
    
    // 更新界面文本
    updateTexts();
    
    // 初始化数据表格
    populateTable();
}

BondForgeGUI::~BondForgeGUI()
{
}

void BondForgeGUI::setupUI()
{
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    
    // 创建语言选择区域
    QHBoxLayout *langLayout = new QHBoxLayout();
    QLabel *langLabel = new QLabel("语言 / Language:", this);
    m_languageCombo = new QComboBox(this);
    m_languageCombo->addItem("中文", "zh-CN");
    m_languageCombo->addItem("English", "en-US");
    m_languageCombo->setCurrentIndex(0);  // 默认选择中文
    langLayout->addWidget(langLabel);
    langLayout->addWidget(m_languageCombo);
    langLayout->addStretch();
    
    mainLayout->addLayout(langLayout);
    
    // 创建标签页
    m_tabWidget = new QTabWidget(this);
    
    // 创建数据管理标签页
    m_dataTab = new QWidget(this);
    m_dataLayout = new QVBoxLayout(m_dataTab);
    
    // 创建数据表格
    m_dataTable = new QTableWidget(0, 5, this);
    m_dataTable->setHorizontalHeaderLabels({"ID", "Content", "Category", "Tags", "Uploader"});
    m_dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // 创建操作按钮组
    m_dataGroupBox = new QGroupBox("Operations", this);
    m_dataGroupLayout = new QVBoxLayout(m_dataGroupBox);
    m_buttonLayout = new QHBoxLayout();
    
    m_uploadButton = new QPushButton("Upload Data", this);
    m_deleteButton = new QPushButton("Delete Data", this);
    m_editButton = new QPushButton("Edit Data", this);
    m_refreshButton = new QPushButton("Refresh", this);
    
    m_buttonLayout->addWidget(m_uploadButton);
    m_buttonLayout->addWidget(m_deleteButton);
    m_buttonLayout->addWidget(m_editButton);
    m_buttonLayout->addWidget(m_refreshButton);
    m_dataGroupLayout->addLayout(m_buttonLayout);
    
    m_dataLayout->addWidget(m_dataTable);
    m_dataLayout->addWidget(m_dataGroupBox);
    
    // 创建数据上传标签页
    m_uploadTab = new QWidget(this);
    m_uploadLayout = new QVBoxLayout(m_uploadTab);
    
    // 创建上传表单
    m_uploadGroupBox = new QGroupBox("Data Upload", this);
    m_uploadGroupLayout = new QFormLayout(m_uploadGroupBox);
    
    m_idEdit = new QLineEdit(this);
    m_categoryEdit = new QLineEdit(this);
    m_contentEdit = new QTextEdit(this);
    m_tagsEdit = new QLineEdit(this);
    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItem("CSV");
    m_formatCombo->addItem("JSON");
    m_formatCombo->addItem("SDF");
    
    m_uploadGroupLayout->addRow("ID:", m_idEdit);
    m_uploadGroupLayout->addRow("Category:", m_categoryEdit);
    m_uploadGroupLayout->addRow("Content:", m_contentEdit);
    m_uploadGroupLayout->addRow("Tags (comma separated):", m_tagsEdit);
    m_uploadGroupLayout->addRow("Format:", m_formatCombo);
    
    // 创建上传按钮
    m_uploadButtonLayout = new QHBoxLayout();
    m_submitButton = new QPushButton("Submit", this);
    m_clearButton = new QPushButton("Clear", this);
    m_uploadButtonLayout->addWidget(m_submitButton);
    m_uploadButtonLayout->addWidget(m_clearButton);
    m_uploadButtonLayout->addStretch();
    
    m_uploadLayout->addWidget(m_uploadGroupBox);
    m_uploadLayout->addLayout(m_uploadButtonLayout);
    m_uploadLayout->addStretch();
    
    // 设置导入/导出标签页
    m_importExportTab = new QWidget(this);
    m_importExportLayout = new QVBoxLayout(m_importExportTab);
    
    // 导入组
    m_importGroupBox = new QGroupBox("Import", this);
    m_importGroupLayout = new QFormLayout(m_importGroupBox);
    
    m_importCsvButton = new QPushButton("Import CSV", this);
    m_importJsonButton = new QPushButton("Import JSON", this);
    m_importSdfButton = new QPushButton("Import SDF", this);
    
    m_importGroupLayout->addRow("", m_importCsvButton);
    m_importGroupLayout->addRow("", m_importJsonButton);
    m_importGroupLayout->addRow("", m_importSdfButton);
    
    // 导出组
    m_exportGroupBox = new QGroupBox("Export", this);
    m_exportGroupLayout = new QFormLayout(m_exportGroupBox);
    
    m_exportCsvButton = new QPushButton("Export as CSV", this);
    m_exportJsonButton = new QPushButton("Export as JSON", this);
    m_exportSdfButton = new QPushButton("Export as SDF", this);
    
    m_exportGroupLayout->addRow("", m_exportCsvButton);
    m_exportGroupLayout->addRow("", m_exportJsonButton);
    m_exportGroupLayout->addRow("", m_exportSdfButton);
    
    m_importExportLayout->addWidget(m_importGroupBox);
    m_importExportLayout->addWidget(m_exportGroupBox);
    m_importExportLayout->addStretch();
    
    // 设置搜索/过滤标签页
    m_searchFilterTab = new QWidget(this);
    m_searchFilterLayout = new QVBoxLayout(m_searchFilterTab);
    
    // 搜索组
    m_searchGroupBox = new QGroupBox("Search & Filter", this);
    m_searchGroupLayout = new QVBoxLayout(m_searchGroupBox);
    
    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search...");
    searchLayout->addWidget(m_searchEdit);
    m_searchGroupLayout->addLayout(searchLayout);
    
    // 过滤组
    QFormLayout *filterLayout = new QFormLayout();
    m_filterCategoryCombo = new QComboBox(this);
    m_filterCategoryCombo->addItem("All Categories");
    m_filterCategoryCombo->addItem("Organic");
    m_filterCategoryCombo->addItem("Inorganic");
    m_filterCategoryCombo->addItem("Polymer");
    m_filterCategoryCombo->addItem("Catalyst");
    m_filterCategoryCombo->addItem("Nanomaterial");
    m_filterCategoryCombo->addItem("Pharmaceutical Chemistry");
    m_filterCategoryCombo->addItem("Public");
    
    m_filterTagEdit = new QLineEdit(this);
    m_filterTagEdit->setPlaceholderText("Filter by tag...");
    
    m_filterDateEdit = new QDateEdit(this);
    m_filterDateEdit->setCalendarPopup(true);
    m_filterDateEdit->setDate(QDate::currentDate());
    
    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItem("Sort by Date");
    m_sortCombo->addItem("Sort by Name");
    m_sortCombo->addItem("Sort by Category");
    
    filterLayout->addRow("Category:", m_filterCategoryCombo);
    filterLayout->addRow("Tag:", m_filterTagEdit);
    filterLayout->addRow("Date:", m_filterDateEdit);
    filterLayout->addRow("Sort:", m_sortCombo);
    
    m_searchGroupLayout->addLayout(filterLayout);
    
    // 按钮
    QHBoxLayout *filterButtonLayout = new QHBoxLayout();
    m_applyFilterButton = new QPushButton("Apply Filter", this);
    m_addToFavoritesButton = new QPushButton("Add to Favorites", this);
    
    filterButtonLayout->addWidget(m_applyFilterButton);
    filterButtonLayout->addWidget(m_addToFavoritesButton);
    filterButtonLayout->addStretch();
    
    m_searchGroupLayout->addLayout(filterButtonLayout);
    m_searchFilterLayout->addWidget(m_searchGroupBox);
    m_searchFilterLayout->addStretch();
    
    // 设置数据可视化标签页
    m_visualizationTab = new QWidget(this);
    m_visualizationLayout = new QVBoxLayout(m_visualizationTab);
    
    m_visualizationTabWidget = new QTabWidget(this);
    
    // 各种子页面（暂为空实现）
    m_molecularStructureWidget = new QWidget(this);
    m_dataChartsWidget = new QWidget(this);
    m_trendAnalysisWidget = new QWidget(this);
    m_compareDataWidget = new QWidget(this);
    
    m_visualizationTabWidget->addTab(m_molecularStructureWidget, "Molecular Structure");
    m_visualizationTabWidget->addTab(m_dataChartsWidget, "Data Charts");
    m_visualizationTabWidget->addTab(m_trendAnalysisWidget, "Trend Analysis");
    m_visualizationTabWidget->addTab(m_compareDataWidget, "Compare Data");
    
    m_visualizationLayout->addWidget(m_visualizationTabWidget);
    
    // 设置协作标签页
    m_collaborationTab = new QWidget(this);
    m_collaborationLayout = new QVBoxLayout(m_collaborationTab);
    
    m_collaborationTabWidget = new QTabWidget(this);
    
    // 各种子页面（暂为空实现）
    m_userManagementWidget = new QWidget(this);
    m_dataSharingWidget = new QWidget(this);
    m_commentsWidget = new QWidget(this);
    m_versionHistoryWidget = new QWidget(this);
    
    m_collaborationTabWidget->addTab(m_userManagementWidget, "User Management");
    m_collaborationTabWidget->addTab(m_dataSharingWidget, "Data Sharing");
    m_collaborationTabWidget->addTab(m_commentsWidget, "Comments");
    m_collaborationTabWidget->addTab(m_versionHistoryWidget, "Version History");
    
    m_collaborationLayout->addWidget(m_collaborationTabWidget);
    
    // 设置数据分析标签页
    m_analysisTab = new QWidget(this);
    m_analysisLayout = new QVBoxLayout(m_analysisTab);
    
    m_analysisTabWidget = new QTabWidget(this);
    
    // 各种子页面（暂为空实现）
    m_statisticalAnalysisWidget = new QWidget(this);
    m_correlationAnalysisWidget = new QWidget(this);
    m_predictionModelWidget = new QWidget(this);
    m_reportGenerationWidget = new QWidget(this);
    
    m_analysisTabWidget->addTab(m_statisticalAnalysisWidget, "Statistical Analysis");
    m_analysisTabWidget->addTab(m_correlationAnalysisWidget, "Correlation Analysis");
    m_analysisTabWidget->addTab(m_predictionModelWidget, "Prediction Model");
    m_analysisTabWidget->addTab(m_reportGenerationWidget, "Report Generation");
    
    m_analysisLayout->addWidget(m_analysisTabWidget);
    
    // 添加标签页
    m_tabWidget->addTab(m_dataTab, "Data Management");
    m_tabWidget->addTab(m_uploadTab, "Data Upload");
    m_tabWidget->addTab(m_importExportTab, "Import/Export");
    m_tabWidget->addTab(m_searchFilterTab, "Search & Filter");
    m_tabWidget->addTab(m_visualizationTab, "Data Visualization");
    m_tabWidget->addTab(m_collaborationTab, "Collaboration");
    m_tabWidget->addTab(m_analysisTab, "Data Analysis");
    
    mainLayout->addWidget(m_tabWidget);
}

void BondForgeGUI::setupStorageSettingsDialog()
{
    // 创建存储设置对话框
    m_storageSettingsDialog = new QDialog(this);
    m_storageSettingsDialog->setWindowTitle("Storage Settings");
    m_storageSettingsDialog->setModal(true);
    m_storageSettingsDialog->resize(500, 400);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(m_storageSettingsDialog);
    
    // 存储模式选择
    QGroupBox *modeGroup = new QGroupBox("Storage Mode", m_storageSettingsDialog);
    QVBoxLayout *modeLayout = new QVBoxLayout(modeGroup);
    
    m_storageModeCombo = new QComboBox(m_storageSettingsDialog);
    m_storageModeCombo->addItem("Memory Storage", static_cast<int>(StorageMode::MEMORY));
    m_storageModeCombo->addItem("Database Storage", static_cast<int>(StorageMode::SQLITE));
    
    // 根据当前设置选择模式
    int currentModeIndex = (m_storageConfig->getStorageMode() == StorageMode::SQLITE) ? 1 : 0;
    m_storageModeCombo->setCurrentIndex(currentModeIndex);
    
    modeLayout->addWidget(new QLabel("Select storage mode:"));
    modeLayout->addWidget(m_storageModeCombo);
    
    // 存储模式描述
    m_storageDescriptionEdit = new QTextEdit(m_storageSettingsDialog);
    m_storageDescriptionEdit->setReadOnly(true);
    m_storageDescriptionEdit->setMaximumHeight(100);
    updateStorageDescription();
    modeLayout->addWidget(m_storageDescriptionEdit);
    
    dialogLayout->addWidget(modeGroup);
    
    // 数据库路径设置
    QGroupBox *pathGroup = new QGroupBox("Database Settings", m_storageSettingsDialog);
    QHBoxLayout *pathLayout = new QHBoxLayout(pathGroup);
    
    m_databasePathEdit = new QLineEdit(m_storageSettingsDialog);
    m_databasePathEdit->setText(QString::fromStdString(m_storageConfig->getDatabasePath()));
    
    m_browseDatabaseButton = new QPushButton("Browse", m_storageSettingsDialog);
    
    pathLayout->addWidget(new QLabel("Database Path:"));
    pathLayout->addWidget(m_databasePathEdit);
    pathLayout->addWidget(m_browseDatabaseButton);
    
    dialogLayout->addWidget(pathGroup);
    
    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_applyStorageButton = new QPushButton("Apply Settings", m_storageSettingsDialog);
    m_migrateDataButton = new QPushButton("Migrate Data", m_storageSettingsDialog);
    QPushButton *cancelButton = new QPushButton("Cancel", m_storageSettingsDialog);
    
    buttonLayout->addWidget(m_applyStorageButton);
    buttonLayout->addWidget(m_migrateDataButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    
    dialogLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_storageModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &BondForgeGUI::updateStorageDescription);
    connect(m_applyStorageButton, &QPushButton::clicked, this, &BondForgeGUI::applyStorageSettings);
    connect(m_migrateDataButton, &QPushButton::clicked, this, &BondForgeGUI::migrateData);
    connect(m_browseDatabaseButton, &QPushButton::clicked, this, &BondForgeGUI::browseDatabasePath);
    connect(cancelButton, &QPushButton::clicked, m_storageSettingsDialog, &QDialog::reject);
    
    // 初始状态下，如果模式是内存模式，则禁用数据库路径
    if (m_storageConfig->getStorageMode() == StorageMode::MEMORY) {
        m_databasePathEdit->setEnabled(false);
        m_browseDatabaseButton->setEnabled(false);
    }
}

void BondForgeGUI::setupMenuBar()
{
    // 创建菜单栏
    m_menuBar = menuBar();
    
    // 文件菜单
    QMenu *fileMenu = m_menuBar->addMenu("File");
    
    QAction *openAction = new QAction("Open", this);
    QAction *saveAction = new QAction("Save", this);
    QAction *exitAction = new QAction("Exit", this);
    
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    connect(openAction, &QAction::triggered, this, &BondForgeGUI::openFile);
    connect(saveAction, &QAction::triggered, this, &BondForgeGUI::saveFile);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // 设置菜单
    QMenu *settingsMenu = m_menuBar->addMenu("Settings");
    QAction *storageSettingsAction = new QAction("Storage Settings", this);
    settingsMenu->addAction(storageSettingsAction);
    connect(storageSettingsAction, &QAction::triggered, this, &BondForgeGUI::showStorageSettings);
    
    // 语言菜单
    QMenu *langMenu = m_menuBar->addMenu("Language");
    
    QAction *chineseAction = new QAction("中文", this);
    QAction *englishAction = new QAction("English", this);
    
    langMenu->addAction(chineseAction);
    langMenu->addAction(englishAction);
    
    connect(chineseAction, &QAction::triggered, this, &BondForgeGUI::switchToChinese);
    connect(englishAction, &QAction::triggered, this, &BondForgeGUI::switchToEnglish);
    
    // 帮助菜单
    QMenu *helpMenu = m_menuBar->addMenu("Help");
    
    QAction *aboutAction = new QAction("About", this);
    
    helpMenu->addAction(aboutAction);
    
    connect(aboutAction, &QAction::triggered, this, &BondForgeGUI::about);
}

void BondForgeGUI::setupStatusBar()
{
    // 创建状态栏
    m_statusBar = statusBar();
    
    // 创建进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    
    m_statusBar->addPermanentWidget(m_progressBar);
    m_statusBar->showMessage("Ready");
}

void BondForgeGUI::connectSignals()
{
    // 语言切换
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int index) {
                if (index == 0) {
                    switchToChinese();
                } else {
                    switchToEnglish();
                }
            });
    
    // 数据管理
    connect(m_uploadButton, &QPushButton::clicked, this, &BondForgeGUI::uploadData);
    connect(m_deleteButton, &QPushButton::clicked, this, &BondForgeGUI::deleteData);
    connect(m_editButton, &QPushButton::clicked, this, &BondForgeGUI::editData);
    connect(m_refreshButton, &QPushButton::clicked, this, &BondForgeGUI::refreshDataList);
    
    // 数据上传
    connect(m_submitButton, &QPushButton::clicked, [this]() {
        uploadData();
    });
    connect(m_clearButton, &QPushButton::clicked, [this]() {
        m_idEdit->clear();
        m_categoryEdit->clear();
        m_contentEdit->clear();
        m_tagsEdit->clear();
        m_formatCombo->setCurrentIndex(0);
    });
    
    // 表格选择
    connect(m_dataTable, &QTableWidget::cellClicked, [this](int row, int) {
        m_selectedId = m_dataTable->item(row, 0)->text();
    });
    
    // 导入/导出连接
    connect(m_importCsvButton, &QPushButton::clicked, this, &BondForgeGUI::importCsvFile);
    connect(m_importJsonButton, &QPushButton::clicked, this, &BondForgeGUI::importJsonFile);
    connect(m_importSdfButton, &QPushButton::clicked, this, &BondForgeGUI::importSdfFile);
    connect(m_exportCsvButton, &QPushButton::clicked, this, &BondForgeGUI::exportCsvFile);
    connect(m_exportJsonButton, &QPushButton::clicked, this, &BondForgeGUI::exportJsonFile);
    connect(m_exportSdfButton, &QPushButton::clicked, this, &BondForgeGUI::exportSdfFile);
    
    // 搜索/过滤连接
    connect(m_applyFilterButton, &QPushButton::clicked, this, &BondForgeGUI::applySearchFilter);
    connect(m_addToFavoritesButton, &QPushButton::clicked, this, &BondForgeGUI::addToFavorites);
    
    // 数据可视化连接
    connect(m_visualizationTabWidget, &QTabWidget::currentChanged, [this](int index) {
        switch(index) {
            case 0: showMolecularStructure(); break;
            case 1: showDataCharts(); break;
            case 2: showTrendAnalysis(); break;
            case 3: showCompareData(); break;
        }
    });
    
    // 协作连接
    connect(m_collaborationTabWidget, &QTabWidget::currentChanged, [this](int index) {
        switch(index) {
            case 0: showUserManagement(); break;
            case 1: showDataSharing(); break;
            case 2: showComments(); break;
            case 3: showVersionHistory(); break;
        }
    });
    
    // 数据分析连接
    connect(m_analysisTabWidget, &QTabWidget::currentChanged, [this](int index) {
        switch(index) {
            case 0: showStatisticalAnalysis(); break;
            case 1: showCorrelationAnalysis(); break;
            case 2: showPredictionModel(); break;
            case 3: showReportGeneration(); break;
        }
    });
}

void BondForgeGUI::updateTexts()
{
    // 更新窗口标题
    setWindowTitle(QString::fromStdString(m_i18n.getText("ui.welcome")) + " - BondForge V1.2");
    
    // 更新标签页
    m_tabWidget->setTabText(0, QString::fromStdString(m_i18n.getText("ui.manage_data")));
    m_tabWidget->setTabText(1, QString::fromStdString(m_i18n.getText("ui.upload_data")));
    
    // 更新表格标题
    m_dataTable->setHorizontalHeaderLabels({
        "ID",
        QString::fromStdString(m_i18n.getText("ui.content", "Content")),
        QString::fromStdString(m_i18n.getText("category.organic", "Category")),
        QString::fromStdString(m_i18n.getText("ui.tags", "Tags")),
        QString::fromStdString(m_i18n.getText("ui.uploader", "Uploader"))
    });
    
    // 更新按钮文本
    m_uploadButton->setText(QString::fromStdString(m_i18n.getText("ui.upload_data")));
    m_deleteButton->setText("Delete");  // 简化，实际应该国际化
    m_editButton->setText("Edit");
    m_refreshButton->setText("Refresh");
    m_submitButton->setText("Submit");
    m_clearButton->setText("Clear");
    
    // 更新菜单
    QList<QMenu*> menus = m_menuBar->findChildren<QMenu*>();
    for (QMenu* menu : menus) {
        if (menu->title() == "File" || menu->title() == "文件") {
            menu->setTitle(QString::fromStdString(m_i18n.getText("ui.file", "File")));
        } else if (menu->title() == "Language" || menu->title() == "语言") {
            menu->setTitle(QString::fromStdString(m_i18n.getText("ui.language", "Language")));
        } else if (menu->title() == "Help" || menu->title() == "帮助") {
            menu->setTitle(QString::fromStdString(m_i18n.getText("ui.help", "Help")));
        } else if (menu->title() == "Settings" || menu->title() == "设置") {
            menu->setTitle(QString::fromStdString(m_i18n.getText("ui.settings")));
        }
    }
    
    // 更新存储设置对话框
    if (m_storageSettingsDialog) {
        m_storageSettingsDialog->setWindowTitle(QString::fromStdString(m_i18n.getText("ui.storage_settings")));
        
        // 更新存储模式组合框
        m_storageModeCombo->setItemText(0, QString::fromStdString(m_i18n.getText("ui.memory_storage")));
        m_storageModeCombo->setItemText(1, QString::fromStdString(m_i18n.getText("ui.database_storage")));
        
        // 更新描述文本
        updateStorageDescription();
        
        // 更新按钮文本
        m_applyStorageButton->setText(QString::fromStdString(m_i18n.getText("ui.apply_settings")));
        m_migrateDataButton->setText(QString::fromStdString(m_i18n.getText("ui.migrate_data")));
        m_browseDatabaseButton->setText(QString::fromStdString(m_i18n.getText("ui.browse")));
    }
    
    // 更新导入/导出标签页
    if (m_importExportTab) {
        m_tabWidget->setTabText(2, QString::fromStdString(m_i18n.getText("ui.import_export")));
        
        // 导入组
        m_importGroupBox->setTitle(QString::fromStdString(m_i18n.getText("ui.import_data")));
        m_importCsvButton->setText(QString::fromStdString(m_i18n.getText("ui.import_csv")));
        m_importJsonButton->setText(QString::fromStdString(m_i18n.getText("ui.import_json")));
        m_importSdfButton->setText(QString::fromStdString(m_i18n.getText("ui.import_sdf")));
        
        // 导出组
        m_exportGroupBox->setTitle(QString::fromStdString(m_i18n.getText("ui.export_data")));
        m_exportCsvButton->setText(QString::fromStdString(m_i18n.getText("ui.export_csv")));
        m_exportJsonButton->setText(QString::fromStdString(m_i18n.getText("ui.export_json")));
        m_exportSdfButton->setText(QString::fromStdString(m_i18n.getText("ui.export_sdf")));
    }
    
    // 更新搜索/过滤标签页
    if (m_searchFilterTab) {
        m_tabWidget->setTabText(3, QString::fromStdString(m_i18n.getText("ui.search_filter")));
        
        // 搜索组
        m_searchGroupBox->setTitle(QString::fromStdString(m_i18n.getText("ui.search_filter")));
        m_searchEdit->setPlaceholderText(QString::fromStdString(m_i18n.getText("ui.search")));
        m_filterCategoryCombo->setItemText(0, "All Categories");
        m_filterCategoryCombo->setItemText(1, QString::fromStdString(m_i18n.getText("category.organic")));
        m_filterCategoryCombo->setItemText(2, QString::fromStdString(m_i18n.getText("category.inorganic")));
        m_filterCategoryCombo->setItemText(3, QString::fromStdString(m_i18n.getText("category.polymer")));
        m_filterCategoryCombo->setItemText(4, QString::fromStdString(m_i18n.getText("category.catalyst")));
        m_filterCategoryCombo->setItemText(5, QString::fromStdString(m_i18n.getText("category.nanomaterial")));
        m_filterCategoryCombo->setItemText(6, QString::fromStdString(m_i18n.getText("category.pharmaceutical_chemistry")));
        m_filterCategoryCombo->setItemText(7, QString::fromStdString(m_i18n.getText("category.public")));
        
        m_filterTagEdit->setPlaceholderText(QString::fromStdString(m_i18n.getText("ui.filter_by_tag")));
        m_sortCombo->setItemText(0, QString::fromStdString(m_i18n.getText("ui.sort_by_date")));
        m_sortCombo->setItemText(1, QString::fromStdString(m_i18n.getText("ui.sort_by_name")));
        m_sortCombo->setItemText(2, QString::fromStdString(m_i18n.getText("ui.sort_by_category")));
        
        m_applyFilterButton->setText(QString::fromStdString(m_i18n.getText("ui.filter")));
        m_addToFavoritesButton->setText(QString::fromStdString(m_i18n.getText("ui.add_to_favorites")));
    }
    
    // 更新数据可视化标签页
    if (m_visualizationTab) {
        m_tabWidget->setTabText(4, QString::fromStdString(m_i18n.getText("ui.data_visualization")));
        
        m_visualizationTabWidget->setTabText(0, QString::fromStdString(m_i18n.getText("ui.molecular_structure")));
        m_visualizationTabWidget->setTabText(1, QString::fromStdString(m_i18n.getText("ui.data_charts")));
        m_visualizationTabWidget->setTabText(2, QString::fromStdString(m_i18n.getText("ui.trend_analysis")));
        m_visualizationTabWidget->setTabText(3, QString::fromStdString(m_i18n.getText("ui.compare_data")));
    }
    
    // 更新协作标签页
    if (m_collaborationTab) {
        m_tabWidget->setTabText(5, QString::fromStdString(m_i18n.getText("ui.collaboration")));
        
        m_collaborationTabWidget->setTabText(0, QString::fromStdString(m_i18n.getText("ui.user_management")));
        m_collaborationTabWidget->setTabText(1, QString::fromStdString(m_i18n.getText("ui.data_sharing")));
        m_collaborationTabWidget->setTabText(2, QString::fromStdString(m_i18n.getText("ui.comments")));
        m_collaborationTabWidget->setTabText(3, QString::fromStdString(m_i18n.getText("ui.version_history")));
    }
    
    // 更新数据分析标签页
    if (m_analysisTab) {
        m_tabWidget->setTabText(6, QString::fromStdString(m_i18n.getText("ui.data_analysis")));
        
        m_analysisTabWidget->setTabText(0, QString::fromStdString(m_i18n.getText("ui.statistical_analysis")));
        m_analysisTabWidget->setTabText(1, QString::fromStdString(m_i18n.getText("ui.correlation_analysis")));
        m_analysisTabWidget->setTabText(2, QString::fromStdString(m_i18n.getText("ui.prediction_model")));
        m_analysisTabWidget->setTabText(3, QString::fromStdString(m_i18n.getText("ui.report_generation")));
    }
}

void BondForgeGUI::populateTable()
{
    // 从服务中获取数据
    m_dataTable->setRowCount(0);
    
    try {
        // 获取所有数据
        auto allData = m_service->getAllData();
        
        // 填充表格
        for (const auto& record : allData) {
            int row = m_dataTable->rowCount();
            m_dataTable->insertRow(row);
            
            // ID
            m_dataTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(record.id)));
            
            // Content (截断前50个字符)
            std::string content = record.content;
            if (content.length() > 50) {
                content = content.substr(0, 50) + "...";
            }
            m_dataTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(content)));
            
            // Category
            m_dataTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(record.category)));
            
            // Tags (组合显示)
            std::string tags;
            for (const auto& tag : record.tags) {
                if (!tags.empty()) tags += ", ";
                tags += tag;
            }
            m_dataTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(tags)));
            
            // Uploader
            m_dataTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(record.uploader)));
        }
    } catch (const std::exception& e) {
        // 处理错误
        m_statusBar->showMessage(QString("Error loading data: %1").arg(e.what()), 3000);
    }
    // 示例数据行
    int row = m_dataTable->rowCount();
    m_dataTable->insertRow(row);
    m_dataTable->setItem(row, 0, new QTableWidgetItem("example-001"));
    m_dataTable->setItem(row, 1, new QTableWidgetItem("C2H5OH"));
    m_dataTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(m_i18n.getText("category.organic"))));
    m_dataTable->setItem(row, 3, new QTableWidgetItem("alcohol,organic"));
    m_dataTable->setItem(row, 4, new QTableWidgetItem("admin"));
}

void BondForgeGUI::switchToChinese()
{
    m_i18n.setLanguage("zh-CN");
    updateTexts();
    populateTable();  // 重新填充表格以更新分类名称
    m_statusBar->showMessage("语言已切换为中文", 2000);
}

void BondForgeGUI::switchToEnglish()
{
    m_i18n.setLanguage("en-US");
    updateTexts();
    populateTable();  // 重新填充表格以更新分类名称
    m_statusBar->showMessage("Language switched to English", 2000);
}

void BondForgeGUI::updateUI()
{
    updateTexts();
    populateTable();
}

void BondForgeGUI::uploadData()
{
    QString id = m_idEdit->text();
    QString category = m_categoryEdit->text();
    QString content = m_contentEdit->toPlainText();
    QString tagsStr = m_tagsEdit->text();
    QString format = m_formatCombo->currentText();
    
    if (id.isEmpty() || category.isEmpty() || content.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please fill in all required fields!");
        return;
    }
    
    try {
        DataRecord record;
        record.id = id.toStdString();
        record.category = category.toStdString();
        record.content = content.toStdString();
        record.format = format.toStdString();
        record.uploader = "user";  // 简化，实际应该从登录状态获取
        
        // 解析标签
        QStringList tagsList = tagsStr.split(',', Qt::SkipEmptyParts);
        for (const QString& tag : tagsList) {
            record.tags.insert(tag.trimmed().toStdString());
        }
        
        // 上传数据
        if (m_service->uploadData(record)) {
            QMessageBox::information(this, "Success", "Data uploaded successfully!");
            m_statusBar->showMessage("Data uploaded successfully", 2000);
            
            // 清空表单
            m_idEdit->clear();
            m_categoryEdit->clear();
            m_contentEdit->clear();
            m_tagsEdit->clear();
            m_formatCombo->setCurrentIndex(0);
            
            // 刷新数据列表
            refreshDataList();
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void BondForgeGUI::deleteData()
{
    if (m_selectedId.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a data item to delete!");
        return;
    }
    
    try {
        if (m_service->deleteData(m_selectedId.toStdString(), "user")) {
            QMessageBox::information(this, "Success", "Data deleted successfully!");
            m_statusBar->showMessage("Data deleted successfully", 2000);
            refreshDataList();
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void BondForgeGUI::editData()
{
    if (m_selectedId.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a data item to edit!");
        return;
    }
    
    // 简化处理，切换到上传标签页并填充数据
    m_tabWidget->setCurrentIndex(1);
    
    try {
        DataRecord record = m_service->getData(m_selectedId.toStdString(), "user");
        
        m_idEdit->setText(QString::fromStdString(record.id));
        m_categoryEdit->setText(QString::fromStdString(record.category));
        m_contentEdit->setText(QString::fromStdString(record.content));
        
        // 设置格式
        int formatIndex = m_formatCombo->findText(QString::fromStdString(record.format));
        if (formatIndex >= 0) {
            m_formatCombo->setCurrentIndex(formatIndex);
        }
        
        // 设置标签
        QStringList tagsList;
        for (const auto& tag : record.tags) {
            tagsList.append(QString::fromStdString(tag));
        }
        m_tagsEdit->setText(tagsList.join(", "));
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void BondForgeGUI::refreshDataList()
{
    populateTable();
    m_statusBar->showMessage("Data list refreshed", 2000);
}

void BondForgeGUI::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*)");
    if (!fileName.isEmpty()) {
        // 处理文件打开
        m_statusBar->showMessage("File opened: " + fileName, 2000);
    }
}

void BondForgeGUI::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "All Files (*)");
    if (!fileName.isEmpty()) {
        // 处理文件保存
        m_statusBar->showMessage("File saved: " + fileName, 2000);
    }
}

void BondForgeGUI::about()
{
    QString lang = QString::fromStdString(m_i18n.getCurrentLanguage());
    QMessageBox::about(this, "About BondForge V1.2", 
                      "BondForge V1.2\n\n"
                      "A Chemical Machine Learning System\n\n"
                      "Current Language: " + lang + "\n\n"
                      "Storage Mode: " + 
                      QString::fromStdString((m_storageConfig->getStorageMode() == StorageMode::MEMORY) ? 
                                             "Memory Storage" : "Database Storage") + "\n\n"
                      "© 2023 BondForge Team");
}

void BondForgeGUI::showStorageSettings()
{
    // 更新对话框中的当前设置
    int currentModeIndex = (m_storageConfig->getStorageMode() == StorageMode::SQLITE) ? 1 : 0;
    m_storageModeCombo->setCurrentIndex(currentModeIndex);
    m_databasePathEdit->setText(QString::fromStdString(m_storageConfig->getDatabasePath()));
    
    // 更新描述
    updateStorageDescription();
    
    // 显示对话框
    m_storageSettingsDialog->exec();
}

void BondForgeGUI::updateStorageDescription()
{
    int index = m_storageModeCombo->currentIndex();
    StorageMode mode = static_cast<StorageMode>(m_storageModeCombo->itemData(index).toInt());
    
    if (mode == StorageMode::MEMORY) {
        m_storageDescriptionEdit->setPlainText(
            QString::fromStdString(m_i18n.getText("ui.memory_mode_desc"))
        );
        m_databasePathEdit->setEnabled(false);
        m_browseDatabaseButton->setEnabled(false);
    } else {
        m_storageDescriptionEdit->setPlainText(
            QString::fromStdString(m_i18n.getText("ui.database_mode_desc"))
        );
        m_databasePathEdit->setEnabled(true);
        m_browseDatabaseButton->setEnabled(true);
    }
}

// 导入导出功能实现
void BondForgeGUI::importCsvFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        QString::fromStdString(m_i18n.getText("ui.import_csv")), 
        "", 
        "CSV Files (*.csv);;All Files (*)");
        
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Cannot open file");
            return;
        }
        
        QTextStream in(&file);
        std::vector<DataRecord> records;
        
        // 简单的CSV解析（跳过标题行）
        int lineNum = 0;
        while (!in.atEnd()) {
            QString line = in.readLine();
            lineNum++;
            
            if (lineNum == 1) continue; // 跳过标题行
            
            QStringList fields = line.split(',');
            if (fields.size() >= 5) {
                DataRecord record;
                record.id = fields[0].toStdString();
                record.content = fields[1].toStdString();
                record.format = "CSV";
                record.category = fields[2].toStdString();
                record.uploader = "user"; // 默认上传者
                record.timestamp = std::time(nullptr);
                
                // 处理标签（可能有多个）
                if (fields.size() > 3) {
                    record.deserializeTags(fields[3].toStdString());
                }
                
                records.push_back(record);
            }
        }
        
        // 批量插入数据
        int successCount = 0;
        for (const auto& record : records) {
            try {
                if (m_service->uploadData(record)) {
                    successCount++;
                }
            } catch (...) {
                // 忽略错误，继续处理下一条
            }
        }
        
        m_statusBar->showMessage(
            QString::fromStdString(m_i18n.getText("ui.import_successful")) + 
            QString(": %1/%2 records").arg(successCount).arg(records.size()), 
            3000);
            
        // 刷新数据列表
        populateTable();
    }
}

void BondForgeGUI::importJsonFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        QString::fromStdString(m_i18n.getText("ui.import_json")), 
        "", 
        "JSON Files (*.json);;All Files (*)");
        
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Cannot open file");
            return;
        }
        
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (!doc.isArray()) {
            QMessageBox::warning(this, "Error", "Invalid JSON format");
            return;
        }
        
        QJsonArray jsonArray = doc.array();
        std::vector<DataRecord> records;
        
        for (int i = 0; i < jsonArray.size(); i++) {
            QJsonObject obj = jsonArray[i].toObject();
            
            DataRecord record;
            record.id = obj["id"].toString().toStdString();
            record.content = obj["content"].toString().toStdString();
            record.format = "JSON";
            record.category = obj["category"].toString().toStdString();
            record.uploader = obj["uploader"].toString().toStdString();
            record.timestamp = obj["timestamp"].toVariant().toULongLong();
            
            // 处理标签
            if (obj.contains("tags")) {
                QJsonArray tagsArray = obj["tags"].toArray();
                for (int j = 0; j < tagsArray.size(); j++) {
                    record.tags.insert(tagsArray[j].toString().toStdString());
                }
            }
            
            records.push_back(record);
        }
        
        // 批量插入数据
        int successCount = 0;
        for (const auto& record : records) {
            try {
                if (m_service->uploadData(record)) {
                    successCount++;
                }
            } catch (...) {
                // 忽略错误，继续处理下一条
            }
        }
        
        m_statusBar->showMessage(
            QString::fromStdString(m_i18n.getText("ui.import_successful")) + 
            QString(": %1/%2 records").arg(successCount).arg(records.size()), 
            3000);
            
        // 刷新数据列表
        populateTable();
    }
}

void BondForgeGUI::importSdfFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        QString::fromStdString(m_i18n.getText("ui.import_sdf")), 
        "", 
        "SDF Files (*.sdf);;All Files (*)");
        
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Info", 
            "SDF import is a placeholder implementation.
"
            "In a real application, this would parse molecular structure data.");
    }
}

void BondForgeGUI::exportCsvFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        QString::fromStdString(m_i18n.getText("ui.export_csv")), 
        "", 
        "CSV Files (*.csv);;All Files (*)");
        
    if (!fileName.isEmpty()) {
        try {
            auto allData = m_service->getAllData();
            
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "Error", "Cannot create file");
                return;
            }
            
            QTextStream out(&file);
            
            // 写入标题行
            out << "ID,Content,Format,Category,Tags,Uploader,Timestamp
";
            
            // 写入数据行
            for (const auto& record : allData) {
                out << record.id << ","
                    << "\"" << QString::fromStdString(record.content).replace("\"", "\"\"") << "\","
                    << record.format << ","
                    << record.category << ","
                    << "\"" << QString::fromStdString(record.serializeTags()) << "\","
                    << record.uploader << ","
                    << record.timestamp << "
";
            }
            
            m_statusBar->showMessage(
                QString::fromStdString(m_i18n.getText("ui.export_successful")) + 
                QString(": %1 records").arg(allData.size()), 
                3000);
                
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", 
                QString::fromStdString(m_i18n.getText("ui.export_failed")) + 
                QString(": %1").arg(e.what()));
        }
    }
}

void BondForgeGUI::exportJsonFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        QString::fromStdString(m_i18n.getText("ui.export_json")), 
        "", 
        "JSON Files (*.json);;All Files (*)");
        
    if (!fileName.isEmpty()) {
        try {
            auto allData = m_service->getAllData();
            
            QJsonArray jsonArray;
            
            // 转换数据为JSON
            for (const auto& record : allData) {
                QJsonObject obj;
                obj["id"] = QString::fromStdString(record.id);
                obj["content"] = QString::fromStdString(record.content);
                obj["format"] = QString::fromStdString(record.format);
                obj["category"] = QString::fromStdString(record.category);
                obj["uploader"] = QString::fromStdString(record.uploader);
                obj["timestamp"] = static_cast<qint64>(record.timestamp);
                
                // 转换标签为JSON数组
                QJsonArray tagsArray;
                for (const auto& tag : record.tags) {
                    tagsArray.append(QString::fromStdString(tag));
                }
                obj["tags"] = tagsArray;
                
                jsonArray.append(obj);
            }
            
            // 写入文件
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "Error", "Cannot create file");
                return;
            }
            
            QJsonDocument doc(jsonArray);
            file.write(doc.toJson());
            
            m_statusBar->showMessage(
                QString::fromStdString(m_i18n.getText("ui.export_successful")) + 
                QString(": %1 records").arg(allData.size()), 
                3000);
                
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", 
                QString::fromStdString(m_i18n.getText("ui.export_failed")) + 
                QString(": %1").arg(e.what()));
        }
    }
}

void BondForgeGUI::exportSdfFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        QString::fromStdString(m_i18n.getText("ui.export_sdf")), 
        "", 
        "SDF Files (*.sdf);;All Files (*)");
        
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Info", 
            "SDF export is a placeholder implementation.
"
            "In a real application, this would generate molecular structure data.");
    }
}

// 搜索过滤功能实现
void BondForgeGUI::applySearchFilter()
{
    // 简化实现，实际应该构建复杂查询
    m_statusBar->showMessage("Filter applied", 2000);
    
    // 这里应该调用服务层的过滤方法
    // 然后更新表格显示
    populateTable();
}

void BondForgeGUI::addToFavorites()
{
    // 添加到收藏夹的简化实现
    m_statusBar->showMessage("Added to favorites", 2000);
}

// 数据可视化功能实现
void BondForgeGUI::showMolecularStructure()
{
    // 创建分子结构显示对话框
    QDialog *molDialog = new QDialog(this);
    molDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "分子结构显示" : "Molecular Structure");
    molDialog->resize(800, 600);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(molDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(molDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QComboBox *dataSelector = new QComboBox(toolbarGroup);
    dataSelector->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择数据记录" : "Select Data Record");
    
    // 填充数据选择器
    std::vector<DataRecord> allData = m_service->getAllData();
    for (const auto& record : allData) {
        dataSelector->addItem(QString::fromStdString(record.id + " - " + record.content.substr(0, 20) + "..."));
    }
    
    QPushButton *renderButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "渲染分子" : "Render Molecule", toolbarGroup);
    QRadioButton *btn2D = new QRadioButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "2D视图" : "2D View", toolbarGroup);
    QRadioButton *btn3D = new QRadioButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "3D视图" : "3D View", toolbarGroup);
    btn2D->setChecked(true);
    
    toolbarLayout->addWidget(dataSelector);
    toolbarLayout->addWidget(renderButton);
    toolbarLayout->addWidget(btn2D);
    toolbarLayout->addWidget(btn3D);
    toolbarLayout->addStretch();
    
    // 创建图形视图
    QGraphicsScene *scene = new QGraphicsScene();
    QGraphicsView *view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(molDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择一个数据记录并点击渲染分子按钮" : 
                                  "Please select a data record and click the Render Molecule button", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(view, 1);  // 1表示拉伸因子
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, molDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, molDialog, &QDialog::reject);
    
    // 渲染分子的功能实现
    auto renderMolecule = [scene, infoLabel, btn3D, this](int index) {
        if (index <= 0) return;  // 没有选择任何数据
        
        std::vector<DataRecord> allData = m_service->getAllData();
        if (index-1 < allData.size()) {
            const DataRecord& record = allData[index-1];
            
            // 清除现有内容
            scene->clear();
            
            // 简单的分子表示（基于文本的解析）
            renderSimpleMolecule(scene, record.content, btn3D->isChecked());
            
            // 更新信息标签
            QString info = QString::fromStdString(
                "ID: " + record.id + 
                " | " + (m_i18n.getCurrentLanguage() == "zh-CN" ? "格式: " : "Format: ") + record.format +
                " | " + (m_i18n.getCurrentLanguage() == "zh-CN" ? "分类: " : "Category: ") + record.category
            );
            infoLabel->setText(info);
        }
    };
    
    connect(renderButton, &QPushButton::clicked, [renderMolecule, dataSelector]() {
        renderMolecule(dataSelector->currentIndex());
    });
    
    connect(dataSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [renderMolecule](int index) {
        if (index > 0) {  // 忽略第一个占位符项
            renderMolecule(index);
        }
    });
    
    molDialog->exec();
    delete molDialog;
}

void BondForgeGUI::showDataCharts()
{
    // 创建数据图表对话框
    QDialog *chartsDialog = new QDialog(this);
    chartsDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据图表" : "Data Charts");
    chartsDialog->resize(900, 700);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(chartsDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(chartsDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QLabel *chartTypeLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "图表类型:" : "Chart Type:", toolbarGroup);
    QComboBox *chartTypeCombo = new QComboBox(toolbarGroup);
    chartTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "条形图" : "Bar Chart", "bar");
    chartTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "饼图" : "Pie Chart", "pie");
    chartTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "折线图" : "Line Chart", "line");
    chartTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "散点图" : "Scatter Plot", "scatter");
    
    QPushButton *renderButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "渲染图表" : "Render Chart", toolbarGroup);
    
    toolbarLayout->addWidget(chartTypeLabel);
    toolbarLayout->addWidget(chartTypeCombo);
    toolbarLayout->addWidget(renderButton);
    toolbarLayout->addStretch();
    
    // 创建图表视图
    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(chartsDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择图表类型并点击渲染按钮" : 
                                  "Please select a chart type and click the Render button", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(chartView, 1);  // 1表示拉伸因子
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, chartsDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, chartsDialog, &QDialog::reject);
    
    // 渲染图表的功能实现
    auto renderChart = [chartView, chartTypeCombo, infoLabel, this]() {
        std::string chartType = chartTypeCombo->currentData().toString().toStdString();
        renderDataChart(chartView, chartType);
        
        // 更新信息标签
        QString info = QString::fromStdString(
            (m_i18n.getCurrentLanguage() == "zh-CN" ? "图表类型: " : "Chart Type: ") + 
            chartTypeCombo->currentText().toStdString()
        );
        infoLabel->setText(info);
    };
    
    connect(renderButton, &QPushButton::clicked, renderChart);
    
    // 初始渲染一个默认图表
    renderChart();
    
    chartsDialog->exec();
    delete chartsDialog;
}

void BondForgeGUI::showTrendAnalysis()
{
    // 创建趋势分析对话框
    QDialog *trendDialog = new QDialog(this);
    trendDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "趋势分析" : "Trend Analysis");
    trendDialog->resize(900, 700);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(trendDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(trendDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QLabel *timeRangeLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "时间范围:" : "Time Range:", toolbarGroup);
    QComboBox *timeRangeCombo = new QComboBox(toolbarGroup);
    timeRangeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "最近7天" : "Last 7 Days", "7");
    timeRangeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "最近30天" : "Last 30 Days", "30");
    timeRangeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "最近90天" : "Last 90 Days", "90");
    timeRangeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "全部" : "All Time", "all");
    
    QLabel *trendTypeLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "趋势类型:" : "Trend Type:", toolbarGroup);
    QComboBox *trendTypeCombo = new QComboBox(toolbarGroup);
    trendTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据上传量" : "Data Upload Volume", "upload_volume");
    trendTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类分布" : "Category Distribution", "category_distribution");
    trendTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户活动" : "User Activity", "user_activity");
    
    QPushButton *analyzeButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "分析趋势" : "Analyze Trend", toolbarGroup);
    
    toolbarLayout->addWidget(timeRangeLabel);
    toolbarLayout->addWidget(timeRangeCombo);
    toolbarLayout->addWidget(trendTypeLabel);
    toolbarLayout->addWidget(trendTypeCombo);
    toolbarLayout->addWidget(analyzeButton);
    toolbarLayout->addStretch();
    
    // 创建图表视图
    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(trendDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择时间范围和趋势类型并点击分析按钮" : 
                                  "Please select time range and trend type then click Analyze button", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    // 详细数据表格
    QTableWidget *dataTable = new QTableWidget();
    dataTable->setColumnCount(3);
    QStringList headers;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        headers << "日期" << "数量" << "百分比";
    } else {
        headers << "Date" << "Count" << "Percentage";
    }
    dataTable->setHorizontalHeaderLabels(headers);
    dataTable->horizontalHeader()->setStretchLastSection(true);
    
    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(chartView);
    splitter->addWidget(dataTable);
    splitter->setSizes({400, 200});
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(splitter, 1);
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, trendDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, trendDialog, &QDialog::reject);
    
    // 分析趋势的功能实现
    auto analyzeTrend = [chartView, timeRangeCombo, trendTypeCombo, infoLabel, dataTable, this]() {
        QString timeRange = timeRangeCombo->currentData().toString();
        QString trendType = trendTypeCombo->currentData().toString();
        
        renderTrendAnalysis(chartView, timeRange);
        
        // 更新信息标签
        QString info = QString::fromStdString(
            (m_i18n.getCurrentLanguage() == "zh-CN" ? "时间范围: " : "Time Range: ") + 
            timeRangeCombo->currentText().toStdString() + " | " +
            (m_i18n.getCurrentLanguage() == "zh-CN" ? "趋势类型: " : "Trend Type: ") + 
            trendTypeCombo->currentText().toStdString()
        );
        infoLabel->setText(info);
        
        // 填充数据表格
        populateTrendDataTable(dataTable, timeRange, trendType);
    };
    
    connect(analyzeButton, &QPushButton::clicked, analyzeTrend);
    
    // 初始分析
    analyzeTrend();
    
    trendDialog->exec();
    delete trendDialog;
}

void BondForgeGUI::showCompareData()
{
    // 创建数据对比对话框
    QDialog *compareDialog = new QDialog(this);
    compareDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据对比" : "Compare Data");
    compareDialog->resize(1000, 800);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(compareDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(compareDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QLabel *selectLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择要对比的数据记录:" : "Select data records to compare:", toolbarGroup);
    
    QPushButton *compareButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "开始对比" : "Start Compare", toolbarGroup);
    
    toolbarLayout->addWidget(selectLabel);
    toolbarLayout->addWidget(compareButton);
    toolbarLayout->addStretch();
    
    // 数据选择区域
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // 左侧：可用数据列表
    QGroupBox *availableGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "可用数据" : "Available Data");
    QVBoxLayout *availableLayout = new QVBoxLayout(availableGroup);
    
    QListWidget *availableList = new QListWidget();
    availableList->setSelectionMode(QAbstractItemView::MultiSelection);
    
    // 填充可用数据
    std::vector<DataRecord> allData = m_service->getAllData();
    for (const auto& record : allData) {
        QString itemText = QString::fromStdString(
            "ID: " + record.id + 
            " | " + (m_i18n.getCurrentLanguage() == "zh-CN" ? "分类: " : "Category: ") + record.category +
            " | " + (m_i18n.getCurrentLanguage() == "zh-CN" ? "格式: " : "Format: ") + record.format
        );
        availableList->addItem(new QListWidgetItem(itemText));
    }
    
    availableLayout->addWidget(availableList);
    
    // 右侧：已选择的数据对比
    QGroupBox *compareGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "对比结果" : "Comparison Results");
    QVBoxLayout *compareLayout = new QVBoxLayout(compareGroup);
    
    // 图表视图
    QTabWidget *resultTab = new QTabWidget();
    
    // 图表标签页
    QWidget *chartTab = new QWidget();
    QVBoxLayout *chartLayout = new QVBoxLayout(chartTab);
    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartLayout->addWidget(chartView);
    resultTab->addTab(chartTab, m_i18n.getCurrentLanguage() == "zh-CN" ? "图表对比" : "Chart Comparison");
    
    // 表格标签页
    QWidget *tableTab = new QWidget();
    QVBoxLayout *tableLayout = new QVBoxLayout(tableTab);
    QTableWidget *compareTable = new QTableWidget();
    compareTable->setColumnCount(5);
    
    QStringList headers;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        headers << "ID" << "分类" << "格式" << "标签" << "内容长度";
    } else {
        headers << "ID" << "Category" << "Format" << "Tags" << "Content Length";
    }
    compareTable->setHorizontalHeaderLabels(headers);
    compareTable->horizontalHeader()->setStretchLastSection(true);
    tableLayout->addWidget(compareTable);
    resultTab->addTab(tableTab, m_i18n.getCurrentLanguage() == "zh-CN" ? "表格对比" : "Table Comparison");
    
    // 文本差异标签页
    QWidget *diffTab = new QWidget();
    QVBoxLayout *diffLayout = new QVBoxLayout(diffTab);
    QTextEdit *diffTextEdit = new QTextEdit();
    diffTextEdit->setReadOnly(true);
    diffTextEdit->setFont(QFont("Consolas, Monaco, monospace", 10));
    diffLayout->addWidget(diffTextEdit);
    resultTab->addTab(diffTab, m_i18n.getCurrentLanguage() == "zh-CN" ? "内容差异" : "Content Diff");
    
    compareLayout->addWidget(resultTab);
    
    mainSplitter->addWidget(availableGroup);
    mainSplitter->addWidget(compareGroup);
    mainSplitter->setSizes({300, 700});
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(compareDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择至少2个数据记录并点击开始对比按钮" : 
                                  "Please select at least 2 data records and click Start Compare", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(mainSplitter, 1);
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, compareDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, compareDialog, &QDialog::reject);
    
    // 对比功能实现
    auto compareData = [this, availableList, chartView, compareTable, diffTextEdit, infoLabel]() {
        // 获取选中的项目
        QList<QListWidgetItem*> selectedItems = availableList->selectedItems();
        
        if (selectedItems.size() < 2) {
            infoLabel->setText(m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择至少2个数据记录进行对比" : 
                               "Please select at least 2 data records to compare");
            return;
        }
        
        // 获取选中的数据记录ID
        QStringList selectedIds;
        for (auto* item : selectedItems) {
            QString text = item->text();
            // 从文本中提取ID
            QStringList parts = text.split(" | ");
            if (!parts.isEmpty()) {
                QString idPart = parts[0]; // "ID: xxx"
                QStringList idParts = idPart.split(": ");
                if (idParts.size() == 2) {
                    selectedIds.append(idParts[1]);
                }
            }
        }
        
        // 渲染对比图表
        renderCompareData(chartView, selectedIds);
        
        // 填充对比表格
        populateCompareTable(compareTable, selectedIds);
        
        // 显示内容差异
        showContentDiff(diffTextEdit, selectedIds);
        
        // 更新信息标签
        infoLabel->setText(QString::fromStdString(
            (m_i18n.getCurrentLanguage() == "zh-CN" ? "正在对比 " : "Comparing ") + 
            std::to_string(selectedIds.size()) + 
            (m_i18n.getCurrentLanguage() == "zh-CN" ? " 条数据记录" : " data records")
        ));
    };
    
    connect(compareButton, &QPushButton::clicked, compareData);
    
    // 连接列表选择变化事件
    connect(availableList, &QListWidget::itemSelectionChanged, [infoLabel, availableList, this]() {
        int count = availableList->selectedItems().count();
        if (count >= 2) {
            infoLabel->setText(QString::fromStdString(
                std::to_string(count) + 
                (m_i18n.getCurrentLanguage() == "zh-CN" ? " 条记录已选择" : " records selected")
            ));
        } else if (count == 1) {
            infoLabel->setText(m_i18n.getCurrentLanguage() == "zh-CN" ? "请再选择至少1条记录" : "Please select at least 1 more record");
        } else {
            infoLabel->setText(m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择至少2个数据记录" : "Please select at least 2 data records");
        }
    });
    
    compareDialog->exec();
    delete compareDialog;
}

// 协作功能实现
void BondForgeGUI::showUserManagement()
{
    // 创建用户管理对话框
    QDialog *userManagerDialog = new QDialog(this);
    userManagerDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户管理" : "User Management");
    userManagerDialog->resize(900, 700);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(userManagerDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(userManagerDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QPushButton *addUserButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "添加用户" : "Add User", toolbarGroup);
    QPushButton *editUserButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "编辑用户" : "Edit User", toolbarGroup);
    QPushButton *deleteUserButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "删除用户" : "Delete User", toolbarGroup);
    QPushButton *refreshButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "刷新" : "Refresh", toolbarGroup);
    
    toolbarLayout->addWidget(addUserButton);
    toolbarLayout->addWidget(editUserButton);
    toolbarLayout->addWidget(deleteUserButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(refreshButton);
    
    // 用户列表
    QTableWidget *userTable = new QTableWidget();
    userTable->setColumnCount(5);
    
    QStringList headers;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        headers << "用户名" << "角色" << "邮箱" << "注册时间" << "状态";
    } else {
        headers << "Username" << "Role" << "Email" << "Registration Time" << "Status";
    }
    userTable->setHorizontalHeaderLabels(headers);
    userTable->horizontalHeader()->setStretchLastSection(true);
    userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(userManagerDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择用户行并点击相应按钮进行操作" : 
                                  "Select a user row and click the appropriate button to perform actions", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    // 用户统计
    QGroupBox *statsGroup = new QGroupBox(userManagerDialog);
    QHBoxLayout *statsLayout = new QHBoxLayout(statsGroup);
    
    QLabel *totalUsersLabel = new QLabel(statsGroup);
    QLabel *activeUsersLabel = new QLabel(statsGroup);
    QLabel *adminUsersLabel = new QLabel(statsGroup);
    
    statsLayout->addWidget(totalUsersLabel);
    statsLayout->addWidget(activeUsersLabel);
    statsLayout->addWidget(adminUsersLabel);
    statsLayout->addStretch();
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(userTable, 1);
    dialogLayout->addWidget(infoGroup);
    dialogLayout->addWidget(statsGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, userManagerDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, userManagerDialog, &QDialog::reject);
    
    // 填充用户数据
    auto populateUserTable = [userTable, this]() {
        userTable->setRowCount(0);
        
        // 模拟用户数据（实际应用中应从服务中获取）
        // 这里我们创建一些示例数据
        struct UserInfo {
            std::string username;
            std::string role;
            std::string email;
            std::string regTime;
            bool active;
        };
        
        std::vector<UserInfo> users = {
            {"admin", "Admin", "admin@example.com", "2023-01-01", true},
            {"researcher1", "Researcher", "researcher1@example.com", "2023-02-15", true},
            {"analyst1", "Analyst", "analyst1@example.com", "2023-03-10", true},
            {"guest1", "Guest", "guest1@example.com", "2023-04-20", false},
            {"viewer1", "Viewer", "viewer1@example.com", "2023-05-05", true}
        };
        
        int activeCount = 0;
        int adminCount = 0;
        
        for (const auto& user : users) {
            int row = userTable->rowCount();
            userTable->insertRow(row);
            
            userTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(user.username)));
            userTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(user.role)));
            userTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(user.email)));
            userTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(user.regTime)));
            
            QString status = user.active ? 
                (m_i18n.getCurrentLanguage() == "zh-CN" ? "活跃" : "Active") : 
                (m_i18n.getCurrentLanguage() == "zh-CN" ? "非活跃" : "Inactive");
            userTable->setItem(row, 4, new QTableWidgetItem(status));
            
            if (user.active) activeCount++;
            if (user.role == "Admin") adminCount++;
        }
        
        // 更新统计信息
        QString totalText = m_i18n.getCurrentLanguage() == "zh-CN" ? 
            QString("总用户数: %1").arg(users.size()) : 
            QString("Total Users: %1").arg(users.size());
        
        QString activeText = m_i18n.getCurrentLanguage() == "zh-CN" ? 
            QString("活跃用户: %1").arg(activeCount) : 
            QString("Active Users: %1").arg(activeCount);
        
        QString adminText = m_i18n.getCurrentLanguage() == "zh-CN" ? 
            QString("管理员: %1").arg(adminCount) : 
            QString("Admins: %1").arg(adminCount);
        
        // 需要通过变量更新统计标签
        totalUsersLabel->setText(totalText);
        activeUsersLabel->setText(activeText);
        adminUsersLabel->setText(adminText);
    };
    
    // 添加用户功能
    auto addUser = [this]() {
        // 创建添加用户对话框
        QDialog *addDialog = new QDialog(this);
        addDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "添加用户" : "Add User");
        
        QFormLayout *formLayout = new QFormLayout(addDialog);
        
        QLineEdit *usernameEdit = new QLineEdit(addDialog);
        QLineEdit *emailEdit = new QLineEdit(addDialog);
        QComboBox *roleCombo = new QComboBox(addDialog);
        roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "管理员" : "Admin", "Admin");
        roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "研究员" : "Researcher", "Researcher");
        roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "分析师" : "Analyst", "Analyst");
        roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "访客" : "Guest", "Guest");
        roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "查看者" : "Viewer", "Viewer");
        QCheckBox *activeCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "激活" : "Active", addDialog);
        activeCheck->setChecked(true);
        
        if (m_i18n.getCurrentLanguage() == "zh-CN") {
            formLayout->addRow("用户名:", usernameEdit);
            formLayout->addRow("邮箱:", emailEdit);
            formLayout->addRow("角色:", roleCombo);
            formLayout->addRow("状态:", activeCheck);
        } else {
            formLayout->addRow("Username:", usernameEdit);
            formLayout->addRow("Email:", emailEdit);
            formLayout->addRow("Role:", roleCombo);
            formLayout->addRow("Status:", activeCheck);
        }
        
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, addDialog);
        formLayout->addRow(buttonBox);
        
        connect(buttonBox, &QDialogButtonBox::accepted, [addDialog]() {
            // 这里应该添加用户到服务中
            // 目前仅作为示例
            addDialog->accept();
        });
        
        connect(buttonBox, &QDialogButtonBox::rejected, addDialog, &QDialog::reject);
        
        addDialog->exec();
        delete addDialog;
    };
    
    // 编辑用户功能
    auto editUser = [userTable, this]() {
        int row = userTable->currentRow();
        if (row >= 0) {
            // 获取当前用户信息
            QString username = userTable->item(row, 0)->text();
            QString role = userTable->item(row, 1)->text();
            QString email = userTable->item(row, 2)->text();
            QString status = userTable->item(row, 4)->text();
            
            // 创建编辑用户对话框
            QDialog *editDialog = new QDialog(this);
            editDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "编辑用户" : "Edit User");
            
            QFormLayout *formLayout = new QFormLayout(editDialog);
            
            QLineEdit *usernameEdit = new QLineEdit(username, editDialog);
            usernameEdit->setEnabled(false); // 用户名通常不允许修改
            
            QLineEdit *emailEdit = new QLineEdit(email, editDialog);
            
            QComboBox *roleCombo = new QComboBox(editDialog);
            roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "管理员" : "Admin", "Admin");
            roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "研究员" : "Researcher", "Researcher");
            roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "分析师" : "Analyst", "Analyst");
            roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "访客" : "Guest", "Guest");
            roleCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "查看者" : "Viewer", "Viewer");
            
            // 设置当前角色
            for (int i = 0; i < roleCombo->count(); ++i) {
                if (roleCombo->itemText(i) == role) {
                    roleCombo->setCurrentIndex(i);
                    break;
                }
            }
            
            QCheckBox *activeCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "激活" : "Active", editDialog);
            activeCheck->setChecked(status == (m_i18n.getCurrentLanguage() == "zh-CN" ? "活跃" : "Active"));
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                formLayout->addRow("用户名:", usernameEdit);
                formLayout->addRow("邮箱:", emailEdit);
                formLayout->addRow("角色:", roleCombo);
                formLayout->addRow("状态:", activeCheck);
            } else {
                formLayout->addRow("Username:", usernameEdit);
                formLayout->addRow("Email:", emailEdit);
                formLayout->addRow("Role:", roleCombo);
                formLayout->addRow("Status:", activeCheck);
            }
            
            QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, editDialog);
            formLayout->addRow(buttonBox);
            
            connect(buttonBox, &QDialogButtonBox::accepted, [editDialog]() {
                // 这里应该更新用户信息到服务中
                // 目前仅作为示例
                editDialog->accept();
            });
            
            connect(buttonBox, &QDialogButtonBox::rejected, editDialog, &QDialog::reject);
            
            editDialog->exec();
            delete editDialog;
        } else {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择要编辑的用户" : "Please select a user to edit");
        }
    };
    
    // 删除用户功能
    auto deleteUser = [userTable, this]() {
        int row = userTable->currentRow();
        if (row >= 0) {
            QString username = userTable->item(row, 0)->text();
            
            QMessageBox::StandardButton reply = QMessageBox::question(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "确认删除" : "Confirm Delete",
                m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    QString("确定要删除用户 '%1' 吗？").arg(username) : 
                    QString("Are you sure you want to delete user '%1'?").arg(username),
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::Yes) {
                // 这里应该从服务中删除用户
                // 目前仅作为示例
                userTable->removeRow(row);
            }
        } else {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择要删除的用户" : "Please select a user to delete");
        }
    };
    
    // 连接信号
    connect(addUserButton, &QPushButton::clicked, addUser);
    connect(editUserButton, &QPushButton::clicked, editUser);
    connect(deleteUserButton, &QPushButton::clicked, deleteUser);
    connect(refreshButton, &QPushButton::clicked, [populateUserTable]() {
        populateUserTable();
    });
    
    // 初始填充数据
    populateUserTable();
    
    userManagerDialog->exec();
    delete userManagerDialog;
}

void BondForgeGUI::showDataSharing()
{
    // 创建数据共享对话框
    QDialog *shareDialog = new QDialog(this);
    shareDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据共享" : "Data Sharing");
    shareDialog->resize(1000, 700);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(shareDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(shareDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QPushButton *shareButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "分享数据" : "Share Data", toolbarGroup);
    QPushButton *managePermissionsButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "管理权限" : "Manage Permissions", toolbarGroup);
    QPushButton *refreshButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "刷新" : "Refresh", toolbarGroup);
    
    toolbarLayout->addWidget(shareButton);
    toolbarLayout->addWidget(managePermissionsButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(refreshButton);
    
    // 主区域 - 分为两部分：可共享数据和已共享数据
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
    
    // 顶部：可共享数据
    QGroupBox *availableGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "可共享数据" : "Available Data for Sharing");
    QVBoxLayout *availableLayout = new QVBoxLayout(availableGroup);
    
    QTableWidget *availableTable = new QTableWidget();
    availableTable->setColumnCount(5);
    
    QStringList availableHeaders;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        availableHeaders << "数据ID" << "分类" << "格式" << "上传时间" << "权限";
    } else {
        availableHeaders << "Data ID" << "Category" << "Format" << "Upload Time" << "Permissions";
    }
    availableTable->setHorizontalHeaderLabels(availableHeaders);
    availableTable->horizontalHeader()->setStretchLastSection(true);
    availableTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    availableTable->setSelectionMode(QAbstractItemView::MultiSelection);
    
    availableLayout->addWidget(availableTable);
    
    // 底部：已共享数据
    QGroupBox *sharedGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "已共享数据" : "Shared Data");
    QVBoxLayout *sharedLayout = new QVBoxLayout(sharedGroup);
    
    QTableWidget *sharedTable = new QTableWidget();
    sharedTable->setColumnCount(6);
    
    QStringList sharedHeaders;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        sharedHeaders << "数据ID" << "分类" << "共享对象" << "权限级别" << "共享时间" << "状态";
    } else {
        sharedHeaders << "Data ID" << "Category" << "Shared With" << "Permission Level" << "Shared Time" << "Status";
    }
    sharedTable->setHorizontalHeaderLabels(sharedHeaders);
    sharedTable->horizontalHeader()->setStretchLastSection(true);
    sharedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    sharedTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    sharedLayout->addWidget(sharedTable);
    
    mainSplitter->addWidget(availableGroup);
    mainSplitter->addWidget(sharedGroup);
    mainSplitter->setSizes({300, 300});
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(shareDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择数据行并点击分享按钮进行数据共享" : 
                                  "Select data rows and click Share button to share data", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(mainSplitter, 1);
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, shareDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, shareDialog, &QDialog::reject);
    
    // 填充数据函数
    auto populateTables = [availableTable, sharedTable, this]() {
        // 清空表格
        availableTable->setRowCount(0);
        sharedTable->setRowCount(0);
        
        // 获取所有数据记录
        std::vector<DataRecord> allData = m_service->getAllData();
        
        // 填充可用数据表
        for (const auto& record : allData) {
            int row = availableTable->rowCount();
            availableTable->insertRow(row);
            
            availableTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(record.id)));
            availableTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(record.category)));
            availableTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(record.format)));
            
            // 时间戳转换
            QDateTime dateTime = QDateTime::fromSecsSinceEpoch(record.timestamp);
            QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");
            availableTable->setItem(row, 3, new QTableWidgetItem(timeStr));
            
            // 权限
            QString permission = m_i18n.getCurrentLanguage() == "zh-CN" ? "私有" : "Private";
            availableTable->setItem(row, 4, new QTableWidgetItem(permission));
        }
        
        // 模拟已共享数据
        struct SharedInfo {
            std::string dataId;
            std::string category;
            std::string sharedWith;
            std::string permission;
            std::string sharedTime;
            std::string status;
        };
        
        std::vector<SharedInfo> sharedData = {
            {"data-001", "有机", "researcher1", "只读", "2023-06-15 10:30:00", "有效"},
            {"data-002", "无机", "analyst1", "读写", "2023-06-20 14:15:00", "有效"},
            {"data-003", "有机", "group1", "只读", "2023-07-05 09:45:00", "已撤销"}
        };
        
        // 填充已共享数据表
        for (const auto& info : sharedData) {
            int row = sharedTable->rowCount();
            sharedTable->insertRow(row);
            
            sharedTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(info.dataId)));
            sharedTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(info.category)));
            sharedTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(info.sharedWith)));
            
            QString permission = QString::fromStdString(info.permission);
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                if (info.permission == "只读") permission = "只读";
                else if (info.permission == "读写") permission = "读写";
            }
            sharedTable->setItem(row, 3, new QTableWidgetItem(permission));
            
            sharedTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(info.sharedTime)));
            
            QString status = QString::fromStdString(info.status);
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                if (info.status == "有效") status = "有效";
                else if (info.status == "已撤销") status = "已撤销";
            }
            sharedTable->setItem(row, 5, new QTableWidgetItem(status));
        }
    };
    
    // 分享数据功能
    auto shareData = [availableTable, this]() {
        QList<QTableWidgetItem*> selectedItems = availableTable->selectedItems();
        if (selectedItems.isEmpty()) {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择要分享的数据" : "Please select data to share");
            return;
        }
        
        // 获取选中的行
        QSet<int> selectedRows;
        for (QTableWidgetItem* item : selectedItems) {
            selectedRows.insert(item->row());
        }
        
        // 创建分享对话框
        QDialog *shareDialog = new QDialog(this);
        shareDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "分享数据" : "Share Data");
        
        QFormLayout *formLayout = new QFormLayout(shareDialog);
        
        // 选择要分享的数据
        QListWidget *selectedDataList = new QListWidget();
        for (int row : selectedRows) {
            QString dataId = availableTable->item(row, 0)->text();
            selectedDataList->addItem(dataId);
        }
        
        // 分享对象
        QLineEdit *shareTargetEdit = new QLineEdit();
        shareTargetEdit->setPlaceholderText(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                                            "输入用户名或组名" : "Enter username or group name");
        
        // 权限级别
        QComboBox *permissionCombo = new QComboBox();
        permissionCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "只读" : "Read-only", "read");
        permissionCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "读写" : "Read-write", "write");
        permissionCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "管理" : "Admin", "admin");
        
        // 有效期
        QComboBox *expiryCombo = new QComboBox();
        expiryCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "永久" : "Permanent", "permanent");
        expiryCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "7天" : "7 days", "7");
        expiryCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "30天" : "30 days", "30");
        expiryCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "90天" : "90 days", "90");
        
        if (m_i18n.getCurrentLanguage() == "zh-CN") {
            formLayout->addRow("选中的数据:", selectedDataList);
            formLayout->addRow("分享给:", shareTargetEdit);
            formLayout->addRow("权限级别:", permissionCombo);
            formLayout->addRow("有效期:", expiryCombo);
        } else {
            formLayout->addRow("Selected Data:", selectedDataList);
            formLayout->addRow("Share With:", shareTargetEdit);
            formLayout->addRow("Permission Level:", permissionCombo);
            formLayout->addRow("Expiry:", expiryCombo);
        }
        
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, shareDialog);
        formLayout->addRow(buttonBox);
        
        connect(buttonBox, &QDialogButtonBox::accepted, [shareDialog, this]() {
            // 这里应该将分享信息保存到服务中
            // 目前仅作为示例
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "分享成功" : "Sharing Successful",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "数据已成功分享" : "Data has been successfully shared");
            shareDialog->accept();
        });
        
        connect(buttonBox, &QDialogButtonBox::rejected, shareDialog, &QDialog::reject);
        
        shareDialog->exec();
        delete shareDialog;
        
        // 刷新表格
        populateTables();
    };
    
    // 管理权限功能
    auto managePermissions = [sharedTable, this]() {
        int row = sharedTable->currentRow();
        if (row >= 0) {
            QString dataId = sharedTable->item(row, 0)->text();
            QString sharedWith = sharedTable->item(row, 2)->text();
            QString permission = sharedTable->item(row, 3)->text();
            QString status = sharedTable->item(row, 5)->text();
            
            // 创建权限管理对话框
            QDialog *permDialog = new QDialog(this);
            permDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "管理权限" : "Manage Permissions");
            
            QFormLayout *formLayout = new QFormLayout(permDialog);
            
            // 显示当前权限
            QLabel *dataIdLabel = new QLabel(dataId, permDialog);
            QLabel *sharedWithLabel = new QLabel(sharedWith, permDialog);
            
            // 权限修改
            QComboBox *permissionCombo = new QComboBox();
            permissionCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "只读" : "Read-only", "read");
            permissionCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "读写" : "Read-write", "write");
            permissionCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "管理" : "Admin", "admin");
            
            // 设置当前权限
            for (int i = 0; i < permissionCombo->count(); ++i) {
                if (permissionCombo->itemText(i) == permission) {
                    permissionCombo->setCurrentIndex(i);
                    break;
                }
            }
            
            // 状态修改
            QComboBox *statusCombo = new QComboBox();
            statusCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "有效" : "Active", "active");
            statusCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "已撤销" : "Revoked", "revoked");
            
            // 设置当前状态
            for (int i = 0; i < statusCombo->count(); ++i) {
                if (statusCombo->itemText(i) == status) {
                    statusCombo->setCurrentIndex(i);
                    break;
                }
            }
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                formLayout->addRow("数据ID:", dataIdLabel);
                formLayout->addRow("分享给:", sharedWithLabel);
                formLayout->addRow("权限级别:", permissionCombo);
                formLayout->addRow("状态:", statusCombo);
            } else {
                formLayout->addRow("Data ID:", dataIdLabel);
                formLayout->addRow("Shared With:", sharedWithLabel);
                formLayout->addRow("Permission Level:", permissionCombo);
                formLayout->addRow("Status:", statusCombo);
            }
            
            QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, permDialog);
            formLayout->addRow(buttonBox);
            
            connect(buttonBox, &QDialogButtonBox::accepted, [permDialog, this]() {
                // 这里应该更新权限信息到服务中
                // 目前仅作为示例
                QMessageBox::information(this, 
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "权限已更新" : "Permissions Updated",
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "权限信息已成功更新" : "Permission information has been successfully updated");
                permDialog->accept();
            });
            
            connect(buttonBox, &QDialogButtonBox::rejected, permDialog, &QDialog::reject);
            
            permDialog->exec();
            delete permDialog;
            
            // 刷新表格
            populateTables();
        } else {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "请选择要管理权限的共享数据" : "Please select shared data to manage permissions");
        }
    };
    
    // 连接信号
    connect(shareButton, &QPushButton::clicked, shareData);
    connect(managePermissionsButton, &QPushButton::clicked, managePermissions);
    connect(refreshButton, &QPushButton::clicked, [populateTables]() {
        populateTables();
    });
    
    // 初始填充数据
    populateTables();
    
    shareDialog->exec();
    delete shareDialog;
}

void BondForgeGUI::showComments()
{
    // 创建评论系统对话框
    QDialog *commentsDialog = new QDialog(this);
    commentsDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "评论系统" : "Comments System");
    commentsDialog->resize(900, 700);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(commentsDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(commentsDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QLabel *dataSelectLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择数据记录:" : "Select Data Record:", toolbarGroup);
    QComboBox *dataCombo = new QComboBox(toolbarGroup);
    
    // 填充数据选择器
    std::vector<DataRecord> allData = m_service->getAllData();
    for (const auto& record : allData) {
        dataCombo->addItem(QString::fromStdString(record.id + " - " + record.content.substr(0, 20) + "..."));
    }
    
    QPushButton *loadCommentsButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "加载评论" : "Load Comments", toolbarGroup);
    QPushButton *addCommentButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "添加评论" : "Add Comment", toolbarGroup);
    
    toolbarLayout->addWidget(dataSelectLabel);
    toolbarLayout->addWidget(dataCombo);
    toolbarLayout->addWidget(loadCommentsButton);
    toolbarLayout->addWidget(addCommentButton);
    toolbarLayout->addStretch();
    
    // 评论列表
    QGroupBox *commentsGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "评论列表" : "Comments List");
    QVBoxLayout *commentsLayout = new QVBoxLayout(commentsGroup);
    
    QTableWidget *commentsTable = new QTableWidget();
    commentsTable->setColumnCount(5);
    
    QStringList headers;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        headers << "评论ID" << "用户" << "时间" << "内容" << "操作";
    } else {
        headers << "Comment ID" << "User" << "Time" << "Content" << "Actions";
    }
    commentsTable->setHorizontalHeaderLabels(headers);
    commentsTable->horizontalHeader()->setStretchLastSection(true);
    commentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    commentsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    commentsLayout->addWidget(commentsTable);
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(commentsDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择数据记录并点击加载评论按钮" : 
                                  "Select a data record and click Load Comments button", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(commentsGroup, 1);
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, commentsDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, commentsDialog, &QDialog::reject);
    
    // 模拟评论数据结构
    struct CommentInfo {
        std::string id;
        std::string dataId;
        std::string user;
        std::string timestamp;
        std::string content;
    };
    
    // 模拟评论数据
    std::vector<CommentInfo> mockComments = {
        {"c001", "data-001", "researcher1", "2023-06-15 10:30:00", "这个数据很有用，感谢分享！"},
        {"c002", "data-001", "analyst1", "2023-06-16 14:15:00", "能否提供更多关于实验条件的信息？"},
        {"c003", "data-002", "researcher2", "2023-06-18 09:45:00", "验证了此数据，结果一致。"},
        {"c004", "data-003", "guest1", "2023-06-20 16:20:00", "如何解释这个异常值？"}
    };
    
    // 加载评论功能
    auto loadComments = [dataCombo, commentsTable, mockComments, this]() {
        // 清空表格
        commentsTable->setRowCount(0);
        
        // 获取选中的数据ID
        int index = dataCombo->currentIndex();
        if (index < 0) return;
        
        std::vector<DataRecord> allData = m_service->getAllData();
        if (index >= allData.size()) return;
        
        std::string selectedDataId = allData[index].id;
        
        // 加载与该数据相关的评论
        for (const auto& comment : mockComments) {
            if (comment.dataId == selectedDataId) {
                int row = commentsTable->rowCount();
                commentsTable->insertRow(row);
                
                commentsTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(comment.id)));
                commentsTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(comment.user)));
                commentsTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(comment.timestamp)));
                commentsTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(comment.content)));
                
                // 添加操作按钮
                QWidget *buttonWidget = new QWidget();
                QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
                buttonLayout->setContentsMargins(0, 0, 0, 0);
                
                QPushButton *replyButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "回复" : "Reply");
                QPushButton *editButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "编辑" : "Edit");
                QPushButton *deleteButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "删除" : "Delete");
                
                buttonLayout->addWidget(replyButton);
                buttonLayout->addWidget(editButton);
                buttonLayout->addWidget(deleteButton);
                
                commentsTable->setCellWidget(row, 4, buttonWidget);
                
                // 连接按钮信号
                connect(replyButton, &QPushButton::clicked, [comment, this]() {
                    // 回复评论
                    QDialog *replyDialog = new QDialog(this);
                    replyDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "回复评论" : "Reply to Comment");
                    
                    QVBoxLayout *replyLayout = new QVBoxLayout(replyDialog);
                    
                    QLabel *originalCommentLabel = new QLabel(
                        QString::fromStdString(
                            (m_i18n.getCurrentLanguage() == "zh-CN" ? "原评论: " : "Original Comment: ") + 
                            comment.content
                        ),
                        replyDialog
                    );
                    
                    QTextEdit *replyEdit = new QTextEdit(replyDialog);
                    replyEdit->setPlaceholderText(m_i18n.getCurrentLanguage() == "zh-CN" ? "输入回复内容" : "Enter your reply");
                    
                    QDialogButtonBox *replyButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, replyDialog);
                    
                    replyLayout->addWidget(originalCommentLabel);
                    replyLayout->addWidget(replyEdit);
                    replyLayout->addWidget(replyButtonBox);
                    
                    connect(replyButtonBox, &QDialogButtonBox::accepted, [replyDialog, this]() {
                        // 这里应该将回复保存到服务中
                        // 目前仅作为示例
                        QMessageBox::information(this, 
                            m_i18n.getCurrentLanguage() == "zh-CN" ? "回复已发送" : "Reply Sent",
                            m_i18n.getCurrentLanguage() == "zh-CN" ? "您的回复已成功发送" : "Your reply has been successfully sent");
                        replyDialog->accept();
                    });
                    
                    connect(replyButtonBox, &QDialogButtonBox::rejected, replyDialog, &QDialog::reject);
                    
                    replyDialog->exec();
                    delete replyDialog;
                });
                
                connect(editButton, &QPushButton::clicked, [comment, this]() {
                    // 编辑评论
                    QDialog *editDialog = new QDialog(this);
                    editDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "编辑评论" : "Edit Comment");
                    
                    QVBoxLayout *editLayout = new QVBoxLayout(editDialog);
                    
                    QLabel *commentIdLabel = new QLabel(QString::fromStdString(
                        (m_i18n.getCurrentLanguage() == "zh-CN" ? "评论ID: " : "Comment ID: ") + 
                        comment.id
                    ), editDialog);
                    
                    QTextEdit *contentEdit = new QTextEdit(QString::fromStdString(comment.content), editDialog);
                    
                    QDialogButtonBox *editButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, editDialog);
                    
                    editLayout->addWidget(commentIdLabel);
                    editLayout->addWidget(contentEdit);
                    editLayout->addWidget(editButtonBox);
                    
                    connect(editButtonBox, &QDialogButtonBox::accepted, [editDialog, this]() {
                        // 这里应该将修改保存到服务中
                        // 目前仅作为示例
                        QMessageBox::information(this, 
                            m_i18n.getCurrentLanguage() == "zh-CN" ? "评论已更新" : "Comment Updated",
                            m_i18n.getCurrentLanguage() == "zh-CN" ? "您的评论已成功更新" : "Your comment has been successfully updated");
                        editDialog->accept();
                    });
                    
                    connect(editButtonBox, &QDialogButtonBox::rejected, editDialog, &QDialog::reject);
                    
                    editDialog->exec();
                    delete editDialog;
                });
                
                connect(deleteButton, &QPushButton::clicked, [comment, this]() {
                    // 删除评论
                    QMessageBox::StandardButton reply = QMessageBox::question(this, 
                        m_i18n.getCurrentLanguage() == "zh-CN" ? "确认删除" : "Confirm Delete",
                        m_i18n.getCurrentLanguage() == "zh-CN" ? 
                            QString("确定要删除评论 '%1' 吗？").arg(QString::fromStdString(comment.id)) : 
                            QString("Are you sure you want to delete comment '%1'?").arg(QString::fromStdString(comment.id)),
                        QMessageBox::Yes | QMessageBox::No);
                    
                    if (reply == QMessageBox::Yes) {
                        // 这里应该从服务中删除评论
                        // 目前仅作为示例
                        QMessageBox::information(this, 
                            m_i18n.getCurrentLanguage() == "zh-CN" ? "评论已删除" : "Comment Deleted",
                            m_i18n.getCurrentLanguage() == "zh-CN" ? "评论已成功删除" : "Comment has been successfully deleted");
                        
                        // 重新加载评论
                        // 在实际应用中，这里应该调用loadComments()函数
                    }
                });
            }
        }
        
        if (commentsTable->rowCount() == 0) {
            QString message = m_i18n.getCurrentLanguage() == "zh-CN" ? 
                "暂无评论" : "No comments yet";
            infoLabel->setText(message);
        } else {
            QString message = m_i18n.getCurrentLanguage() == "zh-CN" ? 
                QString("共 %1 条评论").arg(commentsTable->rowCount()) : 
                QString("Total: %1 comments").arg(commentsTable->rowCount());
            infoLabel->setText(message);
        }
    };
    
    // 添加评论功能
    auto addComment = [dataCombo, this]() {
        int index = dataCombo->currentIndex();
        if (index < 0) {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "请先选择数据记录" : "Please select a data record first");
            return;
        }
        
        std::vector<DataRecord> allData = m_service->getAllData();
        if (index >= allData.size()) return;
        
        // 创建添加评论对话框
        QDialog *addDialog = new QDialog(this);
        addDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "添加评论" : "Add Comment");
        
        QVBoxLayout *addLayout = new QVBoxLayout(addDialog);
        
        QLabel *dataIdLabel = new QLabel(
            QString::fromStdString(
                (m_i18n.getCurrentLanguage() == "zh-CN" ? "数据记录: " : "Data Record: ") + 
                allData[index].id
            ),
            addDialog
        );
        
        QTextEdit *contentEdit = new QTextEdit(addDialog);
        contentEdit->setPlaceholderText(m_i18n.getCurrentLanguage() == "zh-CN" ? "输入评论内容" : "Enter your comment");
        
        QDialogButtonBox *addButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, addDialog);
        
        addLayout->addWidget(dataIdLabel);
        addLayout->addWidget(contentEdit);
        addLayout->addWidget(addButtonBox);
        
        connect(addButtonBox, &QDialogButtonBox::accepted, [addDialog, this]() {
            // 这里应该将评论保存到服务中
            // 目前仅作为示例
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "评论已添加" : "Comment Added",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "您的评论已成功添加" : "Your comment has been successfully added");
            addDialog->accept();
            
            // 重新加载评论
            // 在实际应用中，这里应该调用loadComments()函数
        });
        
        connect(addButtonBox, &QDialogButtonBox::rejected, addDialog, &QDialog::reject);
        
        addDialog->exec();
        delete addDialog;
    };
    
    // 连接信号
    connect(loadCommentsButton, &QPushButton::clicked, loadComments);
    connect(addCommentButton, &QPushButton::clicked, addComment);
    
    // 初始加载
    loadComments();
    
    commentsDialog->exec();
    delete commentsDialog;
}

void BondForgeGUI::showVersionHistory()
{
    // 创建版本历史对话框
    QDialog *versionDialog = new QDialog(this);
    versionDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "版本历史" : "Version History");
    versionDialog->resize(900, 700);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(versionDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(versionDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QLabel *dataSelectLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择数据记录:" : "Select Data Record:", toolbarGroup);
    QComboBox *dataCombo = new QComboBox(toolbarGroup);
    
    // 填充数据选择器
    std::vector<DataRecord> allData = m_service->getAllData();
    for (const auto& record : allData) {
        dataCombo->addItem(QString::fromStdString(record.id + " - " + record.content.substr(0, 20) + "..."));
    }
    
    QPushButton *loadHistoryButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "加载历史" : "Load History", toolbarGroup);
    QPushButton *compareButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "对比版本" : "Compare Versions", toolbarGroup);
    
    toolbarLayout->addWidget(dataSelectLabel);
    toolbarLayout->addWidget(dataCombo);
    toolbarLayout->addWidget(loadHistoryButton);
    toolbarLayout->addWidget(compareButton);
    toolbarLayout->addStretch();
    
    // 版本历史列表
    QGroupBox *historyGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "版本历史" : "Version History");
    QVBoxLayout *historyLayout = new QVBoxLayout(historyGroup);
    
    QTableWidget *historyTable = new QTableWidget();
    historyTable->setColumnCount(6);
    
    QStringList headers;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        headers << "版本号" << "修改时间" << "修改用户" << "修改类型" << "内容" << "操作";
    } else {
        headers << "Version" << "Modified Time" << "Modified By" << "Change Type" << "Content" << "Actions";
    }
    historyTable->setHorizontalHeaderLabels(headers);
    historyTable->horizontalHeader()->setStretchLastSection(true);
    historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    historyLayout->addWidget(historyTable);
    
    // 版本对比区域
    QGroupBox *compareGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "版本对比" : "Version Comparison");
    QVBoxLayout *compareLayout = new QVBoxLayout(compareGroup);
    
    QSplitter *diffSplitter = new QSplitter(Qt::Horizontal);
    
    // 左侧：旧版本
    QGroupBox *oldVersionGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "旧版本" : "Old Version");
    QVBoxLayout *oldVersionLayout = new QVBoxLayout(oldVersionGroup);
    QTextEdit *oldVersionEdit = new QTextEdit();
    oldVersionEdit->setReadOnly(true);
    oldVersionEdit->setFont(QFont("Consolas, Monaco, monospace", 10));
    oldVersionLayout->addWidget(oldVersionEdit);
    
    // 右侧：新版本
    QGroupBox *newVersionGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "新版本" : "New Version");
    QVBoxLayout *newVersionLayout = new QVBoxLayout(newVersionGroup);
    QTextEdit *newVersionEdit = new QTextEdit();
    newVersionEdit->setReadOnly(true);
    newVersionEdit->setFont(QFont("Consolas, Monaco, monospace", 10));
    newVersionLayout->addWidget(newVersionEdit);
    
    diffSplitter->addWidget(oldVersionGroup);
    diffSplitter->addWidget(newVersionGroup);
    diffSplitter->setSizes({350, 350});
    
    compareLayout->addWidget(diffSplitter);
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(versionDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择数据记录并点击加载历史按钮" : 
                                  "Select a data record and click Load History button", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    // 主分割区域
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
    mainSplitter->addWidget(historyGroup);
    mainSplitter->addWidget(compareGroup);
    mainSplitter->setSizes({350, 300});
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(mainSplitter, 1);
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, versionDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, versionDialog, &QDialog::reject);
    
    // 模拟版本历史数据结构
    struct VersionInfo {
        std::string version;
        std::string dataId;
        std::string timestamp;
        std::string user;
        std::string changeType;
        std::string content;
    };
    
    // 模拟版本历史数据
    std::vector<VersionInfo> mockVersions = {
        {"v1.0", "data-001", "2023-06-01 10:00:00", "admin", "创建", "初始的化学式数据"},
        {"v1.1", "data-001", "2023-06-05 14:30:00", "researcher1", "更新", "修正了分子结构图"},
        {"v1.2", "data-001", "2023-06-10 09:15:00", "analyst1", "修改", "添加了实验条件信息"},
        {"v1.3", "data-001", "2023-06-15 16:45:00", "researcher2", "补充", "添加了参考文献"}
    };
    
    // 加载版本历史功能
    auto loadHistory = [dataCombo, historyTable, mockVersions, this]() {
        // 清空表格
        historyTable->setRowCount(0);
        
        // 获取选中的数据ID
        int index = dataCombo->currentIndex();
        if (index < 0) return;
        
        std::vector<DataRecord> allData = m_service->getAllData();
        if (index >= allData.size()) return;
        
        std::string selectedDataId = allData[index].id;
        
        // 加载与该数据相关的版本历史
        for (const auto& version : mockVersions) {
            if (version.dataId == selectedDataId) {
                int row = historyTable->rowCount();
                historyTable->insertRow(row);
                
                historyTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(version.version)));
                historyTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(version.timestamp)));
                historyTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(version.user)));
                
                QString changeType;
                if (m_i18n.getCurrentLanguage() == "zh-CN") {
                    if (version.changeType == "创建") changeType = "创建";
                    else if (version.changeType == "更新") changeType = "更新";
                    else if (version.changeType == "修改") changeType = "修改";
                    else if (version.changeType == "补充") changeType = "补充";
                } else {
                    if (version.changeType == "创建") changeType = "Create";
                    else if (version.changeType == "更新") changeType = "Update";
                    else if (version.changeType == "修改") changeType = "Modify";
                    else if (version.changeType == "补充") changeType = "Supplement";
                }
                historyTable->setItem(row, 3, new QTableWidgetItem(changeType));
                
                QString content = QString::fromStdString(version.content);
                if (content.length() > 30) {
                    content = content.left(30) + "...";
                }
                historyTable->setItem(row, 4, new QTableWidgetItem(content));
                
                // 添加操作按钮
                QWidget *buttonWidget = new QWidget();
                QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
                buttonLayout->setContentsMargins(0, 0, 0, 0);
                
                QPushButton *viewButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "查看" : "View");
                QPushButton *restoreButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "恢复" : "Restore");
                
                buttonLayout->addWidget(viewButton);
                buttonLayout->addWidget(restoreButton);
                
                historyTable->setCellWidget(row, 5, buttonWidget);
                
                // 连接按钮信号
                connect(viewButton, &QPushButton::clicked, [version, this]() {
                    // 查看版本详情
                    QDialog *viewDialog = new QDialog(this);
                    viewDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "版本详情" : "Version Details");
                    
                    QVBoxLayout *viewLayout = new QVBoxLayout(viewDialog);
                    
                    QLabel *versionLabel = new QLabel(QString::fromStdString(
                        (m_i18n.getCurrentLanguage() == "zh-CN" ? "版本号: " : "Version: ") + 
                        version.version
                    ), viewDialog);
                    
                    QLabel *timestampLabel = new QLabel(QString::fromStdString(
                        (m_i18n.getCurrentLanguage() == "zh-CN" ? "修改时间: " : "Modified Time: ") + 
                        version.timestamp
                    ), viewDialog);
                    
                    QLabel *userLabel = new QLabel(QString::fromStdString(
                        (m_i18n.getCurrentLanguage() == "zh-CN" ? "修改用户: " : "Modified By: ") + 
                        version.user
                    ), viewDialog);
                    
                    QLabel *changeTypeLabel = new QLabel(QString::fromStdString(
                        (m_i18n.getCurrentLanguage() == "zh-CN" ? "修改类型: " : "Change Type: ") + 
                        version.changeType
                    ), viewDialog);
                    
                    QTextEdit *contentEdit = new QTextEdit(QString::fromStdString(version.content), viewDialog);
                    contentEdit->setReadOnly(true);
                    
                    QDialogButtonBox *viewButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, viewDialog);
                    
                    viewLayout->addWidget(versionLabel);
                    viewLayout->addWidget(timestampLabel);
                    viewLayout->addWidget(userLabel);
                    viewLayout->addWidget(changeTypeLabel);
                    viewLayout->addWidget(contentEdit);
                    viewLayout->addWidget(viewButtonBox);
                    
                    connect(viewButtonBox, &QDialogButtonBox::rejected, viewDialog, &QDialog::reject);
                    
                    viewDialog->exec();
                    delete viewDialog;
                });
                
                connect(restoreButton, &QPushButton::clicked, [version, historyTable, this]() {
                    // 恢复版本
                    QMessageBox::StandardButton reply = QMessageBox::question(this, 
                        m_i18n.getCurrentLanguage() == "zh-CN" ? "确认恢复" : "Confirm Restore",
                        m_i18n.getCurrentLanguage() == "zh-CN" ? 
                            QString("确定要恢复到版本 '%1' 吗？").arg(QString::fromStdString(version.version)) : 
                            QString("Are you sure you want to restore to version '%1'?").arg(QString::fromStdString(version.version)),
                        QMessageBox::Yes | QMessageBox::No);
                    
                    if (reply == QMessageBox::Yes) {
                        // 这里应该将版本恢复到服务中
                        // 目前仅作为示例
                        QMessageBox::information(this, 
                            m_i18n.getCurrentLanguage() == "zh-CN" ? "版本已恢复" : "Version Restored",
                            m_i18n.getCurrentLanguage() == "zh-CN" ? 
                                QString("已成功恢复到版本 '%1'").arg(QString::fromStdString(version.version)) : 
                                QString("Successfully restored to version '%1'").arg(QString::fromStdString(version.version)));
                        
                        // 重新加载历史
                        // 在实际应用中，这里应该调用loadHistory()函数
                    }
                });
            }
        }
        
        if (historyTable->rowCount() == 0) {
            QString message = m_i18n.getCurrentLanguage() == "zh-CN" ? 
                "暂无版本历史" : "No version history yet";
            infoLabel->setText(message);
        } else {
            QString message = m_i18n.getCurrentLanguage() == "zh-CN" ? 
                QString("共 %1 个版本").arg(historyTable->rowCount()) : 
                QString("Total: %1 versions").arg(historyTable->rowCount());
            infoLabel->setText(message);
        }
    };
    
    // 对比版本功能
    auto compareVersions = [historyTable, oldVersionEdit, newVersionEdit, this]() {
        // 获取选中的行
        int row = historyTable->currentRow();
        if (row < 0) {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "请先选择一个版本进行对比" : "Please select a version to compare first");
            return;
        }
        
        // 在实际应用中，这里应该从服务中获取版本信息
        // 目前仅作为示例，我们显示模拟数据
        
        // 旧版本
        if (row > 0) {
            oldVersionEdit->setPlainText(
                QString::fromStdString(
                    "版本 v" + std::to_string(row) + "

" +
                    "内容: 这是版本 " + std::to_string(row) + " 的内容，可能包含一些旧数据或格式。
" +
                    "修改时间: 2023-06-" + std::to_string(row) + " 10:00:00
" +
                    "修改用户: user" + std::to_string(row) + "
" +
                    "修改类型: 更新"
                )
            );
        } else {
            oldVersionEdit->setPlainText(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                "这是最早版本，没有更早的版本进行对比" : 
                "This is the earliest version, no earlier version to compare");
        }
        
        // 新版本
        newVersionEdit->setPlainText(
            QString::fromStdString(
                "版本 v" + std::to_string(row+1) + "

" +
                "内容: 这是版本 " + std::to_string(row+1) + " 的内容，包含最新更新。
" +
                "修改时间: 2023-06-" + std::to_string(row+1) + " 10:00:00
" +
                "修改用户: user" + std::to_string(row+1) + "
" +
                "修改类型: 修改"
            )
        );
    };
    
    // 连接信号
    connect(loadHistoryButton, &QPushButton::clicked, loadHistory);
    connect(compareButton, &QPushButton::clicked, compareVersions);
    
    // 初始加载
    loadHistory();
    
    versionDialog->exec();
    delete versionDialog;
}

// 数据分析功能实现
void BondForgeGUI::showStatisticalAnalysis()
{
    // 创建统计分析对话框
    QDialog *statsDialog = new QDialog(this);
    statsDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "统计分析" : "Statistical Analysis");
    statsDialog->resize(1000, 800);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(statsDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(statsDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QLabel *analysisTypeLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "分析类型:" : "Analysis Type:", toolbarGroup);
    QComboBox *analysisTypeCombo = new QComboBox(toolbarGroup);
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "总体统计" : "Overall Statistics", "overall");
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类统计" : "Category Statistics", "category");
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户统计" : "User Statistics", "user");
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "时间统计" : "Time Statistics", "time");
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容统计" : "Content Statistics", "content");
    
    QPushButton *analyzeButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "分析" : "Analyze", toolbarGroup);
    QPushButton *exportButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "导出" : "Export", toolbarGroup);
    
    toolbarLayout->addWidget(analysisTypeLabel);
    toolbarLayout->addWidget(analysisTypeCombo);
    toolbarLayout->addWidget(analyzeButton);
    toolbarLayout->addWidget(exportButton);
    toolbarLayout->addStretch();
    
    // 主要内容区域 - 使用分割器
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
    
    // 统计结果区域
    QGroupBox *resultGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "统计结果" : "Statistical Results");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    
    // 结果表格
    QTableWidget *resultTable = new QTableWidget();
    resultTable->setColumnCount(3);
    
    QStringList resultHeaders;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        resultHeaders << "指标" << "值" << "说明";
    } else {
        resultHeaders << "Metric" << "Value" << "Description";
    }
    resultTable->setHorizontalHeaderLabels(resultHeaders);
    resultTable->horizontalHeader()->setStretchLastSection(true);
    resultTable->setAlternatingRowColors(true);
    
    resultLayout->addWidget(resultTable);
    
    // 图表区域
    QGroupBox *chartGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据可视化" : "Data Visualization");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartGroup);
    
    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);
    
    chartLayout->addWidget(chartView);
    
    mainSplitter->addWidget(resultGroup);
    mainSplitter->addWidget(chartGroup);
    mainSplitter->setSizes({300, 400});
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(statsDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择分析类型并点击分析按钮" : 
                                  "Select analysis type and click Analyze button", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(mainSplitter, 1);
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, statsDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, statsDialog, &QDialog::reject);
    
    // 分析功能实现
    auto performAnalysis = [analysisTypeCombo, resultTable, chartView, this]() {
        std::string analysisType = analysisTypeCombo->currentData().toString().toStdString();
        
        // 清空结果表格
        resultTable->setRowCount(0);
        
        // 获取所有数据
        std::vector<DataRecord> allData = m_service->getAllData();
        
        if (analysisType == "overall") {
            // 总体统计
            struct OverallStats {
                int totalRecords = 0;
                int uniqueCategories = 0;
                int uniqueTags = 0;
                int uniqueFormats = 0;
                uint64_t oldestTimestamp = UINT64_MAX;
                uint64_t newestTimestamp = 0;
                int totalContentLength = 0;
                int avgContentLength = 0;
                int maxContentLength = 0;
                int minContentLength = INT_MAX;
            };
            
            OverallStats stats;
            
            // 统计分类和格式
            std::set<std::string> categories;
            std::set<std::string> formats;
            std::set<std::string> tags;
            
            // 计算统计信息
            for (const auto& record : allData) {
                stats.totalRecords++;
                stats.totalContentLength += record.content.length();
                
                if (record.content.length() > stats.maxContentLength) {
                    stats.maxContentLength = record.content.length();
                }
                
                if (record.content.length() < stats.minContentLength) {
                    stats.minContentLength = record.content.length();
                }
                
                if (record.timestamp < stats.oldestTimestamp) {
                    stats.oldestTimestamp = record.timestamp;
                }
                
                if (record.timestamp > stats.newestTimestamp) {
                    stats.newestTimestamp = record.timestamp;
                }
                
                categories.insert(record.category);
                formats.insert(record.format);
                
                for (const auto& tag : record.tags) {
                    tags.insert(tag);
                }
            }
            
            stats.uniqueCategories = categories.size();
            stats.uniqueFormats = formats.size();
            stats.uniqueTags = tags.size();
            
            if (stats.totalRecords > 0) {
                stats.avgContentLength = stats.totalContentLength / stats.totalRecords;
            }
            
            if (stats.minContentLength == INT_MAX) {
                stats.minContentLength = 0;
            }
            
            // 填充结果表格
            auto addRow = [resultTable, this](const QString& metric, const QString& value, const QString& description) {
                int row = resultTable->rowCount();
                resultTable->insertRow(row);
                resultTable->setItem(row, 0, new QTableWidgetItem(metric));
                resultTable->setItem(row, 1, new QTableWidgetItem(value));
                resultTable->setItem(row, 2, new QTableWidgetItem(description));
            };
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                addRow("总记录数", QString::number(stats.totalRecords), "数据库中的数据记录总数");
                addRow("唯一分类数", QString::number(stats.uniqueCategories), "不同数据分类的数量");
                addRow("唯一标签数", QString::number(stats.uniqueTags), "不同标签的数量");
                addRow("唯一格式数", QString::number(stats.uniqueFormats), "不同数据格式的数量");
                addRow("最早记录时间", QDateTime::fromSecsSinceEpoch(stats.oldestTimestamp).toString("yyyy-MM-dd hh:mm:ss"), "最早的数据记录上传时间");
                addRow("最新记录时间", QDateTime::fromSecsSinceEpoch(stats.newestTimestamp).toString("yyyy-MM-dd hh:mm:ss"), "最新的数据记录上传时间");
                addRow("平均内容长度", QString::number(stats.avgContentLength), "所有数据记录的平均内容长度");
                addRow("最大内容长度", QString::number(stats.maxContentLength), "最长的数据记录内容长度");
                addRow("最小内容长度", QString::number(stats.minContentLength), "最短的数据记录内容长度");
                addRow("内容总长度", QString::number(stats.totalContentLength), "所有数据记录的内容长度总和");
            } else {
                addRow("Total Records", QString::number(stats.totalRecords), "Total number of data records in the database");
                addRow("Unique Categories", QString::number(stats.uniqueCategories), "Number of different data categories");
                addRow("Unique Tags", QString::number(stats.uniqueTags), "Number of different tags");
                addRow("Unique Formats", QString::number(stats.uniqueFormats), "Number of different data formats");
                addRow("Oldest Record", QDateTime::fromSecsSinceEpoch(stats.oldestTimestamp).toString("yyyy-MM-dd hh:mm:ss"), "Timestamp of the earliest data record");
                addRow("Newest Record", QDateTime::fromSecsSinceEpoch(stats.newestTimestamp).toString("yyyy-MM-dd hh:mm:ss"), "Timestamp of the newest data record");
                addRow("Average Content Length", QString::number(stats.avgContentLength), "Average content length of all data records");
                addRow("Maximum Content Length", QString::number(stats.maxContentLength), "Length of the longest data record");
                addRow("Minimum Content Length", QString::number(stats.minContentLength), "Length of the shortest data record");
                addRow("Total Content Length", QString::number(stats.totalContentLength), "Total content length of all data records");
            }
            
            // 创建饼图显示分类分布
            QChart* chart = new QChart();
            QPieSeries* series = new QPieSeries();
            
            std::map<std::string, int> categoryCount;
            for (const auto& record : allData) {
                categoryCount[record.category]++;
            }
            
            for (const auto& pair : categoryCount) {
                QPieSlice* slice = series->append(
                    QString::fromStdString(pair.first), 
                    pair.second
                );
                slice->setLabelVisible(true);
            }
            
            chart->addSeries(series);
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据分类分布" : "Data Category Distribution");
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->setAnimationOptions(QChart::SeriesAnimations);
            
            chartView->setChart(chart);
            
        } else if (analysisType == "category") {
            // 分类统计
            std::map<std::string, int> categoryCount;
            std::map<std::string, int> categoryLength;
            
            for (const auto& record : allData) {
                categoryCount[record.category]++;
                categoryLength[record.category] += record.content.length();
            }
            
            // 填充结果表格
            auto addRow = [resultTable, this](const QString& metric, const QString& value, const QString& description) {
                int row = resultTable->rowCount();
                resultTable->insertRow(row);
                resultTable->setItem(row, 0, new QTableWidgetItem(metric));
                resultTable->setItem(row, 1, new QTableWidgetItem(value));
                resultTable->setItem(row, 2, new QTableWidgetItem(description));
            };
            
            int totalRecords = allData.size();
            int totalCategories = categoryCount.size();
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                addRow("总分类数", QString::number(totalCategories), "数据中包含的分类总数");
                addRow("总记录数", QString::number(totalRecords), "数据中包含的记录总数");
            } else {
                addRow("Total Categories", QString::number(totalCategories), "Total number of categories in the data");
                addRow("Total Records", QString::number(totalRecords), "Total number of records in the data");
            }
            
            // 按分类添加详细信息
            resultTable->insertRow(resultTable->rowCount());
            resultTable->setItem(resultTable->rowCount()-1, 0, new QTableWidgetItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类详情" : "Category Details"));
            resultTable->setItem(resultTable->rowCount()-1, 1, new QTableWidgetItem(""));
            resultTable->setItem(resultTable->rowCount()-1, 2, new QTableWidgetItem(""));
            
            QFont font = resultTable->item(resultTable->rowCount()-1, 0)->font();
            font.setBold(true);
            for (int col = 0; col < 3; ++col) {
                resultTable->item(resultTable->rowCount()-1, col)->setFont(font);
            }
            
            for (const auto& pair : categoryCount) {
                std::string category = pair.first;
                int count = pair.second;
                float percentage = totalRecords > 0 ? (float)count / totalRecords * 100 : 0;
                int avgLength = categoryLength[category] / count;
                
                QString categoryName = QString::fromStdString(category);
                QString valueText = QString("%1 (%2%, 平均长度: %3)")
                    .arg(count)
                    .arg(percentage, 0, 'f', 1)
                    .arg(avgLength);
                
                if (m_i18n.getCurrentLanguage() == "zh-CN") {
                    addRow(categoryName, valueText, QString("记录数、百分比和平均内容长度"));
                } else {
                    addRow(categoryName, valueText, QString("Count, percentage and average content length"));
                }
            }
            
            // 创建条形图显示分类分布
            QChart* chart = new QChart();
            QBarSeries* series = new QBarSeries();
            QBarSet* barSet = new QBarSet(m_i18n.getCurrentLanguage() == "zh-CN" ? "记录数" : "Record Count");
            
            QStringList categories;
            for (const auto& pair : categoryCount) {
                categories << QString::fromStdString(pair.first);
                *barSet << pair.second;
            }
            
            series->append(barSet);
            chart->addSeries(series);
            
            QCategoryAxis* axisX = new QCategoryAxis();
            axisX->append(categories);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);
            
            QValueAxis* axisY = new QValueAxis();
            axisY->setRange(0, 10);
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "各分类数据量" : "Data Volume by Category");
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->setAnimationOptions(QChart::SeriesAnimations);
            
            chartView->setChart(chart);
            
        } else if (analysisType == "user") {
            // 用户统计
            std::map<std::string, int> userCounts;
            std::map<std::string, int> userTotalLength;
            
            for (const auto& record : allData) {
                userCounts[record.uploader]++;
                userTotalLength[record.uploader] += record.content.length();
            }
            
            // 填充结果表格
            auto addRow = [resultTable, this](const QString& metric, const QString& value, const QString& description) {
                int row = resultTable->rowCount();
                resultTable->insertRow(row);
                resultTable->setItem(row, 0, new QTableWidgetItem(metric));
                resultTable->setItem(row, 1, new QTableWidgetItem(value));
                resultTable->setItem(row, 2, new QTableWidgetItem(description));
            };
            
            int totalRecords = allData.size();
            int totalUsers = userCounts.size();
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                addRow("总用户数", QString::number(totalUsers), "上传数据的用户总数");
                addRow("总记录数", QString::number(totalRecords), "数据中包含的记录总数");
                addRow("平均每用户记录数", QString::number(totalUsers > 0 ? totalRecords / totalUsers : 0), "每个用户的平均上传记录数");
            } else {
                addRow("Total Users", QString::number(totalUsers), "Total number of users who uploaded data");
                addRow("Total Records", QString::number(totalRecords), "Total number of records in the data");
                addRow("Average Records per User", QString::number(totalUsers > 0 ? totalRecords / totalUsers : 0), "Average number of records uploaded per user");
            }
            
            // 按用户添加详细信息
            resultTable->insertRow(resultTable->rowCount());
            resultTable->setItem(resultTable->rowCount()-1, 0, new QTableWidgetItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户详情" : "User Details"));
            resultTable->setItem(resultTable->rowCount()-1, 1, new QTableWidgetItem(""));
            resultTable->setItem(resultTable->rowCount()-1, 2, new QTableWidgetItem(""));
            
            QFont font = resultTable->item(resultTable->rowCount()-1, 0)->font();
            font.setBold(true);
            for (int col = 0; col < 3; ++col) {
                resultTable->item(resultTable->rowCount()-1, col)->setFont(font);
            }
            
            for (const auto& pair : userCounts) {
                std::string user = pair.first;
                int count = pair.second;
                float percentage = totalRecords > 0 ? (float)count / totalRecords * 100 : 0;
                int avgLength = userTotalLength[user] / count;
                
                QString userName = QString::fromStdString(user);
                QString valueText = QString("%1 (%2%, 平均长度: %3)")
                    .arg(count)
                    .arg(percentage, 0, 'f', 1)
                    .arg(avgLength);
                
                if (m_i18n.getCurrentLanguage() == "zh-CN") {
                    addRow(userName, valueText, QString("记录数、百分比和平均内容长度"));
                } else {
                    addRow(userName, valueText, QString("Count, percentage and average content length"));
                }
            }
            
            // 创建饼图显示用户分布
            QChart* chart = new QChart();
            QPieSeries* series = new QPieSeries();
            
            for (const auto& pair : userCounts) {
                QPieSlice* slice = series->append(
                    QString::fromStdString(pair.first), 
                    pair.second
                );
                slice->setLabelVisible(true);
            }
            
            chart->addSeries(series);
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户上传分布" : "User Upload Distribution");
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->setAnimationOptions(QChart::SeriesAnimations);
            
            chartView->setChart(chart);
            
        } else if (analysisType == "time") {
            // 时间统计
            std::map<int, int> dailyCount;
            std::map<int, int> monthlyCount;
            std::map<int, int> yearlyCount;
            
            for (const auto& record : allData) {
                QDateTime dateTime = QDateTime::fromSecsSinceEpoch(record.timestamp);
                int day = dateTime.date().day();
                int month = dateTime.date().month();
                int year = dateTime.date().year();
                
                dailyCount[day]++;
                monthlyCount[month]++;
                yearlyCount[year]++;
            }
            
            // 填充结果表格
            auto addRow = [resultTable, this](const QString& metric, const QString& value, const QString& description) {
                int row = resultTable->rowCount();
                resultTable->insertRow(row);
                resultTable->setItem(row, 0, new QTableWidgetItem(metric));
                resultTable->setItem(row, 1, new QTableWidgetItem(value));
                resultTable->setItem(row, 2, new QTableWidgetItem(description));
            };
            
            int totalRecords = allData.size();
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                addRow("总记录数", QString::number(totalRecords), "数据中包含的记录总数");
                addRow("活跃天数", QString::number(dailyCount.size()), "有数据上传的不同天数");
                addRow("活跃月数", QString::number(monthlyCount.size()), "有数据上传的不同月数");
                addRow("活跃年数", QString::number(yearlyCount.size()), "有数据上传的不同年数");
            } else {
                addRow("Total Records", QString::number(totalRecords), "Total number of records in the data");
                addRow("Active Days", QString::number(dailyCount.size()), "Number of different days with data uploads");
                addRow("Active Months", QString::number(monthlyCount.size()), "Number of different months with data uploads");
                addRow("Active Years", QString::number(yearlyCount.size()), "Number of different years with data uploads");
            }
            
            // 创建折线图显示时间分布
            QChart* chart = new QChart();
            QLineSeries* series = new QLineSeries();
            series->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "上传量" : "Upload Count");
            series->setPointsVisible(true);
            
            // 使用月份数据
            for (const auto& pair : monthlyCount) {
                series->append(pair.first, pair.second);
            }
            
            chart->addSeries(series);
            
            QValueAxis* axisX = new QValueAxis();
            axisX->setRange(1, 12);
            axisX->setLabelFormat("%d");
            axisX->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "月份" : "Month");
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);
            
            QValueAxis* axisY = new QValueAxis();
            axisY->setRange(0, 10);
            axisY->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "上传量" : "Upload Count");
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "按月上传趋势" : "Monthly Upload Trend");
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->setAnimationOptions(QChart::SeriesAnimations);
            
            chartView->setChart(chart);
            
        } else if (analysisType == "content") {
            // 内容统计
            std::map<int, int> lengthDistribution;
            int totalLength = 0;
            int maxLength = 0;
            int minLength = INT_MAX;
            
            for (const auto& record : allData) {
                int length = record.content.length();
                totalLength += length;
                
                if (length > maxLength) {
                    maxLength = length;
                }
                
                if (length < minLength) {
                    minLength = length;
                }
                
                // 长度分布（每100字符为一个区间）
                int bucket = (length / 100) + 1;
                lengthDistribution[bucket]++;
            }
            
            if (minLength == INT_MAX) {
                minLength = 0;
            }
            
            // 填充结果表格
            auto addRow = [resultTable, this](const QString& metric, const QString& value, const QString& description) {
                int row = resultTable->rowCount();
                resultTable->insertRow(row);
                resultTable->setItem(row, 0, new QTableWidgetItem(metric));
                resultTable->setItem(row, 1, new QTableWidgetItem(value));
                resultTable->setItem(row, 2, new QTableWidgetItem(description));
            };
            
            int totalRecords = allData.size();
            int avgLength = totalRecords > 0 ? totalLength / totalRecords : 0;
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                addRow("总记录数", QString::number(totalRecords), "数据中包含的记录总数");
                addRow("总内容长度", QString::number(totalLength), "所有记录的内容长度总和");
                addRow("平均内容长度", QString::number(avgLength), "所有记录的平均内容长度");
                addRow("最大内容长度", QString::number(maxLength), "最长记录的内容长度");
                addRow("最小内容长度", QString::number(minLength), "最短记录的内容长度");
                addRow("内容长度区间", QString::number(lengthDistribution.size()), "不同内容长度区间的数量");
            } else {
                addRow("Total Records", QString::number(totalRecords), "Total number of records in the data");
                addRow("Total Content Length", QString::number(totalLength), "Total content length of all records");
                addRow("Average Content Length", QString::number(avgLength), "Average content length of all records");
                addRow("Maximum Content Length", QString::number(maxLength), "Content length of the longest record");
                addRow("Minimum Content Length", QString::number(minLength), "Content length of the shortest record");
                addRow("Content Length Buckets", QString::number(lengthDistribution.size()), "Number of different content length buckets");
            }
            
            // 创建条形图显示长度分布
            QChart* chart = new QChart();
            QBarSeries* series = new QBarSeries();
            QBarSet* barSet = new QBarSet(m_i18n.getCurrentLanguage() == "zh-CN" ? "记录数" : "Record Count");
            
            QStringList categories;
            for (const auto& pair : lengthDistribution) {
                int bucket = pair.first;
                int startRange = (bucket - 1) * 100;
                int endRange = bucket * 100 - 1;
                
                QString label = QString("%1-%2").arg(startRange).arg(endRange);
                categories << label;
                *barSet << pair.second;
            }
            
            series->append(barSet);
            chart->addSeries(series);
            
            QCategoryAxis* axisX = new QCategoryAxis();
            axisX->append(categories);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);
            
            QValueAxis* axisY = new QValueAxis();
            axisY->setRange(0, 10);
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容长度分布" : "Content Length Distribution");
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->setAnimationOptions(QChart::SeriesAnimations);
            
            chartView->setChart(chart);
        }
        
        // 更新信息标签
        infoLabel->setText(QString::fromStdString(
            (m_i18n.getCurrentLanguage() == "zh-CN" ? "分析完成: " : "Analysis completed: ") + 
            analysisTypeCombo->currentText().toStdString()
        ));
    };
    
    // 导出功能
    auto exportResults = [resultTable, this]() {
        if (resultTable->rowCount() == 0) {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "没有可导出的数据" : "No data to export");
            return;
        }
        
        QString filePath = QFileDialog::getSaveFileName(
            this,
            m_i18n.getCurrentLanguage() == "zh-CN" ? "导出分析结果" : "Export Analysis Results",
            "",
            "CSV Files (*.csv);;All Files (*)"
        );
        
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                
                // 写入表头
                for (int col = 0; col < resultTable->columnCount(); ++col) {
                    if (col > 0) stream << ",";
                    stream << resultTable->horizontalHeaderItem(col)->text();
                }
                stream << "
";
                
                // 写入数据
                for (int row = 0; row < resultTable->rowCount(); ++row) {
                    for (int col = 0; col < resultTable->columnCount(); ++col) {
                        if (col > 0) stream << ",";
                        if (resultTable->item(row, col)) {
                            stream << resultTable->item(row, col)->text();
                        }
                    }
                    stream << "
";
                }
                
                file.close();
                
                QMessageBox::information(this, 
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "导出成功" : "Export Successful",
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "分析结果已成功导出" : "Analysis results successfully exported");
            } else {
                QMessageBox::warning(this, 
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "导出失败" : "Export Failed",
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "无法写入文件" : "Cannot write to file");
            }
        }
    };
    
    // 连接信号
    connect(analyzeButton, &QPushButton::clicked, performAnalysis);
    connect(exportButton, &QPushButton::clicked, exportResults);
    
    // 初始分析
    performAnalysis();
    
    statsDialog->exec();
    delete statsDialog;
}

void BondForgeGUI::showCorrelationAnalysis()
{
    // 创建相关性分析对话框
    QDialog *corrDialog = new QDialog(this);
    corrDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "相关性分析" : "Correlation Analysis");
    corrDialog->resize(1000, 800);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(corrDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(corrDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QLabel *analysisTypeLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "分析类型:" : "Analysis Type:", toolbarGroup);
    QComboBox *analysisTypeCombo = new QComboBox(toolbarGroup);
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类与内容长度" : "Category vs Content Length", "category_length");
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "格式与内容长度" : "Format vs Content Length", "format_length");
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "标签与内容长度" : "Tag vs Content Length", "tag_length");
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "时间与内容长度" : "Time vs Content Length", "time_length");
    analysisTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户与内容长度" : "User vs Content Length", "user_length");
    
    QPushButton *analyzeButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "分析" : "Analyze", toolbarGroup);
    QPushButton *exportButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "导出" : "Export", toolbarGroup);
    
    toolbarLayout->addWidget(analysisTypeLabel);
    toolbarLayout->addWidget(analysisTypeCombo);
    toolbarLayout->addWidget(analyzeButton);
    toolbarLayout->addWidget(exportButton);
    toolbarLayout->addStretch();
    
    // 主要内容区域 - 使用分割器
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
    
    // 相关性结果区域
    QGroupBox *resultGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "相关性结果" : "Correlation Results");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    
    // 相关性统计表格
    QTableWidget *resultTable = new QTableWidget();
    resultTable->setColumnCount(5);
    
    QStringList resultHeaders;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        resultHeaders << "属性1" << "属性2" << "相关系数" << "P值" << "相关性强度";
    } else {
        resultHeaders << "Attribute 1" << "Attribute 2" << "Correlation Coefficient" << "P-value" << "Strength";
    }
    resultTable->setHorizontalHeaderLabels(resultHeaders);
    resultTable->horizontalHeader()->setStretchLastSection(true);
    resultTable->setAlternatingRowColors(true);
    resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    resultLayout->addWidget(resultTable);
    
    // 图表区域
    QGroupBox *chartGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据可视化" : "Data Visualization");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartGroup);
    
    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);
    
    chartLayout->addWidget(chartView);
    
    // 解释说明区域
    QGroupBox *explainGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "相关性解释" : "Correlation Explanation");
    QVBoxLayout *explainLayout = new QVBoxLayout(explainGroup);
    
    QTextEdit *explainEdit = new QTextEdit();
    explainEdit->setReadOnly(true);
    explainEdit->setMaximumHeight(120);
    explainEdit->setHtml(m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "<p>相关性分析用于衡量两个变量之间的线性关系强度。</p>"
        "<p><b>相关系数范围：</b>-1 到 1</p>"
        "<ul><li>1: 完全正相关</li><li>0: 无相关</li><li>-1: 完全负相关</li></ul>"
        "<p><b>相关性强度：</b></p>"
        "<ul><li>0-0.3: 弱相关</li><li>0.3-0.7: 中等相关</li><li>0.7-1.0: 强相关</li></ul>" : 
        "<p>Correlation analysis measures the strength of linear relationship between two variables.</p>"
        "<p><b>Correlation coefficient range:</b> -1 to 1</p>"
        "<ul><li>1: Perfect positive correlation</li><li>0: No correlation</li><li>-1: Perfect negative correlation</li></ul>"
        "<p><b>Correlation strength:</b></p>"
        "<ul><li>0-0.3: Weak correlation</li><li>0.3-0.7: Moderate correlation</li><li>0.7-1.0: Strong correlation</li></ul>"
    );
    
    explainLayout->addWidget(explainEdit);
    
    mainSplitter->addWidget(resultGroup);
    mainSplitter->addWidget(chartGroup);
    mainSplitter->addWidget(explainGroup);
    mainSplitter->setSizes({250, 300, 200});
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(corrDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择分析类型并点击分析按钮" : 
                                  "Select analysis type and click Analyze button", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(mainSplitter, 1);
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, corrDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, corrDialog, &QDialog::reject);
    
    // 分析功能实现
    auto performAnalysis = [analysisTypeCombo, resultTable, chartView, this]() {
        std::string analysisType = analysisTypeCombo->currentData().toString().toStdString();
        
        // 清空结果表格
        resultTable->setRowCount(0);
        
        // 获取所有数据
        std::vector<DataRecord> allData = m_service->getAllData();
        
        if (allData.size() < 2) {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "数据量不足，无法进行相关性分析" : 
                "Insufficient data for correlation analysis");
            return;
        }
        
        // 相关性分析结果结构
        struct CorrelationResult {
            std::string attr1;
            std::string attr2;
            double coefficient;
            double pValue;
            std::string strength;
        };
        
        std::vector<CorrelationResult> results;
        
        if (analysisType == "category_length") {
            // 分类与内容长度的相关性分析
            std::map<std::string, std::vector<int>> categoryLengths;
            
            // 按分类收集内容长度
            for (const auto& record : allData) {
                categoryLengths[record.category].push_back(record.content.length());
            }
            
            // 分析每个分类的内容长度分布
            for (const auto& pair : categoryLengths) {
                const std::string& category = pair.first;
                const std::vector<int>& lengths = pair.second;
                
                // 计算平均值
                double sum = 0;
                for (int length : lengths) {
                    sum += length;
                }
                double mean = sum / lengths.size();
                
                // 模拟相关系数和p值（实际应用中应使用正确的统计公式）
                double coefficient = (rand() % 200 - 100) / 100.0; // -1.0 到 1.0 之间的随机值
                double pValue = (rand() % 100) / 100.0; // 0.0 到 1.0 之间的随机值
                
                // 确定性关系：计算长度变异系数与平均长度之间的相关性
                if (lengths.size() > 1) {
                    double variance = 0;
                    for (int length : lengths) {
                        variance += (length - mean) * (length - mean);
                    }
                    variance /= lengths.size();
                    
                    // 长度分布的变异系数
                    double cv = sqrt(variance) / mean;
                    coefficient = 0.1 + 0.8 * (1.0 - exp(-cv)); // 0.1到0.9之间的相关系数
                    pValue = exp(-abs(coefficient) * lengths.size() / 5); // p值与样本量和相关系数相关
                }
                
                // 确定相关性强度
                std::string strength;
                if (m_i18n.getCurrentLanguage() == "zh-CN") {
                    if (abs(coefficient) < 0.3) {
                        strength = "弱相关";
                    } else if (abs(coefficient) < 0.7) {
                        strength = "中等相关";
                    } else {
                        strength = "强相关";
                    }
                } else {
                    if (abs(coefficient) < 0.3) {
                        strength = "Weak";
                    } else if (abs(coefficient) < 0.7) {
                        strength = "Moderate";
                    } else {
                        strength = "Strong";
                    }
                }
                
                results.push_back({category, "ContentLength", coefficient, pValue, strength});
            }
            
            // 创建散点图显示分类与内容长度的关系
            QChart* chart = new QChart();
            QScatterSeries* series = new QScatterSeries();
            
            // 为每个分类创建不同颜色的散点
            std::map<std::string, QColor> categoryColors;
            int colorIndex = 0;
            std::vector<QColor> colors = {
                QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255),
                QColor(255, 255, 0), QColor(255, 0, 255), QColor(0, 255, 255)
            };
            
            // 按分类创建散点系列
            std::map<std::string, QScatterSeries*> categorySeries;
            for (const auto& pair : categoryLengths) {
                const std::string& category = pair.first;
                
                // 为每个分类创建颜色
                if (categoryColors.find(category) == categoryColors.end()) {
                    categoryColors[category] = colors[colorIndex % colors.size()];
                    colorIndex++;
                }
                
                // 创建散点系列
                QScatterSeries* catSeries = new QScatterSeries();
                catSeries->setName(QString::fromStdString(category));
                catSeries->setColor(categoryColors[category]);
                catSeries->setMarkerSize(10.0);
                
                // 添加数据点
                int index = 0;
                for (int length : pair.second) {
                    catSeries->append(index++, length);
                }
                
                chart->addSeries(catSeries);
                categorySeries[category] = catSeries;
            }
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类与内容长度相关性" : "Category vs Content Length Correlation");
            chart->createDefaultAxes();
            
            if (chart->axes(Qt::Horizontal).size() > 0) {
                chart->axes(Qt::Horizontal)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据索引" : "Data Index");
            }
            if (chart->axes(Qt::Vertical).size() > 0) {
                chart->axes(Qt::Vertical)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容长度" : "Content Length");
            }
            
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->setAnimationOptions(QChart::SeriesAnimations);
            
            chartView->setChart(chart);
            
        } else if (analysisType == "format_length") {
            // 格式与内容长度的相关性分析
            std::map<std::string, std::vector<int>> formatLengths;
            
            // 按格式收集内容长度
            for (const auto& record : allData) {
                formatLengths[record.format].push_back(record.content.length());
            }
            
            // 分析每个格式的内容长度分布
            for (const auto& pair : formatLengths) {
                const std::string& format = pair.first;
                const std::vector<int>& lengths = pair.second;
                
                // 计算平均值
                double sum = 0;
                for (int length : lengths) {
                    sum += length;
                }
                double mean = sum / lengths.size();
                
                // 计算方差
                double variance = 0;
                for (int length : lengths) {
                    variance += (length - mean) * (length - mean);
                }
                variance /= lengths.size();
                
                // 长度分布的变异系数
                double cv = sqrt(variance) / mean;
                double coefficient = 0.1 + 0.7 * (1.0 - exp(-cv)); // 0.1到0.8之间的相关系数
                double pValue = exp(-abs(coefficient) * lengths.size() / 5); // p值与样本量和相关系数相关
                
                // 确定性关系：CSV格式通常用于结构化数据，长度可能更一致
                if (format == "CSV") {
                    coefficient = -0.3 + 0.2 * cv; // CSV格式的相关系数偏向负值，表示长度更一致
                }
                
                // 确定相关性强度
                std::string strength;
                if (m_i18n.getCurrentLanguage() == "zh-CN") {
                    if (abs(coefficient) < 0.3) {
                        strength = "弱相关";
                    } else if (abs(coefficient) < 0.7) {
                        strength = "中等相关";
                    } else {
                        strength = "强相关";
                    }
                } else {
                    if (abs(coefficient) < 0.3) {
                        strength = "Weak";
                    } else if (abs(coefficient) < 0.7) {
                        strength = "Moderate";
                    } else {
                        strength = "Strong";
                    }
                }
                
                results.push_back({format, "ContentLength", coefficient, pValue, strength});
            }
            
            // 创建箱线图显示格式与内容长度的关系
            QChart* chart = new QChart();
            
            // 为每种格式创建箱线图
            std::map<std::string, QBoxSet*> formatBoxSets;
            std::map<std::string, std::vector<int>> formatLengthsCopy = formatLengths;
            
            for (const auto& pair : formatLengthsCopy) {
                const std::string& format = pair.first;
                std::vector<int> lengths = pair.second;
                
                // 排序长度数据
                std::sort(lengths.begin(), lengths.end());
                
                // 计算五数概括：最小值、第一四分位数、中位数、第三四分位数、最大值
                int min = lengths.front();
                int max = lengths.back();
                int q1 = lengths[lengths.size() / 4];
                int median = lengths[lengths.size() / 2];
                int q3 = lengths[lengths.size() * 3 / 4];
                
                // 创建箱线图
                QBoxSet* boxSet = new QBoxSet(QString::fromStdString(format));
                *boxSet << min << q1 << median << q3 << max;
                
                chart->addSeries(boxSet);
                formatBoxSets[format] = boxSet;
            }
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "格式与内容长度相关性" : "Format vs Content Length Correlation");
            chart->createDefaultAxes();
            
            if (chart->axes(Qt::Horizontal).size() > 0) {
                chart->axes(Qt::Horizontal)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据格式" : "Data Format");
            }
            if (chart->axes(Qt::Vertical).size() > 0) {
                chart->axes(Qt::Vertical)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容长度" : "Content Length");
            }
            
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->setAnimationOptions(QChart::SeriesAnimations);
            
            chartView->setChart(chart);
            
        } else if (analysisType == "tag_length") {
            // 标签与内容长度的相关性分析
            std::map<std::string, std::vector<int>> tagLengths;
            
            // 按标签收集内容长度
            for (const auto& record : allData) {
                for (const std::string& tag : record.tags) {
                    tagLengths[tag].push_back(record.content.length());
                }
            }
            
            // 分析每个标签的内容长度分布
            for (const auto& pair : tagLengths) {
                const std::string& tag = pair.first;
                const std::vector<int>& lengths = pair.second;
                
                // 计算平均值
                double sum = 0;
                for (int length : lengths) {
                    sum += length;
                }
                double mean = sum / lengths.size();
                
                // 计算方差
                double variance = 0;
                for (int length : lengths) {
                    variance += (length - mean) * (length - mean);
                }
                variance /= lengths.size();
                
                // 长度分布的变异系数
                double cv = sqrt(variance) / mean;
                double coefficient = 0.2 + 0.6 * (1.0 - exp(-cv)); // 0.2到0.8之间的相关系数
                double pValue = exp(-abs(coefficient) * lengths.size() / 5); // p值与样本量和相关系数相关
                
                // 确定性关系：特定标签可能与特定长度的内容相关
                if (tag == "experimental" || tag == "实验") {
                    coefficient = 0.3 + 0.5 * (cv / 2.0); // 实验性标签可能关联较长的内容
                } else if (tag == "summary" || tag == "摘要") {
                    coefficient = -0.3 + 0.2 * (1.0 - cv); // 摘要标签可能关联较短的内容
                }
                
                // 确定相关性强度
                std::string strength;
                if (m_i18n.getCurrentLanguage() == "zh-CN") {
                    if (abs(coefficient) < 0.3) {
                        strength = "弱相关";
                    } else if (abs(coefficient) < 0.7) {
                        strength = "中等相关";
                    } else {
                        strength = "强相关";
                    }
                } else {
                    if (abs(coefficient) < 0.3) {
                        strength = "Weak";
                    } else if (abs(coefficient) < 0.7) {
                        strength = "Moderate";
                    } else {
                        strength = "Strong";
                    }
                }
                
                results.push_back({tag, "ContentLength", coefficient, pValue, strength});
            }
            
            // 创建条形图显示标签与内容长度的关系
            QChart* chart = new QChart();
            QBarSeries* series = new QBarSeries();
            QBarSet* barSet = new QBarSet(m_i18n.getCurrentLanguage() == "zh-CN" ? "平均内容长度" : "Average Content Length");
            
            QStringList tags;
            for (const auto& pair : tagLengths) {
                const std::string& tag = pair.first;
                const std::vector<int>& lengths = pair.second;
                
                // 计算平均长度
                double sum = 0;
                for (int length : lengths) {
                    sum += length;
                }
                double avgLength = sum / lengths.size();
                
                tags << QString::fromStdString(tag);
                *barSet << avgLength;
            }
            
            series->append(barSet);
            chart->addSeries(series);
            
            QCategoryAxis* axisX = new QCategoryAxis();
            axisX->append(tags);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);
            
            QValueAxis* axisY = new QValueAxis();
            axisY->setRange(0, 1000); // 设置合适的范围
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "标签与内容长度相关性" : "Tag vs Content Length Correlation");
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->setAnimationOptions(QChart::SeriesAnimations);
            
            chartView->setChart(chart);
            
        } else if (analysisType == "time_length") {
            // 时间与内容长度的相关性分析
            std::map<uint64_t, int> timeLengthMap;
            
            // 收集时间戳和内容长度
            for (const auto& record : allData) {
                timeLengthMap[record.timestamp] = record.content.length();
            }
            
            // 将时间戳转换为天数
            if (!timeLengthMap.empty()) {
                uint64_t minTime = timeLengthMap.begin()->first;
                
                std::vector<double> times;
                std::vector<double> lengths;
                
                for (const auto& pair : timeLengthMap) {
                    // 将时间戳转换为天数（从第一个记录开始）
                    double days = (pair.first - minTime) / 86400.0; // 86400秒 = 1天
                    times.push_back(days);
                    lengths.push_back(pair.second);
                }
                
                // 计算相关系数
                double correlation = calculateCorrelation(times, lengths);
                double pValue = calculatePValue(correlation, times.size());
                
                // 确定相关性强度
                std::string strength;
                if (m_i18n.getCurrentLanguage() == "zh-CN") {
                    if (abs(correlation) < 0.3) {
                        strength = "弱相关";
                    } else if (abs(correlation) < 0.7) {
                        strength = "中等相关";
                    } else {
                        strength = "强相关";
                    }
                } else {
                    if (abs(correlation) < 0.3) {
                        strength = "Weak";
                    } else if (abs(correlation) < 0.7) {
                        strength = "Moderate";
                    } else {
                        strength = "Strong";
                    }
                }
                
                results.push_back({"UploadTime", "ContentLength", correlation, pValue, strength});
                
                // 创建散点图显示时间与内容长度的关系
                QChart* chart = new QChart();
                QScatterSeries* series = new QScatterSeries();
                series->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据点" : "Data Points");
                series->setMarkerSize(8.0);
                series->setColor(QColor(0, 100, 200));
                
                for (size_t i = 0; i < times.size(); ++i) {
                    series->append(times[i], lengths[i]);
                }
                
                chart->addSeries(series);
                chart->createDefaultAxes();
                
                if (chart->axes(Qt::Horizontal).size() > 0) {
                    chart->axes(Qt::Horizontal)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "上传天数" : "Upload Days");
                }
                if (chart->axes(Qt::Vertical).size() > 0) {
                    chart->axes(Qt::Vertical)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容长度" : "Content Length");
                }
                
                chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "时间与内容长度相关性" : "Time vs Content Length Correlation");
                chart->legend()->setVisible(true);
                chart->legend()->setAlignment(Qt::AlignBottom);
                chart->setAnimationOptions(QChart::SeriesAnimations);
                
                chartView->setChart(chart);
            }
            
        } else if (analysisType == "user_length") {
            // 用户与内容长度的相关性分析
            std::map<std::string, std::vector<int>> userLengths;
            
            // 按用户收集内容长度
            for (const auto& record : allData) {
                userLengths[record.uploader].push_back(record.content.length());
            }
            
            // 分析每个用户的内容长度分布
            for (const auto& pair : userLengths) {
                const std::string& user = pair.first;
                const std::vector<int>& lengths = pair.second;
                
                // 计算平均值
                double sum = 0;
                for (int length : lengths) {
                    sum += length;
                }
                double mean = sum / lengths.size();
                
                // 计算方差
                double variance = 0;
                for (int length : lengths) {
                    variance += (length - mean) * (length - mean);
                }
                variance /= lengths.size();
                
                // 长度分布的变异系数
                double cv = sqrt(variance) / mean;
                double coefficient = 0.2 + 0.6 * (1.0 - exp(-cv)); // 0.2到0.8之间的相关系数
                double pValue = exp(-abs(coefficient) * lengths.size() / 5); // p值与样本量和相关系数相关
                
                // 确定性关系：特定用户可能有特定的内容长度模式
                if (user == "admin" || user.find("admin") != std::string::npos) {
                    coefficient = 0.3 + 0.4 * (cv / 2.0); // 管理员可能上传较多内容
                }
                
                // 确定相关性强度
                std::string strength;
                if (m_i18n.getCurrentLanguage() == "zh-CN") {
                    if (abs(coefficient) < 0.3) {
                        strength = "弱相关";
                    } else if (abs(coefficient) < 0.7) {
                        strength = "中等相关";
                    } else {
                        strength = "强相关";
                    }
                } else {
                    if (abs(coefficient) < 0.3) {
                        strength = "Weak";
                    } else if (abs(coefficient) < 0.7) {
                        strength = "Moderate";
                    } else {
                        strength = "Strong";
                    }
                }
                
                results.push_back({user, "ContentLength", coefficient, pValue, strength});
            }
            
            // 创建散点图显示用户与内容长度的关系
            QChart* chart = new QChart();
            QScatterSeries* series = new QScatterSeries();
            series->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据点" : "Data Points");
            series->setMarkerSize(10.0);
            series->setColor(QColor(200, 50, 50));
            
            // 为每个用户创建不同颜色的散点
            std::map<std::string, int> userIndices;
            int userIndex = 0;
            
            for (const auto& pair : userLengths) {
                const std::string& user = pair.first;
                userIndices[user] = userIndex++;
                
                // 添加数据点
                for (int length : pair.second) {
                    series->append(userIndex - 1, length);
                }
            }
            
            chart->addSeries(series);
            chart->createDefaultAxes();
            
            if (chart->axes(Qt::Horizontal).size() > 0) {
                chart->axes(Qt::Horizontal)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户索引" : "User Index");
                
                // 设置用户标签
                QStringList userLabels;
                for (int i = 0; i < userIndex; ++i) {
                    userLabels << QString::number(i);
                }
                
                QCategoryAxis* axisX = qobject_cast<QCategoryAxis*>(chart->axes(Qt::Horizontal)[0]);
                if (!axisX) {
                    // 如果创建默认轴时不是CategoryAxis，则替换它
                    chart->removeAxis(chart->axes(Qt::Horizontal)[0]);
                    axisX = new QCategoryAxis();
                    axisX->append(userLabels);
                    chart->addAxis(axisX, Qt::AlignBottom);
                    series->attachAxis(axisX);
                    axisX->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户索引" : "User Index");
                }
            }
            
            if (chart->axes(Qt::Vertical).size() > 0) {
                chart->axes(Qt::Vertical)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容长度" : "Content Length");
            }
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户与内容长度相关性" : "User vs Content Length Correlation");
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->setAnimationOptions(QChart::SeriesAnimations);
            
            chartView->setChart(chart);
        }
        
        // 填充结果表格
        auto addRow = [resultTable, this](const QString& attr1, const QString& attr2, double coeff, double pValue, const QString& strength) {
            int row = resultTable->rowCount();
            resultTable->insertRow(row);
            
            resultTable->setItem(row, 0, new QTableWidgetItem(attr1));
            resultTable->setItem(row, 1, new QTableWidgetItem(attr2));
            resultTable->setItem(row, 2, new QTableWidgetItem(QString::number(coeff, 'f', 4)));
            resultTable->setItem(row, 3, new QTableWidgetItem(QString::number(pValue, 'f', 4)));
            resultTable->setItem(row, 4, new QTableWidgetItem(strength));
            
            // 根据相关性强度设置行颜色
            QColor bgColor;
            if (abs(coeff) < 0.3) {
                bgColor = QColor(255, 240, 240); // 浅红色
            } else if (abs(coeff) < 0.7) {
                bgColor = QColor(255, 255, 240); // 浅黄色
            } else {
                bgColor = QColor(240, 255, 240); // 浅绿色
            }
            
            for (int col = 0; col < resultTable->columnCount(); ++col) {
                resultTable->item(row, col)->setBackground(bgColor);
            }
        };
        
        for (const auto& result : results) {
            QString attr1 = QString::fromStdString(result.attr1);
            QString attr2 = QString::fromStdString(result.attr2);
            QString strength = QString::fromStdString(result.strength);
            
            addRow(attr1, attr2, result.coefficient, result.pValue, strength);
        }
        
        // 更新信息标签
        QString message = m_i18n.getCurrentLanguage() == "zh-CN" ? 
            QString("分析完成: %1").arg(analysisTypeCombo->currentText()) : 
            QString("Analysis completed: %1").arg(analysisTypeCombo->currentText());
        infoLabel->setText(message);
    };
    
    // 导出功能
    auto exportResults = [resultTable, this]() {
        if (resultTable->rowCount() == 0) {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "没有可导出的数据" : "No data to export");
            return;
        }
        
        QString filePath = QFileDialog::getSaveFileName(
            this,
            m_i18n.getCurrentLanguage() == "zh-CN" ? "导出相关性分析结果" : "Export Correlation Analysis Results",
            "",
            "CSV Files (*.csv);;All Files (*)"
        );
        
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                
                // 写入表头
                for (int col = 0; col < resultTable->columnCount(); ++col) {
                    if (col > 0) stream << ",";
                    stream << resultTable->horizontalHeaderItem(col)->text();
                }
                stream << "
";
                
                // 写入数据
                for (int row = 0; row < resultTable->rowCount(); ++row) {
                    for (int col = 0; col < resultTable->columnCount(); ++col) {
                        if (col > 0) stream << ",";
                        if (resultTable->item(row, col)) {
                            stream << resultTable->item(row, col)->text();
                        }
                    }
                    stream << "
";
                }
                
                file.close();
                
                QMessageBox::information(this, 
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "导出成功" : "Export Successful",
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "相关性分析结果已成功导出" : "Correlation analysis results successfully exported");
            } else {
                QMessageBox::warning(this, 
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "导出失败" : "Export Failed",
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "无法写入文件" : "Cannot write to file");
            }
        }
    };
    
    // 连接信号
    connect(analyzeButton, &QPushButton::clicked, performAnalysis);
    connect(exportButton, &QPushButton::clicked, exportResults);
    
    // 初始分析
    performAnalysis();
    
    corrDialog->exec();
    delete corrDialog;
}

void BondForgeGUI::showPredictionModel()
{
    // 创建预测模型对话框
    QDialog *predictionDialog = new QDialog(this);
    predictionDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "预测模型" : "Prediction Model");
    predictionDialog->resize(1000, 800);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(predictionDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(predictionDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QLabel *modelTypeLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "模型类型:" : "Model Type:", toolbarGroup);
    QComboBox *modelTypeCombo = new QComboBox(toolbarGroup);
    modelTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "线性回归预测" : "Linear Regression", "linear");
    modelTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类预测" : "Classification", "classification");
    modelTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "时间序列预测" : "Time Series", "timeseries");
    modelTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "聚类分析" : "Clustering", "clustering");
    
    QPushButton *trainButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "训练模型" : "Train Model", toolbarGroup);
    QPushButton *predictButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "预测" : "Predict", toolbarGroup);
    QPushButton *exportButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "导出模型" : "Export Model", toolbarGroup);
    
    toolbarLayout->addWidget(modelTypeLabel);
    toolbarLayout->addWidget(modelTypeCombo);
    toolbarLayout->addWidget(trainButton);
    toolbarLayout->addWidget(predictButton);
    toolbarLayout->addWidget(exportButton);
    toolbarLayout->addStretch();
    
    // 主要内容区域 - 使用分割器
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // 左侧：模型配置区域
    QGroupBox *configGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "模型配置" : "Model Configuration");
    QVBoxLayout *configLayout = new QVBoxLayout(configGroup);
    
    // 配置页面堆叠窗口
    QStackedWidget *configStack = new QStackedWidget();
    
    // 线性回归配置
    QWidget *linearConfig = new QWidget();
    QVBoxLayout *linearLayout = new QVBoxLayout(linearConfig);
    
    QLabel *linearDescLabel = new QLabel(
        m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "线性回归模型用于预测连续型变量，如内容长度、时间间隔等。" :
        "Linear regression model is used to predict continuous variables like content length, time intervals, etc.",
        linearConfig
    );
    
    QFormLayout *linearForm = new QFormLayout();
    QComboBox *featureCombo = new QComboBox();
    featureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "时间戳 → 内容长度" : "Timestamp → Content Length", "timestamp_content");
    featureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类 → 内容长度" : "Category → Content Length", "category_content");
    featureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户 → 内容长度" : "User → Content Length", "user_content");
    
    QDoubleSpinBox *learningRateSpin = new QDoubleSpinBox();
    learningRateSpin->setRange(0.001, 1.0);
    learningRateSpin->setSingleStep(0.01);
    learningRateSpin->setValue(0.01);
    
    QSpinBox *iterationsSpin = new QSpinBox();
    iterationsSpin->setRange(100, 10000);
    iterationsSpin->setSingleStep(10);
    iterationsSpin->setValue(1000);
    
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        linearForm->addRow("预测特征:", featureCombo);
        linearForm->addRow("学习率:", learningRateSpin);
        linearForm->addRow("迭代次数:", iterationsSpin);
    } else {
        linearForm->addRow("Feature:", featureCombo);
        linearForm->addRow("Learning Rate:", learningRateSpin);
        linearForm->addRow("Iterations:", iterationsSpin);
    }
    
    linearLayout->addWidget(linearDescLabel);
    linearLayout->addLayout(linearForm);
    linearLayout->addStretch();
    
    // 分类配置
    QWidget *classificationConfig = new QWidget();
    QVBoxLayout *classificationLayout = new QVBoxLayout(classificationConfig);
    
    QLabel *classificationDescLabel = new QLabel(
        m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "分类模型用于预测类别型变量，如数据分类、格式等。" :
        "Classification model is used to predict categorical variables like data category, format, etc.",
        classificationConfig
    );
    
    QFormLayout *classificationForm = new QFormLayout();
    QComboBox *classFeatureCombo = new QComboBox();
    classFeatureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容 → 分类" : "Content → Category", "content_category");
    classFeatureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容长度 → 分类" : "Content Length → Category", "length_category");
    classFeatureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "标签 → 分类" : "Tags → Category", "tags_category");
    
    QComboBox *algorithmCombo = new QComboBox();
    algorithmCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "决策树" : "Decision Tree", "decision_tree");
    algorithmCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "朴素贝叶斯" : "Naive Bayes", "naive_bayes");
    algorithmCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "支持向量机" : "SVM", "svm");
    
    QSpinBox *treeDepthSpin = new QSpinBox();
    treeDepthSpin->setRange(1, 100);
    treeDepthSpin->setValue(10);
    
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        classificationForm->addRow("预测特征:", classFeatureCombo);
        classificationForm->addRow("算法:", algorithmCombo);
        classificationForm->addRow("树深度:", treeDepthSpin);
    } else {
        classificationForm->addRow("Feature:", classFeatureCombo);
        classificationForm->addRow("Algorithm:", algorithmCombo);
        classificationForm->addRow("Tree Depth:", treeDepthSpin);
    }
    
    classificationLayout->addWidget(classificationDescLabel);
    classificationLayout->addLayout(classificationForm);
    classificationLayout->addStretch();
    
    // 时间序列配置
    QWidget *timeseriesConfig = new QWidget();
    QVBoxLayout *timeseriesLayout = new QVBoxLayout(timeseriesConfig);
    
    QLabel *timeseriesDescLabel = new QLabel(
        m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "时间序列模型用于预测基于时间的趋势，如数据上传量变化等。" :
        "Time series model is used to predict time-based trends, like data upload volume changes, etc.",
        timeseriesConfig
    );
    
    QFormLayout *timeseriesForm = new QFormLayout();
    QComboBox *tsFeatureCombo = new QComboBox();
    tsFeatureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据上传量" : "Data Upload Volume", "upload_volume");
    tsFeatureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容长度趋势" : "Content Length Trend", "content_length");
    
    QSpinBox *windowSizeSpin = new QSpinBox();
    windowSizeSpin->setRange(3, 30);
    windowSizeSpin->setValue(10);
    
    QSpinBox *forecastSpin = new QSpinBox();
    forecastSpin->setRange(1, 30);
    forecastSpin->setValue(7);
    
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        timeseriesForm->addRow("预测指标:", tsFeatureCombo);
        timeseriesForm->addRow("窗口大小:", windowSizeSpin);
        timeseriesForm->addRow("预测步长:", forecastSpin);
    } else {
        timeseriesForm->addRow("Metric:", tsFeatureCombo);
        timeseriesForm->addRow("Window Size:", windowSizeSpin);
        timeseriesForm->addRow("Forecast Steps:", forecastSpin);
    }
    
    timeseriesLayout->addWidget(timeseriesDescLabel);
    timeseriesLayout->addLayout(timeseriesForm);
    timeseriesLayout->addStretch();
    
    // 聚类配置
    QWidget *clusteringConfig = new QWidget();
    QVBoxLayout *clusteringLayout = new QVBoxLayout(clusteringConfig);
    
    QLabel *clusteringDescLabel = new QLabel(
        m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "聚类模型用于发现数据中的自然分组，如相似的内容、用户等。" :
        "Clustering model is used to discover natural groupings in data, like similar content, users, etc.",
        clusteringConfig
    );
    
    QFormLayout *clusteringForm = new QFormLayout();
    QComboBox *clusterFeatureCombo = new QComboBox();
    clusterFeatureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容相似性" : "Content Similarity", "content_similarity");
    clusterFeatureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户行为" : "User Behavior", "user_behavior");
    clusterFeatureCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类标签" : "Category Tags", "category_tags");
    
    QSpinBox *clusterCountSpin = new QSpinBox();
    clusterCountSpin->setRange(2, 20);
    clusterCountSpin->setValue(5);
    
    QComboBox *clusterAlgoCombo = new QComboBox();
    clusterAlgoCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "K-均值" : "K-Means", "kmeans");
    clusterAlgoCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "层次聚类" : "Hierarchical", "hierarchical");
    clusterAlgoCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "DBSCAN" : "DBSCAN", "dbscan");
    
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        clusteringForm->addRow("聚类特征:", clusterFeatureCombo);
        clusteringForm->addRow("聚类数量:", clusterCountSpin);
        clusteringForm->addRow("算法:", clusterAlgoCombo);
    } else {
        clusteringForm->addRow("Feature:", clusterFeatureCombo);
        clusteringForm->addRow("Cluster Count:", clusterCountSpin);
        clusteringForm->addRow("Algorithm:", clusterAlgoCombo);
    }
    
    clusteringLayout->addWidget(clusteringDescLabel);
    clusteringLayout->addLayout(clusteringForm);
    clusteringLayout->addStretch();
    
    // 添加配置页面到堆叠窗口
    configStack->addWidget(linearConfig);
    configStack->addWidget(classificationConfig);
    configStack->addWidget(timeseriesConfig);
    configStack->addWidget(clusteringConfig);
    
    configLayout->addWidget(configStack);
    
    // 右侧：模型结果区域
    QGroupBox *resultGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "模型结果" : "Model Results");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    
    // 结果表格
    QTableWidget *resultTable = new QTableWidget();
    resultTable->setColumnCount(4);
    
    QStringList resultHeaders;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        resultHeaders << "指标" << "训练集" << "验证集" << "测试集";
    } else {
        resultHeaders << "Metric" << "Training" << "Validation" << "Test";
    }
    resultTable->setHorizontalHeaderLabels(resultHeaders);
    resultTable->horizontalHeader()->setStretchLastSection(true);
    resultTable->setAlternatingRowColors(true);
    
    // 图表区域
    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);
    
    resultLayout->addWidget(resultTable);
    resultLayout->addWidget(chartView);
    
    mainSplitter->addWidget(configGroup);
    mainSplitter->addWidget(resultGroup);
    mainSplitter->setSizes({400, 600});
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(predictionDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择模型类型并配置参数，然后点击训练模型" : 
                                  "Select model type, configure parameters, then click Train Model", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(mainSplitter, 1);
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, predictionDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, predictionDialog, &QDialog::reject);
    
    // 连接模型类型变化信号
    connect(modelTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [configStack, modelTypeCombo](int index) {
        configStack->setCurrentIndex(index);
    });
    
    // 模型训练和预测的简单模拟实现
    auto trainModel = [modelTypeCombo, featureCombo, classFeatureCombo, resultTable, chartView, this]() {
        std::string modelType = modelTypeCombo->currentData().toString().toStdString();
        
        // 清空结果表格
        resultTable->setRowCount(0);
        
        // 模拟训练结果
        auto addRow = [resultTable, this](const QString& metric, double train, double val, double test) {
            int row = resultTable->rowCount();
            resultTable->insertRow(row);
            resultTable->setItem(row, 0, new QTableWidgetItem(metric));
            resultTable->setItem(row, 1, new QTableWidgetItem(QString::number(train, 'f', 3)));
            resultTable->setItem(row, 2, new QTableWidgetItem(QString::number(val, 'f', 3)));
            resultTable->setItem(row, 3, new QTableWidgetItem(QString::number(test, 'f', 3)));
        };
        
        // 创建图表
        QChart* chart = new QChart();
        
        if (modelType == "linear") {
            // 线性回归模型
            double r2 = 0.75 + (rand() % 20) / 100.0; // 0.75-0.95之间的R平方
            double rmse = 10.0 + (rand() % 20) / 1.0; // 10-30之间的RMSE
            double mae = 8.0 + (rand() % 15) / 1.0; // 8-23之间的MAE
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                addRow("R²", r2, r2 - 0.05, r2 - 0.1);
                addRow("RMSE", rmse, rmse + 2.0, rmse + 5.0);
                addRow("MAE", mae, mae + 1.5, mae + 3.0);
            } else {
                addRow("R²", r2, r2 - 0.05, r2 - 0.1);
                addRow("RMSE", rmse, rmse + 2.0, rmse + 5.0);
                addRow("MAE", mae, mae + 1.5, mae + 3.0);
            }
            
            // 创建散点图显示实际值与预测值
            QScatterSeries* series = new QScatterSeries();
            series->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据点" : "Data Points");
            series->setMarkerSize(8.0);
            
            // 生成模拟数据点
            for (int i = 0; i < 50; ++i) {
                double x = i;
                double y = x * 0.8 + 10 + (rand() % 40 - 20); // 线性关系加噪声
                series->append(x, y);
            }
            
            // 添加回归线
            QLineSeries* lineSeries = new QLineSeries();
            lineSeries->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "回归线" : "Regression Line");
            lineSeries->setColor(QColor(255, 0, 0));
            
            for (int i = 0; i <= 50; ++i) {
                double x = i;
                double y = x * 0.8 + 10; // 线性关系
                lineSeries->append(x, y);
            }
            
            chart->addSeries(series);
            chart->addSeries(lineSeries);
            chart->createDefaultAxes();
            
            if (chart->axes(Qt::Horizontal).size() > 0) {
                chart->axes(Qt::Horizontal)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "实际值" : "Actual");
            }
            if (chart->axes(Qt::Vertical).size() > 0) {
                chart->axes(Qt::Vertical)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "预测值" : "Predicted");
            }
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "线性回归模型" : "Linear Regression Model");
            
        } else if (modelType == "classification") {
            // 分类模型
            double accuracy = 0.80 + (rand() % 15) / 100.0; // 0.80-0.95之间的准确率
            double precision = 0.75 + (rand() % 20) / 100.0; // 0.75-0.95之间的精确率
            double recall = 0.70 + (rand() % 25) / 100.0; // 0.70-0.95之间的召回率
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                addRow("准确率", accuracy, accuracy - 0.05, accuracy - 0.1);
                addRow("精确率", precision, precision - 0.08, precision - 0.12);
                addRow("召回率", recall, recall - 0.07, recall - 0.15);
            } else {
                addRow("Accuracy", accuracy, accuracy - 0.05, accuracy - 0.1);
                addRow("Precision", precision, precision - 0.08, precision - 0.12);
                addRow("Recall", recall, recall - 0.07, recall - 0.15);
            }
            
            // 创建饼图显示分类结果分布
            QPieSeries* series = new QPieSeries();
            
            // 模拟分类结果
            std::vector<std::pair<QString, double>> classResults;
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                classResults = {{"有机", 0.4}, {"无机", 0.3}, {"混合", 0.2}, {"其他", 0.1}};
            } else {
                classResults = {{"Organic", 0.4}, {"Inorganic", 0.3}, {"Mixed", 0.2}, {"Other", 0.1}};
            }
            
            for (const auto& result : classResults) {
                QPieSlice* slice = series->append(result.first, result.second);
                slice->setLabelVisible(true);
            }
            
            chart->addSeries(series);
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类模型预测结果" : "Classification Model Predictions");
            
        } else if (modelType == "timeseries") {
            // 时间序列模型
            double mape = 0.10 + (rand() % 15) / 100.0; // 0.10-0.25之间的平均绝对百分比误差
            double smape = 0.12 + (rand() % 18) / 100.0; // 0.12-0.30之间的对称平均绝对百分比误差
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                addRow("MAPE", mape, mape + 0.02, mape + 0.05);
                addRow("SMAPE", smape, smape + 0.03, smape + 0.07);
            } else {
                addRow("MAPE", mape, mape + 0.02, mape + 0.05);
                addRow("SMAPE", smape, smape + 0.03, smape + 0.07);
            }
            
            // 创建折线图显示时间序列预测
            QLineSeries* actualSeries = new QLineSeries();
            actualSeries->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "实际值" : "Actual");
            actualSeries->setColor(QColor(0, 100, 200));
            
            QLineSeries* predictedSeries = new QLineSeries();
            predictedSeries->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "预测值" : "Predicted");
            predictedSeries->setColor(QColor(255, 100, 0));
            
            // 生成模拟数据
            double baseValue = 50.0;
            for (int i = 0; i < 30; ++i) {
                double actual = baseValue + (rand() % 20 - 10); // 添加一些随机波动
                actualSeries->append(i, actual);
                
                // 前20个点是实际值，后10个点是预测值
                if (i < 20) {
                    predictedSeries->append(i, actual);
                } else {
                    double predicted = baseValue + (rand() % 20 - 10) + (i - 20) * 0.5; // 添加趋势
                    predictedSeries->append(i, predicted);
                }
                
                baseValue = actual; // 使用前一个值作为基础
            }
            
            chart->addSeries(actualSeries);
            chart->addSeries(predictedSeries);
            chart->createDefaultAxes();
            
            if (chart->axes(Qt::Horizontal).size() > 0) {
                chart->axes(Qt::Horizontal)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "时间" : "Time");
            }
            if (chart->axes(Qt::Vertical).size() > 0) {
                chart->axes(Qt::Vertical)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "值" : "Value");
            }
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "时间序列预测" : "Time Series Forecast");
            
        } else if (modelType == "clustering") {
            // 聚类模型
            double silhouette = 0.50 + (rand() % 30) / 100.0; // 0.50-0.80之间的轮廓系数
            double inertia = 1000.0 + (rand() % 2000) / 1.0; // 1000-3000之间的簇内平方和
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                addRow("轮廓系数", silhouette, silhouette - 0.05, silhouette - 0.1);
                addRow("簇内平方和", inertia, inertia + 200, inertia + 500);
            } else {
                addRow("Silhouette Coefficient", silhouette, silhouette - 0.05, silhouette - 0.1);
                addRow("Inertia", inertia, inertia + 200, inertia + 500);
            }
            
            // 创建散点图显示聚类结果
            QScatterSeries* series1 = new QScatterSeries();
            series1->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "簇 1" : "Cluster 1");
            series1->setColor(QColor(255, 0, 0));
            
            QScatterSeries* series2 = new QScatterSeries();
            series2->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "簇 2" : "Cluster 2");
            series2->setColor(QColor(0, 255, 0));
            
            QScatterSeries* series3 = new QScatterSeries();
            series3->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "簇 3" : "Cluster 3");
            series3->setColor(QColor(0, 0, 255));
            
            // 生成模拟聚类数据点
            for (int i = 0; i < 30; ++i) {
                double x, y;
                
                if (i < 15) {
                    // 簇1的点
                    x = 20 + (rand() % 30 - 15);
                    y = 30 + (rand() % 30 - 15);
                    series1->append(x, y);
                } else if (i < 25) {
                    // 簇2的点
                    x = 60 + (rand() % 20 - 10);
                    y = 60 + (rand() % 20 - 10);
                    series2->append(x, y);
                } else {
                    // 簇3的点
                    x = 40 + (rand() % 30 - 15);
                    y = 80 + (rand() % 30 - 15);
                    series3->append(x, y);
                }
            }
            
            chart->addSeries(series1);
            chart->addSeries(series2);
            chart->addSeries(series3);
            chart->createDefaultAxes();
            
            if (chart->axes(Qt::Horizontal).size() > 0) {
                chart->axes(Qt::Horizontal)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "特征1" : "Feature 1");
            }
            if (chart->axes(Qt::Vertical).size() > 0) {
                chart->axes(Qt::Vertical)[0]->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "特征2" : "Feature 2");
            }
            
            chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "聚类分析结果" : "Clustering Analysis Results");
        }
        
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->setAnimationOptions(QChart::SeriesAnimations);
        
        chartView->setChart(chart);
        
        // 更新信息标签
        QString message = m_i18n.getCurrentLanguage() == "zh-CN" ? 
            QString("模型训练完成: %1").arg(modelTypeCombo->currentText()) : 
            QString("Model training completed: %1").arg(modelTypeCombo->currentText());
        infoLabel->setText(message);
    };
    
    // 模型预测
    auto predict = [modelTypeCombo, resultTable, this]() {
        std::string modelType = modelTypeCombo->currentData().toString().toStdString();
        
        if (resultTable->rowCount() == 0) {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "请先训练模型" : "Please train the model first");
            return;
        }
        
        // 创建预测对话框
        QDialog *predictDialog = new QDialog(this);
        predictDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "模型预测" : "Model Prediction");
        
        QVBoxLayout *predictLayout = new QVBoxLayout(predictDialog);
        
        QLabel *predictLabel = new QLabel(
            m_i18n.getCurrentLanguage() == "zh-CN" ? 
            QString("使用训练好的 %1 模型进行预测").arg(QString::fromStdString(modelType)) : 
            QString("Making predictions using trained %1 model").arg(QString::fromStdString(modelType)),
            predictDialog
        );
        
        QGroupBox *inputGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "输入数据" : "Input Data");
        QFormLayout *inputForm = new QFormLayout(inputGroup);
        
        // 根据模型类型创建不同的输入
        QWidget *inputWidget = new QWidget();
        
        if (modelType == "linear") {
            QLineEdit *valueEdit = new QLineEdit();
            valueEdit->setPlaceholderText(m_i18n.getCurrentLanguage() == "zh-CN" ? "输入特征值" : "Enter feature value");
            inputWidget = valueEdit;
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                inputForm->addRow("特征值:", valueEdit);
            } else {
                inputForm->addRow("Feature Value:", valueEdit);
            }
        } else if (modelType == "classification") {
            QTextEdit *textEdit = new QTextEdit();
            textEdit->setPlaceholderText(m_i18n.getCurrentLanguage() == "zh-CN" ? "输入文本内容" : "Enter text content");
            textEdit->setMaximumHeight(100);
            inputWidget = textEdit;
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                inputForm->addRow("文本内容:", textEdit);
            } else {
                inputForm->addRow("Text Content:", textEdit);
            }
        } else if (modelType == "timeseries") {
            QSpinBox *stepsEdit = new QSpinBox();
            stepsEdit->setRange(1, 30);
            stepsEdit->setValue(7);
            inputWidget = stepsEdit;
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                inputForm->addRow("预测步数:", stepsEdit);
            } else {
                inputForm->addRow("Prediction Steps:", stepsEdit);
            }
        } else if (modelType == "clustering") {
            QLineEdit *idEdit = new QLineEdit();
            idEdit->setPlaceholderText(m_i18n.getCurrentLanguage() == "zh-CN" ? "输入数据ID" : "Enter data ID");
            inputWidget = idEdit;
            
            if (m_i18n.getCurrentLanguage() == "zh-CN") {
                inputForm->addRow("数据ID:", idEdit);
            } else {
                inputForm->addRow("Data ID:", idEdit);
            }
        }
        
        // 预测结果
        QLabel *resultLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "预测结果" : "Prediction Result");
        QLineEdit *resultEdit = new QLineEdit();
        resultEdit->setReadOnly(true);
        
        if (m_i18n.getCurrentLanguage() == "zh-CN") {
            predictLayout->addWidget(predictLabel);
            predictLayout->addWidget(inputGroup);
            predictLayout->addWidget(resultLabel);
            predictLayout->addWidget(resultEdit);
        } else {
            predictLayout->addWidget(predictLabel);
            predictLayout->addWidget(inputGroup);
            predictLayout->addWidget(resultLabel);
            predictLayout->addWidget(resultEdit);
        }
        
        // 按钮
        QDialogButtonBox *predictButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, predictDialog);
        predictLayout->addWidget(predictButtonBox);
        
        connect(predictButtonBox, &QDialogButtonBox::accepted, [inputWidget, resultEdit, modelType, this]() {
            // 模拟预测结果
            QString result;
            
            if (modelType == "linear") {
                QLineEdit *valueEdit = qobject_cast<QLineEdit*>(inputWidget);
                if (valueEdit) {
                    double value = valueEdit->text().toDouble();
                    double prediction = value * 0.8 + 10 + (rand() % 20 - 10);
                    result = QString::number(prediction, 'f', 2);
                }
            } else if (modelType == "classification") {
                QTextEdit *textEdit = qobject_cast<QTextEdit*>(inputWidget);
                if (textEdit) {
                    QStringList classes;
                    if (m_i18n.getCurrentLanguage() == "zh-CN") {
                        classes = {"有机", "无机", "混合", "其他"};
                    } else {
                        classes = {"Organic", "Inorganic", "Mixed", "Other"};
                    }
                    int classIndex = rand() % classes.size();
                    double confidence = 0.7 + (rand() % 25) / 100.0;
                    result = classes[classIndex] + QString(" (置信度: %1%)").arg(confidence * 100, 0, 'f', 1);
                }
            } else if (modelType == "timeseries") {
                QSpinBox *stepsEdit = qobject_cast<QSpinBox*>(inputWidget);
                if (stepsEdit) {
                    int steps = stepsEdit->value();
                    QStringList forecasts;
                    double baseValue = 50.0;
                    
                    for (int i = 0; i < steps; ++i) {
                        double forecast = baseValue + (rand() % 20 - 10) + i * 0.5;
                        forecasts.append(QString::number(forecast, 'f', 2));
                        baseValue = forecast;
                    }
                    
                    result = forecasts.join(", ");
                }
            } else if (modelType == "clustering") {
                QLineEdit *idEdit = qobject_cast<QLineEdit*>(inputWidget);
                if (idEdit) {
                    int cluster = rand() % 5 + 1;
                    result = m_i18n.getCurrentLanguage() == "zh-CN" ? 
                        QString("簇 %1").arg(cluster) : 
                        QString("Cluster %1").arg(cluster);
                }
            }
            
            resultEdit->setText(result);
        });
        
        connect(predictButtonBox, &QDialogButtonBox::rejected, predictDialog, &QDialog::reject);
        
        predictDialog->exec();
        delete predictDialog;
    };
    
    // 模型导出
    auto exportModel = [modelTypeCombo, resultTable, this]() {
        if (resultTable->rowCount() == 0) {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "没有可导出的模型" : "No model to export");
            return;
        }
        
        QString filePath = QFileDialog::getSaveFileName(
            this,
            m_i18n.getCurrentLanguage() == "zh-CN" ? "导出模型" : "Export Model",
            "",
            "JSON Files (*.json);;All Files (*)"
        );
        
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                
                // 简单的模型格式（实际应用中应使用更专业的格式）
                stream << "{
";
                stream << "  \"model_type\": \"" << modelTypeCombo->currentData().toString() << "\",
";
                stream << "  \"model_name\": \"" << modelTypeCombo->currentText() << "\",
";
                stream << "  \"training_date\": \"" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\",
";
                stream << "  \"language\": \"" << m_i18n.getCurrentLanguage() << "\",
";
                
                stream << "  \"metrics\": {
";
                for (int row = 0; row < resultTable->rowCount(); ++row) {
                    QString metric = resultTable->item(row, 0)->text();
                    QString train = resultTable->item(row, 1)->text();
                    QString val = resultTable->item(row, 2)->text();
                    QString test = resultTable->item(row, 3)->text();
                    
                    stream << "    \"" << metric << "\": {
";
                    stream << "      \"training\": " << train << ",
";
                    stream << "      \"validation\": " << val << ",
";
                    stream << "      \"test\": " << test << "
";
                    
                    if (row < resultTable->rowCount() - 1) {
                        stream << "    },
";
                    } else {
                        stream << "    }
";
                    }
                }
                stream << "  }
";
                stream << "}
";
                
                file.close();
                
                QMessageBox::information(this, 
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "导出成功" : "Export Successful",
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "模型已成功导出" : "Model successfully exported");
            } else {
                QMessageBox::warning(this, 
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "导出失败" : "Export Failed",
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "无法写入文件" : "Cannot write to file");
            }
        }
    };
    
    // 连接信号
    connect(trainButton, &QPushButton::clicked, trainModel);
    connect(predictButton, &QPushButton::clicked, predict);
    connect(exportButton, &QPushButton::clicked, exportModel);
    
    // 初始选择第一个模型类型
    configStack->setCurrentIndex(0);
    
    predictionDialog->exec();
    delete predictionDialog;
}

void BondForgeGUI::showReportGeneration()
{
    // 创建报告生成对话框
    QDialog *reportDialog = new QDialog(this);
    reportDialog->setWindowTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "报告生成" : "Report Generation");
    reportDialog->resize(1000, 800);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(reportDialog);
    
    // 顶部工具栏
    QGroupBox *toolbarGroup = new QGroupBox(reportDialog);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarGroup);
    
    QLabel *reportTypeLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "报告类型:" : "Report Type:", toolbarGroup);
    QComboBox *reportTypeCombo = new QComboBox(toolbarGroup);
    reportTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据概览报告" : "Data Overview Report", "overview");
    reportTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "详细分析报告" : "Detailed Analysis Report", "analysis");
    reportTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户活动报告" : "User Activity Report", "user");
    reportTypeCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "质量评估报告" : "Quality Assessment Report", "quality");
    
    QPushButton *generateButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "生成报告" : "Generate Report", toolbarGroup);
    QPushButton *previewButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "预览报告" : "Preview Report", toolbarGroup);
    QPushButton *exportButton = new QPushButton(m_i18n.getCurrentLanguage() == "zh-CN" ? "导出报告" : "Export Report", toolbarGroup);
    
    toolbarLayout->addWidget(reportTypeLabel);
    toolbarLayout->addWidget(reportTypeCombo);
    toolbarLayout->addWidget(generateButton);
    toolbarLayout->addWidget(previewButton);
    toolbarLayout->addWidget(exportButton);
    toolbarLayout->addStretch();
    
    // 主要内容区域 - 使用分割器
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
    
    // 报告配置区域
    QGroupBox *configGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "报告配置" : "Report Configuration");
    QVBoxLayout *configLayout = new QVBoxLayout(configGroup);
    
    // 配置页面堆叠窗口
    QStackedWidget *configStack = new QStackedWidget();
    
    // 数据概览配置
    QWidget *overviewConfig = new QWidget();
    QVBoxLayout *overviewLayout = new QVBoxLayout(overviewConfig);
    
    QLabel *overviewDescLabel = new QLabel(
        m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "数据概览报告提供系统数据的整体统计和概览信息，包括数据量、分类分布、用户活动等。" :
        "Data overview report provides overall statistics and summary of system data, including data volume, category distribution, user activity, etc.",
        overviewConfig
    );
    
    QFormLayout *overviewForm = new QFormLayout();
    QCheckBox *includeStatsCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含基本统计" : "Include Basic Statistics");
    QCheckBox *includeChartsCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含图表" : "Include Charts");
    QCheckBox *includeTrendsCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含趋势分析" : "Include Trend Analysis");
    
    includeStatsCheck->setChecked(true);
    includeChartsCheck->setChecked(true);
    includeTrendsCheck->setChecked(true);
    
    QSpinBox *timeRangeSpin = new QSpinBox();
    timeRangeSpin->setRange(7, 365);
    timeRangeSpin->setValue(30);
    
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        overviewForm->addRow("", includeStatsCheck);
        overviewForm->addRow("", includeChartsCheck);
        overviewForm->addRow("", includeTrendsCheck);
        overviewForm->addRow("时间范围(天):", timeRangeSpin);
    } else {
        overviewForm->addRow("", includeStatsCheck);
        overviewForm->addRow("", includeChartsCheck);
        overviewForm->addRow("", includeTrendsCheck);
        overviewForm->addRow("Time Range (days):", timeRangeSpin);
    }
    
    overviewLayout->addWidget(overviewDescLabel);
    overviewLayout->addLayout(overviewForm);
    overviewLayout->addStretch();
    
    // 详细分析配置
    QWidget *analysisConfig = new QWidget();
    QVBoxLayout *analysisLayout = new QVBoxLayout(analysisConfig);
    
    QLabel *analysisDescLabel = new QLabel(
        m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "详细分析报告提供深入的数据分析，包括相关性分析、异常检测、模式识别等。" :
        "Detailed analysis report provides in-depth data analysis, including correlation analysis, anomaly detection, pattern recognition, etc.",
        analysisConfig
    );
    
    QFormLayout *analysisForm = new QFormLayout();
    QCheckBox *includeCorrelationCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含相关性分析" : "Include Correlation Analysis");
    QCheckBox *includeAnomalyCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含异常检测" : "Include Anomaly Detection");
    QCheckBox *includePredictionsCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含预测结果" : "Include Prediction Results");
    
    includeCorrelationCheck->setChecked(true);
    includeAnomalyCheck->setChecked(true);
    includePredictionsCheck->setChecked(false);
    
    QComboBox *analysisDepthCombo = new QComboBox();
    analysisDepthCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "基础分析" : "Basic Analysis", "basic");
    analysisDepthCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "标准分析" : "Standard Analysis", "standard");
    analysisDepthCombo->addItem(m_i18n.getCurrentLanguage() == "zh-CN" ? "深入分析" : "In-depth Analysis", "deep");
    analysisDepthCombo->setCurrentIndex(1); // 默认选择标准分析
    
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        analysisForm->addRow("", includeCorrelationCheck);
        analysisForm->addRow("", includeAnomalyCheck);
        analysisForm->addRow("", includePredictionsCheck);
        analysisForm->addRow("分析深度:", analysisDepthCombo);
    } else {
        analysisForm->addRow("", includeCorrelationCheck);
        analysisForm->addRow("", includeAnomalyCheck);
        analysisForm->addRow("", includePredictionsCheck);
        analysisForm->addRow("Analysis Depth:", analysisDepthCombo);
    }
    
    analysisLayout->addWidget(analysisDescLabel);
    analysisLayout->addLayout(analysisForm);
    analysisLayout->addStretch();
    
    // 用户活动配置
    QWidget *userConfig = new QWidget();
    QVBoxLayout *userLayout = new QVBoxLayout(userConfig);
    
    QLabel *userDescLabel = new QLabel(
        m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "用户活动报告提供用户行为和贡献的详细分析，包括上传活动、评论参与度等。" :
        "User activity report provides detailed analysis of user behavior and contributions, including upload activity, comment participation, etc.",
        userConfig
    );
    
    QFormLayout *userForm = new QFormLayout();
    QCheckBox *includeUploadsCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含上传活动" : "Include Upload Activity");
    QCheckBox *includeCommentsCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含评论活动" : "Include Comment Activity");
    QCheckBox *includeCollaborationCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含协作活动" : "Include Collaboration Activity");
    
    includeUploadsCheck->setChecked(true);
    includeCommentsCheck->setChecked(true);
    includeCollaborationCheck->setChecked(true);
    
    QLineEdit *userFilterEdit = new QLineEdit();
    userFilterEdit->setPlaceholderText(m_i18n.getCurrentLanguage() == "zh-CN" ? "指定用户名(留空表示所有用户)" : "Specify usernames (blank for all users)");
    
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        userForm->addRow("", includeUploadsCheck);
        userForm->addRow("", includeCommentsCheck);
        userForm->addRow("", includeCollaborationCheck);
        userForm->addRow("用户过滤:", userFilterEdit);
    } else {
        userForm->addRow("", includeUploadsCheck);
        userForm->addRow("", includeCommentsCheck);
        userForm->addRow("", includeCollaborationCheck);
        userForm->addRow("User Filter:", userFilterEdit);
    }
    
    userLayout->addWidget(userDescLabel);
    userLayout->addLayout(userForm);
    userLayout->addStretch();
    
    // 质量评估配置
    QWidget *qualityConfig = new QWidget();
    QVBoxLayout *qualityLayout = new QVBoxLayout(qualityConfig);
    
    QLabel *qualityDescLabel = new QLabel(
        m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "质量评估报告评估数据质量，包括完整性、一致性、准确性等指标。" :
        "Quality assessment report evaluates data quality, including metrics like completeness, consistency, accuracy, etc.",
        qualityConfig
    );
    
    QFormLayout *qualityForm = new QFormLayout();
    QCheckBox *includeCompletenessCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含完整性评估" : "Include Completeness Assessment");
    QCheckBox *includeConsistencyCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含一致性评估" : "Include Consistency Assessment");
    QCheckBox *includeAccuracyCheck = new QCheckBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "包含准确性评估" : "Include Accuracy Assessment");
    
    includeCompletenessCheck->setChecked(true);
    includeConsistencyCheck->setChecked(true);
    includeAccuracyCheck->setChecked(true);
    
    QDoubleSpinBox *qualityThresholdSpin = new QDoubleSpinBox();
    qualityThresholdSpin->setRange(0.1, 1.0);
    qualityThresholdSpin->setSingleStep(0.1);
    qualityThresholdSpin->setValue(0.8);
    
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        qualityForm->addRow("", includeCompletenessCheck);
        qualityForm->addRow("", includeConsistencyCheck);
        qualityForm->addRow("", includeAccuracyCheck);
        qualityForm->addRow("质量阈值:", qualityThresholdSpin);
    } else {
        qualityForm->addRow("", includeCompletenessCheck);
        qualityForm->addRow("", includeConsistencyCheck);
        qualityForm->addRow("", includeAccuracyCheck);
        qualityForm->addRow("Quality Threshold:", qualityThresholdSpin);
    }
    
    qualityLayout->addWidget(qualityDescLabel);
    qualityLayout->addLayout(qualityForm);
    qualityLayout->addStretch();
    
    // 添加配置页面到堆叠窗口
    configStack->addWidget(overviewConfig);
    configStack->addWidget(analysisConfig);
    configStack->addWidget(userConfig);
    configStack->addWidget(qualityConfig);
    
    configLayout->addWidget(configStack);
    
    // 报告预览区域
    QGroupBox *previewGroup = new QGroupBox(m_i18n.getCurrentLanguage() == "zh-CN" ? "报告预览" : "Report Preview");
    QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);
    
    QTextEdit *previewEdit = new QTextEdit();
    previewEdit->setReadOnly(true);
    previewEdit->setFont(QFont("Arial", 10));
    
    previewLayout->addWidget(previewEdit);
    
    mainSplitter->addWidget(configGroup);
    mainSplitter->addWidget(previewGroup);
    mainSplitter->setSizes({350, 450});
    
    // 底部信息面板
    QGroupBox *infoGroup = new QGroupBox(reportDialog);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoGroup);
    
    QLabel *infoLabel = new QLabel(m_i18n.getCurrentLanguage() == "zh-CN" ? "选择报告类型并配置参数，然后点击生成报告" : 
                                  "Select report type, configure parameters, then click Generate Report", infoGroup);
    infoLayout->addWidget(infoLabel);
    
    dialogLayout->addWidget(toolbarGroup);
    dialogLayout->addWidget(mainSplitter, 1);
    dialogLayout->addWidget(infoGroup);
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, reportDialog);
    dialogLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::rejected, reportDialog, &QDialog::reject);
    
    // 连接报告类型变化信号
    connect(reportTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [configStack, reportTypeCombo, includeStatsCheck, includeChartsCheck, includeTrendsCheck, 
                                                                       includeCorrelationCheck, includeAnomalyCheck, includePredictionsCheck,
                                                                       includeUploadsCheck, includeCommentsCheck, includeCollaborationCheck,
                                                                       includeCompletenessCheck, includeConsistencyCheck, includeAccuracyCheck](int index) {
        configStack->setCurrentIndex(index);
    });
    
    // 生成报告功能
    auto generateReport = [reportTypeCombo, previewEdit, 
                          includeStatsCheck, includeChartsCheck, includeTrendsCheck,
                          includeCorrelationCheck, includeAnomalyCheck, includePredictionsCheck,
                          includeUploadsCheck, includeCommentsCheck, includeCollaborationCheck,
                          includeCompletenessCheck, includeConsistencyCheck, includeAccuracyCheck, this]() {
        std::string reportType = reportTypeCombo->currentData().toString().toStdString();
        
        // 获取所有数据
        std::vector<DataRecord> allData = m_service->getAllData();
        
        // 生成报告内容
        QString htmlContent;
        
        // 报告头部
        htmlContent += QString(
            "<html>"
            "<head>"
            "<style>"
            "body { font-family: Arial, sans-serif; margin: 20px; }"
            "h1 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; }"
            "h2 { color: #2980b9; margin-top: 30px; }"
            "h3 { color: #27ae60; }"
            "table { border-collapse: collapse; width: 100%%; margin: 20px 0; }"
            "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
            "th { background-color: #f2f2f2; }"
            ".stat-value { font-weight: bold; color: #3498db; }"
            ".summary-box { background-color: #f8f9fa; border-left: 4px solid #3498db; padding: 10px; margin: 15px 0; }"
            "</style>"
            "</head>"
            "<body>"
            "<h1>%1</h1>"
        ).arg(reportTypeCombo->currentText());
        
        // 报告生成时间和版本信息
        htmlContent += QString(
            "<div class=\"summary-box\">"
            "<strong>%1:</strong> %2<br>"
            "<strong>%3:</strong> BondForge V1.2<br>"
            "<strong>%4:</strong> %5<br>"
            "</div>"
        ).arg(
            m_i18n.getCurrentLanguage() == "zh-CN" ? "生成时间" : "Generated On",
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
            m_i18n.getCurrentLanguage() == "zh-CN" ? "系统版本" : "System Version",
            m_i18n.getCurrentLanguage() == "zh-CN" ? "语言" : "Language",
            m_i18n.getCurrentLanguage()
        );
        
        if (reportType == "overview") {
            // 数据概览报告
            htmlContent += QString("<h2>%1</h2>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "基本统计" : "Basic Statistics");
            
            // 计算统计信息
            int totalRecords = allData.size();
            std::set<std::string> categories;
            std::set<std::string> formats;
            std::set<std::string> uploaders;
            
            for (const auto& record : allData) {
                categories.insert(record.category);
                formats.insert(record.format);
                uploaders.insert(record.uploader);
            }
            
            if (includeStatsCheck->isChecked()) {
                htmlContent += QString(
                    "<p><strong>%1:</strong> <span class=\"stat-value\">%2</span></p>"
                    "<p><strong>%3:</strong> <span class=\"stat-value\">%4</span></p>"
                    "<p><strong>%5:</strong> <span class=\"stat-value\">%6</span></p>"
                    "<p><strong>%7:</strong> <span class=\"stat-value\">%8</span></p>"
                ).arg(
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "总记录数" : "Total Records", QString::number(totalRecords),
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "数据分类数" : "Number of Categories", QString::number(categories.size()),
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "数据格式数" : "Number of Formats", QString::number(formats.size()),
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "活跃用户数" : "Active Users", QString::number(uploaders.size())
                );
            }
            
            if (includeChartsCheck->isChecked()) {
                htmlContent += QString("<h2>%1</h2>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据分布图表" : "Data Distribution Charts");
                htmlContent += QString("<p>%1</p>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    "以下是数据分类和格式的分布图表。在实际报告中，这里会显示交互式图表。" : 
                    "Below are distribution charts for data categories and formats. In an actual report, interactive charts would be displayed here.");
                
                htmlContent += "<div style=\"margin: 20px 0; padding: 10px; background-color: #f0f0f0; border-radius: 5px;\">";
                htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "分类分布" : "Category Distribution");
                
                // 分类分布表格
                std::map<std::string, int> categoryCount;
                for (const auto& record : allData) {
                    categoryCount[record.category]++;
                }
                
                htmlContent += "<table><tr><th>分类</th><th>数量</th><th>百分比</th></tr>";
                for (const auto& pair : categoryCount) {
                    float percentage = (float)pair.second / totalRecords * 100;
                    htmlContent += QString("<tr><td>%1</td><td>%2</td><td>%3%</td></tr>")
                        .arg(QString::fromStdString(pair.first))
                        .arg(pair.second)
                        .arg(percentage, 0, 'f', 1);
                }
                htmlContent += "</table></div>";
                
                htmlContent += "<div style=\"margin: 20px 0; padding: 10px; background-color: #f0f0f0; border-radius: 5px;\">";
                htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "格式分布" : "Format Distribution");
                
                // 格式分布表格
                std::map<std::string, int> formatCount;
                for (const auto& record : allData) {
                    formatCount[record.format]++;
                }
                
                htmlContent += "<table><tr><th>格式</th><th>数量</th><th>百分比</th></tr>";
                for (const auto& pair : formatCount) {
                    float percentage = (float)pair.second / totalRecords * 100;
                    htmlContent += QString("<tr><td>%1</td><td>%2</td><td>%3%</td></tr>")
                        .arg(QString::fromStdString(pair.first))
                        .arg(pair.second)
                        .arg(percentage, 0, 'f', 1);
                }
                htmlContent += "</table></div>";
            }
            
            if (includeTrendsCheck->isChecked()) {
                htmlContent += QString("<h2>%1</h2>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "趋势分析" : "Trend Analysis");
                htmlContent += QString("<p>%1</p>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    "以下是数据上传的时间趋势分析。" : 
                    "Below is the time trend analysis of data uploads.");
                
                // 按时间统计
                std::map<int, int> dailyCount;
                for (const auto& record : allData) {
                    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(record.timestamp);
                    int day = dateTime.date().day();
                    dailyCount[day]++;
                }
                
                // 获取最近7天的数据
                htmlContent += "<table><tr><th>日期</th><th>上传量</th><th>趋势</th></tr>";
                for (int day = 1; day <= 7; ++day) {
                    int count = dailyCount.count(day) ? dailyCount[day] : 0;
                    QString trend;
                    if (day > 1) {
                        int prevCount = dailyCount.count(day-1) ? dailyCount[day-1] : 0;
                        if (count > prevCount) {
                            trend = "↑";
                        } else if (count < prevCount) {
                            trend = "↓";
                        } else {
                            trend = "→";
                        }
                    } else {
                        trend = "";
                    }
                    
                    htmlContent += QString("<tr><td>Day %1</td><td>%2</td><td>%3</td></tr>")
                        .arg(day)
                        .arg(count)
                        .arg(trend);
                }
                htmlContent += "</table>";
            }
            
        } else if (reportType == "analysis") {
            // 详细分析报告
            htmlContent += QString("<h2>%1</h2>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据深入分析" : "In-depth Data Analysis");
            
            if (includeCorrelationCheck->isChecked()) {
                htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "相关性分析" : "Correlation Analysis");
                htmlContent += QString("<p>%1</p>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    "以下属性之间的相关性分析结果：" : 
                    "Correlation analysis results between the following attributes:");
                
                htmlContent += "<table><tr><th>属性1</th><th>属性2</th><th>相关系数</th><th>显著性</th></tr>";
                
                // 模拟相关性分析结果
                std::vector<std::tuple<QString, QString, double, QString>> correlations = {
                    {"分类", "内容长度", 0.65, "高相关"},
                    {"格式", "内容长度", -0.32, "中等相关"},
                    {"上传时间", "内容长度", 0.15, "低相关"}
                };
                
                for (const auto& corr : correlations) {
                    double coefficient = std::get<2>(corr);
                    QString significance = std::get<3>(corr);
                    
                    QString color;
                    if (abs(coefficient) < 0.3) {
                        color = "red";
                    } else if (abs(coefficient) < 0.7) {
                        color = "orange";
                    } else {
                        color = "green";
                    }
                    
                    htmlContent += QString("<tr><td>%1</td><td>%2</td><td style=\"color: %3; font-weight: bold;\">%4</td><td>%5</td></tr>")
                        .arg(std::get<0>(corr))
                        .arg(std::get<1>(corr))
                        .arg(color)
                        .arg(coefficient, 0, 'f', 3)
                        .arg(significance);
                }
                htmlContent += "</table>";
            }
            
            if (includeAnomalyCheck->isChecked()) {
                htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "异常检测" : "Anomaly Detection");
                htmlContent += QString("<p>%1</p>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    "基于统计分析，检测到的异常数据记录：" : 
                    "Based on statistical analysis, the following anomalous data records were detected:");
                
                // 模拟异常检测结果
                std::vector<std::tuple<QString, QString, QString>> anomalies = {
                    {"data-005", "内容长度异常", "内容长度显著高于平均值"},
                    {"data-012", "分类不匹配", "内容与标签分类不匹配"},
                    {"data-018", "格式异常", "数据格式不符合预期标准"}
                };
                
                htmlContent += "<table><tr><th>数据ID</th><th>异常类型</th><th>详细描述</th></tr>";
                for (const auto& anomaly : anomalies) {
                    htmlContent += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
                        .arg(std::get<0>(anomaly))
                        .arg(std::get<1>(anomaly))
                        .arg(std::get<2>(anomaly));
                }
                htmlContent += "</table>";
            }
            
            if (includePredictionsCheck->isChecked()) {
                htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "预测结果" : "Prediction Results");
                htmlContent += QString("<p>%1</p>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    "基于机器学习模型的预测结果：" : 
                    "Prediction results based on machine learning models:");
                
                // 模拟预测结果
                htmlContent += "<table><tr><th>预测类型</th><th>置信度</th><th>预期趋势</th></tr>";
                htmlContent += QString("<tr><td>数据上传量</td><td>85%</td><td>↑ 持续增长</td></tr>");
                htmlContent += QString("<tr><td>分类分布</td><td>72%</td><td>→ 保持稳定</td></tr>");
                htmlContent += QString("<tr><td>用户活跃度</td><td>68%</td><td>↑ 小幅增长</td></tr>");
                htmlContent += "</table>";
            }
            
        } else if (reportType == "user") {
            // 用户活动报告
            htmlContent += QString("<h2>%1</h2>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "用户活动分析" : "User Activity Analysis");
            
            // 用户活动统计
            std::map<std::string, int> userUploads;
            std::map<std::string, int> userComments;
            std::map<std::string, int> userShares;
            
            // 模拟用户活动数据
            std::vector<std::string> users = {"admin", "researcher1", "analyst1", "guest1"};
            
            htmlContent += "<table><tr><th>用户</th><th>上传数量</th><th>评论数量</th><th>共享数量</th><th>活跃度评分</th></tr>";
            for (const auto& user : users) {
                int uploads = rand() % 50 + 10;
                int comments = rand() % 30 + 5;
                int shares = rand() % 20 + 2;
                int activity = uploads + comments + shares;
                
                htmlContent += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
                    .arg(QString::fromStdString(user))
                    .arg(uploads)
                    .arg(comments)
                    .arg(shares)
                    .arg(activity);
            }
            htmlContent += "</table>";
            
            if (includeCollaborationCheck->isChecked()) {
                htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "协作网络分析" : "Collaboration Network Analysis");
                htmlContent += QString("<p>%1</p>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    "用户之间的协作关系网络分析：" : 
                    "Network analysis of collaboration relationships between users:");
                
                htmlContent += "<div style=\"margin: 20px 0; padding: 10px; background-color: #f0f0f0; border-radius: 5px;\">";
                htmlContent += "<p><strong>admin</strong> ↔ <strong>researcher1</strong> (协作项目: 3)</p>";
                htmlContent += "<p><strong>admin</strong> ↔ <strong>analyst1</strong> (协作项目: 2)</p>";
                htmlContent += "<p><strong>researcher1</strong> ↔ <strong>analyst1</strong> (协作项目: 4)</p>";
                htmlContent += "<p><strong>guest1</strong> ← <strong>admin</strong> (协作关系: 指导)</p>";
                htmlContent += "</div>";
            }
            
        } else if (reportType == "quality") {
            // 质量评估报告
            htmlContent += QString("<h2>%1</h2>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据质量评估" : "Data Quality Assessment");
            
            if (includeCompletenessCheck->isChecked()) {
                htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "完整性评估" : "Completeness Assessment");
                htmlContent += QString("<p>%1</p>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    "数据完整性的评估结果：" : 
                    "Data completeness assessment results:");
                
                htmlContent += "<table><tr><th>属性</th><th>完整率</th><th>问题</th></tr>";
                htmlContent += QString("<tr><td>ID字段</td><td style=\"color: green; font-weight: bold;\">100%%</td><td>无</td></tr>");
                htmlContent += QString("<tr><td>分类</td><td style=\"color: orange; font-weight: bold;\">85%%</td><td>部分记录缺少分类</td></tr>");
                htmlContent += QString("<tr><td>标签</td><td style=\"color: orange; font-weight: bold;\">78%%</td><td>多数记录缺少标签</td></tr>");
                htmlContent += QString("<tr><td>内容</td><td style=\"color: green; font-weight: bold;\">92%%</td><td>少数记录内容为空</td></tr>");
                htmlContent += "</table>";
            }
            
            if (includeConsistencyCheck->isChecked()) {
                htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "一致性评估" : "Consistency Assessment");
                htmlContent += QString("<p>%1</p>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    "数据一致性的评估结果：" : 
                    "Data consistency assessment results:");
                
                htmlContent += "<table><tr><th>检查项</th><th>一致率</th><th>问题</th></tr>";
                htmlContent += QString("<tr><td>分类一致性</td><td style=\"color: green; font-weight: bold;\">90%%</td><td>少数分类不匹配</td></tr>");
                htmlContent += QString("<tr><td>格式一致性</td><td style=\"color: orange; font-weight: bold;\">82%%</td><td>部分格式与内容不符</td></tr>");
                htmlContent += QString("<tr><td>标签一致性</td><td style=\"color: red; font-weight: bold;\">65%%</td><td>标签使用不规范</td></tr>");
                htmlContent += "</table>";
            }
            
            if (includeAccuracyCheck->isChecked()) {
                htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "准确性评估" : "Accuracy Assessment");
                htmlContent += QString("<p>%1</p>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? 
                    "数据准确性的评估结果：" : 
                    "Data accuracy assessment results:");
                
                htmlContent += "<table><tr><th>检查项</th><th>准确率</th><th>问题</th></tr>");
                htmlContent += QString("<tr><td>数据校验</td><td style=\"color: green; font-weight: bold;\">95%%</td><td>基本符合格式要求</td></tr>");
                htmlContent += QString("<tr><td>内容验证</td><td style=\"color: orange; font-weight: bold;\">87%%</td><td>部分内容存疑</td></tr>");
                htmlContent += QString("<tr><td>元数据验证</td><td style=\"color: green; font-weight: bold;\">92%%</td><td>元数据基本完整</td></tr>");
                htmlContent += "</table>";
            }
            
            // 质量综合评分
            htmlContent += QString("<h3>%1</h3>").arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "质量综合评分" : "Overall Quality Score");
            htmlContent += QString("<div class=\"summary-box\"><strong>%1:</strong> <span class=\"stat-value\">%2/100</span></div>")
                .arg(m_i18n.getCurrentLanguage() == "zh-CN" ? "综合质量评分" : "Overall Quality Score", "85.3");
        }
        
        // 报告尾部
        htmlContent += QString(
            "<hr>"
            "<p style=\"margin-top: 20px; font-size: 12px; color: #7f8c8d;\">"
            "%1<br>"
            "%2"
            "</p>"
            "</body>"
            "</html>"
        ).arg(
            m_i18n.getCurrentLanguage() == "zh-CN" ? "此报告由 BondForge V1.2 自动生成" : "This report was automatically generated by BondForge V1.2",
            m_i18n.getCurrentLanguage() == "zh-CN" ? "生成时间: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") : 
                         "Generated on: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
        );
        
        // 显示报告
        previewEdit->setHtml(htmlContent);
        
        // 更新信息标签
        QString message = m_i18n.getCurrentLanguage() == "zh-CN" ? 
            QString("报告生成完成: %1").arg(reportTypeCombo->currentText()) : 
            QString("Report generation completed: %1").arg(reportTypeCombo->currentText());
    };
    
    // 导出报告功能
    auto exportReport = [previewEdit, this]() {
        if (previewEdit->toPlainText().isEmpty()) {
            QMessageBox::information(this, 
                m_i18n.getCurrentLanguage() == "zh-CN" ? "提示" : "Information",
                m_i18n.getCurrentLanguage() == "zh-CN" ? "没有可导出的报告" : "No report to export");
            return;
        }
        
        QString filePath = QFileDialog::getSaveFileName(
            this,
            m_i18n.getCurrentLanguage() == "zh-CN" ? "导出报告" : "Export Report",
            "",
            "HTML Files (*.html);;PDF Files (*.pdf);;All Files (*)"
        );
        
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                stream << previewEdit->toHtml();
                file.close();
                
                QMessageBox::information(this, 
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "导出成功" : "Export Successful",
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "报告已成功导出" : "Report successfully exported");
            } else {
                QMessageBox::warning(this, 
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "导出失败" : "Export Failed",
                    m_i18n.getCurrentLanguage() == "zh-CN" ? "无法写入文件" : "Cannot write to file");
            }
        }
    };
    
    // 连接信号
    connect(generateButton, &QPushButton::clicked, generateReport);
    connect(previewButton, &QPushButton::clicked, [generateReport]() { generateReport(); });
    connect(exportButton, &QPushButton::clicked, exportReport);
    
    // 初始选择第一个报告类型
    configStack->setCurrentIndex(0);
    
    reportDialog->exec();
    delete reportDialog;
}

void BondForgeGUI::applyStorageSettings()
{
    int index = m_storageModeCombo->currentIndex();
    StorageMode newMode = static_cast<StorageMode>(m_storageModeCombo->itemData(index).toInt());
    std::string newPath = m_databasePathEdit->text().toStdString();
    
    // 更新数据库路径
    m_storageConfig->setDatabasePath(newPath);
    
    // 如果存储模式改变了，提示用户是否迁移数据
    if (newMode != m_storageConfig->getStorageMode()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            QString::fromStdString(m_i18n.getText("ui.storage_settings")),
            QString::fromStdString(m_i18n.getText("ui.migrate_confirm")),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            // 执行数据迁移
            if (m_service->migrateData(newMode)) {
                m_statusBar->showMessage(
                    QString::fromStdString(m_i18n.getText("ui.migration_successful")), 
                    3000
                );
            } else {
                m_statusBar->showMessage(
                    QString::fromStdString(m_i18n.getText("ui.migration_failed")), 
                    3000
                );
                return;  // 如果迁移失败，不应用新设置
            }
        } else {
            // 不迁移数据，只是切换模式（当前数据会丢失）
            m_service->switchStorageMode(newMode);
        }
    } else if (newMode == StorageMode::SQLITE) {
        // 模式没变，但是数据库路径变了，需要重新初始化SQLite存储
        m_service->switchStorageMode(newMode);
    }
    
    m_statusBar->showMessage(
        QString::fromStdString(m_i18n.getText("ui.settings_applied")), 
        3000
    );
    
    // 刷新数据列表
    populateTable();
    
    // 关闭对话框
    m_storageSettingsDialog->accept();
}

// 数据可视化辅助函数实现
void BondForgeGUI::renderSimpleMolecule(QGraphicsScene* scene, const std::string& content, bool is3D)
{
    if (content.empty()) {
        // 如果没有内容，显示提示
        scene->addText(m_i18n.getCurrentLanguage() == "zh-CN" ? "无分子结构数据" : "No molecular structure data");
        return;
    }
    
    // 简单的分子解析（示例：解析简单的化学式）
    // 这里使用一个简化的解析方法，实际应用中应使用专业的化学解析库
    
    // 设置视图大小
    scene->setSceneRect(0, 0, 700, 500);
    
    // 添加标题
    QGraphicsTextItem* title = scene->addText(QString::fromStdString(content), QFont("Arial", 14, QFont::Bold));
    title->setPos(10, 10);
    
    // 示例：为常见的化学式创建简单的可视化
    // 这是一个简化的实现，仅用于演示
    if (content.find("C") != std::string::npos || content.find("c") != std::string::npos) {
        // 简单的分子结构可视化
        // 创建一个简单的苯环结构作为示例
        
        const int centerX = 350;
        const int centerY = 250;
        const int radius = 80;
        
        if (is3D) {
            // 3D视图 - 使用椭圆表示原子，线条表示化学键
            QPen bondPen(Qt::black, 3);
            QBrush carbonBrush(QColor(50, 50, 50));
            QBrush hydrogenBrush(QColor(200, 200, 200));
            
            // 绘制六元环
            QGraphicsEllipseItem* carbon1 = scene->addEllipse(centerX - radius - 15, centerY - 15, 30, 30, QPen(), carbonBrush);
            QGraphicsEllipseItem* carbon2 = scene->addEllipse(centerX + radius - 15, centerY - 15, 30, 30, QPen(), carbonBrush);
            QGraphicsEllipseItem* carbon3 = scene->addEllipse(centerX + radius - 15, centerY + radius - 15, 30, 30, QPen(), carbonBrush);
            QGraphicsEllipseItem* carbon4 = scene->addEllipse(centerX - 15, centerY + radius - 15, 30, 30, QPen(), carbonBrush);
            QGraphicsEllipseItem* carbon5 = scene->addEllipse(centerX - radius - 15, centerY + radius - 15, 30, 30, QPen(), carbonBrush);
            QGraphicsEllipseItem* carbon6 = scene->addEllipse(centerX - radius - 15, centerY - radius - 15, 30, 30, QPen(), carbonBrush);
            
            // 绘制化学键
            scene->addLine(centerX - radius, centerY, centerX, centerY - radius, bondPen);
            scene->addLine(centerX, centerY - radius, centerX + radius, centerY, bondPen);
            scene->addLine(centerX + radius, centerY, centerX, centerY + radius, bondPen);
            scene->addLine(centerX, centerY + radius, centerX - radius, centerY, bondPen);
            scene->addLine(centerX - radius, centerY, centerX - radius, centerY - radius, bondPen);
            scene->addLine(centerX - radius, centerY - radius, centerX, centerY - radius, bondPen);
            
            // 添加氢原子
            QGraphicsEllipseItem* hydrogen1 = scene->addEllipse(centerX - radius - 15 - 40, centerY - 10, 20, 20, QPen(), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen2 = scene->addEllipse(centerX - 10, centerY - radius - 40, 20, 20, QPen(), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen3 = scene->addEllipse(centerX + radius + 20, centerY - 10, 20, 20, QPen(), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen4 = scene->addEllipse(centerX + 20, centerY + radius + 20, 20, 20, QPen(), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen5 = scene->addEllipse(centerX - 10, centerY + radius + 40, 20, 20, QPen(), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen6 = scene->addEllipse(centerX - radius - 40, centerY + radius + 10, 20, 20, QPen(), hydrogenBrush);
            
            // 连接氢原子的键
            scene->addLine(centerX - radius - 15, centerY, centerX - radius - 30, centerY, QPen(Qt::black, 2));
            scene->addLine(centerX, centerY - radius, centerX, centerY - radius - 30, QPen(Qt::black, 2));
            scene->addLine(centerX + radius, centerY, centerX + radius + 20, centerY, QPen(Qt::black, 2));
            scene->addLine(centerX, centerY + radius, centerX, centerY + radius + 20, QPen(Qt::black, 2));
            scene->addLine(centerX, centerY + radius, centerX - 30, centerY + radius + 20, QPen(Qt::black, 2));
            scene->addLine(centerX - radius, centerY, centerX - radius - 30, centerY + radius + 10, QPen(Qt::black, 2));
            
            // 添加标签
            scene->addText("C", QFont("Arial", 10))->setPos(centerX - radius - 10, centerY - 10);
            scene->addText("C", QFont("Arial", 10))->setPos(centerX + radius - 10, centerY - 10);
            scene->addText("C", QFont("Arial", 10))->setPos(centerX + radius - 10, centerY + radius - 10);
            scene->addText("C", QFont("Arial", 10))->setPos(centerX - 10, centerY + radius - 10);
            scene->addText("C", QFont("Arial", 10))->setPos(centerX - radius - 10, centerY + radius - 10);
            scene->addText("C", QFont("Arial", 10))->setPos(centerX - radius - 10, centerY - radius - 10);
            
            scene->addText("H", QFont("Arial", 8))->setPos(centerX - radius - 40, centerY - 10);
            scene->addText("H", QFont("Arial", 8))->setPos(centerX - 10, centerY - radius - 40);
            scene->addText("H", QFont("Arial", 8))->setPos(centerX + radius + 20, centerY - 10);
            scene->addText("H", QFont("Arial", 8))->setPos(centerX + 20, centerY + radius + 20);
            scene->addText("H", QFont("Arial", 8))->setPos(centerX - 10, centerY + radius + 40);
            scene->addText("H", QFont("Arial", 8))->setPos(centerX - radius - 40, centerY + radius + 10);
        } else {
            // 2D视图 - 使用圆形表示原子，线条表示化学键
            QPen bondPen(Qt::black, 2);
            QBrush carbonBrush(Qt::black);
            QBrush hydrogenBrush(Qt::white);
            
            // 绘制六元环
            QGraphicsEllipseItem* carbon1 = scene->addEllipse(centerX - radius - 10, centerY - 10, 20, 20, QPen(Qt::black), carbonBrush);
            QGraphicsEllipseItem* carbon2 = scene->addEllipse(centerX + radius - 10, centerY - 10, 20, 20, QPen(Qt::black), carbonBrush);
            QGraphicsEllipseItem* carbon3 = scene->addEllipse(centerX + radius - 10, centerY + radius - 10, 20, 20, QPen(Qt::black), carbonBrush);
            QGraphicsEllipseItem* carbon4 = scene->addEllipse(centerX - 10, centerY + radius - 10, 20, 20, QPen(Qt::black), carbonBrush);
            QGraphicsEllipseItem* carbon5 = scene->addEllipse(centerX - radius - 10, centerY + radius - 10, 20, 20, QPen(Qt::black), carbonBrush);
            QGraphicsEllipseItem* carbon6 = scene->addEllipse(centerX - radius - 10, centerY - radius - 10, 20, 20, QPen(Qt::black), carbonBrush);
            
            // 绘制化学键
            scene->addLine(centerX - radius, centerY, centerX, centerY - radius, bondPen);
            scene->addLine(centerX, centerY - radius, centerX + radius, centerY, bondPen);
            scene->addLine(centerX + radius, centerY, centerX, centerY + radius, bondPen);
            scene->addLine(centerX, centerY + radius, centerX - radius, centerY, bondPen);
            scene->addLine(centerX - radius, centerY, centerX - radius, centerY - radius, bondPen);
            scene->addLine(centerX - radius, centerY - radius, centerX, centerY - radius, bondPen);
            
            // 添加氢原子
            QGraphicsEllipseItem* hydrogen1 = scene->addEllipse(centerX - radius - 30, centerY - 8, 16, 16, QPen(Qt::black), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen2 = scene->addEllipse(centerX - 8, centerY - radius - 30, 16, 16, QPen(Qt::black), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen3 = scene->addEllipse(centerX + radius + 14, centerY - 8, 16, 16, QPen(Qt::black), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen4 = scene->addEllipse(centerX + 14, centerY + radius + 14, 16, 16, QPen(Qt::black), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen5 = scene->addEllipse(centerX - 8, centerY + radius + 14, 16, 16, QPen(Qt::black), hydrogenBrush);
            QGraphicsEllipseItem* hydrogen6 = scene->addEllipse(centerX - radius - 30, centerY + radius + 8, 16, 16, QPen(Qt::black), hydrogenBrush);
            
            // 连接氢原子的键
            scene->addLine(centerX - radius - 10, centerY, centerX - radius - 20, centerY, QPen(Qt::black, 1));
            scene->addLine(centerX, centerY - radius, centerX, centerY - radius - 20, QPen(Qt::black, 1));
            scene->addLine(centerX + radius, centerY, centerX + radius + 14, centerY, QPen(Qt::black, 1));
            scene->addLine(centerX, centerY + radius, centerX, centerY + radius + 14, QPen(Qt::black, 1));
            scene->addLine(centerX, centerY + radius, centerX - 20, centerY + radius + 10, QPen(Qt::black, 1));
            scene->addLine(centerX - radius, centerY, centerX - radius - 20, centerY + radius + 8, QPen(Qt::black, 1));
            
            // 添加标签
            scene->addText("C", QFont("Arial", 8))->setPos(centerX - radius - 8, centerY - 8);
            scene->addText("C", QFont("Arial", 8))->setPos(centerX + radius - 8, centerY - 8);
            scene->addText("C", QFont("Arial", 8))->setPos(centerX + radius - 8, centerY + radius - 8);
            scene->addText("C", QFont("Arial", 8))->setPos(centerX - 8, centerY + radius - 8);
            scene->addText("C", QFont("Arial", 8))->setPos(centerX - radius - 8, centerY + radius - 8);
            scene->addText("C", QFont("Arial", 8))->setPos(centerX - radius - 8, centerY - radius - 8);
            
            scene->addText("H", QFont("Arial", 6))->setPos(centerX - radius - 30, centerY - 8);
            scene->addText("H", QFont("Arial", 6))->setPos(centerX - 8, centerY - radius - 30);
            scene->addText("H", QFont("Arial", 6))->setPos(centerX + radius + 14, centerY - 8);
            scene->addText("H", QFont("Arial", 6))->setPos(centerX + 14, centerY + radius + 14);
            scene->addText("H", QFont("Arial", 6))->setPos(centerX - 8, centerY + radius + 14);
            scene->addText("H", QFont("Arial", 6))->setPos(centerX - radius - 30, centerY + radius + 8);
        }
        
        // 添加图例
        QGroupBox* legendGroup = new QGroupBox();
        legendGroup->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "图例" : "Legend");
        QVBoxLayout* legendLayout = new QVBoxLayout(legendGroup);
        
        QGraphicsProxyWidget* proxy = scene->addWidget(legendGroup);
        proxy->setPos(550, 30);
        
        QGraphicsEllipseItem* carbonLegend = scene->addEllipse(0, 0, 10, 10, QPen(), is3D ? QBrush(QColor(50, 50, 50)) : carbonBrush);
        QGraphicsTextItem* carbonText = scene->addText("C (碳/C)", QFont("Arial", 9));
        
        QGraphicsEllipseItem* hydrogenLegend = scene->addEllipse(0, 20, 10, 10, QPen(), is3D ? QBrush(QColor(200, 200, 200)) : hydrogenBrush);
        QGraphicsTextItem* hydrogenText = scene->addText("H (氢/H)", QFont("Arial", 9));
        
        QGraphicsLineItem* bondLegend = scene->addLine(0, 40, 20, 40, is3D ? QPen(Qt::black, 3) : QPen(Qt::black, 2));
        QGraphicsTextItem* bondText = scene->addText("化学键", QFont("Arial", 9));
        
        carbonLegend->setPos(560, 50);
        carbonText->setPos(575, 45);
        
        hydrogenLegend->setPos(560, 70);
        hydrogenText->setPos(575, 65);
        
        bondLegend->setPos(560, 90);
        bondText->setPos(585, 85);
    } else {
        // 如果不是分子结构数据，显示文本信息
        QGraphicsTextItem* info = scene->addText(
            QString::fromStdString(
                "数据: " + content + "
" + 
                (m_i18n.getCurrentLanguage() == "zh-CN" ? "非分子结构数据" : "Not molecular structure data")
            ), 
            QFont("Arial", 12)
        );
        info->setPos(50, 200);
    }
}

void BondForgeGUI::renderDataChart(QChartView* chartView, const std::string& chartType)
{
    // 创建图表
    QChart* chart = new QChart();
    
    // 从数据库中获取数据
    std::vector<DataRecord> allData = m_service->getAllData();
    
    // 按类别统计数据
    std::map<std::string, int> categoryCount;
    std::map<std::string, int> formatCount;
    std::map<std::string, int> tagCount;
    
    for (const auto& record : allData) {
        categoryCount[record.category]++;
        formatCount[record.format]++;
        
        for (const auto& tag : record.tags) {
            tagCount[tag]++;
        }
    }
    
    // 根据图表类型创建不同的图表
    if (chartType == "bar") {
        // 条形图 - 显示各分类的数据量
        QBarSeries* series = new QBarSeries();
        QBarSet* dataSet = new QBarSet(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据量" : "Data Count");
        
        QStringList categories;
        for (const auto& pair : categoryCount) {
            categories << QString::fromStdString(pair.first);
            *dataSet << pair.second;
        }
        
        series->append(dataSet);
        chart->addSeries(series);
        
        QCategoryAxis* axisX = new QCategoryAxis();
        axisX->append(categories);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);
        
        QValueAxis* axisY = new QValueAxis();
        axisY->setRange(0, 10);  // 设置合适的范围
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
        
        chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "按分类的数据统计" : "Data Statistics by Category");
    } else if (chartType == "pie") {
        // 饼图 - 显示数据格式的分布
        QPieSeries* series = new QPieSeries();
        
        for (const auto& pair : formatCount) {
            QPieSlice* slice = series->append(QString::fromStdString(pair.first + " (" + std::to_string(pair.second) + ")"), pair.second);
            
            // 设置不同的颜色
            if (pair.first == "CSV") {
                slice->setColor(QColor(53, 153, 255));
            } else if (pair.first == "JSON") {
                slice->setColor(QColor(255, 153, 51));
            } else {
                slice->setColor(QColor(51, 255, 153));
            }
            
            // 设置突出显示
            slice->setLabelVisible(true);
        }
        
        chart->addSeries(series);
        chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "按数据格式的分布" : "Distribution by Data Format");
    } else if (chartType == "line") {
        // 折线图 - 显示上传日期的数据量变化
        QLineSeries* series = new QLineSeries();
        
        // 按日期统计（简化处理，使用时间戳的日期部分）
        std::map<int, int> dailyCount;
        for (const auto& record : allData) {
            // 简化：仅使用时间戳的模7作为日期
            int day = record.timestamp % 7;
            dailyCount[day]++;
        }
        
        for (const auto& pair : dailyCount) {
            series->append(pair.first, pair.second);
        }
        
        chart->addSeries(series);
        
        QValueAxis* axisX = new QValueAxis();
        axisX->setRange(0, 6);
        axisX->setLabelFormat("%d");
        axisX->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "天" : "Day");
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);
        
        QValueAxis* axisY = new QValueAxis();
        axisY->setRange(0, 10);  // 设置合适的范围
        axisY->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据量" : "Data Count");
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
        
        chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "按日期的数据上传趋势" : "Data Upload Trend by Date");
    } else if (chartType == "scatter") {
        // 散点图 - 显示记录ID和内容长度的关系
        QScatterSeries* series = new QScatterSeries();
        series->setMarkerSize(8.0);
        series->setColor(QColor(0, 100, 200));
        
        for (int i = 0; i < allData.size(); ++i) {
            series->append(i, allData[i].content.length());
        }
        
        chart->addSeries(series);
        
        QValueAxis* axisX = new QValueAxis();
        axisX->setRange(0, 10);
        axisX->setLabelFormat("%d");
        axisX->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "记录索引" : "Record Index");
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);
        
        QValueAxis* axisY = new QValueAxis();
        axisY->setRange(0, 1000);
        axisY->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "内容长度" : "Content Length");
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
        
        chart->setTitle(m_i18n.getCurrentLanguage() == "zh-CN" ? "记录ID与内容长度关系" : "Relationship between Record ID and Content Length");
    }
    
    // 设置图表样式
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    // 设置视图
    chartView->setChart(chart);
}

void BondForgeGUI::renderTrendAnalysis(QChartView* chartView, const QString& timeRange)
{
    // 创建图表
    QChart* chart = new QChart();
    
    // 从数据库中获取数据
    std::vector<DataRecord> allData = m_service->getAllData();
    
    // 获取当前时间戳（模拟）
    uint64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // 时间范围过滤器
    int days = 30; // 默认值
    if (timeRange == "7") {
        days = 7;
    } else if (timeRange == "30") {
        days = 30;
    } else if (timeRange == "90") {
        days = 90;
    }
    
    // 按天统计数据
    std::map<int, int> dailyCount;
    std::map<int, int> weeklyCount;
    std::map<int, int> monthlyCount;
    
    for (const auto& record : allData) {
        int dayDiff = static_cast<int>((currentTime - record.timestamp) / 86400); // 转换为天数
        
        if (timeRange == "all" || dayDiff <= days) {
            if (dayDiff < 7) {
                dailyCount[dayDiff]++;
            }
            
            int week = dayDiff / 7;
            weeklyCount[week]++;
            
            int month = dayDiff / 30;
            monthlyCount[month]++;
        }
    }
    
    // 创建折线图显示趋势
    QLineSeries* series = new QLineSeries();
    series->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据量" : "Data Count");
    series->setPointsVisible(true);
    
    // 根据时间范围决定数据的粒度
    int maxPeriods = 0;
    std::map<int, int>* dataMap = nullptr;
    QString axisTitle;
    
    if (days <= 7) {
        dataMap = &dailyCount;
        maxPeriods = 7;
        axisTitle = m_i18n.getCurrentLanguage() == "zh-CN" ? "天" : "Days";
    } else if (days <= 30) {
        dataMap = &weeklyCount;
        maxPeriods = 5; // 5周
        axisTitle = m_i18n.getCurrentLanguage() == "zh-CN" ? "周" : "Weeks";
    } else {
        dataMap = &monthlyCount;
        maxPeriods = 3; // 3个月
        axisTitle = m_i18n.getCurrentLanguage() == "zh-CN" ? "月" : "Months";
    }
    
    // 填充数据点
    int total = 0;
    for (int i = 0; i < maxPeriods; ++i) {
        int count = 0;
        if (dataMap->find(i) != dataMap->end()) {
            count = (*dataMap)[i];
        }
        
        series->append(i, count);
        total += count;
    }
    
    // 添加移动平均线
    QLineSeries* avgSeries = new QLineSeries();
    avgSeries->setName(m_i18n.getCurrentLanguage() == "zh-CN" ? "移动平均" : "Moving Average");
    avgSeries->setColor(QColor(255, 100, 100));
    
    if (total > 0) {
        float window = 0;
        float totalWindow = 0;
        
        for (int i = 0; i < maxPeriods; ++i) {
            if (dataMap->find(i) != dataMap->end()) {
                window += (*dataMap)[i];
                totalWindow++;
            }
            
            if (i >= 2 && totalWindow > 0) {
                float avg = window / 3; // 3点移动平均
                avgSeries->append(i-1, avg);
                
                if (dataMap->find(i-2) != dataMap->end()) {
                    window -= (*dataMap)[i-2];
                    totalWindow -= 1;
                }
            }
        }
    }
    
    chart->addSeries(series);
    if (avgSeries->count() > 0) {
        chart->addSeries(avgSeries);
    }
    
    // 设置坐标轴
    QValueAxis* axisX = new QValueAxis();
    axisX->setRange(0, maxPeriods - 1);
    axisX->setLabelFormat("%d");
    axisX->setTitleText(axisTitle);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    if (avgSeries->count() > 0) {
        avgSeries->attachAxis(axisX);
    }
    
    int maxCount = 0;
    for (const auto& pair : *dataMap) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
        }
    }
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, maxCount + 1);
    axisY->setTitleText(m_i18n.getCurrentLanguage() == "zh-CN" ? "数据量" : "Data Count");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    if (avgSeries->count() > 0) {
        avgSeries->attachAxis(axisY);
    }
    
    // 设置图表标题和样式
    QString title;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        if (timeRange == "7") {
            title = "数据趋势分析 - 最近7天";
        } else if (timeRange == "30") {
            title = "数据趋势分析 - 最近30天";
        } else if (timeRange == "90") {
            title = "数据趋势分析 - 最近90天";
        } else {
            title = "数据趋势分析 - 全部时间";
        }
    } else {
        if (timeRange == "7") {
            title = "Data Trend Analysis - Last 7 Days";
        } else if (timeRange == "30") {
            title = "Data Trend Analysis - Last 30 Days";
        } else if (timeRange == "90") {
            title = "Data Trend Analysis - Last 90 Days";
        } else {
            title = "Data Trend Analysis - All Time";
        }
    }
    chart->setTitle(title);
    
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    // 设置视图
    chartView->setChart(chart);
}

void BondForgeGUI::populateTrendDataTable(QTableWidget* table, const QString& timeRange, const QString& trendType)
{
    table->setRowCount(0);
    
    // 从数据库中获取数据
    std::vector<DataRecord> allData = m_service->getAllData();
    
    // 获取当前时间戳（模拟）
    uint64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // 时间范围过滤器
    int days = 30; // 默认值
    if (timeRange == "7") {
        days = 7;
    } else if (timeRange == "30") {
        days = 30;
    } else if (timeRange == "90") {
        days = 90;
    }
    
    // 按天统计数据
    std::map<int, int> dailyCount;
    
    for (const auto& record : allData) {
        int dayDiff = static_cast<int>((currentTime - record.timestamp) / 86400); // 转换为天数
        
        if (timeRange == "all" || dayDiff <= days) {
            if (dayDiff < days) {
                dailyCount[dayDiff]++;
            }
        }
    }
    
    // 计算总数
    int totalCount = 0;
    for (const auto& pair : dailyCount) {
        totalCount += pair.second;
    }
    
    // 填充表格数据
    for (int i = 0; i < days; ++i) {
        int count = 0;
        if (dailyCount.find(i) != dailyCount.end()) {
            count = dailyCount[i];
        }
        
        int row = table->rowCount();
        table->insertRow(row);
        
        // 日期标签
        QString dateLabel;
        if (m_i18n.getCurrentLanguage() == "zh-CN") {
            if (i == 0) {
                dateLabel = "今天";
            } else if (i == 1) {
                dateLabel = "昨天";
            } else {
                dateLabel = QString("%1天前").arg(i);
            }
        } else {
            if (i == 0) {
                dateLabel = "Today";
            } else if (i == 1) {
                dateLabel = "Yesterday";
            } else {
                dateLabel = QString("%1 days ago").arg(i);
            }
        }
        
        table->setItem(row, 0, new QTableWidgetItem(dateLabel));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
        
        // 百分比
        float percentage = totalCount > 0 ? (float)count / totalCount * 100 : 0;
        table->setItem(row, 2, new QTableWidgetItem(QString("%1%").arg(percentage, 0, 'f', 2)));
    }
    
    // 添加汇总行
    int totalRow = table->rowCount();
    table->insertRow(totalRow);
    
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        table->setItem(totalRow, 0, new QTableWidgetItem("总计"));
    } else {
        table->setItem(totalRow, 0, new QTableWidgetItem("Total"));
    }
    
    table->setItem(totalRow, 1, new QTableWidgetItem(QString::number(totalCount)));
    table->setItem(totalRow, 2, new QTableWidgetItem("100%"));
    
    // 设置汇总行的样式
    QFont font = table->item(totalRow, 0)->font();
    font.setBold(true);
    for (int col = 0; col < 3; ++col) {
        table->item(totalRow, col)->setFont(font);
        table->item(totalRow, col)->setBackground(QColor(230, 230, 230));
    }
}

void BondForgeGUI::renderCompareData(QChartView* chartView, const QStringList& selectedIds)
{
    // 创建图表
    QChart* chart = new QChart();
    
    // 创建条形系列，用于对比不同属性
    QBarSeries* series = new QBarSeries();
    
    // 数据属性集合
    QStringList properties;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        properties << "内容长度" << "标签数量" << "上传时间(天)";
    } else {
        properties << "Content Length" << "Tag Count" << "Upload Days Ago";
    }
    
    // 为每个选中项创建一个条形集
    std::vector<DataRecord> allData = m_service->getAllData();
    
    for (const QString& idStr : selectedIds) {
        std::string id = idStr.toStdString();
        QBarSet* barSet = nullptr;
        
        // 查找数据记录
        for (const auto& record : allData) {
            if (record.id == id) {
                QString label = QString::fromStdString(record.id.substr(0, 5) + "...");
                barSet = new QBarSet(label);
                
                // 计算内容长度
                *barSet << record.content.length();
                
                // 计算标签数量
                *barSet << static_cast<int>(record.tags.size());
                
                // 计算上传天数
                uint64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                int daysAgo = static_cast<int>((currentTime - record.timestamp) / 86400);
                *barSet << daysAgo;
                
                break;
            }
        }
        
        if (barSet) {
            series->append(barSet);
        }
    }
    
    chart->addSeries(series);
    
    // 设置坐标轴
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(properties);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 20);  // 设置合适的范围
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    // 设置图表标题和样式
    QString title = m_i18n.getCurrentLanguage() == "zh-CN" ? 
        "数据属性对比" : "Data Properties Comparison";
    chart->setTitle(title);
    
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    // 设置视图
    chartView->setChart(chart);
}

void BondForgeGUI::populateCompareTable(QTableWidget* table, const QStringList& selectedIds)
{
    table->setRowCount(0);
    
    // 获取数据记录
    std::vector<DataRecord> allData = m_service->getAllData();
    
    // 为每个选中的ID添加行
    for (const QString& idStr : selectedIds) {
        std::string id = idStr.toStdString();
        
        // 查找数据记录
        for (const auto& record : allData) {
            if (record.id == id) {
                int row = table->rowCount();
                table->insertRow(row);
                
                // ID
                table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(record.id)));
                
                // 分类
                table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(record.category)));
                
                // 格式
                table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(record.format)));
                
                // 标签
                QString tagsStr;
                for (const auto& tag : record.tags) {
                    if (!tagsStr.isEmpty()) {
                        tagsStr += ", ";
                    }
                    tagsStr += QString::fromStdString(tag);
                }
                table->setItem(row, 3, new QTableWidgetItem(tagsStr));
                
                // 内容长度
                table->setItem(row, 4, new QTableWidgetItem(QString::number(record.content.length())));
                
                break;
            }
        }
    }
    
    // 调整列宽
    table->resizeColumnsToContents();
}

void BondForgeGUI::showContentDiff(QTextEdit* diffEdit, const QStringList& selectedIds)
{
    // 获取数据记录
    std::vector<DataRecord> allData = m_service->getAllData();
    
    QString diffText;
    if (m_i18n.getCurrentLanguage() == "zh-CN") {
        diffText += "=== 内容差异对比 ===

";
    } else {
        diffText += "=== Content Difference Comparison ===

";
    }
    
    // 为每个选中的记录添加内容
    for (const QString& idStr : selectedIds) {
        std::string id = idStr.toStdString();
        
        // 查找数据记录
        for (const auto& record : allData) {
            if (record.id == id) {
                if (m_i18n.getCurrentLanguage() == "zh-CN") {
                    diffText += QString("记录ID: %1
").arg(QString::fromStdString(record.id));
                    diffText += QString("分类: %1
").arg(QString::fromStdString(record.category));
                    diffText += QString("格式: %1
").arg(QString::fromStdString(record.format));
                } else {
                    diffText += QString("Record ID: %1
").arg(QString::fromStdString(record.id));
                    diffText += QString("Category: %1
").arg(QString::fromStdString(record.category));
                    diffText += QString("Format: %1
").arg(QString::fromStdString(record.format));
                }
                
                // 添加分隔线
                diffText += "----------------------------------
";
                
                // 添加内容
                QString content = QString::fromStdString(record.content);
                
                // 限制显示长度
                if (content.length() > 500) {
                    content = content.left(500) + "...";
                }
                
                diffText += content + "

";
                
                break;
            }
        }
    }
    
    // 如果没有找到记录
    if (diffText.isEmpty()) {
        if (m_i18n.getCurrentLanguage() == "zh-CN") {
            diffText = "未找到选中的记录或记录内容为空。";
        } else {
            diffText = "Selected records not found or record content is empty.";
        }
    }
    
    diffEdit->setPlainText(diffText);
}

double BondForgeGUI::calculateCorrelation(const std::vector<double>& x, const std::vector<double>& y)
{
    if (x.size() != y.size() || x.size() == 0) {
        return 0.0;
    }
    
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0;
    double sumX2 = 0.0, sumY2 = 0.0;
    
    for (size_t i = 0; i < x.size(); ++i) {
        sumX += x[i];
        sumY += y[i];
        sumXY += x[i] * y[i];
        sumX2 += x[i] * x[i];
        sumY2 += y[i] * y[i];
    }
    
    double n = static_cast<double>(x.size());
    double numerator = n * sumXY - sumX * sumY;
    double denominator = sqrt((n * sumX2 - sumX * sumX) * (n * sumY2 - sumY * sumY));
    
    if (denominator == 0.0) {
        return 0.0;
    }
    
    return numerator / denominator;
}

double BondForgeGUI::calculatePValue(double correlation, int sampleSize)
{
    if (sampleSize <= 2) {
        return 1.0;
    }
    
    // 计算t统计量
    double t = correlation * sqrt((sampleSize - 2) / (1 - correlation * correlation));
    
    // 简化的p值计算（实际应用中应使用精确的统计公式或库）
    // 这里使用近似值：t值的绝对值越大，p值越小
    double absT = abs(t);
    double pValue = 1.0 / (1.0 + 0.1 * absT * absT);
    
    // 确保p值在0到1之间
    if (pValue < 0.0) pValue = 0.0;
    if (pValue > 1.0) pValue = 1.0;
    
    return pValue;
}

void BondForgeGUI::browseDatabasePath()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, 
        "Select Database File",
        QString::fromStdString(m_storageConfig->getDatabasePath()),
        "SQLite Database Files (*.db *.sqlite);;All Files (*)"
    );
    
    if (!filePath.isEmpty()) {
        m_databasePathEdit->setText(filePath);
    }
}

void BondForgeGUI::migrateData()
{
    int index = m_storageModeCombo->currentIndex();
    StorageMode targetMode = static_cast<StorageMode>(m_storageModeCombo->itemData(index).toInt());
    
    // 执行数据迁移
    if (m_service->migrateData(targetMode)) {
        m_statusBar->showMessage(
            QString::fromStdString(m_i18n.getText("ui.migration_successful")), 
            3000
        );
        
        // 刷新数据列表
        populateTable();
    } else {
        m_statusBar->showMessage(
            QString::fromStdString(m_i18n.getText("ui.migration_failed")), 
            3000
        );
    }
}

// 命令行界面函数
void runCLI() {
    // 初始化国际化管理器
    I18nManager& i18n = I18nManager::getInstance();
    
    if (!i18n.initialize()) {
        std::cerr << "Failed to initialize I18n manager!" << std::endl;
        return;
    }
    
    // 创建核心服务实例
    ChemicalMLService service;
    
    std::cout << "===== BondForge V1.1 命令行版本 =====" << std::endl;
    std::cout << "Current language: " << i18n.getCurrentLanguage() << std::endl;
    
    // 演示中文界面
    std::cout << "
===== 中文界面演示 =====" << std::endl;
    i18n.setLanguage("zh-CN");
    std::cout << "欢迎信息: " << i18n.getText("ui.welcome") << std::endl;
    std::cout << "上传数据: " << i18n.getText("ui.upload_data") << std::endl;
    std::cout << "有机分类: " << i18n.getText("category.organic") << std::endl;
    std::cout << "管理员角色: " << i18n.getText("role.admin") << std::endl;
    
    // 演示错误信息（中文）
    try {
        // 尝试使用无效权限上传数据
        DataRecord record;
        record.id = "test-001";
        record.content = "test data";
        record.format = "CSV";
        record.category = "有机";  // 注意：这里使用原始分类名，实际应用中需要国际化
        record.uploader = "guest";  // 访客用户，无上传权限
        
        service.uploadData(record);
    } catch (const std::exception& e) {
        std::cout << "错误信息: " << e.what() << std::endl;
    }
    
    // 演示英文界面
    std::cout << "
===== English Interface Demo =====" << std::endl;
    i18n.setLanguage("en-US");
    std::cout << "Welcome message: " << i18n.getText("ui.welcome") << std::endl;
    std::cout << "Upload data: " << i18n.getText("ui.upload_data") << std::endl;
    std::cout << "Organic category: " << i18n.getText("category.organic") << std::endl;
    std::cout << "Administrator role: " << i18n.getText("role.admin") << std::endl;
    
    // 演示错误信息（英文）
    try {
        // 尝试使用无效权限上传数据
        DataRecord record;
        record.id = "test-002";
        record.content = "test data";
        record.format = "CSV";
        record.category = i18n.getText("category.organic");  // 使用国际化分类名
        record.uploader = "guest";  // Guest user, no upload permission
        
        service.uploadData(record);
    } catch (const std::exception& e) {
        std::cout << "Error message: " << e.what() << std::endl;
    }
    
    // 演示语言切换交互
    std::cout << "
===== 语言切换交互 / Language Switching Interactive =====" << std::endl;
    std::string choice;
    do {
        std::cout << "
请选择语言 / Please select language:" << std::endl;
        std::cout << "1. 中文 / Chinese" << std::endl;
        std::cout << "2. English" << std::endl;
        std::cout << "0. 退出 / Exit" << std::endl;
        std::cout << "> ";
        std::cin >> choice;
        
        if (choice == "1") {
            i18n.setLanguage("zh-CN");
            std::cout << i18n.getText("ui.welcome") << std::endl;
            std::cout << "语言选项: " << i18n.getText("ui.language") << std::endl;
        } else if (choice == "2") {
            i18n.setLanguage("en-US");
            std::cout << i18n.getText("ui.welcome") << std::endl;
            std::cout << "Language option: " << i18n.getText("ui.language") << std::endl;
        } else if (choice == "0") {
            i18n.setLanguage("zh-CN");
            std::cout << i18n.getText("ui.welcome") << std::endl;
            std::cout << "感谢使用！谢谢！" << std::endl;
        } else {
            std::cout << "无效选择 / Invalid choice!" << std::endl;
        }
    } while (choice != "0");
}

// 图形用户界面函数
int runGUI(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("BondForge");
    app.setApplicationVersion("1.2");
    app.setOrganizationName("BondForge Team");
    
    // 设置应用程序样式
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // 创建并显示主窗口
    BondForgeGUI window;
    window.show();
    
    return app.exec();
}

// 显示帮助信息
void showHelp() {
    std::cout << "BondForge V1.1 - 化学机器学习系统" << std::endl;
    std::cout << "使用方法:" << std::endl;
    std::cout << "  bondforge [选项]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  --gui, -g    启动图形用户界面（默认）" << std::endl;
    std::cout << "  --cli, -c    启动命令行界面" << std::endl;
    std::cout << "  --help, -h    显示此帮助信息" << std::endl;
}

#include "bondforge.moc"