#include "MainWindow.h"
#include "DataManagementWidget.h"
#include "VisualizationWidget.h"
#include "MLAnalysisWidget.h"
#include "CollaborationWidget.h"
#include "SettingsWidget.h"

#include <QApplication>
#include <QCloseEvent>
#include <QShowEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QDir>

#include "../core/data/DataService.h"
#include "../services/NetworkService.h"
#include "../services/DatabaseService.h"
#include "../utils/ConfigManager.h"
#include "../utils/Logger.h"

namespace BondForge {
namespace UI {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_mainSplitter(nullptr)
    , m_centralStack(nullptr)
    , m_dataDock(nullptr)
    , m_propertiesDock(nullptr)
    , m_logDock(nullptr)
    , m_fileMenu(nullptr)
    , m_viewMenu(nullptr)
    , m_toolsMenu(nullptr)
    , m_helpMenu(nullptr)
    , m_mainToolBar(nullptr)
    , m_newProjectAction(nullptr)
    , m_openProjectAction(nullptr)
    , m_saveProjectAction(nullptr)
    , m_saveAsProjectAction(nullptr)
    , m_exitAction(nullptr)
    , m_dataManagementAction(nullptr)
    , m_visualizationAction(nullptr)
    , m_mlAnalysisAction(nullptr)
    , m_collaborationAction(nullptr)
    , m_settingsAction(nullptr)
    , m_aboutAction(nullptr)
    , m_helpAction(nullptr)
    , m_statusLabel(nullptr)
    , m_networkStatus(nullptr)
    , m_userLabel(nullptr)
    , m_progressBar(nullptr)
    , m_statusTimer(nullptr)
    , m_isProjectModified(false)
{
    // 设置窗口属性
    setWindowTitle("BondForge V1.2 - Professional Chemical ML Platform");
    setMinimumSize(1200, 800);
    resize(1600, 1000);
    
    // 初始化服务层
    try {
        m_dataService = std::make_shared<Core::Data::DataService>();
        m_networkService = std::make_shared<Services::NetworkService>();
        m_databaseService = std::make_shared<Services::DatabaseService>();
        
        // 连接服务信号
        connect(m_networkService.get(), &Services::NetworkService::connectionChanged,
                this, &MainWindow::onNetworkStatusChanged);
    } catch (const std::exception& e) {
        Utils::Logger::error("Failed to initialize services: " + std::string(e.what()));
    }
    
    // 设置UI
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupDockWidgets();
    setupCentralWidget();
    setupConnections();
    
    // 加载设置
    loadSettings();
    
    // 设置状态更新定时器
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    m_statusTimer->start(1000); // 每秒更新一次状态
    
    // 显示数据管理视图作为默认视图
    showDataManagement();
    
    Utils::Logger::info("MainWindow initialized successfully");
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUI()
{
    // 设置应用程序图标和样式
    setWindowIcon(QIcon(":/icons/bondforge.png"));
    
    // 设置中央部件分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);
    
    // 设置中央堆叠部件
    m_centralStack = new QStackedWidget(this);
    m_mainSplitter->addWidget(m_centralStack);
    
    // 设置初始分割比例
    m_mainSplitter->setStretchFactor(0, 1);
}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    
    m_newProjectAction = new QAction(tr("&New Project"), this);
    m_newProjectAction->setShortcut(QKeySequence::New);
    m_newProjectAction->setStatusTip(tr("Create a new project"));
    m_newProjectAction->setIcon(QIcon(":/icons/new.png"));
    connect(m_newProjectAction, &QAction::triggered, this, &MainWindow::newProject);
    m_fileMenu->addAction(m_newProjectAction);
    
    m_openProjectAction = new QAction(tr("&Open Project"), this);
    m_openProjectAction->setShortcut(QKeySequence::Open);
    m_openProjectAction->setStatusTip(tr("Open an existing project"));
    m_openProjectAction->setIcon(QIcon(":/icons/open.png"));
    connect(m_openProjectAction, &QAction::triggered, this, &MainWindow::openProject);
    m_fileMenu->addAction(m_openProjectAction);
    
    m_fileMenu->addSeparator();
    
    m_saveProjectAction = new QAction(tr("&Save Project"), this);
    m_saveProjectAction->setShortcut(QKeySequence::Save);
    m_saveProjectAction->setStatusTip(tr("Save the current project"));
    m_saveProjectAction->setIcon(QIcon(":/icons/save.png"));
    connect(m_saveProjectAction, &QAction::triggered, this, &MainWindow::saveProject);
    m_fileMenu->addAction(m_saveProjectAction);
    
    m_saveAsProjectAction = new QAction(tr("Save Project &As..."), this);
    m_saveAsProjectAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsProjectAction->setStatusTip(tr("Save the project with a new name"));
    m_saveAsProjectAction->setIcon(QIcon(":/icons/save-as.png"));
    connect(m_saveAsProjectAction, &QAction::triggered, this, &MainWindow::saveAsProject);
    m_fileMenu->addAction(m_saveAsProjectAction);
    
    m_fileMenu->addSeparator();
    
    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    m_fileMenu->addAction(m_exitAction);
    
    // 视图菜单
    m_viewMenu = menuBar()->addMenu(tr("&View"));
    
    m_dataManagementAction = new QAction(tr("&Data Management"), this);
    m_dataManagementAction->setShortcut(QKeySequence("Ctrl+1"));
    m_dataManagementAction->setStatusTip(tr("Show data management interface"));
    m_dataManagementAction->setCheckable(true);
    m_dataManagementAction->setChecked(true);
    connect(m_dataManagementAction, &QAction::triggered, this, &MainWindow::showDataManagement);
    m_viewMenu->addAction(m_dataManagementAction);
    
    m_visualizationAction = new QAction(tr("&Visualization"), this);
    m_visualizationAction->setShortcut(QKeySequence("Ctrl+2"));
    m_visualizationAction->setStatusTip(tr("Show molecular visualization interface"));
    m_visualizationAction->setCheckable(true);
    connect(m_visualizationAction, &QAction::triggered, this, &MainWindow::showVisualization);
    m_viewMenu->addAction(m_visualizationAction);
    
    m_mlAnalysisAction = new QAction(tr("&ML Analysis"), this);
    m_mlAnalysisAction->setShortcut(QKeySequence("Ctrl+3"));
    m_mlAnalysisAction->setStatusTip(tr("Show machine learning analysis interface"));
    m_mlAnalysisAction->setCheckable(true);
    connect(m_mlAnalysisAction, &QAction::triggered, this, &MainWindow::showMLAnalysis);
    m_viewMenu->addAction(m_mlAnalysisAction);
    
    m_collaborationAction = new QAction(tr("&Collaboration"), this);
    m_collaborationAction->setShortcut(QKeySequence("Ctrl+4"));
    m_collaborationAction->setStatusTip(tr("Show collaboration interface"));
    m_collaborationAction->setCheckable(true);
    connect(m_collaborationAction, &QAction::triggered, this, &MainWindow::showCollaboration);
    m_viewMenu->addAction(m_collaborationAction);
    
    m_settingsAction = new QAction(tr("&Settings"), this);
    m_settingsAction->setShortcut(QKeySequence("Ctrl+5"));
    m_settingsAction->setStatusTip(tr("Show application settings"));
    m_settingsAction->setCheckable(true);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
    m_viewMenu->addAction(m_settingsAction);
    
    // 工具菜单
    m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
    // 这里可以添加工具特定的菜单项
    
    // 帮助菜单
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    
    m_helpAction = new QAction(tr("&Help"), this);
    m_helpAction->setShortcut(QKeySequence::HelpContents);
    m_helpAction->setStatusTip(tr("Show application help"));
    connect(m_helpAction, &QAction::triggered, this, &MainWindow::showHelp);
    m_helpMenu->addAction(m_helpAction);
    
    m_aboutAction = new QAction(tr("&About BondForge"), this);
    m_aboutAction->setStatusTip(tr("Show information about BondForge"));
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);
    m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::setupToolBar()
{
    m_mainToolBar = addToolBar(tr("Main"));
    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    m_mainToolBar->addAction(m_newProjectAction);
    m_mainToolBar->addAction(m_openProjectAction);
    m_mainToolBar->addAction(m_saveProjectAction);
    m_mainToolBar->addSeparator();
    
    m_mainToolBar->addAction(m_dataManagementAction);
    m_mainToolBar->addAction(m_visualizationAction);
    m_mainToolBar->addAction(m_mlAnalysisAction);
    m_mainToolBar->addAction(m_collaborationAction);
    m_mainToolBar->addSeparator();
    
    m_mainToolBar->addAction(m_helpAction);
}

void MainWindow::setupStatusBar()
{
    // 状态栏
    m_statusLabel = new QLabel(tr("Ready"));
    statusBar()->addWidget(m_statusLabel, 1);
    
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(150);
    statusBar()->addWidget(m_progressBar);
    
    m_networkStatus = new QLabel(tr("Network: Offline"));
    m_networkStatus->setMinimumWidth(120);
    statusBar()->addWidget(m_networkStatus);
    
    m_userLabel = new QLabel(tr("User: Not logged in"));
    m_userLabel->setMinimumWidth(150);
    statusBar()->addWidget(m_userLabel);
}

void MainWindow::setupDockWidgets()
{
    // 数据面板停靠窗口
    m_dataDock = new QDockWidget(tr("Data Panel"), this);
    m_dataDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    // 这里可以添加数据面板的具体内容
    // m_dataDock->setWidget(new DataPanelWidget());
    
    addDockWidget(Qt::LeftDockWidgetArea, m_dataDock);
    
    // 属性面板停靠窗口
    m_propertiesDock = new QDockWidget(tr("Properties"), this);
    m_propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    // 这里可以添加属性面板的具体内容
    // m_propertiesDock->setWidget(new PropertiesPanelWidget());
    
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
    
    // 日志面板停靠窗口
    m_logDock = new QDockWidget(tr("Application Log"), this);
    m_logDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    
    QTextEdit* logWidget = new QTextEdit();
    logWidget->setReadOnly(true);
    logWidget->setMaximumHeight(150);
    m_logDock->setWidget(logWidget);
    
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);
    
