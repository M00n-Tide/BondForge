#ifndef UPDATESCHEDULER_H
#define UPDATESCHEDULER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QVector>
#include <QMap>
#include <QQueue>
#include <QMutex>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QEventLoop>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QSemaphore>
#include <QThreadPool>
#include <QRunnable>
#include <QFuture>
#include <QtConcurrent>
#include <memory>
#include <atomic>
#include <condition_variable>

#include "UpdateService.h"

namespace Services {

/**
 * @brief 更新任务优先级
 */
enum class UpdatePriority {
    Low = 1,           // 低优先级，可以延迟
    Normal = 2,         // 正常优先级，按计划执行
    High = 3,          // 高优先级，尽快执行
    Critical = 4        // 关键优先级，立即执行
};

/**
 * @brief 更新任务状态
 */
enum class TaskStatus {
    Pending,           // 等待执行
    Running,           // 正在执行
    Paused,            // 已暂停
    Completed,         // 已完成
    Failed,            // 执行失败
    Cancelled,         // 已取消
    Retry              // 需要重试
};

/**
 * @brief 更新任务类型
 */
enum class TaskType {
    Check,             // 检查更新
    Download,          // 下载更新
    Install,           // 安装更新
    Backup,            // 备份数据
    Validate           // 验证数据
};

/**
 * @brief 更新任务
 */
struct UpdateTask {
    QString id;                         // 任务唯一ID
    QString sourceId;                    // 数据源ID
    TaskType type;                       // 任务类型
    UpdatePriority priority;              // 任务优先级
    TaskStatus status;                    // 任务状态
    QDateTime scheduledTime;              // 计划执行时间
    QDateTime startTime;                  // 开始执行时间
    QDateTime endTime;                    // 完成时间
    int retryCount;                      // 重试次数
    int maxRetries;                     // 最大重试次数
    int progress;                        // 进度(0-100)
    QString message;                     // 状态消息
    QString errorMessage;                // 错误消息
    QJsonObject parameters;              // 任务参数
    QJsonObject result;                  // 任务结果
    QStringList dependencies;            // 依赖任务ID
    QString parentTaskId;                // 父任务ID
    QStringList childTaskIds;             // 子任务ID
    bool isRecurring;                   // 是否重复任务
    int recurringIntervalDays;           // 重复间隔(天)
    QDateTime nextRunTime;               // 下次执行时间
    QThread::Priority threadPriority;     // 线程优先级
    qint64 estimatedDuration;            // 预计持续时间(秒)
    qint64 maxDuration;                  // 最大持续时间(秒)
};

/**
 * @brief 调度器状态
 */
enum class SchedulerStatus {
    Idle,              // 空闲状态
    Running,           // 正在运行
    Paused,            // 已暂停
    Stopping,          // 正在停止
    Stopped            // 已停止
};

/**
 * @brief 调度器统计信息
 */
struct SchedulerStatistics {
    int totalTasks;                       // 总任务数
    int completedTasks;                   // 已完成任务数
    int failedTasks;                      // 失败任务数
    int runningTasks;                     // 正在运行的任务数
    int pendingTasks;                     // 等待执行的任务数
    int cancelledTasks;                   // 已取消任务数
    double averageExecutionTime;          // 平均执行时间(秒)
    double averageWaitTime;               // 平均等待时间(秒)
    QDateTime lastExecution;              // 最后执行时间
    QDateTime nextScheduledExecution;     // 下次计划执行时间
    int tasksPerHour;                    // 每小时任务数
    int tasksPerDay;                     // 每天任务数
    QJsonArray taskTypeDistribution;      // 任务类型分布
    QJsonArray taskPriorityDistribution;  // 任务优先级分布
    QJsonObject performanceMetrics;       // 性能指标
};

/**
 * @brief 更新调度器
 * 
 * 负责管理和调度所有更新任务，支持优先级、依赖关系和并发控制
 */
class UpdateScheduler : public QObject
{
    Q_OBJECT

public:
    explicit UpdateScheduler(QObject *parent = nullptr);
    ~UpdateScheduler();

    // 初始化和配置
    bool initialize();
    void loadConfiguration();
    void saveConfiguration();

