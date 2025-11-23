#ifndef UPDATEWIDGET_H
#define UPDATEWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTextEdit>
#include <QSplitter>
#include <QTreeView>
#include <QStandardItemModel>
#include <QDateTimeEdit>
#include <QCalendarWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QStandardPaths>
#include <memory>

#include "../services/UpdateService.h"
#include "../services/UpdateScheduler.h"
#include "../services/DataSourceManager.h"

namespace UI {

/**
 * @brief 数据源表格项
 */
class DataSourceTableWidgetItem : public QTableWidgetItem
{
public:
    DataSourceTableWidgetItem(const QString &text, int type = Type);
    void setDataSourceId(const QString &id);
    QString getDataSourceId() const;
    
    bool operator<(const QTableWidgetItem &other) const override;

private:
    QString m_dataSourceId;
};

/**
 * @brief 更新任务表格项
 */
class UpdateTaskTableWidgetItem : public QTableWidgetItem
{
public:
    UpdateTaskTableWidgetItem(const QString &text, int type = Type);
    void setTaskId(const QString &id);
    QString getTaskId() const;
    
    bool operator<(const QTableWidgetItem &other) const override;

private:
    QString m_taskId;
};

/**
 * @brief 更新管理界面
 * 
 * 提供数据更新管理的用户界面，包括数据源管理、任务调度和更新历史
 */
class UpdateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateWidget(QWidget *parent = nullptr);
    ~UpdateWidget();

    // 初始化界面
    void initialize();
    void refreshAll();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    // 数据源管理槽函数
    void onDataSourceSelectionChanged();
    void onCheckForUpdatesClicked();
    void onUpdateNowClicked();
    void onAddDataSourceClicked();
    void onEditDataSourceClicked();
    void onRemoveDataSourceClicked();
    void onRefreshDataSourcesClicked();
    void onDataSourceDoubleClicked(int row, int column);
    void onDataSourceContextMenu(const QPoint &pos);
    
    // 任务管理槽函数
    void onTaskSelectionChanged();
    void onTaskExecutionClicked();
    void onPauseTaskClicked();
    void onResumeTaskClicked();
    void onCancelTaskClicked();
    void onRetryTaskClicked();
    void onScheduleTaskClicked();
    void onClearCompletedTasksClicked();
    void onRefreshTasksClicked();
    void onTaskDoubleClicked(int row, int column);
    void onTaskContextMenu(const QPoint &pos);
    
    // 配置管理槽函数
    void onAutoUpdateToggled(bool enabled);
    void onMaxConcurrentTasksChanged(int value);
    void onRetryAttemptsChanged(int value);
    void onTaskTimeoutChanged(int value);
    void onMaintenanceIntervalChanged(int value);
    void onCacheEnabledToggled(bool enabled);
    void onCacheSizeChanged(int size);
    void onCacheTtlChanged(int hours);
    void onSaveConfigClicked();
    void onResetConfigClicked();
    void onAdvancedToggled(bool enabled);
    
    // 更新服务信号槽函数
    void onUpdateServiceStatusChanged(const QString &sourceId, int status);
    void onUpdateProgress(const QString &sourceId, int progress, const QString &message);
    void onUpdateCompleted(const QString &sourceId, bool success, const QString &message);
    void onUpdateFailed(const QString &sourceId, const QString &error);
    void onUpdateAvailable(const QString &sourceId, const QString &currentVersion, const QString &latestVersion);
    void onNoUpdateAvailable(const QString &sourceId);
    
    // 调度器信号槽函数
    void onSchedulerStatusChanged(int status);
    void onTaskAdded(const QString &taskId);
    void onTaskUpdated(const QString &taskId);
    void onTaskStarted(const QString &taskId);
    void onTaskPaused(const QString &taskId);
    void onTaskResumed(const QString &taskId);
    void onTaskCompleted(const QString &taskId, bool success, const QString &message);
    void onTaskFailed(const QString &taskId, const QString &errorMessage);
    void onTaskProgress(const QString &taskId, int progress, const QString &message);
    
    // 数据源管理器信号槽函数
    void onDataSourceAdded(const QString &sourceId);
    void onDataSourceRemoved(const QString &sourceId);
    void onDataSourceModified(const QString &sourceId);
    void onDataSourceValidated(const QString &sourceId, bool isValid, const QString &message);
    void onDataSourceAccessed(const QString &sourceId, const QString &query);
    
    // 日志和历史槽函数
    void onRefreshLogClicked();
    void onClearLogClicked();
    void onExportLogClicked();
    void onRefreshHistoryClicked();
    void onClearHistoryClicked();
    void onExportHistoryClicked();
    void onLogFilterChanged();
    void onHistoryFilterChanged();

private:
    void setupUI();
    void setupDataSourceTab();
    void setupTaskTab();
    void setupConfigTab();
    void setupLogTab();
    void setupToolbar();
    void connectSignals();
    
    // 数据源管理
    void loadDataSources();
    void updateDataSourceTable();
    void selectDataSource(const QString &sourceId);
    void refreshDataSourceDetails(const QString &sourceId);
    void showDataSourceDialog(const QString &sourceId = QString());
    void showDataSourceDetails(const QString &sourceId);
    
    // 任务管理
    void loadTasks();
    void updateTaskTable();
    void selectTask(const QString &taskId);
    void refreshTaskDetails(const QString &taskId);
    void showTaskDetails(const QString &taskId);
    void showScheduleDialog(const QString &taskId);
    
