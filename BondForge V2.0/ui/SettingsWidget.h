#ifndef BONDFORGE_SETTINGSWIDGET_H
#define BONDFORGE_SETTINGSWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QListWidget>
#include <QTableWidget>
#include <QColorDialog>
#include <QFontDialog>
#include <QFileDialog>
#include <QProgressBar>
#include <QSlider>
#include <QTextEdit>
#include <QMessageBox>
#include <memory>

// 前向声明
namespace BondForge {
    namespace Core {
        namespace Permissions {
            class PermissionManager;
        }
        namespace Plugins {
            class PluginManager;
        }
    }
    namespace Utils {
        class ConfigManager;
        class Logger;
    }
}

namespace BondForge {
namespace UI {

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    // 标签页变化
    void onTabChanged(int index);
    
    // 通用设置槽函数
    void onLanguageChanged();
    void onThemeChanged();
    void onAutoSaveChanged();
    void onAutoSaveIntervalChanged();
    void onRecentProjectsCountChanged();
    void onFontChanged();
    void onColorChanged();
    void onResetToDefaultsClicked();
    void onImportSettingsClicked();
    void onExportSettingsClicked();
    
    // 数据库设置槽函数
    void onDbTypeChanged();
    void onDbConnectionChanged();
    void onTestDbConnectionClicked();
    void onDbBackupPathChanged();
    void onAutoBackupChanged();
    void onBackupIntervalChanged();
    void onBackupRetentionChanged();
    void onVacuumIntervalChanged();
    void onDbAdvancedChanged();
    
    // 网络设置槽函数
    void onServerUrlChanged();
    void onWebSocketUrlChanged();
    void onApiKeyChanged();
    void onTimeoutChanged();
    void onRetryCountChanged();
    void onEnableSslVerificationChanged();
    void onTestNetworkConnectionClicked();
    
    // 用户和权限槽函数
    void onUserAddClicked();
    void onUserEditClicked();
    void onUserDeleteClicked();
    void onRoleAddClicked();
    void onRoleEditClicked();
    void onRoleDeleteClicked();
    void onPermissionAddClicked();
    void onPermissionEditClicked();
    void onPermissionDeleteClicked();
    
    // 插件设置槽函数
    void onPluginSelectionChanged();
    void onPluginEnableToggled(bool enabled);
    void onPluginConfigureClicked();
    void onPluginInstallClicked();
    void onPluginUninstallClicked();
    void onPluginUpdateClicked();
    void onRefreshPluginsClicked();
    
    // 高级设置槽函数
    void onLogLevelChanged();
    void onLogPathChanged();
    void onMaxLogFilesChanged();
    void onMaxLogFileSizeChanged();
    void onPerformanceModeChanged();
    void onThreadCountChanged();
    void onCacheSizeChanged();
    void onMemoryLimitChanged();
    void onAdvancedResetClicked();
    
    // 按钮槽函数
    void onSaveClicked();
    void onCancelClicked();
    void onApplyClicked();
    void onResetClicked();

private:
    void setupUI();
    void setupGeneralSettingsTab();
    void setupDatabaseSettingsTab();
    void setupNetworkSettingsTab();
    void setupUserPermissionsTab();
    void setupPluginSettingsTab();
    void setupAdvancedSettingsTab();
    void setupButtonBar();
    
    void connectSignals();
    
    // 通用设置
    void loadGeneralSettings();
    void saveGeneralSettings();
    void resetGeneralSettings();
    void importSettings(const QString &filePath);
    void exportSettings(const QString &filePath);
    void selectFont();
    void selectColor();
    
    // 数据库设置
    void loadDatabaseSettings();
    void saveDatabaseSettings();
    void resetDatabaseSettings();
    bool testDatabaseConnection();
    void selectDatabasePath();
    void selectBackupPath();
    
    // 网络设置
    void loadNetworkSettings();
    void saveNetworkSettings();
    void resetNetworkSettings();
    bool testNetworkConnection();
    
    // 用户和权限
    void loadUsersRoles();
    void saveUsersRoles();
    void resetUsersRoles();
    void addUser();
    void editUser(const QString &userId);
    void deleteUser(const QString &userId);
    void addRole();
    void editRole(const QString &roleId);
    void deleteRole(const QString &roleId);
    void addPermission();
    void editPermission(const QString &permissionId);
    void deletePermission(const QString &permissionId);
    
    // 插件设置
    void loadPlugins();
    void savePlugins();
    void resetPlugins();
    void enablePlugin(const QString &pluginId, bool enabled);
    void configurePlugin(const QString &pluginId);
    void installPlugin();
    void uninstallPlugin(const QString &pluginId);
    void updatePlugin(const QString &pluginId);
    
    // 高级设置
    void loadAdvancedSettings();
    void saveAdvancedSettings();
    void resetAdvancedSettings();
    void selectLogPath();
    