    // 设置停靠窗口的默认可见性
    m_dataDock->setVisible(true);
    m_propertiesDock->setVisible(true);
    m_logDock->setVisible(false);
    
    // 在视图菜单中添加停靠窗口的切换选项
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_dataDock->toggleViewAction());
    m_viewMenu->addAction(m_propertiesDock->toggleViewAction());
    m_viewMenu->addAction(m_logDock->toggleViewAction());
}

void MainWindow::setupCentralWidget()
{
    // 初始化各个核心UI组件
    m_dataWidget = std::make_unique<DataManagementWidget>(m_dataService, this);
    m_centralStack->addWidget(m_dataWidget.get());
    
    m_visualizationWidget = std::make_unique<VisualizationWidget>(m_dataService, this);
    m_centralStack->addWidget(m_visualizationWidget.get());
    
    m_mlWidget = std::make_unique<MLAnalysisWidget>(m_dataService, this);
    m_centralStack->addWidget(m_mlWidget.get());
    
    m_collaborationWidget = std::make_unique<CollaborationWidget>(m_dataService, this);
    m_centralStack->addWidget(m_collaborationWidget.get());
    
    m_settingsWidget = std::make_unique<SettingsWidget>(this);
    m_centralStack->addWidget(m_settingsWidget.get());
}