    // 配置管理
    void loadConfiguration();
    void saveConfiguration();
    void updateConfigurationUI();
    void resetConfiguration();
    void showAdvancedOptions();
    
    // 日志和历史
    void loadUpdateLog();
    void loadUpdateHistory();
    void updateLogDisplay();
    void updateHistoryDisplay();
    void exportLogToFile();
    void exportHistoryToFile();
    
    // 状态更新
    void updateStatusDisplay();
    void updateProgressDisplay();
    void showUpdateNotification(const QString &sourceId, const QString &message);
    
    // 工具方法
    QString getStatusText(int status) const;
    QIcon getStatusIcon(int status) const;
    QString getTaskTypeText(int type) const;
    QIcon getTaskTypeIcon(int type) const;
    QString getPriorityText(int priority) const;
    QIcon getPriorityIcon(int priority) const;
    QString formatFileSize(qint64 bytes) const;
    QString formatDuration(qint64 seconds) const;
    QString formatDateTime(const QDateTime &dateTime) const;
    
    // UI组件
    QTabWidget* m_mainTabs;
    
    // 工具栏
    QWidget* m_toolbarWidget;
    QPushButton* m_refreshButton;
    QPushButton* m_checkUpdatesButton;
    QPushButton* m_updateAllButton;
    QPushButton* m_settingsButton;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // 数据源标签页
    QWidget* m_dataSourceTab;
    QSplitter* m_dataSourceSplitter;
    QTableWidget* m_dataSourceTable;
    QWidget* m_dataSourceDetailsWidget;
    QFormLayout* m_dataSourceDetailsLayout;
    QLabel* m_dataSourceNameLabel;
    QLabel* m_dataSourceVersionLabel;
    QLabel* m_dataSourceTypeLabel;
    QLabel* m_dataSourceStatusLabel;
    QLabel* m_dataSourceLastUpdateLabel;
    QLabel* m_dataSourceNextCheckLabel;
    QTextEdit* m_dataSourceDescriptionEdit;
    QPushButton* m_checkForUpdatesButton;
    QPushButton* m_updateNowButton;
    QPushButton* m_addDataSourceButton;
    QPushButton* m_editDataSourceButton;
    QPushButton* m_removeDataSourceButton;
    QPushButton* m_refreshDataSourcesButton;
    
    // 任务标签页
    QWidget* m_taskTab;
    QSplitter* m_taskSplitter;
    QTableWidget* m_taskTable;
    QWidget* m_taskDetailsWidget;
    QFormLayout* m_taskDetailsLayout;
    QLabel* m_taskNameLabel;
    QLabel* m_taskTypeLabel;
    QLabel* m_taskStatusLabel;
    QLabel* m_taskPriorityLabel;
    QLabel* m_taskScheduledTimeLabel;
    QLabel* m_taskStartTimeLabel;
    QLabel* m_taskProgressLabel;
    QTextEdit* m_taskMessageEdit;
    QPushButton* m_executeTaskButton;
    QPushButton* m_pauseTaskButton;
    QPushButton* m_resumeTaskButton;
    QPushButton* m_cancelTaskButton;
    QPushButton* m_retryTaskButton;
    QPushButton* m_scheduleTaskButton;
    QPushButton* m_clearCompletedTasksButton;
    QPushButton* m_refreshTasksButton;
    
    // 配置标签页
    QWidget* m_configTab;
    QWidget* m_basicConfigWidget;
    QWidget* m_advancedConfigWidget;
    QFormLayout* m_basicConfigLayout;
    QFormLayout* m_advancedConfigLayout;
    QCheckBox* m_autoUpdateCheckBox;
    QSpinBox* m_maxConcurrentTasksSpin;
    QSpinBox* m_retryAttemptsSpin;
    QSpinBox* m_taskTimeoutSpin;
    QCheckBox* m_cacheEnabledCheckBox;
    QSpinBox* m_cacheSizeSpin;
    QSpinBox* m_cacheTtlSpin;
    QCheckBox* m_advancedCheckBox;
    QPushButton* m_saveConfigButton;
    QPushButton* m_resetConfigButton;
    
    // 日志标签页
    QWidget* m_logTab;
    QWidget* m_logWidget;
    QWidget* m_historyWidget;
    QVBoxLayout* m_logLayout;
    QVBoxLayout* m_historyLayout;
    QTextEdit* m_logTextEdit;
    QTextEdit* m_historyTextEdit;
    QWidget* m_logFilterWidget;
    QWidget* m_historyFilterWidget;
    QFormLayout* m_logFilterLayout;
    QFormLayout* m_historyFilterLayout;
    QLineEdit* m_logFilterEdit;
    QComboBox* m_logLevelCombo;
    QComboBox* m_logDateRangeCombo;
    QLineEdit* m_historyFilterEdit;
    QComboBox* m_historyTypeCombo;
    QComboBox* m_historyDateRangeCombo;
    QPushButton* m_refreshLogButton;
    QPushButton* m_clearLogButton;
    QPushButton* m_exportLogButton;
    QPushButton* m_refreshHistoryButton;
    QPushButton* m_clearHistoryButton;
    QPushButton* m_exportHistoryButton;
    
    // 服务实例
    std::shared_ptr<Services::UpdateService> m_updateService;
    std::shared_ptr<Services::UpdateScheduler> m_scheduler;
    std::shared_ptr<Services::DataSourceManager> m_dataSourceManager;
    
    // 状态
    QString m_selectedDataSourceId;
    QString m_selectedTaskId;
    bool m_initialized;
};

} // namespace UI

#endif // UPDATEWIDGET_H