#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>
#include <QPixmap>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStyleHints>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "ui/MainWindow.h"
#include "ui/UpdateWidget.h"
#include "core/data/DataService.h"
#include "services/NetworkService.h"
#include "services/DatabaseService.h"
#include "services/UpdateService.h"
#include "services/UpdateScheduler.h"
#include "services/DataSourceManager.h"
#include "utils/ConfigManager.h"
#include "utils/Logger.h"
#include "core/permissions/PermissionManager.h"
#include "core/plugins/PluginManager.h"

// 前向声明函数
void setupApplicationDirectories();
void initializeServices();
void setupApplicationStyle();
void setupTranslations(QApplication &app);
void showSplashScreen(QSplashScreen *&splash);
bool checkSystemRequirements();
void loadConfiguration();
void parseCommandLineOptions(QApplication &app, bool &showUpdateManager, bool &autoCheckUpdates);
int runUpdateManager();
void handleAutoUpdateCheck();

// 全局服务实例
std::shared_ptr<Services::UpdateService> g_updateService;
std::shared_ptr<Services::UpdateScheduler> g_updateScheduler;
std::shared_ptr<Services::DataSourceManager> g_dataSourceManager;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("BondForge");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("BondForge Team");
    app.setOrganizationDomain("bondforge.org");
    
    // 解析命令行选项
    bool showUpdateManager = false;
    bool autoCheckUpdates = false;
    parseCommandLineOptions(app, showUpdateManager, autoCheckUpdates);
    
    // 如果只是运行更新管理器，直接运行并退出
    if (showUpdateManager) {
        return runUpdateManager();
    }
    
    // 检查系统要求
    if (!checkSystemRequirements()) {
        QMessageBox::critical(nullptr, "System Requirements", 
                             "Your system does not meet the minimum requirements to run BondForge.");
        return 1;
    }
    
    // 显示启动画面
    QSplashScreen *splash = nullptr;
    showSplashScreen(splash);
    
    // 设置应用程序目录
    setupApplicationDirectories();
    
    // 加载配置
    loadConfiguration();
    
    // 初始化服务
    initializeServices();
    
    // 处理自动更新检查
    if (autoCheckUpdates) {
        handleAutoUpdateCheck();
    }
    
    // 设置应用程序样式
    setupApplicationStyle();
    
    // 设置翻译
    setupTranslations(app);
    
    // 创建并显示主窗口
    MainWindow window;
    if (splash) {
        QTimer::singleShot(2000, &window, &QWidget::show);
        QTimer::singleShot(2000, splash, &QSplashScreen::close);
    } else {
        window.show();
    }
    
    // 运行应用程序
    return app.exec();
}

void setupApplicationDirectories()
{
    // 创建应用程序数据目录
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir appDataDir(appDataPath);
    if (!appDataDir.exists()) {
        appDataDir.mkpath(".");
    }
    
    // 创建子目录
    QStringList subDirs = {"logs", "data", "backup", "cache", "downloads", "plugins", "config"};
    for (const QString &dir : subDirs) {
        QDir subDir(appDataPath + "/" + dir);
        if (!subDir.exists()) {
            subDir.mkpath(".");
        }
    }
}

void initializeServices()
{
    // 初始化日志系统
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs/bondforge.log";
    Utils::Logger::initialize(logPath);
    
    // 初始化配置管理器
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config/bondforge.json";
    Utils::ConfigManager::initialize(configPath);
    
    // 初始化数据库服务
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/bondforge.db";
    Services::DatabaseService::initialize(dbPath);
    
    // 初始化网络服务
    Services::NetworkService::initialize();
    
    // 初始化权限管理器
    Core::Permissions::PermissionManager::initialize();
    
    // 初始化插件管理器
    QString pluginPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins";
    Core::Plugins::PluginManager::initialize(pluginPath);
    
    // 初始化数据源管理器
    g_dataSourceManager = std::make_shared<Services::DataSourceManager>();
    g_dataSourceManager->initialize();
    
    // 初始化更新服务
    g_updateService = std::make_shared<Services::UpdateService>();
    g_updateService->initialize();
    
    // 初始化更新调度器
    g_updateScheduler = std::make_shared<Services::UpdateScheduler>();
    g_updateScheduler->initialize();
}

void setupApplicationStyle()
{
    // 设置应用程序样式
    QStyle *style = QStyleFactory::create("Fusion");
    if (style) {
        QApplication::setStyle(style);
    }
    
    // 设置暗色主题（如果配置中指定）
    if (Utils::ConfigManager::instance().value("app/theme", "light") == "dark") {
        // 设置暗色主题的调色板
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        
        QApplication::setPalette(darkPalette);
    }
}

void setupTranslations(QApplication &app)
{
    // 获取用户语言设置
    QString language = Utils::ConfigManager::instance().value("app/language", "en").toString();
    
    // 加载应用程序翻译
    QTranslator *appTranslator = new QTranslator(&app);
    QString translationPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config/i18n";
    
    if (language == "zh" || language == "zh-CN") {
        appTranslator->load("zh-CN", translationPath);
        app.installTranslator(appTranslator);
    } else if (language == "en" || language == "en-US") {
        appTranslator->load("en-US", translationPath);
        app.installTranslator(appTranslator);
    }
}