    // 工具函数
    void updateStatusMessage(const QString &message);
    void setModified(bool modified);
    bool isModified() const;
    void validateSettings();
    bool areSettingsValid() const;
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    
    // 主标签页
    QTabWidget* m_mainTabs;
    
    // 通用设置标签页
    QWidget* m_generalTab;
    QComboBox* m_languageCombo;
    QComboBox* m_themeCombo;
    QCheckBox* m_autoSaveCheckBox;
    QSpinBox* m_autoSaveIntervalSpin;
    QSpinBox* m_recentProjectsCountSpin;
    QPushButton* m_fontButton;
    QPushButton* m_colorButton;
    QPushButton* m_resetToDefaultsButton;
    QPushButton* m_importSettingsButton;
    QPushButton* m_exportSettingsButton;
    
    // 数据库设置标签页
    QWidget* m_databaseTab;
    QComboBox* m_dbTypeCombo;
    QLineEdit* m_dbHostEdit;
    QSpinBox* m_dbPortSpin;
    QLineEdit* m_dbNameEdit;
    QLineEdit* m_dbUsernameEdit;
    QLineEdit* m_dbPasswordEdit;
    QLineEdit* m_dbConnectionOptionsEdit;
    QPushButton* m_testDbConnectionButton;
    QLineEdit* m_dbBackupPathEdit;
    QPushButton* m_selectBackupPathButton;
    QCheckBox* m_autoBackupCheckBox;
    QSpinBox* m_backupIntervalSpin;
    QSpinBox* m_backupRetentionSpin;
    QSpinBox* m_vacuumIntervalSpin;
    QCheckBox* m_enableForeignKeysCheckBox;
    QCheckBox* m_enableWALCheckBox;
    
    // 网络设置标签页
    QWidget* m_networkTab;
    QLineEdit* m_serverUrlEdit;
    QLineEdit* m_webSocketUrlEdit;
    QLineEdit* m_apiKeyEdit;
    QCheckBox* m_showApiKeyCheckBox;
    QSpinBox* m_timeoutSpin;
    QSpinBox* m_retryCountSpin;
    QCheckBox* m_enableSslVerificationCheckBox;
    QPushButton* m_testNetworkConnectionButton;
    
    // 用户和权限标签页
    QWidget* m_userPermissionsTab;
    QTabWidget* m_userPermissionsSubTabs;
    QWidget* m_usersSubTab;
    QTableWidget* m_usersTable;
    QPushButton* m_userAddButton;
    QPushButton* m_userEditButton;
    QPushButton* m_userDeleteButton;
    
    QWidget* m_rolesSubTab;
    QTableWidget* m_rolesTable;
    QPushButton* m_roleAddButton;
    QPushButton* m_roleEditButton;
    QPushButton* m_roleDeleteButton;
    
    QWidget* m_permissionsSubTab;
    QTableWidget* m_permissionsTable;
    QPushButton* m_permissionAddButton;
    QPushButton* m_permissionEditButton;
    QPushButton* m_permissionDeleteButton;
    
    // 插件设置标签页
    QWidget* m_pluginsTab;
    QListWidget* m_pluginsList;
    QWidget* m_pluginDetailsWidget;
    QLabel* m_pluginNameLabel;
    QLabel* m_pluginVersionLabel;
    QLabel* m_pluginAuthorLabel;
    QLabel* m_pluginDescriptionLabel;
    QTextEdit* m_pluginConfigEdit;
    QCheckBox* m_pluginEnableCheckBox;
    QPushButton* m_pluginConfigureButton;
    QPushButton* m_pluginInstallButton;
    QPushButton* m_pluginUninstallButton;
    QPushButton* m_pluginUpdateButton;
    QPushButton* m_refreshPluginsButton;
    
    // 高级设置标签页
    QWidget* m_advancedTab;
    QComboBox* m_logLevelCombo;
    QLineEdit* m_logPathEdit;
    QPushButton* m_selectLogPathButton;
    QSpinBox* m_maxLogFilesSpin;
    QSpinBox* m_maxLogFileSizeSpin;
    QComboBox* m_performanceModeCombo;
    QSpinBox* m_threadCountSpin;
    QSpinBox* m_cacheSizeSpin;
    QSpinBox* m_memoryLimitSpin;
    QPushButton* m_advancedResetButton;
    
    // 按钮栏
    QWidget* m_buttonBarWidget;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    
    // 状态栏
    QLabel* m_statusLabel;
    
    // 服务
    std::shared_ptr<Core::Permissions::PermissionManager> m_permissionManager;
    std::shared_ptr<Core::Plugins::PluginManager> m_pluginManager;
    std::shared_ptr<Utils::ConfigManager> m_configManager;
    std::shared_ptr<Utils::Logger> m_logger;
    
    // 状态
    bool m_modified;
    QString m_selectedPluginId;
    QColor m_selectedColor;
    QFont m_selectedFont;
};

} // namespace UI
} // namespace BondForge

#endif // BONDFORGE_SETTINGSWIDGET_H