#include "SettingsWidget.h"
#include "utils/ConfigManager.h"
#include "utils/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QProgressBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QStandardPaths>
#include <QDir>

namespace UI {

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_configManager(nullptr)
{
    setupUI();
    setupConnections();
    
    // 初始化配置管理器
    m_configManager = Utils::ConfigManager::getInstance();
    
    // 加载当前配置
    loadCurrentSettings();
    
    Utils::Logger::info("SettingsWidget initialized");
}

SettingsWidget::~SettingsWidget()
{
}

void SettingsWidget::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 设置选项卡
    m_settingsTabWidget = new QTabWidget(this);
    
    // 通用设置选项卡
    setupGeneralTab();
    
    // 分子可视化设置选项卡
    setupVisualizationTab();
    
    // 机器学习设置选项卡
    setupMachineLearningTab();
    
    // 协作设置选项卡
    setupCollaborationTab();
    
    // 性能设置选项卡
    setupPerformanceTab();
    
    // 高级设置选项卡
    setupAdvancedTab();
    
    mainLayout->addWidget(m_settingsTabWidget);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* resetButton = new QPushButton(tr("Reset to Defaults"), this);
    connect(resetButton, &QPushButton::clicked, this, &SettingsWidget::resetToDefaults);
    buttonLayout->addWidget(resetButton);
    
    buttonLayout->addStretch();
    
    QPushButton* applyButton = new QPushButton(tr("Apply"), this);
    connect(applyButton, &QPushButton::clicked, this, &SettingsWidget::applySettings);
    buttonLayout->addWidget(applyButton);
    
    QPushButton* saveButton = new QPushButton(tr("Save"), this);
    connect(saveButton, &QPushButton::clicked, this, &SettingsWidget::saveSettings);
    buttonLayout->addWidget(saveButton);
    
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsWidget::cancelSettings);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void SettingsWidget::setupGeneralTab()
{
    QWidget* generalTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(generalTab);
    
    // 语言设置
    QGroupBox* languageGroup = new QGroupBox(tr("Language"), this);
    QFormLayout* languageLayout = new QFormLayout(languageGroup);
    
    m_languageCombo = new QComboBox(this);
    m_languageCombo->addItem(tr("English"), "en");
    m_languageCombo->addItem(tr("Chinese (Simplified)"), "zh_CN");
    m_languageCombo->addItem(tr("Chinese (Traditional)"), "zh_TW");
    m_languageCombo->addItem(tr("Japanese"), "ja");
    m_languageCombo->addItem(tr("German"), "de");
    m_languageCombo->addItem(tr("French"), "fr");
    languageLayout->addRow(tr("Interface Language:"), m_languageCombo);
    
    layout->addWidget(languageGroup);
    
    // 主题设置
    QGroupBox* themeGroup = new QGroupBox(tr("Theme"), this);
    QFormLayout* themeLayout = new QFormLayout(themeGroup);
    
    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItem(tr("Light"), "light");
    m_themeCombo->addItem(tr("Dark"), "dark");
    m_themeCombo->addItem(tr("System Default"), "system");
    themeLayout->addRow(tr("Application Theme:"), m_themeCombo);
    
    m_useSystemThemeCheck = new QCheckBox(tr("Use System Theme"), this);
    themeLayout->addRow("", m_useSystemThemeCheck);
    
    layout->addWidget(themeGroup);
    
    // 外观设置
    QGroupBox* appearanceGroup = new QGroupBox(tr("Appearance"), this);
    QFormLayout* appearanceLayout = new QFormLayout(appearanceGroup);
    
    m_fontButton = new QPushButton(tr("Select Font..."), this);
    connect(m_fontButton, &QPushButton::clicked, this, &SettingsWidget::selectFont);
    appearanceLayout->addRow(tr("Application Font:"), m_fontButton);
    
    m_fontSizeSpin = new QSpinBox(this);
    m_fontSizeSpin->setRange(8, 24);
    m_fontSizeSpin->setValue(10);
    appearanceLayout->addRow(tr("Font Size:"), m_fontSizeSpin);
    
    layout->addWidget(appearanceGroup);
    
    // 启动设置
    QGroupBox* startupGroup = new QGroupBox(tr("Startup"), this);
    QFormLayout* startupLayout = new QFormLayout(startupGroup);
    
    m_showSplashCheck = new QCheckBox(tr("Show Splash Screen"), this);
    m_showSplashCheck->setChecked(true);
    startupLayout->addRow("", m_showSplashCheck);
    
    m_loadLastProjectCheck = new QCheckBox(tr("Load Last Project on Startup"), this);
    startupLayout->addRow("", m_loadLastProjectCheck);
    
    m_autoSaveCheck = new QCheckBox(tr("Auto-save Projects"), this);
    m_autoSaveCheck->setChecked(true);
    startupLayout->addRow("", m_autoSaveCheck);
    
    m_autoSaveIntervalSpin = new QSpinBox(this);
    m_autoSaveIntervalSpin->setRange(1, 60);
    m_autoSaveIntervalSpin->setValue(5);
    m_autoSaveIntervalSpin->setSuffix(" " + tr("minutes"));
    startupLayout->addRow(tr("Auto-save Interval:"), m_autoSaveIntervalSpin);
    
    layout->addWidget(startupGroup);
    
    layout->addStretch();
    
    m_settingsTabWidget->addTab(generalTab, tr("General"));
}

