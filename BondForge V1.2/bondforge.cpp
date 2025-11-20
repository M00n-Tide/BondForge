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

// 数据结构定义
struct DataRecord { 
    std::string id;                // 数据唯一标识
    std::string content;           // 数据内容（如化学式、分子结构）
    std::string format;            // 数据格式（CSV/JSON/SDF等）
    std::unordered_set<std::string> tags;  // 数据标签集合
    std::string category;          // 数据分类
    std::string uploader;          // 上传用户
    uint64_t timestamp;            // 上传时间戳
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
    UNKNOWN_ERROR         // 未知错误
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
    enum class Role { ADMIN, USER, GUEST };  // 权限角色
private: 
    std::unordered_map<std::string, Role> userRoles;  // 用户名-角色映射
    std::mutex mutex_;  // 线程安全锁
public: 
    // 构造函数：初始化默认用户（admin/guest）
    PermissionManager() { 
        userRoles["admin"] = Role::ADMIN; 
        userRoles["guest"] = Role::GUEST; 
    }

    // 设置用户角色
    void setUserRole(const std::string& username, Role role) { 
        std::lock_guard<std::mutex> lock(mutex_); 
        userRoles[username] = role;
    }

    // 获取用户角色
    Role getUserRole(const std::string& username) { 
        std::lock_guard<std::mutex> lock(mutex_); 
        auto it = userRoles.find(username); 
        return (it != userRoles.end()) ? it->second : Role::GUEST;
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

// 核心服务类
class ChemicalMLService { 
private: 
    std::unordered_map<std::string, DataRecord> dataStore;  // ID-数据映射
    std::unordered_map<std::string, std::unordered_set<std::string>> categoryIndex;  // 分类-ID索引
    std::unordered_map<std::string, std::unordered_set<std::string>> tagIndex;        // 标签-ID索引
    std::mutex mutex_;                // 线程安全锁
    DataQualityChecker qualityChecker;// 质量检测实例
    PermissionManager permissionManager;// 权限管理实例
    I18nManager& i18n;              // 国际化管理器引用
public:
    ChemicalMLService() : i18n(I18nManager::getInstance()) {}  // 构造函数，初始化国际化管理器引用

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

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void updateTexts();
    void populateTable();
    void connectSignals();
    
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
    
    // 业务逻辑
    ChemicalMLService m_service;
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
    if (!permissionManager.canUpload(rawData.uploader)) { 
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
    if (dataStore.find(data.id) != dataStore.end()) { 
        throw std::runtime_error(i18n.getText("error.data_id_exists")); 
    }
    // 5. 存储数据与索引
    dataStore[data.id] = data; 
    categoryIndex[data.category].insert(data.id); 
    for (const auto& tag : data.tags) { 
        tagIndex[tag].insert(data.id); 
    }
    return true;
} 

bool ChemicalMLService::deleteData(const std::string& id, const std::string& username) { 
    std::lock_guard<std::mutex> lock(mutex_); 
    auto it = dataStore.find(id);
    if (it == dataStore.end()) { 
        throw std::runtime_error(i18n.getText("error.data_not_found")); 
    }
    // 权限检查
    if (!permissionManager.canModify(username, it->second)) { 
        throw std::runtime_error(i18n.getText("error.no_deletion_permission")); 
    }
    // 1. 移除分类索引（空分类需删除键）
    const std::string& oldCategory = it->second.category;
    categoryIndex[oldCategory].erase(id);
    if (categoryIndex[oldCategory].empty()) { 
        categoryIndex.erase(oldCategory); 
    }
    // 3. 移除标签索引（空标签需删除键）
    for (const auto& tag : it->second.tags) { 
        auto tagIt = tagIndex.find(tag);
        if (tagIt != tagIndex.end()) { 
            tagIt->second.erase(id); 
            if (tagIt->second.empty()) { 
                tagIndex.erase(tagIt); 
            }
        }
    }
    // 4. 删除数据
    dataStore.erase(it); 
    return true;
}

bool ChemicalMLService::updateData(const DataRecord& newData, const std::string& username) { 
    std::lock_guard<std::mutex> lock(mutex_); 
    // 修正：先声明迭代器再判断（避免作用域错误）
    auto it = dataStore.find(newData.id);
    if (it == dataStore.end()) { 
        throw std::runtime_error(i18n.getText("error.data_not_found")); 
    }
    // 权限检查
    if (!permissionManager.canModify(username, it->second)) {
        throw std::runtime_error(i18n.getText("error.no_edit_permission")); 
    }
    
    // 2. 校验新数据
    if (!qualityChecker.checkFormat(newData.content, newData.format)) { 
        throw std::runtime_error(i18n.getText("error.data_format_validation_failed")); 
    }
    if (!qualityChecker.checkTags(newData.tags)) { 
        throw std::runtime_error(i18n.getText("error.tag_validation_failed"));
    }
    if (!qualityChecker.checkCategory(newData.category)) { 
        throw std::runtime_error(i18n.getText("error.category_validation_failed"));
    }
    
    // 4. 移除旧索引
    const std::string& oldCategory = it->second.category;
    categoryIndex[oldCategory].erase(it->second.id);
    if (categoryIndex[oldCategory].empty()) { 
        categoryIndex.erase(oldCategory); 
    }
    for (const auto& tag : it->second.tags) { 
        auto tagIt = tagIndex.find(tag);
        if (tagIt != tagIndex.end()) { 
            tagIt->second.erase(it->second.id); 
            if (tagIt->second.empty()) { 
                tagIndex.erase(tagIt); 
            }
        }
    }
    // 5. 插入新索引
    categoryIndex[newData.category].insert(newData.id); 
    for (const auto& tag : newData.tags) {
        tagIndex[tag].insert(newData.id); 
    } 
    // 6. 更新数据
    it->second = std::move(newData);
    return true;
}

DataRecord ChemicalMLService::getData(const std::string& id, const std::string& username) { 
    std::lock_guard<std::mutex> lock(mutex_); 
    auto it = dataStore.find(id); 
    if (it == dataStore.end()) { 
        throw std::runtime_error(i18n.getText("error.data_not_found")); 
    }
    if (!permissionManager.canAccess(username, it->second.category)) { 
        throw std::runtime_error(i18n.getText("error.no_access_permission")); 
    }
    return it->second; 
}

std::vector<std::string> ChemicalMLService::listDataByCategory(const std::string& category, const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_); 
    if (!permissionManager.canAccess(username, category)) { 
        throw std::runtime_error(i18n.getText("error.no_access_permission")); 
    }
    std::vector<std::string> results; 
    auto it = categoryIndex.find(category);
    if (it != categoryIndex.end()) { 
        for (const auto& id : it->second) { 
            results.push_back(id); 
        }
    }
    return results;
}

std::vector<std::string> ChemicalMLService::listDataByTag(const std::string& tag, const std::string& username) { 
    std::lock_guard<std::mutex> lock(mutex_); 
    std::vector<std::string> results; 
    auto tagIt = tagIndex.find(tag);
    if (tagIt != tagIndex.end()) { 
        for (const auto& id : tagIt->second) { 
            auto dataIt = dataStore.find(id); 
            if (dataIt != dataStore.end() && permissionManager.canAccess(username, dataIt->second.category)) { 
                results.push_back(id); 
            }
        }
    }
    return results;
}

bool ChemicalMLService::setUserRole(const std::string& adminUser, const std::string& username, PermissionManager::Role role) { 
    if (permissionManager.getUserRole(adminUser) != PermissionManager::Role::ADMIN) { 
        throw std::runtime_error(i18n.getText("error.not_admin")); 
    }
    permissionManager.setUserRole(username, role); 
    return true;
}

// GUI实现
BondForgeGUI::BondForgeGUI(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_i18n(I18nManager::getInstance())
{
    // 初始化国际化管理器
    if (!m_i18n.initialize()) {
        QMessageBox::critical(this, "Error", "Failed to initialize I18n manager!");
    }
    
    // 设置默认语言为中文
    m_i18n.setLanguage("zh-CN");
    
    // 设置窗口属性
    setWindowTitle("BondForge V1.1");
    resize(1024, 768);
    
    // 设置UI
    setupUI();
    setupMenuBar();
    setupStatusBar();
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
    
    // 添加标签页
    m_tabWidget->addTab(m_dataTab, "Data Management");
    m_tabWidget->addTab(m_uploadTab, "Data Upload");
    
    mainLayout->addWidget(m_tabWidget);
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
}

void BondForgeGUI::updateTexts()
{
    // 更新窗口标题
    setWindowTitle(QString::fromStdString(m_i18n.getText("ui.welcome")) + " - BondForge V1.1");
    
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
        }
    }
}

void BondForgeGUI::populateTable()
{
    // 示例数据，实际应该从服务中获取
    m_dataTable->setRowCount(0);
    
    // 这里只是示例，实际应该从服务中获取数据
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
        if (m_service.uploadData(record)) {
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
        if (m_service.deleteData(m_selectedId.toStdString(), "user")) {
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
        DataRecord record = m_service.getData(m_selectedId.toStdString(), "user");
        
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
    QMessageBox::about(this, "About BondForge V1.1", 
                      "BondForge V1.1\n\n"
                      "A Chemical Machine Learning System\n\n"
                      "Current Language: " + lang + "\n\n"
                      "© 2023 BondForge Team");
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
    std::cout << "\n===== 中文界面演示 =====" << std::endl;
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
    std::cout << "\n===== English Interface Demo =====" << std::endl;
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
    std::cout << "\n===== 语言切换交互 / Language Switching Interactive =====" << std::endl;
    std::string choice;
    do {
        std::cout << "\n请选择语言 / Please select language:" << std::endl;
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
    app.setApplicationVersion("1.1");
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