    // 调度器控制
    bool start();
    bool stop();
    bool pause();
    bool resume();
    SchedulerStatus getStatus() const;

    // 任务管理
    QString addTask(const UpdateTask &task);
    bool removeTask(const QString &taskId);
    bool updateTask(const QString &taskId, const UpdateTask &task);
    UpdateTask getTask(const QString &taskId) const;
    QVector<UpdateTask> getTasks(TaskStatus status = TaskStatus::Pending) const;
    QVector<UpdateTask> getTasksBySourceId(const QString &sourceId) const;
    QVector<UpdateTask> getTasksByType(TaskType type) const;

    // 任务执行
    bool executeTask(const QString &taskId);
    bool pauseTask(const QString &taskId);
    bool resumeTask(const QString &taskId);
    bool cancelTask(const QString &taskId);
    bool retryTask(const QString &taskId);

    // 调度和计划
    bool scheduleTask(const QString &taskId, const QDateTime &scheduledTime);
    bool rescheduleTask(const QString &taskId, const QDateTime &newTime);
    bool setTaskPriority(const QString &taskId, UpdatePriority priority);
    bool addTaskDependency(const QString &taskId, const QString &dependencyId);
    bool removeTaskDependency(const QString &taskId, const QString &dependencyId);

    // 批量操作
    QStringList addTasks(const QVector<UpdateTask> &tasks);
    bool cancelAllTasks();
    bool cancelTasksBySourceId(const QString &sourceId);
    bool pauseAllTasks();
    bool resumeAllTasks();
    bool retryFailedTasks();

    // 配置管理
    void setMaxConcurrentTasks(int maxTasks);
    int getMaxConcurrentTasks() const;
    void setMaxRetryAttempts(int maxRetries);
    int getMaxRetryAttempts() const;
    void setRetryInterval(int seconds);
    int getRetryInterval() const;
    void setTaskTimeout(int seconds);
    int getTaskTimeout() const;

    // 统计和监控
    SchedulerStatistics getStatistics() const;
    QJsonObject getDetailedStatistics() const;
    void resetStatistics();
    QDateTime getEstimatedCompletionTime(const QString &taskId) const;
    QVector<QString> getRunningTasks() const;
    QVector<QString> getPendingTasks() const;

    // 维护和优化
    void cleanupCompletedTasks();
    void cleanupCancelledTasks();
    void optimizeSchedule();
    void generateReport();

signals:
    // 调度器状态信号
    void schedulerStarted();
    void schedulerStopped();
    void schedulerPaused();
    void schedulerResumed();
    void schedulerStatusChanged(SchedulerStatus status);

    // 任务状态信号
    void taskAdded(const QString &taskId);
    void taskRemoved(const QString &taskId);
    void taskUpdated(const QString &taskId);
    void taskStarted(const QString &taskId);
    void taskPaused(const QString &taskId);
    void taskResumed(const QString &taskId);
    void taskCompleted(const QString &taskId, bool success, const QString &message);
    void taskFailed(const QString &taskId, const QString &errorMessage);
    void taskCancelled(const QString &taskId);
    void taskProgress(const QString &taskId, int progress, const QString &message);

    // 调度信号
    void taskScheduled(const QString &taskId, const QDateTime &scheduledTime);
    void taskRescheduled(const QString &taskId, const QDateTime &newTime);
    void dependencyAdded(const QString &taskId, const QString &dependencyId);
    void dependencyRemoved(const QString &taskId, const QString &dependencyId);

    // 统计信号
    void statisticsUpdated(const SchedulerStatistics &stats);
    void reportGenerated(const QString &report);

private slots:
    void onSchedulerTimer();
    void onMaintenanceTimer();
    void onTaskFinished(const QString &taskId, bool success, const QJsonObject &result);

private:
    // 内部调度逻辑
    void processPendingTasks();
    void processScheduledTasks();
    void processRunningTasks();
    void updateTaskStatus(const QString &taskId, TaskStatus status);
    void updateTaskProgress(const QString &taskId, int progress, const QString &message);

    // 任务执行逻辑
    void executeTaskInternal(const QString &taskId);
    bool canExecuteTask(const QString &taskId) const;
    bool checkTaskDependencies(const QString &taskId) const;
    void updateTaskDependencies(const QString &taskId);