void MainWindow::setupConnections()
{
    // 连接核心组件的信号和槽
    // 这些连接可以根据需要添加
}

void MainWindow::loadSettings()
{
    QSettings settings;
    
    // 恢复窗口几何状态
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    restoreState(settings.value("MainWindow/windowState").toByteArray());
    
    // 恢复最近的项目路径
    m_currentProjectPath = settings.value("Project/currentPath").toString();
    
    // 恢复视图设置
    if (m_dataDock) {
        m_dataDock->setVisible(settings.value("DockWidgets/dataVisible", true).toBool());
    }
    if (m_propertiesDock) {
        m_propertiesDock->setVisible(settings.value("DockWidgets/propertiesVisible", true).toBool());
    }
    if (m_logDock) {
        m_logDock->setVisible(settings.value("DockWidgets/logVisible", false).toBool());
    }
}

void MainWindow::saveSettings()
{
    QSettings settings;
    
    // 保存窗口几何状态
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/windowState", saveState());
    
    // 保存当前项目路径
    if (!m_currentProjectPath.isEmpty()) {
        settings.setValue("Project/currentPath", m_currentProjectPath);
    }
    
    // 保存视图设置
    if (m_dataDock) {
        settings.setValue("DockWidgets/dataVisible", m_dataDock->isVisible());
    }
    if (m_propertiesDock) {
        settings.setValue("DockWidgets/propertiesVisible", m_propertiesDock->isVisible());
    }
    if (m_logDock) {
        settings.setValue("DockWidgets/logVisible", m_logDock->isVisible());
    }
}

