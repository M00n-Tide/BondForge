#ifndef DATASOURCEMANAGER_H
#define DATASOURCEMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <memory>
#include <map>
#include <vector>

#include "UpdateService.h"

namespace Services {

/**
 * @brief 数据源元数据
 */
struct DataSourceMetadata {
    QString id;                     // 数据源唯一标识
    QString name;                   // 数据源名称
    QString description;            // 数据源描述
    QString provider;               // 数据提供者
    QString license;               // 许可证信息
    QUrl infoUrl;                  // 详细信息URL
    QUrl citation;                 // 引用信息
    QStringList tags;               // 标签
    QJsonObject schema;             // 数据模式
    QJsonArray fields;             // 字段定义
    int recordCount;               // 记录数量
    QDateTime lastModified;         // 最后修改时间
    QDateTime createdAt;           // 创建时间
    QString checksum;              // 数据完整性校验
    QString format;                // 数据格式 (JSON, CSV, SDF, etc.)
    QSize size;                   // 数据大小
    QJsonObject statistics;         // 统计信息
};

/**
 * @brief 数据源使用统计
 */
struct DataSourceUsageStats {
    int accessCount;               // 访问次数
    QDateTime lastAccess;           // 最后访问时间
    QDateTime lastQuery;            // 最后查询时间
    QStringList recentQueries;       // 最近查询记录
    QJsonArray queryStatistics;     // 查询统计
    double averageResponseTime;     // 平均响应时间
    QString mostAccessedField;      // 最常访问的字段
    QJsonObject usagePatterns;      // 使用模式分析
};

/**
 * @brief 数据源验证结果
 */
struct DataSourceValidationResult {
    bool isValid;                  // 是否有效
    QString errorMessage;           // 错误信息
    QJsonArray warnings;           // 警告信息
    QJsonArray errors;             // 错误详情
    QString checksum;              // 计算得到的校验和
    bool checksumValid;            // 校验和是否匹配
    bool formatValid;              // 格式是否有效
    QJsonObject validationDetails;  // 验证详情
};

/**
 * @brief 数据源缓存信息
 */
struct DataSourceCache {
    QString cachePath;              // 缓存路径
    QDateTime cacheTime;            // 缓存时间
    QDateTime expiryTime;           // 过期时间
    QString cacheType;              // 缓存类型
    qint64 cacheSize;              // 缓存大小
    bool isValid;                  // 是否有效
    QJsonArray cachedQueries;       // 已缓存的查询
    QJsonObject cacheStats;         // 缓存统计
};

/**
 * @brief 数据源管理器
 * 
 * 负责管理BondForge V2.0中所有数据源的元数据、验证、缓存和使用统计
 */
class DataSourceManager : public QObject
{
    Q_OBJECT

public:
    explicit DataSourceManager(QObject *parent = nullptr);
    ~DataSourceManager();

    // 初始化和配置
    bool initialize();
    void loadConfiguration();
    void saveConfiguration();

    // 数据源管理
    void addDataSource(const DataSourceMetadata &metadata);
    void removeDataSource(const QString &sourceId);
    bool hasDataSource(const QString &sourceId) const;
    DataSourceMetadata getDataSourceMetadata(const QString &sourceId) const;
    std::vector<DataSourceMetadata> getAllDataSources() const;
    std::vector<DataSourceMetadata> getDataSourcesByTag(const QString &tag) const;
    std::vector<DataSourceMetadata> searchDataSources(const QString &query) const;

    // 数据源验证
    DataSourceValidationResult validateDataSource(const QString &sourceId);
    bool isDataSourceValid(const QString &sourceId) const;
    QString calculateChecksum(const QString &filePath, const QString &algorithm = "SHA256");
    bool validateFormat(const QString &filePath, const QString &expectedFormat);

    // 数据源访问和使用
    QJsonObject accessDataSource(const QString &sourceId, const QString &query = QString());
    QByteArray getRawData(const QString &sourceId);
    QJsonArray queryDataSource(const QString &sourceId, const QJsonObject &queryParameters);
    QJsonObject getDataSourceStatistics(const QString &sourceId) const;

    // 缓存管理
    void cacheDataSource(const QString &sourceId, const QJsonObject &data, int ttlHours = 24);
    QJsonObject getCachedData(const QString &sourceId, const QString &query = QString());
    void clearCache(const QString &sourceId = QString());
    void setCacheEnabled(bool enabled);
    bool isCacheEnabled() const;

    // 使用统计
    void recordAccess(const QString &sourceId, const QString &query = QString());
    void recordQuery(const QString &sourceId, const QString &query, double responseTime);
    DataSourceUsageStats getUsageStats(const QString &sourceId) const;
    std::vector<DataSourceUsageStats> getAllUsageStats() const;
    void resetUsageStats(const QString &sourceId = QString());