void SettingsWidget::setupVisualizationTab()
{
    QWidget* visualizationTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(visualizationTab);
    
    // 渲染设置
    QGroupBox* renderingGroup = new QGroupBox(tr("Rendering"), this);
    QFormLayout* renderingLayout = new QFormLayout(renderingGroup);
    
    m_defaultRendererCombo = new QComboBox(this);
    m_defaultRendererCombo->addItem(tr("2D Renderer"), "2d");
    m_defaultRendererCombo->addItem(tr("3D Renderer"), "3d");
    m_defaultRendererCombo->addItem(tr("WebGL Renderer"), "webgl");
    renderingLayout->addRow(tr("Default Renderer:"), m_defaultRendererCombo);
    
    m_antialiasingCheck = new QCheckBox(tr("Enable Antialiasing"), this);
    m_antialiasingCheck->setChecked(true);
    renderingLayout->addRow("", m_antialiasingCheck);
    
    m_vsyncCheck = new QCheckBox(tr("Enable Vertical Sync"), this);
    m_vsyncCheck->setChecked(true);
    renderingLayout->addRow("", m_vsyncCheck);
    
    layout->addWidget(renderingGroup);
    
    // 视图设置
    QGroupBox* viewGroup = new QGroupBox(tr("View"), this);
    QFormLayout* viewLayout = new QFormLayout(viewGroup);
    
    m_defaultViewAngleCombo = new QComboBox(this);
    m_defaultViewAngleCombo->addItem(tr("Top"), "top");
    m_defaultViewAngleCombo->addItem(tr("Front"), "front");
    m_defaultViewAngleCombo->addItem(tr("Side"), "side");
    m_defaultViewAngleCombo->addItem(tr("Isometric"), "isometric");
    viewLayout->addRow(tr("Default View Angle:"), m_defaultViewAngleCombo);
    
    m_rotateSpeedSlider = new QSlider(Qt::Horizontal, this);
    m_rotateSpeedSlider->setRange(1, 10);
    m_rotateSpeedSlider->setValue(5);
    viewLayout->addRow(tr("Rotation Speed:"), m_rotateSpeedSlider);
    
    m_zoomSpeedSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSpeedSlider->setRange(1, 10);
    m_zoomSpeedSlider->setValue(5);
    viewLayout->addRow(tr("Zoom Speed:"), m_zoomSpeedSlider);
    
    layout->addWidget(viewGroup);
    
    // 颜色设置
    QGroupBox* colorGroup = new QGroupBox(tr("Colors"), this);
    QFormLayout* colorLayout = new QFormLayout(colorGroup);
    
    m_defaultColorSchemeCombo = new QComboBox(this);
    m_defaultColorSchemeCombo->addItem(tr("Element"), "element");
    m_defaultColorSchemeCombo->addItem(tr("Chain"), "chain");
    m_defaultColorSchemeCombo->addItem(tr("Secondary Structure"), "secondary_structure");
    m_defaultColorSchemeCombo->addItem(tr("Temperature"), "temperature");
    colorLayout->addRow(tr("Default Color Scheme:"), m_defaultColorSchemeCombo);
    
    m_backgroundColorButton = new QPushButton(tr("Select Color..."), this);
    connect(m_backgroundColorButton, &QPushButton::clicked, this, &SettingsWidget::selectBackgroundColor);
    colorLayout->addRow(tr("Background Color:"), m_backgroundColorButton);
    
    m_selectionColorButton = new QPushButton(tr("Select Color..."), this);
    connect(m_selectionColorButton, &QPushButton::clicked, this, &SettingsWidget::selectSelectionColor);
    colorLayout->addRow(tr("Selection Color:"), m_selectionColorButton);
    
    layout->addWidget(colorGroup);
    
    // 标签设置
    QGroupBox* labelGroup = new QGroupBox(tr("Labels"), this);
    QFormLayout* labelLayout = new QFormLayout(labelGroup);
    
    m_showAtomLabelsCheck = new QCheckBox(tr("Show Atom Labels by Default"), this);
    m_showAtomLabelsCheck->setChecked(true);
    labelLayout->addRow("", m_showAtomLabelsCheck);
    
    m_labelSizeSpin = new QSpinBox(this);
    m_labelSizeSpin->setRange(8, 24);
    m_labelSizeSpin->setValue(12);
    labelLayout->addRow(tr("Label Size:"), m_labelSizeSpin);
    
    m_labelFontButton = new QPushButton(tr("Select Font..."), this);
    connect(m_labelFontButton, &QPushButton::clicked, this, &SettingsWidget::selectLabelFont);
    labelLayout->addRow(tr("Label Font:"), m_labelFontButton);
    
    layout->addWidget(labelGroup);
    
    layout->addStretch();
    
    m_settingsTabWidget->addTab(visualizationTab, tr("Visualization"));
}

