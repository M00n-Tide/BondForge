#ifndef UPDATESERVICE_H
#define UPDATESERVICE_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <memory>
#include <map>
#include <vector>

namespace Services {

/**
 * @brief 数据源类型枚举
 */
enum class DataSourceType {
    MolecularData,      // 分子数据
    MLModels,           // 机器学习模型
    ChemicalDatabases,  // 化学数据库
    ReferenceData       // 参考数据
};

/**
 * @brief 更新状态枚举
 */
enum class UpdateStatus {
    Idle,               // 空闲状态
    Checking,           // 检查更新中
    Downloading,        // 下载中
    Installing,         // 安装中
    Success,            // 更新成功
    Failed,             // 更新失败
    Scheduled          // 已计划
};

/**
 * @brief 数据源信息结构
 */
struct DataSourceInfo {
    QString id;                     // 数据源唯一标识
    QString name;                   // 数据源名称
    QString description;            // 描述
    DataSourceType type;            // 数据源类型
    QString version;                // 当前版本
    QString latestVersion;          // 最新版本
    QUrl updateUrl;                // 更新URL
    QUrl checksumUrl;              // 校验和URL
    QString localPath;             // 本地路径
    QDateTime lastUpdate;          // 最后更新时间
    QDateTime nextCheck;           // 下次检查时间
    int checkIntervalDays;          // 检查间隔天数
    bool autoUpdate;               // 是否自动更新
    bool critical;                 // 是否为关键数据
    QString checksum;              // 文件校验和
    QJsonObject metadata;           // 元数据
};

/**
 * @brief 更新结果结构
 */
struct UpdateResult {
    UpdateStatus status;            // 更新状态
    QString sourceId;              // 数据源ID
    QString message;               // 状态消息
    QString errorDetails;           // 错误详情
    QDateTime timestamp;           // 时间戳
};

/**
 * @brief 更新进度信息
 */
struct UpdateProgress {
    int total;                     // 总进度
    int current;                   // 当前进度
    QString stage;                 // 当前阶段
    QString details;                // 详细信息
};

/**
 * @brief 数据更新服务
 * 
 * 负责管理BondForge V2.0中所有数据的自动更新
 */
class UpdateService : public QObject
{
    Q_OBJECT

public:
    explicit UpdateService(QObject *parent = nullptr);
    ~UpdateService();

    // 初始化服务
    bool initialize();
    
    // 配置管理
    void setAutoUpdateEnabled(bool enabled);
    bool isAutoUpdateEnabled() const;
    
    void setCheckInterval(const QString &sourceId, int days);
    void setAutoUpdate(const QString &sourceId, bool enabled);
    
    // 手动更新操作
    void checkForUpdates(const QString &sourceId = QString());
    void updateAll();
    void updateSource(const QString &sourceId);
    
    // 数据源管理
    void addDataSource(const DataSourceInfo &source);
    void removeDataSource(const QString &sourceId);
    bool hasDataSource(const QString &sourceId) const;
    std::vector<DataSourceInfo> getDataSources() const;
    DataSourceInfo getDataSource(const QString &sourceId) const;
    
    // 状态查询
    UpdateStatus getUpdateStatus(const QString &sourceId) const;
    UpdateProgress getUpdateProgress() const;
    QDateTime getLastUpdateTime(const QString &sourceId) const;
    QDateTime getNextCheckTime(const QString &sourceId) const;
    
    // 配置和日志
    void saveConfiguration();
    void loadConfiguration();
    QStringList getUpdateHistory(int limit = 50) const;
    
    // 自动更新调度
    void scheduleNextCheck(const QString &sourceId);
    void startAutoScheduler();
    void stopAutoScheduler();

signals:
    // 更新状态信号
    void updateStarted(const QString &sourceId);
    void updateProgress(const QString &sourceId, int progress, const QString &message);
    void updateCompleted(const QString &sourceId, bool success, const QString &message);
    void updateFailed(const QString &sourceId, const QString &error);
    
    // 数据源信号
    void dataSourceAdded(const QString &sourceId);
    void dataSourceRemoved(const QString &sourceId);
    void dataSourceModified(const QString &sourceId);
    
    // 检查更新信号
    void updateAvailable(const QString &sourceId, const QString &currentVersion, const QString &latestVersion);
    void noUpdateAvailable(const QString &sourceId);
    
    // 全局状态信号
    void autoUpdateChanged(bool enabled);
    void schedulerStarted();
    void schedulerStopped();

private slots:
    void onCheckTimer();
    void onNetworkReplyFinished(QNetworkReply *reply);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    // 网络操作
    void checkForUpdatesInternal(const QString &sourceId);
    void downloadUpdate(const QString &sourceId);
    void installUpdate(const QString &sourceId);
    void verifyChecksum(const QString &filePath, const QString &checksum);
    
    // 工具方法
    QString generateChecksum(const QString &filePath);
    QString formatFileSize(qint64 bytes);
    QString getUpdateConfigPath();
    QString getUpdateLogPath();
    QString getDownloadPath();
    
    // 备份和恢复
    void backupCurrentData(const QString &sourceId);
    void restoreFromBackup(const QString &sourceId);
    bool createBackup(const QString &source, const QString &backupPath);
    
    // 日志和状态管理
    void logUpdateEvent(const QString &sourceId, const QString &event, const QString &details);
    void setUpdateStatus(const QString &sourceId, UpdateStatus status);
    void setUpdateProgress(int total, int current, const QString &stage, const QString &details);
    
    // 默认数据源配置
    void loadDefaultDataSources();
    
    // 成员变量
    std::map<QString, DataSourceInfo> m_dataSources;
    std::map<QString, UpdateStatus> m_updateStatuses;
    std::map<QString, QDateTime> m_lastUpdateTimes;
    UpdateProgress m_globalProgress;
    
    QNetworkAccessManager *m_networkManager;
    QTimer *m_checkTimer;
    QString m_currentDownloadingSource;
    
    bool m_autoUpdateEnabled;
    bool m_schedulerRunning;
    QString m_configPath;
    QString m_logPath;
    QString m_downloadPath;
    
    // 更新历史记录
    QStringList m_updateHistory;
    static const int MAX_HISTORY_ENTRIES = 1000;
};

} // namespace Services

#endif // UPDATESERVICE_H