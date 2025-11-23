#ifndef BONDFORGE_MAINWINDOW_H
#define BONDFORGE_MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <memory>

// 前向声明
namespace BondForge {
    namespace Core {
        namespace Data {
            class DataService;
        }
        namespace Collaboration {
            class User;
        }
    }
    namespace Services {
        class NetworkService;
        class DatabaseService;
    }
    namespace UI {
        class DataManagementWidget;
        class VisualizationWidget;
        class MLAnalysisWidget;
        class CollaborationWidget;
        class SettingsWidget;
    }
}

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QDockWidget;
QT_END_NAMESPACE

namespace BondForge {
namespace UI {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    // 菜单动作
    void newProject();
    void openProject();
    void saveProject();
    void saveAsProject();
    void exitApplication();
    
    // 视图切换
    void showDataManagement();
    void showVisualization();
    void showMLAnalysis();
    void showCollaboration();
    void showSettings();
    
    // 帮助菜单
    void about();
    void showHelp();
    
    // 状态更新
    void updateStatusBar();
    void onNetworkStatusChanged(bool connected);
    void onUserStatusChanged(const Core::Collaboration::User& user);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupDockWidgets();
    void setupCentralWidget();
    void setupConnections();
    
    // 加载和保存设置
    void loadSettings();
    void saveSettings();
    
    // UI组件
    QSplitter* m_mainSplitter;
    QStackedWidget* m_centralStack;
    QDockWidget* m_dataDock;
    QDockWidget* m_propertiesDock;
    QDockWidget* m_logDock;
    
    // 菜单和工具栏
    QMenu* m_fileMenu;
    QMenu* m_viewMenu;
    QMenu* m_toolsMenu;
    QMenu* m_helpMenu;
    QToolBar* m_mainToolBar;
    
    // 菜单动作
    QAction* m_newProjectAction;
    QAction* m_openProjectAction;
    QAction* m_saveProjectAction;
    QAction* m_saveAsProjectAction;
    QAction* m_exitAction;
    
    QAction* m_dataManagementAction;
    QAction* m_visualizationAction;
    QAction* m_mlAnalysisAction;
    QAction* m_collaborationAction;
    QAction* m_settingsAction;
    
    QAction* m_aboutAction;
    QAction* m_helpAction;
    
    // 状态栏组件
    QLabel* m_statusLabel;
    QLabel* m_networkStatus;
    QLabel* m_userLabel;
    QProgressBar* m_progressBar;
    QTimer* m_statusTimer;
    
    // 主要窗口部件
    std::unique_ptr<DataManagementWidget> m_dataWidget;
    std::unique_ptr<VisualizationWidget> m_visualizationWidget;
    std::unique_ptr<MLAnalysisWidget> m_mlWidget;
    std::unique_ptr<CollaborationWidget> m_collaborationWidget;
    std::unique_ptr<SettingsWidget> m_settingsWidget;
    
    // 服务层
    std::shared_ptr<Core::Data::DataService> m_dataService;
    std::shared_ptr<Services::NetworkService> m_networkService;
    std::shared_ptr<Services::DatabaseService> m_databaseService;
    
    // 应用状态
    bool m_isProjectModified;
    QString m_currentProjectPath;
};

} // namespace UI
} // namespace BondForge

#endif // BONDFORGE_MAINWINDOW_H