// 菜单动作实现
void MainWindow::newProject()
{
    if (m_isProjectModified) {
        int ret = QMessageBox::warning(this, tr("BondForge"),
                                      tr("The current project has been modified.\n"
                                         "Do you want to save your changes?"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            saveProject();
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    
    // 清空当前项目并重置界面
    m_currentProjectPath.clear();
    m_isProjectModified = false;
    setWindowTitle("BondForge V1.2 - Professional Chemical ML Platform");
    
    // 通知各个组件项目已重置
    // 这里可以添加信号发送给各个组件
    
    Utils::Logger::info("New project created");
}

void MainWindow::openProject()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                   tr("Open Project"),
                                                   "",
                                                   tr("BondForge Projects (*.bfp);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        // 这里添加项目加载逻辑
        m_currentProjectPath = fileName;
        m_isProjectModified = false;
        setWindowTitle("BondForge V1.2 - " + QFileInfo(fileName).fileName());
        
        Utils::Logger::info("Project opened: " + fileName.toStdString());
    }
}

void MainWindow::saveProject()
{
    if (m_currentProjectPath.isEmpty()) {
        saveAsProject();
        return;
    }
    
    // 这里添加项目保存逻辑
    m_isProjectModified = false;
    
    Utils::Logger::info("Project saved: " + m_currentProjectPath.toStdString());
}

void MainWindow::saveAsProject()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                   tr("Save Project As"),
                                                   "",
                                                   tr("BondForge Projects (*.bfp);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        m_currentProjectPath = fileName;
        setWindowTitle("BondForge V1.2 - " + QFileInfo(fileName).fileName());
        saveProject();
    }
}

void MainWindow::exitApplication()
{
    if (m_isProjectModified) {
        int ret = QMessageBox::warning(this, tr("BondForge"),
                                      tr("The current project has been modified.\n"
                                         "Do you want to save your changes?"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            saveProject();
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    
    QApplication::quit();
}

// 视图切换实现
void MainWindow::showDataManagement()
{
    if (!m_dataWidget) return;
    
    m_centralStack->setCurrentWidget(m_dataWidget.get());
    
    // 更新菜单状态
    m_dataManagementAction->setChecked(true);
    m_visualizationAction->setChecked(false);
    m_mlAnalysisAction->setChecked(false);
    m_collaborationAction->setChecked(false);
    m_settingsAction->setChecked(false);
    
    m_statusLabel->setText(tr("Data Management"));
    Utils::Logger::info("Switched to Data Management view");
}

void MainWindow::showVisualization()
{
    if (!m_visualizationWidget) return;
    
    m_centralStack->setCurrentWidget(m_visualizationWidget.get());
    
    // 更新菜单状态
    m_dataManagementAction->setChecked(false);
    m_visualizationAction->setChecked(true);
    m_mlAnalysisAction->setChecked(false);
    m_collaborationAction->setChecked(false);
    m_settingsAction->setChecked(false);
    
    m_statusLabel->setText(tr("Molecular Visualization"));
    Utils::Logger::info("Switched to Molecular Visualization view");
}

void MainWindow::showMLAnalysis()
{
    if (!m_mlWidget) return;
    
    m_centralStack->setCurrentWidget(m_mlWidget.get());
    
    // 更新菜单状态
    m_dataManagementAction->setChecked(false);
    m_visualizationAction->setChecked(false);
    m_mlAnalysisAction->setChecked(true);
    m_collaborationAction->setChecked(false);
    m_settingsAction->setChecked(false);
    
    m_statusLabel->setText(tr("Machine Learning Analysis"));
    Utils::Logger::info("Switched to Machine Learning Analysis view");
}

void MainWindow::showCollaboration()
{
    if (!m_collaborationWidget) return;
    
    m_centralStack->setCurrentWidget(m_collaborationWidget.get());
    
    // 更新菜单状态
    m_dataManagementAction->setChecked(false);
    m_visualizationAction->setChecked(false);
    m_mlAnalysisAction->setChecked(false);
    m_collaborationAction->setChecked(true);
    m_settingsAction->setChecked(false);
    
    m_statusLabel->setText(tr("Collaboration"));
    Utils::Logger::info("Switched to Collaboration view");
}

void MainWindow::showSettings()
{
    if (!m_settingsWidget) return;
    
    m_centralStack->setCurrentWidget(m_settingsWidget.get());
    
    // 更新菜单状态
    m_dataManagementAction->setChecked(false);
    m_visualizationAction->setChecked(false);
    m_mlAnalysisAction->setChecked(false);
    m_collaborationAction->setChecked(false);
    m_settingsAction->setChecked(true);
    
    m_statusLabel->setText(tr("Settings"));
    Utils::Logger::info("Switched to Settings view");
}

// 帮助菜单实现
void MainWindow::about()
{
    QMessageBox::about(this, tr("About BondForge"),
                      tr("<h2>BondForge V1.2</h2>"
                         "<p>Professional Chemical Machine Learning Platform</p>"
                         "<p>BondForge is a comprehensive tool for chemical data management, "
                         "molecular visualization, machine learning analysis, and collaboration.</p>"
                         "<p>Copyright © 2023-2024 BondForge Team</p>"));
}

void MainWindow::showHelp()
{
    // 这里可以添加帮助浏览器或显示帮助文档
    QMessageBox::information(this, tr("Help"),
                            tr("Help documentation is available at:\n"
                               "https://bondforge.org/docs"));
}

// 事件处理
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isProjectModified) {
        int ret = QMessageBox::warning(this, tr("BondForge"),
                                      tr("The current project has been modified.\n"
                                         "Do you want to save your changes?"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            saveProject();
        } else if (ret == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    
    // 保存设置
    saveSettings();
    
    // 停止状态更新定时器
    if (m_statusTimer) {
        m_statusTimer->stop();
    }
    
    Utils::Logger::info("Application closed");
    event->accept();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    
    // 初始化网络连接状态
    if (m_networkService) {
        onNetworkStatusChanged(m_networkService->isConnected());
    }
}

// 状态更新
void MainWindow::updateStatusBar()
{
    // 这里可以添加定期更新的状态信息
    // 例如数据统计、处理状态等
}

void MainWindow::onNetworkStatusChanged(bool connected)
{
    if (connected) {
        m_networkStatus->setText(tr("Network: Online"));
        m_networkStatus->setStyleSheet("color: green;");
    } else {
        m_networkStatus->setText(tr("Network: Offline"));
        m_networkStatus->setStyleSheet("color: red;");
    }
}

void MainWindow::onUserStatusChanged(const Core::Collaboration::User& user)
{
    if (!user.id.empty()) {
        m_userLabel->setText(tr("User: %1").arg(QString::fromStdString(user.name)));
    } else {
        m_userLabel->setText(tr("User: Not logged in"));
    }
}

} // namespace UI
} // namespace BondForge