void SettingsWidget::setupMachineLearningTab()
{
    QWidget* mlTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(mlTab);
    
    // 计算设置
    QGroupBox* computationGroup = new QGroupBox(tr("Computation"), this);
    QFormLayout* computationLayout = new QFormLayout(computationGroup);
    
    m_useGPUCheck = new QCheckBox(tr("Use GPU for Computation (if available)"), this);
    computationLayout->addRow("", m_useGPUCheck);
    
    m_numThreadsSpin = new QSpinBox(this);
    m_numThreadsSpin->setRange(1, 32);
    m_numThreadsSpin->setValue(4);
    computationLayout->addRow(tr("Number of Threads:"), m_numThreadsSpin);
    
    m_memoryLimitSpin = new QSpinBox(this);
    m_memoryLimitSpin->setRange(512, 16384);
    m_memoryLimitSpin->setValue(2048);
    m_memoryLimitSpin->setSuffix(" MB");
    computationLayout->addRow(tr("Memory Limit:"), m_memoryLimitSpin);
    
    layout->addWidget(computationGroup);
    
    // 模型设置
    QGroupBox* modelGroup = new QGroupBox(tr("Models"), this);
    QFormLayout* modelLayout = new QFormLayout(modelGroup);
    
    m_defaultMLFrameworkCombo = new QComboBox(this);
    m_defaultMLFrameworkCombo->addItem(tr("TensorFlow"), "tensorflow");
    m_defaultMLFrameworkCombo->addItem(tr("PyTorch"), "pytorch");
    m_defaultMLFrameworkCombo->addItem(tr("scikit-learn"), "scikit-learn");
    m_defaultMLFrameworkCombo->addItem(tr("mlpack"), "mlpack");
    modelLayout->addRow(tr("Default ML Framework:"), m_defaultMLFrameworkCombo);
    
    m_autoSaveModelsCheck = new QCheckBox(tr("Auto-save Trained Models"), this);
    m_autoSaveModelsCheck->setChecked(true);
    modelLayout->addRow("", m_autoSaveModelsCheck);
    
    m_modelSavePathEdit = new QLineEdit(this);
    m_modelSavePathEdit->setPlaceholderText(tr("Default path to save models"));
    QPushButton* browseModelPathButton = new QPushButton(tr("Browse..."), this);
    connect(browseModelPathButton, &QPushButton::clicked, this, &SettingsWidget::browseModelSavePath);
    
    QHBoxLayout* modelPathLayout = new QHBoxLayout();
    modelPathLayout->addWidget(m_modelSavePathEdit);
    modelPathLayout->addWidget(browseModelPathButton);
    modelLayout->addRow(tr("Model Save Path:"), modelPathLayout);
    
    layout->addWidget(modelGroup);
    
    // 数据处理设置
    QGroupBox* dataGroup = new QGroupBox(tr("Data Processing"), this);
    QFormLayout* dataLayout = new QFormLayout(dataGroup);
    
    m_defaultDataFormatCombo = new QComboBox(this);
    m_defaultDataFormatCombo->addItem(tr("CSV"), "csv");
    m_defaultDataFormatCombo->addItem(tr("JSON"), "json");
    m_defaultDataFormatCombo->addItem(tr("Excel"), "excel");
    m_defaultDataFormatCombo->addItem(tr("HDF5"), "hdf5");
    dataLayout->addRow(tr("Default Data Format:"), m_defaultDataFormatCombo);
    
    m_cacheDataCheck = new QCheckBox(tr("Cache Preprocessed Data"), this);
    m_cacheDataCheck->setChecked(true);
    dataLayout->addRow("", m_cacheDataCheck);
    
    m_dataCachePathEdit = new QLineEdit(this);
    m_dataCachePathEdit->setPlaceholderText(tr("Path for data cache"));
    QPushButton* browseCachePathButton = new QPushButton(tr("Browse..."), this);
    connect(browseCachePathButton, &QPushButton::clicked, this, &SettingsWidget::browseDataCachePath);
    
    QHBoxLayout* cachePathLayout = new QHBoxLayout();
    cachePathLayout->addWidget(m_dataCachePathEdit);
    cachePathLayout->addWidget(browseCachePathButton);
    dataLayout->addRow(tr("Data Cache Path:"), cachePathLayout);
    
    layout->addWidget(dataGroup);
    
    layout->addStretch();
    
    m_settingsTabWidget->addTab(mlTab, tr("Machine Learning"));
}

void SettingsWidget::setupCollaborationTab()
{
    QWidget* collaborationTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(collaborationTab);
    
    // 账户设置
    QGroupBox* accountGroup = new QGroupBox(tr("Account"), this);
    QFormLayout* accountLayout = new QFormLayout(accountGroup);
    
    m_usernameEdit = new QLineEdit(this);
    accountLayout->addRow(tr("Username:"), m_usernameEdit);
    
    m_emailEdit = new QLineEdit(this);
    accountLayout->addRow(tr("Email:"), m_emailEdit);
    
    m_departmentEdit = new QLineEdit(this);
    accountLayout->addRow(tr("Department:"), m_departmentEdit);
    
    m_roleCombo = new QComboBox(this);
    m_roleCombo->addItem(tr("Student"), "student");
    m_roleCombo->addItem(tr("Researcher"), "researcher");
    m_roleCombo->addItem(tr("Professor"), "professor");
    m_roleCombo->addItem(tr("Data Scientist"), "data_scientist");
    m_roleCombo->addItem(tr("Developer"), "developer");
    accountLayout->addRow(tr("Role:"), m_roleCombo);
    
    layout->addWidget(accountGroup);
    
    // 网络设置
    QGroupBox* networkGroup = new QGroupBox(tr("Network"), this);
    QFormLayout* networkLayout = new QFormLayout(networkGroup);
    
    m_serverUrlEdit = new QLineEdit(this);
    m_serverUrlEdit->setPlaceholderText(tr("e.g., https://api.bondforge.org"));
    networkLayout->addRow(tr("Server URL:"), m_serverUrlEdit);
    
    m_timeoutSpin = new QSpinBox(this);
    m_timeoutSpin->setRange(5, 300);
    m_timeoutSpin->setValue(30);
    m_timeoutSpin->setSuffix(" " + tr("seconds"));
    networkLayout->addRow(tr("Connection Timeout:"), m_timeoutSpin);
    
    m_autoReconnectCheck = new QCheckBox(tr("Auto-reconnect on Connection Loss"), this);
    m_autoReconnectCheck->setChecked(true);
    networkLayout->addRow("", m_autoReconnectCheck);
    
    layout->addWidget(networkGroup);
    
    // 同步设置
    QGroupBox* syncGroup = new QGroupBox(tr("Synchronization"), this);
    QFormLayout* syncLayout = new QFormLayout(syncGroup);
    
    m_autoSyncCheck = new QCheckBox(tr("Auto-sync Shared Data"), this);
    m_autoSyncCheck->setChecked(true);
    syncLayout->addRow("", m_autoSyncCheck);
    
    m_syncIntervalSpin = new QSpinBox(this);
    m_syncIntervalSpin->setRange(5, 60);
    m_syncIntervalSpin->setValue(15);
    m_syncIntervalSpin->setSuffix(" " + tr("minutes"));
    syncLayout->addRow(tr("Sync Interval:"), m_syncIntervalSpin);
    
    m_notifyOnShareCheck = new QCheckBox(tr("Notify on Data Share"), this);
    m_notifyOnShareCheck->setChecked(true);
    syncLayout->addRow("", m_notifyOnShareCheck);
    
    m_notifyOnCommentCheck = new QCheckBox(tr("Notify on Comment"), this);
    m_notifyOnCommentCheck->setChecked(true);
    syncLayout->addRow("", m_notifyOnCommentCheck);
    
    layout->addWidget(syncGroup);
    
    // 隐私设置
    QGroupBox* privacyGroup = new QGroupBox(tr("Privacy"), this);
    QFormLayout* privacyLayout = new QFormLayout(privacyGroup);
    
    m_publicProfileCheck = new QCheckBox(tr("Public Profile"), this);
    privacyLayout->addRow("", m_publicProfileCheck);
    
    mShareAnalyticsCheck = new QCheckBox(tr("Share Anonymous Usage Analytics"), this);
    privacyLayout->addRow("", mShareAnalyticsCheck);
    
    layout->addWidget(privacyGroup);
    
    layout->addStretch();
    
    m_settingsTabWidget->addTab(collaborationTab, tr("Collaboration"));
}