    // 任务队列管理
    void addToPendingQueue(const QString &taskId);
    void removeFromPendingQueue(const QString &taskId);
    void addToRunningQueue(const QString &taskId);
    void removeFromRunningQueue(const QString &taskId);
    void sortPendingQueueByPriority();
    void sortPendingQueueByTime();

    // 线程池和并发控制
    void initializeThreadPool();
    void scheduleTaskExecution(const QString &taskId);
    void handleTaskCompletion(const QString &taskId, bool success);
    void handleTaskFailure(const QString &taskId, const QString &error);

    // 依赖关系管理
    void addDependencyEdge(const QString &taskId, const QString &dependencyId);
    void removeDependencyEdge(const QString &taskId, const QString &dependencyId);
    bool hasCircularDependency(const QString &taskId, const QString &dependencyId) const;
    QVector<QString> getDependentTasks(const QString &taskId) const;

    // 统计和监控
    void updateStatistics();
    void calculatePerformanceMetrics();
    void recordTaskExecution(const QString &taskId, bool success, qint64 duration);
    void updateTaskTypeDistribution();
    void updatePriorityDistribution();

    // 配置和持久化
    QString getSchedulerConfigPath() const;
    QString getTaskDataPath() const;
    void saveTasksToDisk();
    void loadTasksFromDisk();
    void saveStatisticsToDisk();
    void loadStatisticsFromDisk();

    // 维护和清理
    void cleanupOldTasks();
    void optimizeTaskQueue();
    void generateTaskReport();
    void checkSystemResources();

    // 任务工厂方法
    UpdateTask createCheckTask(const QString &sourceId);
    UpdateTask createDownloadTask(const QString &sourceId, const QUrl &url);
    UpdateTask createInstallTask(const QString &sourceId, const QString &filePath);
    UpdateTask createBackupTask(const QString &sourceId);
    UpdateTask createValidateTask(const QString &sourceId, const QString &filePath);

    // 成员变量
    QMap<QString, UpdateTask> m_tasks;
    QQueue<QString> m_pendingQueue;
    QVector<QString> m_runningQueue;
    QMap<QString, QStringList> m_dependencies;
    QMap<QString, QStringList> m_reverseDependencies;

    std::atomic<SchedulerStatus> m_status;
    QMutex m_tasksMutex;
    QMutex m_queueMutex;
    QMutex m_dependenciesMutex;
    QMutex m_statisticsMutex;
    QWaitCondition m_taskCondition;

    QThreadPool *m_threadPool;
    QTimer *m_schedulerTimer;
    QTimer *m_maintenanceTimer;

    QString m_configPath;
    QString m_taskDataPath;
    QString m_statisticsPath;

    // 配置参数
    int m_maxConcurrentTasks;
    int m_maxRetryAttempts;
    int m_retryInterval;
    int m_taskTimeout;
    int m_maintenanceIntervalMinutes;
    int m_schedulerIntervalSeconds;
    int m_maxPendingTasks;
    int m_taskRetentionDays;

    // 统计信息
    SchedulerStatistics m_statistics;
    QJsonArray m_taskHistory;
    QDateTime m_lastStatisticsUpdate;

    // 状态和标志
    bool m_initialized;
    bool m_autoCleanup;
    bool m_optimizationEnabled;
    bool m_detailedLogging;
    int m_maxHistoryEntries;
};

/**
 * @brief 更新任务执行器
 * 
 * 在独立线程中执行具体的更新任务
 */
class UpdateTaskExecutor : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit UpdateTaskExecutor(const UpdateTask &task, QObject *parent = nullptr);
    void run() override;

signals:
    void taskFinished(const QString &taskId, bool success, const QJsonObject &result);
    void taskProgress(const QString &taskId, int progress, const QString &message);

private:
    UpdateTask m_task;
    bool executeCheckTask();
    bool executeDownloadTask();
    bool executeInstallTask();
    bool executeBackupTask();
    bool executeValidateTask();
    void emitProgress(int progress, const QString &message);
};

} // namespace Services

#endif // UPDATESCHEDULER_H