void showSplashScreen(QSplashScreen *&splash)
{
    // 创建启动画面
    QPixmap pixmap(800, 400);
    pixmap.fill(Qt::white);
    
    // 在启动画面上绘制内容
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 24, QFont::Bold));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "BondForge V2.0\nProfessional Chemical Machine Learning Platform");
    
    splash = new QSplashScreen(pixmap);
    splash->show();
    
    // 显示进度消息
    splash->showMessage("Loading modules...", Qt::AlignBottom | Qt::AlignCenter, Qt::black);
    QApplication::processEvents();
    
    splash->showMessage("Initializing services...", Qt::AlignBottom | Qt::AlignCenter, Qt::black);
    QApplication::processEvents();
    
    splash->showMessage("Ready to start...", Qt::AlignBottom | Qt::AlignCenter, Qt::black);
    QApplication::processEvents();
}

bool checkSystemRequirements()
{
    // 检查操作系统版本
#if defined(Q_OS_WIN)
    if (QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows10) {
        return false;
    }
#elif defined(Q_OS_MACOS)
    if (QOperatingSystemVersion::current() < QOperatingSystemVersion::MacOSMojave) {
        return false;
    }
#else
    // Linux - 检查特定的库版本等
    // 简化起见，假设所有现代Linux发行版都符合要求
#endif
    
    // 检查内存
    const quint64 minimumMemory = 4 * 1024 * 1024 * 1024; // 4GB
    quint64 totalMemory = 0;
    
#ifdef Q_OS_WIN
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    totalMemory = status.ullTotalPhys;
#else
    // 非Windows系统的内存检查
    QFile memInfo("/proc/meminfo");
    if (memInfo.open(QIODevice::ReadOnly)) {
        QTextStream stream(&memInfo);
        QString line = stream.readLine();
        if (line.startsWith("MemTotal:")) {
            QStringList parts = line.split(QRegExp("\\s+"));
            if (parts.size() > 1) {
                totalMemory = parts[1].toULongLong() * 1024; // Convert KB to bytes
            }
        }
    }
#endif
    
    if (totalMemory > 0 && totalMemory < minimumMemory) {
        qWarning() << "System memory" << (totalMemory / (1024 * 1024)) << "MB is below recommended minimum of 4096 MB";
        // 不返回false，只警告，因为用户可能仍然希望运行应用程序
    }
    
    return true;
}

void loadConfiguration()
{
    // 检查是否存在配置文件
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config/bondforge.json";
    QFile configFile(configPath);
    
    if (!configFile.exists()) {
        // 如果配置文件不存在，创建默认配置
        QDir configDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config");
        if (!configDir.exists()) {
            configDir.mkpath(".");
        }
        
        // 从资源中复制默认配置
        QFile defaultConfig(":/config/bondforge.json");
        if (defaultConfig.exists()) {
            defaultConfig.copy(configPath);
        }
    }
    
    // 加载更新配置
    QString updateConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config/update_config.json";
    QFile updateConfigFile(updateConfigPath);
    
    if (!updateConfigFile.exists()) {
        // 如果更新配置文件不存在，创建默认配置
        QFile defaultUpdateConfig(":/config/update_config.json");
        if (defaultUpdateConfig.exists()) {
            defaultUpdateConfig.copy(updateConfigPath);
        }
    }
}

void parseCommandLineOptions(QApplication &app, bool &showUpdateManager, bool &autoCheckUpdates)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("BondForge V2.0 - Professional Chemical Machine Learning Platform");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加命令行选项
    QCommandLineOption updateManagerOption("update-manager", 
                                       "Show the update manager interface only");
    parser.addOption(updateManagerOption);
    
    QCommandLineOption checkUpdatesOption("check-updates", 
                                       "Check for updates and exit");
    parser.addOption(checkUpdatesOption);
    
    QCommandLineOption noUpdateCheckOption("no-update-check", 
                                        "Disable automatic update check on startup");
    parser.addOption(noUpdateCheckOption);
    
    QCommandLineOption verboseOption("verbose", "Enable verbose logging");
    parser.addOption(verboseOption);
    
    // 解析命令行
    parser.process(app);
    
    // 设置选项值
    showUpdateManager = parser.isSet(updateManagerOption);
    autoCheckUpdates = parser.isSet(checkUpdatesOption);
    
    // 设置详细日志
    if (parser.isSet(verboseOption)) {
        // 设置日志级别为debug
    }
}

int runUpdateManager()
{
    // 创建更新管理器窗口
    UI::UpdateWidget updateWidget;
    updateWidget.show();
    
    // 运行应用程序事件循环
    return QApplication::exec();
}

void handleAutoUpdateCheck()
{
    // 检查是否启用了自动更新
    QString updateConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config/update_config.json";
    QFile updateConfigFile(updateConfigPath);
    
    if (!updateConfigFile.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(updateConfigFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return;
    }
    
    QJsonObject config = doc.object();
    bool autoUpdateEnabled = config["autoUpdateEnabled"].toBool(true);
    
    if (!autoUpdateEnabled) {
        return;
    }
    
    // 检查是否到了更新时间
    QJsonObject scheduler = config["scheduler"].toObject();
    int intervalHours = scheduler["intervalHours"].toInt(24);
    
    QString lastUpdateStr = config["lastUpdated"].toString();
    QDateTime lastUpdate = QDateTime::fromString(lastUpdateStr, Qt::ISODate);
    QDateTime nextUpdate = lastUpdate.addSecs(intervalHours * 3600);
    
    if (QDateTime::currentDateTime() >= nextUpdate) {
        // 启动后台更新检查
        if (g_updateService) {
            g_updateService->checkForUpdates();
        }
    }
}