void SettingsWidget::setupPerformanceTab()
{
    QWidget* performanceTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(performanceTab);
    
    // 内存设置
    QGroupBox* memoryGroup = new QGroupBox(tr("Memory"), this);
    QFormLayout* memoryLayout = new QFormLayout(memoryGroup);
    
    m_maxMemoryUsageSlider = new QSlider(Qt::Horizontal, this);
    m_maxMemoryUsageSlider->setRange(10, 90);
    m_maxMemoryUsageSlider->setValue(50);
    m_maxMemoryUsageLabel = new QLabel(tr("50%"), this);
    
    QHBoxLayout* memorySliderLayout = new QHBoxLayout();
    memorySliderLayout->addWidget(m_maxMemoryUsageSlider);
    memorySliderLayout->addWidget(m_maxMemoryUsageLabel);
    memoryLayout->addRow(tr("Max Memory Usage:"), memorySliderLayout);
    
    connect(m_maxMemoryUsageSlider, &QSlider::valueChanged, this, [this](int value) {
        m_maxMemoryUsageLabel->setText(tr("%1%").arg(value));
    });
    
    m_cacheMemoryUsageSpin = new QSpinBox(this);
    m_cacheMemoryUsageSpin->setRange(64, 1024);
    m_cacheMemoryUsageSpin->setValue(256);
    m_cacheMemoryUsageSpin->setSuffix(" MB");
    memoryLayout->addRow(tr("Cache Memory Usage:"), m_cacheMemoryUsageSpin);
    
    layout->addWidget(memoryGroup);
    
    // 渲染设置
    QGroupBox* renderingGroup = new QGroupBox(tr("Rendering"), this);
    QFormLayout* renderingLayout = new QFormLayout(renderingGroup);
    
    m_fpsLimitSpin = new QSpinBox(this);
    m_fpsLimitSpin->setRange(15, 120);
    m_fpsLimitSpin->setValue(60);
    renderingLayout->addRow(tr("FPS Limit:"), m_fpsLimitSpin);
    
    m_maxAtomsSpin = new QSpinBox(this);
    m_maxAtomsSpin->setRange(100, 50000);
    m_maxAtomsSpin->setValue(10000);
    renderingLayout->addRow(tr("Max Atoms to Render:"), m_maxAtomsSpin);
    
    m_maxBondsSpin = new QSpinBox(this);
    m_maxBondsSpin->setRange(200, 100000);
    m_maxBondsSpin->setValue(20000);
    renderingLayout->addRow(tr("Max Bonds to Render:"), m_maxBondsSpin);
    
    layout->addWidget(renderingGroup);
    
    // 并发设置
    QGroupBox* concurrencyGroup = new QGroupBox(tr("Concurrency"), this);
    QFormLayout* concurrencyLayout = new QFormLayout(concurrencyGroup);
    
    m_renderThreadCountSpin = new QSpinBox(this);
    m_renderThreadCountSpin->setRange(1, 16);
    m_renderThreadCountSpin->setValue(4);
    concurrencyLayout->addRow(tr("Render Thread Count:"), m_renderThreadCountSpin);
    
    m_computationThreadCountSpin = new QSpinBox(this);
    m_computationThreadCountSpin->setRange(1, 32);
    m_computationThreadCountSpin->setValue(8);
    concurrencyLayout->addRow(tr("Computation Thread Count:"), m_computationThreadCountSpin);
    
    layout->addWidget(concurrencyGroup);
    
    // 性能监视器
    QGroupBox* monitorGroup = new QGroupBox(tr("Performance Monitor"), this);
    QVBoxLayout* monitorLayout = new QVBoxLayout(monitorGroup);
    
    m_enablePerformanceMonitorCheck = new QCheckBox(tr("Enable Performance Monitor"), this);
    monitorLayout->addWidget(m_enablePerformanceMonitorCheck);
    
    m_showPerformanceOverlayCheck = new QCheckBox(tr("Show Performance Overlay"), this);
    monitorLayout->addWidget(m_showPerformanceOverlayCheck);
    
    m_logPerformanceDataCheck = new QCheckBox(tr("Log Performance Data"), this);
    monitorLayout->addWidget(m_logPerformanceDataCheck);
    
    layout->addWidget(monitorGroup);
    
    layout->addStretch();
    
    m_settingsTabWidget->addTab(performanceTab, tr("Performance"));
}