    // 数据源版本控制
    bool isNewerVersionAvailable(const QString &sourceId);
    QString getLatestVersion(const QString &sourceId);
    bool upgradeDataSource(const QString &sourceId);
    void rollbackDataSource(const QString &sourceId, const QString &version);

    // 数据源导出和导入
    bool exportDataSource(const QString &sourceId, const QString &filePath);
    bool importDataSource(const QString &filePath);
    QJsonObject exportDataSourceConfig(const QString &sourceId);
    bool importDataSourceConfig(const QJsonObject &config);

    // 自动清理和维护
    void cleanupExpiredCache();
    void validateAllDataSources();
    void optimizeCaches();
    void generateUsageReport();

    // 路径和存储管理
    QString getDataSourcePath(const QString &sourceId) const;
    QString getCachePath(const QString &sourceId) const;
    QString getMetadataPath(const QString &sourceId) const;
    void setBasePath(const QString &path);

signals:
    // 数据源信号
    void dataSourceAdded(const QString &sourceId);
    void dataSourceRemoved(const QString &sourceId);
    void dataSourceModified(const QString &sourceId);
    void dataSourceValidated(const QString &sourceId, bool isValid, const QString &message);
    
    // 访问信号
    void dataSourceAccessed(const QString &sourceId, const QString &query);
    void dataSourceQueried(const QString &sourceId, const QString &query, double responseTime);
    
    // 缓存信号
    void dataCached(const QString &sourceId, qint64 size);
    void cacheCleared(const QString &sourceId);
    
    // 更新信号
    void dataSourceUpdated(const QString &sourceId, const QString &version);
    void dataSourceRollback(const QString &sourceId, const QString &version);
    
    // 维护信号
    void maintenanceStarted();
    void maintenanceCompleted();
    void usageReportGenerated(const QString &report);

private slots:
    void onNetworkReplyFinished(QNetworkReply *reply);

private:
    // 内部辅助方法
    void loadMetadata(const QString &sourceId);
    void saveMetadata(const QString &sourceId, const DataSourceMetadata &metadata);
    void updateUsageStats(const QString &sourceId);
    void validateAndRepairDataSource(const QString &sourceId);
    void createBackup(const QString &sourceId);
    bool restoreFromBackup(const QString &sourceId);
    
    // 路径和文件管理
    QString getBasePath() const;
    QString getMetadataDir() const;
    QString getCacheDir() const;
    QString getBackupDir() const;
    QString getStatsDir() const;
    void ensureDirectoriesExist();
    
    // 数据处理
    QJsonObject parseDataFile(const QString &filePath, const QString &format);
    QByteArray serializeData(const QJsonObject &data, const QString &format);
    QJsonArray executeQuery(const QJsonObject &data, const QJsonObject &queryParameters);
    
    // 缓存实现
    QString generateCacheKey(const QString &sourceId, const QString &query);
    void storeInCache(const QString &key, const QJsonObject &data, int ttlHours);
    QJsonObject loadFromCache(const QString &key);
    bool isCacheValid(const QString &key);
    void removeFromCache(const QString &key);
    
    // 使用统计实现
    void updateAccessStats(const QString &sourceId);
    void updateQueryStats(const QString &sourceId, const QString &query, double responseTime);
    void analyzeUsagePatterns(const QString &sourceId);
    
    // 网络操作
    void fetchRemoteMetadata(const QString &sourceId, const QUrl &url);
    void checkForRemoteUpdates(const QString &sourceId, const QUrl &url);
    
    // 成员变量
    std::map<QString, DataSourceMetadata> m_dataSources;
    std::map<QString, DataSourceUsageStats> m_usageStats;
    std::map<QString, DataSourceCache> m_cache;
    std::map<QString, QString> m_latestVersions;
    
    QNetworkAccessManager *m_networkManager;
    QTimer *m_maintenanceTimer;
    
    QString m_basePath;
    QString m_metadataDir;
    QString m_cacheDir;
    QString m_backupDir;
    QString m_statsDir;
    
    bool m_cacheEnabled;
    bool m_autoCleanupEnabled;
    int m_maintenanceIntervalHours;
    int m_defaultCacheTtlHours;
    int m_maxCacheSizeMB;
    int m_maxHistoryEntries;
    
    // 配置和状态
    bool m_initialized;
    QJsonObject m_config;
    
    // 常量
    static const QString METADATA_EXTENSION;
    static const QString CACHE_EXTENSION;
    static const QString BACKUP_EXTENSION;
    static const QString STATS_EXTENSION;
};

} // namespace Services

#endif // DATASOURCEMANAGER_H