void SettingsWidget::setupAdvancedTab()
{
    QWidget* advancedTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(advancedTab);
    
    // 高级选项
    QGroupBox* advancedGroup = new QGroupBox(tr("Advanced Options"), this);
    QFormLayout* advancedLayout = new QFormLayout(advancedGroup);
    
    m_enableExperimentalFeaturesCheck = new QCheckBox(tr("Enable Experimental Features"), this);
    advancedLayout->addRow("", m_enableExperimentalFeaturesCheck);
    
    m_enableDeveloperModeCheck = new QCheckBox(tr("Enable Developer Mode"), this);
    advancedLayout->addRow("", m_enableDeveloperModeCheck);
    
    m_enableDebugModeCheck = new QCheckBox(tr("Enable Debug Mode"), this);
    advancedLayout->addRow("", m_enableDebugModeCheck);
    
    layout->addWidget(advancedGroup);
    
    // 日志设置
    QGroupBox* logGroup = new QGroupBox(tr("Logging"), this);
    QFormLayout* logLayout = new QFormLayout(logGroup);
    
    m_logLevelCombo = new QComboBox(this);
    m_logLevelCombo->addItem(tr("Trace"), "trace");
    m_logLevelCombo->addItem(tr("Debug"), "debug");
    m_logLevelCombo->addItem(tr("Info"), "info");
    m_logLevelCombo->addItem(tr("Warning"), "warning");
    m_logLevelCombo->addItem(tr("Error"), "error");
    m_logLevelCombo->addItem(tr("Critical"), "critical");
    m_logLevelCombo->setCurrentIndex(2); // Info
    logLayout->addRow(tr("Log Level:"), m_logLevelCombo);
    
    m_logToFileCheck = new QCheckBox(tr("Log to File"), this);
    m_logToFileCheck->setChecked(true);
    logLayout->addRow("", m_logToFileCheck);
    
    m_logFilePathEdit = new QLineEdit(this);
    m_logFilePathEdit->setPlaceholderText(tr("Path to log file"));
    QPushButton* browseLogPathButton = new QPushButton(tr("Browse..."), this);
    connect(browseLogPathButton, &QPushButton::clicked, this, &SettingsWidget::browseLogFilePath);
    
    QHBoxLayout* logPathLayout = new QHBoxLayout();
    logPathLayout->addWidget(m_logFilePathEdit);
    logPathLayout->addWidget(browseLogPathButton);
    logLayout->addRow(tr("Log File Path:"), logPathLayout);
    
    m_maxLogSizeSpin = new QSpinBox(this);
    m_maxLogSizeSpin->setRange(1, 100);
    m_maxLogSizeSpin->setValue(10);
    m_maxLogSizeSpin->setSuffix(" MB");
    logLayout->addRow(tr("Max Log File Size:"), m_maxLogSizeSpin);
    
    layout->addWidget(logGroup);
    
    // 插件设置
    QGroupBox* pluginGroup = new QGroupBox(tr("Plugins"), this);
    QFormLayout* pluginLayout = new QFormLayout(pluginGroup);
    
    m_enablePluginsCheck = new QCheckBox(tr("Enable Plugins"), this);
    m_enablePluginsCheck->setChecked(true);
    pluginLayout->addRow("", m_enablePluginsCheck);
    
    m_autoLoadPluginsCheck = new QCheckBox(tr("Auto-load Plugins on Startup"), this);
    m_autoLoadPluginsCheck->setChecked(true);
    pluginLayout->addRow("", m_autoLoadPluginsCheck);
    
    m_pluginPathEdit = new QLineEdit(this);
    m_pluginPathEdit->setPlaceholderText(tr("Path to plugins directory"));
    QPushButton* browsePluginPathButton = new QPushButton(tr("Browse..."), this);
    connect(browsePluginPathButton, &QPushButton::clicked, this, &SettingsWidget::browsePluginPath);
    
    QHBoxLayout* pluginPathLayout = new QHBoxLayout();
    pluginPathLayout->addWidget(m_pluginPathEdit);
    pluginPathLayout->addWidget(browsePluginPathButton);
    pluginLayout->addRow(tr("Plugin Path:"), pluginPathLayout);
    
    layout->addWidget(pluginGroup);
    
    // 重置设置
    QGroupBox* resetGroup = new QGroupBox(tr("Reset Settings"), this);
    QVBoxLayout* resetLayout = new QVBoxLayout(resetGroup);
    
    QPushButton* resetAllButton = new QPushButton(tr("Reset All Settings"), this);
    connect(resetAllButton, &QPushButton::clicked, this, &SettingsWidget::resetAllSettings);
    resetLayout->addWidget(resetAllButton);
    
    QPushButton* resetAdvancedButton = new QPushButton(tr("Reset Advanced Settings Only"), this);
    connect(resetAdvancedButton, &QPushButton::clicked, this, &SettingsWidget::resetAdvancedSettings);
    resetLayout->addWidget(resetAdvancedButton);
    
    QPushButton* exportSettingsButton = new QPushButton(tr("Export Settings"), this);
    connect(exportSettingsButton, &QPushButton::clicked, this, &SettingsWidget::exportSettings);
    resetLayout->addWidget(exportSettingsButton);
    
    QPushButton* importSettingsButton = new QPushButton(tr("Import Settings"), this);
    connect(importSettingsButton, &QPushButton::clicked, this, &SettingsWidget::importSettings);
    resetLayout->addWidget(importSettingsButton);
    
    layout->addWidget(resetGroup);
    
    layout->addStretch();
    
    m_settingsTabWidget->addTab(advancedTab, tr("Advanced"));
}

void SettingsWidget::setupConnections()
{
    // 连接其他必要的信号和槽
}

void SettingsWidget::loadCurrentSettings()
{
    if (!m_configManager) {
        return;
    }
    
    // 通用设置
    m_languageCombo->setCurrentIndex(m_languageCombo->findData(m_configManager->getValue("general/language", "en")));
    m_themeCombo->setCurrentIndex(m_themeCombo->findData(m_configManager->getValue("general/theme", "light")));
    m_useSystemThemeCheck->setChecked(m_configManager->getValue("general/use_system_theme", false));
    m_fontSizeSpin->setValue(m_configManager->getValue("general/font_size", 10));
    m_showSplashCheck->setChecked(m_configManager->getValue("general/show_splash", true));
    m_loadLastProjectCheck->setChecked(m_configManager->getValue("general/load_last_project", false));
    m_autoSaveCheck->setChecked(m_configManager->getValue("general/auto_save", true));
    m_autoSaveIntervalSpin->setValue(m_configManager->getValue("general/auto_save_interval", 5));
    
    // 分子可视化设置
    m_defaultRendererCombo->setCurrentIndex(m_defaultRendererCombo->findData(m_configManager->getValue("visualization/default_renderer", "2d")));
    m_antialiasingCheck->setChecked(m_configManager->getValue("visualization/antialiasing", true));
    m_vsyncCheck->setChecked(m_configManager->getValue("visualization/vsync", true));
    m_defaultViewAngleCombo->setCurrentIndex(m_defaultViewAngleCombo->findData(m_configManager->getValue("visualization/default_view_angle", "isometric")));
    m_rotateSpeedSlider->setValue(m_configManager->getValue("visualization/rotate_speed", 5));
    m_zoomSpeedSlider->setValue(m_configManager->getValue("visualization/zoom_speed", 5));
    m_defaultColorSchemeCombo->setCurrentIndex(m_defaultColorSchemeCombo->findData(m_configManager->getValue("visualization/default_color_scheme", "element")));
    m_showAtomLabelsCheck->setChecked(m_configManager->getValue("visualization/show_atom_labels", true));
    m_labelSizeSpin->setValue(m_configManager->getValue("visualization/label_size", 12));
    
    // 机器学习设置
    m_useGPUCheck->setChecked(m_configManager->getValue("ml/use_gpu", false));
    m_numThreadsSpin->setValue(m_configManager->getValue("ml/num_threads", 4));
    m_memoryLimitSpin->setValue(m_configManager->getValue("ml/memory_limit", 2048));
    m_defaultMLFrameworkCombo->setCurrentIndex(m_defaultMLFrameworkCombo->findData(m_configManager->getValue("ml/default_framework", "scikit-learn")));
    m_autoSaveModelsCheck->setChecked(m_configManager->getValue("ml/auto_save_models", true));
    m_modelSavePathEdit->setText(m_configManager->getValue("ml/model_save_path", ""));
    m_defaultDataFormatCombo->setCurrentIndex(m_defaultDataFormatCombo->findData(m_configManager->getValue("ml/default_data_format", "csv")));
    m_cacheDataCheck->setChecked(m_configManager->getValue("ml/cache_data", true));
    m_dataCachePathEdit->setText(m_configManager->getValue("ml/data_cache_path", ""));
    
    // 协作设置
    m_usernameEdit->setText(m_configManager->getValue("collaboration/username", ""));
    m_emailEdit->setText(m_configManager->getValue("collaboration/email", ""));
    m_departmentEdit->setText(m_configManager->getValue("collaboration/department", ""));
    m_roleCombo->setCurrentIndex(m_roleCombo->findData(m_configManager->getValue("collaboration/role", "researcher")));
    m_serverUrlEdit->setText(m_configManager->getValue("collaboration/server_url", ""));
    m_timeoutSpin->setValue(m_configManager->getValue("collaboration/timeout", 30));
    m_autoReconnectCheck->setChecked(m_configManager->getValue("collaboration/auto_reconnect", true));
    m_autoSyncCheck->setChecked(m_configManager->getValue("collaboration/auto_sync", true));
    m_syncIntervalSpin->setValue(m_configManager->getValue("collaboration/sync_interval", 15));
    m_notifyOnShareCheck->setChecked(m_configManager->getValue("collaboration/notify_on_share", true));
    m_notifyOnCommentCheck->setChecked(m_configManager->getValue("collaboration/notify_on_comment", true));
    m_publicProfileCheck->setChecked(m_configManager->getValue("collaboration/public_profile", false));
    mShareAnalyticsCheck->setChecked(m_configManager->getValue("collaboration/share_analytics", false));
    
    // 性能设置
    m_maxMemoryUsageSlider->setValue(m_configManager->getValue("performance/max_memory_usage", 50));
    m_maxMemoryUsageLabel->setText(tr("%1%").arg(m_maxMemoryUsageSlider->value()));
    m_cacheMemoryUsageSpin->setValue(m_configManager->getValue("performance/cache_memory_usage", 256));
    m_fpsLimitSpin->setValue(m_configManager->getValue("performance/fps_limit", 60));
    m_maxAtomsSpin->setValue(m_configManager->getValue("performance/max_atoms", 10000));
    m_maxBondsSpin->setValue(m_configManager->getValue("performance/max_bonds", 20000));
    m_renderThreadCountSpin->setValue(m_configManager->getValue("performance/render_thread_count", 4));
    m_computationThreadCountSpin->setValue(m_configManager->getValue("performance/computation_thread_count", 8));
    m_enablePerformanceMonitorCheck->setChecked(m_configManager->getValue("performance/enable_monitor", false));
    m_showPerformanceOverlayCheck->setChecked(m_configManager->getValue("performance/show_overlay", false));
    m_logPerformanceDataCheck->setChecked(m_configManager->getValue("performance/log_data", false));
    
    // 高级设置
    m_enableExperimentalFeaturesCheck->setChecked(m_configManager->getValue("advanced/enable_experimental", false));
    m_enableDeveloperModeCheck->setChecked(m_configManager->getValue("advanced/enable_developer", false));
    m_enableDebugModeCheck->setChecked(m_configManager->getValue("advanced/enable_debug", false));
    m_logLevelCombo->setCurrentIndex(m_logLevelCombo->findData(m_configManager->getValue("advanced/log_level", "info")));
    m_logToFileCheck->setChecked(m_configManager->getValue("advanced/log_to_file", true));
    m_logFilePathEdit->setText(m_configManager->getValue("advanced/log_file_path", ""));
    m_maxLogSizeSpin->setValue(m_configManager->getValue("advanced/max_log_size", 10));
    m_enablePluginsCheck->setChecked(m_configManager->getValue("advanced/enable_plugins", true));
    m_autoLoadPluginsCheck->setChecked(m_configManager->getValue("advanced/auto_load_plugins", true));
    m_pluginPathEdit->setText(m_configManager->getValue("advanced/plugin_path", ""));
}

void SettingsWidget::saveCurrentSettings()
{
    if (!m_configManager) {
        return;
    }
    
    // 通用设置
    m_configManager->setValue("general/language", m_languageCombo->currentData().toString());
    m_configManager->setValue("general/theme", m_themeCombo->currentData().toString());
    m_configManager->setValue("general/use_system_theme", m_useSystemThemeCheck->isChecked());
    m_configManager->setValue("general/font_size", m_fontSizeSpin->value());
    m_configManager->setValue("general/show_splash", m_showSplashCheck->isChecked());
    m_configManager->setValue("general/load_last_project", m_loadLastProjectCheck->isChecked());
    m_configManager->setValue("general/auto_save", m_autoSaveCheck->isChecked());
    m_configManager->setValue("general/auto_save_interval", m_autoSaveIntervalSpin->value());
    
    // 分子可视化设置
    m_configManager->setValue("visualization/default_renderer", m_defaultRendererCombo->currentData().toString());
    m_configManager->setValue("visualization/antialiasing", m_antialiasingCheck->isChecked());
    m_configManager->setValue("visualization/vsync", m_vsyncCheck->isChecked());
    m_configManager->setValue("visualization/default_view_angle", m_defaultViewAngleCombo->currentData().toString());
    m_configManager->setValue("visualization/rotate_speed", m_rotateSpeedSlider->value());
    m_configManager->setValue("visualization/zoom_speed", m_zoomSpeedSlider->value());
    m_configManager->setValue("visualization/default_color_scheme", m_defaultColorSchemeCombo->currentData().toString());
    m_configManager->setValue("visualization/show_atom_labels", m_showAtomLabelsCheck->isChecked());
    m_configManager->setValue("visualization/label_size", m_labelSizeSpin->value());
    
    // 机器学习设置
    m_configManager->setValue("ml/use_gpu", m_useGPUCheck->isChecked());
    m_configManager->setValue("ml/num_threads", m_numThreadsSpin->value());
    m_configManager->setValue("ml/memory_limit", m_memoryLimitSpin->value());
    m_configManager->setValue("ml/default_framework", m_defaultMLFrameworkCombo->currentData().toString());
    m_configManager->setValue("ml/auto_save_models", m_autoSaveModelsCheck->isChecked());
    m_configManager->setValue("ml/model_save_path", m_modelSavePathEdit->text());
    m_configManager->setValue("ml/default_data_format", m_defaultDataFormatCombo->currentData().toString());
    m_configManager->setValue("ml/cache_data", m_cacheDataCheck->isChecked());
    m_configManager->setValue("ml/data_cache_path", m_dataCachePathEdit->text());
    
    // 协作设置
    m_configManager->setValue("collaboration/username", m_usernameEdit->text());
    m_configManager->setValue("collaboration/email", m_emailEdit->text());
    m_configManager->setValue("collaboration/department", m_departmentEdit->text());
    m_configManager->setValue("collaboration/role", m_roleCombo->currentData().toString());
    m_configManager->setValue("collaboration/server_url", m_serverUrlEdit->text());
    m_configManager->setValue("collaboration/timeout", m_timeoutSpin->value());
    m_configManager->setValue("collaboration/auto_reconnect", m_autoReconnectCheck->isChecked());
    m_configManager->setValue("collaboration/auto_sync", m_autoSyncCheck->isChecked());
    m_configManager->setValue("collaboration/sync_interval", m_syncIntervalSpin->value());
    m_configManager->setValue("collaboration/notify_on_share", m_notifyOnShareCheck->isChecked());
    m_configManager->setValue("collaboration/notify_on_comment", m_notifyOnCommentCheck->isChecked());
    m_configManager->setValue("collaboration/public_profile", m_publicProfileCheck->isChecked());
    m_configManager->setValue("collaboration/share_analytics", mShareAnalyticsCheck->isChecked());
    
    // 性能设置
    m_configManager->setValue("performance/max_memory_usage", m_maxMemoryUsageSlider->value());
    m_configManager->setValue("performance/cache_memory_usage", m_cacheMemoryUsageSpin->value());
    m_configManager->setValue("performance/fps_limit", m_fpsLimitSpin->value());
    m_configManager->setValue("performance/max_atoms", m_maxAtomsSpin->value());
    m_configManager->setValue("performance/max_bonds", m_maxBondsSpin->value());
    m_configManager->setValue("performance/render_thread_count", m_renderThreadCountSpin->value());
    m_configManager->setValue("performance/computation_thread_count", m_computationThreadCountSpin->value());
    m_configManager->setValue("performance/enable_monitor", m_enablePerformanceMonitorCheck->isChecked());
    m_configManager->setValue("performance/show_overlay", m_showPerformanceOverlayCheck->isChecked());
    m_configManager->setValue("performance/log_data", m_logPerformanceDataCheck->isChecked());
    
    // 高级设置
    m_configManager->setValue("advanced/enable_experimental", m_enableExperimentalFeaturesCheck->isChecked());
    m_configManager->setValue("advanced/enable_developer", m_enableDeveloperModeCheck->isChecked());
    m_configManager->setValue("advanced/enable_debug", m_enableDebugModeCheck->isChecked());
    m_configManager->setValue("advanced/log_level", m_logLevelCombo->currentData().toString());
    m_configManager->setValue("advanced/log_to_file", m_logToFileCheck->isChecked());
    m_configManager->setValue("advanced/log_file_path", m_logFilePathEdit->text());
    m_configManager->setValue("advanced/max_log_size", m_maxLogSizeSpin->value());
    m_configManager->setValue("advanced/enable_plugins", m_enablePluginsCheck->isChecked());
    m_configManager->setValue("advanced/auto_load_plugins", m_autoLoadPluginsCheck->isChecked());
    m_configManager->setValue("advanced/plugin_path", m_pluginPathEdit->text());
}

// 槽函数实现
void SettingsWidget::applySettings()
{
    // 应用设置
    saveCurrentSettings();
    
    // 应用主题设置
    QString theme = m_themeCombo->currentData().toString();
    if (theme == "dark") {
        // 应用暗色主题
        QMessageBox::information(this, tr("Settings Applied"), tr("Dark theme will be applied on next restart."));
    } else if (theme == "light") {
        // 应用亮色主题
        QMessageBox::information(this, tr("Settings Applied"), tr("Light theme will be applied on next restart."));
    }
    
    // 应用语言设置
    QString language = m_languageCombo->currentData().toString();
    QMessageBox::information(this, tr("Settings Applied"), tr("Language change will be applied on next restart."));
    
    Utils::Logger::info("Settings applied");
}

void SettingsWidget::saveSettings()
{
    // 保存设置
    saveCurrentSettings();
    
    // 保存配置文件
    if (m_configManager) {
        if (m_configManager->save()) {
            QMessageBox::information(this, tr("Settings Saved"), tr("Settings have been saved successfully."));
            Utils::Logger::info("Settings saved");
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to save settings."));
            Utils::Logger::error("Failed to save settings");
        }
    }
}

void SettingsWidget::cancelSettings()
{
    // 取消设置
    loadCurrentSettings();
    
    // 关闭设置窗口（如果是在对话框中）
    // parentWidget()->close();
}

void SettingsWidget::resetToDefaults()
{
    // 重置为默认设置
    if (QMessageBox::question(
        this,
        tr("Reset Settings"),
        tr("Are you sure you want to reset all settings to their default values?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No) == QMessageBox::Yes) {
        
        // 重置所有设置
        if (m_configManager) {
            m_configManager->resetToDefaults();
            loadCurrentSettings();
            
            QMessageBox::information(this, tr("Settings Reset"), tr("All settings have been reset to their default values."));
            Utils::Logger::info("Settings reset to defaults");
        }
    }
}

void SettingsWidget::selectFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this->font(), this);
    
    if (ok) {
        m_fontButton->setText(font.family() + " " + QString::number(font.pointSize()));
        m_fontSizeSpin->setValue(font.pointSize());
    }
}

void SettingsWidget::selectBackgroundColor()
{
    QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Background Color"));
    
    if (color.isValid()) {
        m_backgroundColorButton->setText(color.name());
        m_backgroundColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}

void SettingsWidget::selectSelectionColor()
{
    QColor color = QColorDialog::getColor(Qt::blue, this, tr("Select Selection Color"));
    
    if (color.isValid()) {
        m_selectionColorButton->setText(color.name());
        m_selectionColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}

void SettingsWidget::selectLabelFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this->font(), this);
    
    if (ok) {
        m_labelFontButton->setText(font.family() + " " + QString::number(font.pointSize()));
    }
}

void SettingsWidget::browseModelSavePath()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Model Save Directory"),
        m_modelSavePathEdit->text().isEmpty() ? QDir::homePath() : m_modelSavePathEdit->text());
    
    if (!dir.isEmpty()) {
        m_modelSavePathEdit->setText(dir);
    }
}

void SettingsWidget::browseDataCachePath()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Data Cache Directory"),
        m_dataCachePathEdit->text().isEmpty() ? QDir::tempPath() : m_dataCachePathEdit->text());
    
    if (!dir.isEmpty()) {
        m_dataCachePathEdit->setText(dir);
    }
}

void SettingsWidget::browseLogFilePath()
{
    QString file = QFileDialog::getSaveFileName(
        this,
        tr("Select Log File"),
        m_logFilePathEdit->text().isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/bondforge.log" : m_logFilePathEdit->text(),
        tr("Log Files (*.log);;All Files (*)"));
    
    if (!file.isEmpty()) {
        m_logFilePathEdit->setText(file);
    }
}

void SettingsWidget::browsePluginPath()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Plugin Directory"),
        m_pluginPathEdit->text().isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins" : m_pluginPathEdit->text());
    
    if (!dir.isEmpty()) {
        m_pluginPathEdit->setText(dir);
    }
}

void SettingsWidget::resetAllSettings()
{
    if (QMessageBox::question(
        this,
        tr("Reset All Settings"),
        tr("Are you sure you want to reset all settings to their default values?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No) == QMessageBox::Yes) {
        
        if (m_configManager) {
            m_configManager->resetToDefaults();
            loadCurrentSettings();
            
            QMessageBox::information(this, tr("Settings Reset"), tr("All settings have been reset to their default values."));
            Utils::Logger::info("All settings reset to defaults");
        }
    }
}

void SettingsWidget::resetAdvancedSettings()
{
    if (QMessageBox::question(
        this,
        tr("Reset Advanced Settings"),
        tr("Are you sure you want to reset advanced settings to their default values?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No) == QMessageBox::Yes) {
        
        // 重置高级设置
        if (m_configManager) {
            m_configManager->resetCategoryToDefaults("advanced");
            loadCurrentSettings();
            
            QMessageBox::information(this, tr("Advanced Settings Reset"), tr("Advanced settings have been reset to their default values."));
            Utils::Logger::info("Advanced settings reset to defaults");
        }
    }
}

void SettingsWidget::exportSettings()
{
    QString file = QFileDialog::getSaveFileName(
        this,
        tr("Export Settings"),
        QDir::homePath() + "/bondforge_settings.json",
        tr("JSON Files (*.json);;All Files (*)"));
    
    if (!file.isEmpty()) {
        if (m_configManager) {
            if (m_configManager->exportToFile(file)) {
                QMessageBox::information(this, tr("Settings Exported"), tr("Settings have been exported to %1").arg(file));
                Utils::Logger::info("Settings exported to %1").arg(file);
            } else {
                QMessageBox::critical(this, tr("Error"), tr("Failed to export settings."));
                Utils::Logger::error("Failed to export settings");
            }
        }
    }
}

void SettingsWidget::importSettings()
{
    QString file = QFileDialog::getOpenFileName(
        this,
        tr("Import Settings"),
        QDir::homePath(),
        tr("JSON Files (*.json);;All Files (*)"));
    
    if (!file.isEmpty()) {
        if (m_configManager) {
            if (m_configManager->importFromFile(file)) {
                loadCurrentSettings();
                QMessageBox::information(this, tr("Settings Imported"), tr("Settings have been imported from %1").arg(file));
                Utils::Logger::info("Settings imported from %1").arg(file);
            } else {
                QMessageBox::critical(this, tr("Error"), tr("Failed to import settings."));
                Utils::Logger::error("Failed to import settings from %1").arg(file);
            }
        }
    }
}

} // namespace UI