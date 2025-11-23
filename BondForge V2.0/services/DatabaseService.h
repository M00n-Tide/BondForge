#ifndef BONDFORGE_DATABASESERVICE_H
#define BONDFORGE_DATABASESERVICE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <QVariant>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QThreadPool>
#include <QRunnable>
#include <QFuture>
#include <QtConcurrent>
#include <memory>
#include <functional>
#include <vector>
#include <map>
#include <atomic>

// 前向声明
namespace BondForge {
    namespace Core {
        namespace Data {
            class DataRecord;
        }
        namespace Collaboration {
            class User;
            class Comment;
            class DataSharing;
            class Project;
        }
        namespace Permissions {
            class Permission;
            class Role;
        }
    }
}

namespace BondForge {
namespace Services {

// 数据库类型枚举
enum class DatabaseType {
    SQLITE,
    MYSQL,
    POSTGRESQL,
    ORACLE,
    SQLSERVER
};

// 数据库配置
struct DatabaseConfig {
    DatabaseType type = DatabaseType::SQLITE;
    QString host = "localhost";
    int port = 5432; // PostgreSQL default
    QString databaseName = "bondforge.db";
    QString username = "";
    QString password = "";
    QString connectionOptions = ""; // 连接特定选项
    bool autoConnect = true;
    bool enableForeignKeys = true;
    bool enableWAL = true; // Write-Ahead Logging for SQLite
    int connectionTimeout = 5000; // ms
    int queryTimeout = 30000; // ms
    int maxConnections = 5; // 最大连接数
    bool enableCache = true;
    int cacheSize = 1000; // 缓存记录数
};

// 查询回调函数类型
using QueryCallback = std::function<void(const QSqlQuery&)>;
using ErrorCallback = std::function<void(const QString&)>;
using SuccessCallback = std::function<void(bool)>;

// 数据库迁移信息
struct Migration {
    QString version;
    QString description;
    QString upSql;
    QString downSql;
};

// 查询结果
struct QueryResult {
    bool success = false;
    QSqlQuery query;
    QString errorString;
    int affectedRows = 0;
    QVariant lastInsertId;
};

// 数据库服务统计
struct DatabaseStats {
    int totalQueries = 0;
    int successfulQueries = 0;
    int failedQueries = 0;
    int activeConnections = 0;
    int totalConnections = 0;
    int cacheHits = 0;
    int cacheMisses = 0;
    qint64 totalQueryTime = 0; // 微秒
    qint64 avgQueryTime = 0; // 微秒
};

// 数据库查询任务
class DatabaseTask : public QRunnable
{
public:
    explicit DatabaseTask(std::function<void()> task);
    void run() override;

private:
    std::function<void()> m_task;
};

class DatabaseService : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseService(QObject *parent = nullptr);
    ~DatabaseService();

    // 配置管理
    void setConfig(const DatabaseConfig &config);
    DatabaseConfig config() const;
    
    // 连接管理
    bool connect();
    bool disconnect();
    bool reconnect();
    bool isConnected() const;
    bool isReady() const;
    
    // 数据库操作
    bool createDatabase();
    bool initializeTables();
    bool createIndexes();
    bool executeMigration(const QString &version);
    bool rollbackMigration(const QString &version);
    QStringList pendingMigrations() const;
    
    // 数据记录管理
    bool saveDataRecord(const Core::Data::DataRecord &record);
    bool loadDataRecord(const QString &id, Core::Data::DataRecord &record);
    bool updateDataRecord(const Core::Data::DataRecord &record);
    bool deleteDataRecord(const QString &id);
    QList<Core::Data::DataRecord> findDataRecords(const QString &filter = "", const QString &orderBy = "", int limit = 0);
    QList<Core::Data::DataRecord> findDataRecordsByCategory(const QString &category);
    QList<Core::Data::DataRecord> findDataRecordsByFormat(const QString &format);
    QList<Core::Data::DataRecord> searchDataRecords(const QString &searchTerm);
    int getDataRecordsCount(const QString &filter = "");
    
    // 协作功能
    bool saveUser(const Core::Collaboration::User &user);
    bool loadUser(const QString &id, Core::Collaboration::User &user);
    bool loadUserByUsername(const QString &username, Core::Collaboration::User &user);
    bool updateUser(const Core::Collaboration::User &user);
    bool deleteUser(const QString &id);
    QList<Core::Collaboration::User> getAllUsers();
    
    bool saveComment(const Core::Collaboration::Comment &comment);
    bool updateComment(const Core::Collaboration::Comment &comment);
    bool deleteComment(const QString &id);
    QList<Core::Collaboration::Comment> getCommentsForRecord(const QString &recordId);
    
    bool saveDataSharing(const Core::Collaboration::DataSharing &sharing);
    bool loadDataSharing(const QString &id, Core::Collaboration::DataSharing &sharing);
    bool updateDataSharing(const Core::Collaboration::DataSharing &sharing);
    bool deleteDataSharing(const QString &id);
    QList<Core::Collaboration::DataSharing> getDataSharedWithUser(const QString &userId);
    QList<Core::Collaboration::DataSharing> getDataSharedByUser(const QString &userId);
    
    // 权限管理
    bool savePermission(const Core::Permissions::Permission &permission);
    bool loadPermission(const QString &id, Core::Permissions::Permission &permission);
    bool updatePermission(const Core::Permissions::Permission &permission);
    bool deletePermission(const QString &id);
    QList<Core::Permissions::Permission> getPermissionsForUser(const QString &userId);
    QList<Core::Permissions::Permission> getPermissionsForResource(const QString &resourceId);
    
    bool saveRole(const Core::Permissions::Role &role);
    bool loadRole(const QString &id, Core::Permissions::Role &role);
    bool updateRole(const Core::Permissions::Role &role);
    bool deleteRole(const QString &id);
    QList<Core::Permissions::Role> getAllRoles();
    
    // 原始SQL查询
    QueryResult executeQuery(const QString &sql, const QVariantMap &params = {});
    QueryResult executeSelectQuery(const QString &sql, const QVariantMap &params = {});
    QueryResult executeInsertQuery(const QString &sql, const QVariantMap &params = {});
    QueryResult executeUpdateQuery(const QString &sql, const QVariantMap &params = {});
    QueryResult executeDeleteQuery(const QString &sql, const QVariantMap &params = {});
    
    // 异步查询
    QString executeAsyncQuery(const QString &sql, const QVariantMap &params, 
                               const QueryCallback &callback = nullptr,
                               const ErrorCallback &errorCallback = nullptr);
    void cancelAsyncQuery(const QString &taskId);
    
    // 事务管理
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // 备份和恢复
    bool backupDatabase(const QString &filePath);
    bool restoreDatabase(const QString &filePath);
    bool exportData(const QString &filePath, const QString &format = "json");
    bool importData(const QString &filePath, const QString &format = "json");
    
    // 数据库优化
    bool vacuumDatabase(); // SQLite: 重建数据库，回收空间
    bool analyzeDatabase(); // 更新统计信息
    bool reindexDatabase(); // 重建索引
    bool optimizeDatabase(); // 执行所有优化操作
    
    // 数据库统计
    DatabaseStats getStatistics() const;
    void resetStatistics();
    QString getDatabaseInfo() const;
    QString getTableInfo(const QString &tableName) const;
    
    // 缓存管理
    void clearCache();
    void setCacheSize(int size);
    int getCacheSize() const;
    float getCacheHitRatio() const;

signals:
    // 连接状态信号
    void connectionChanged(bool connected);
    void databaseInitialized();
    void migrationStarted(const QString &version);
    void migrationCompleted(const QString &version);
    void migrationFailed(const QString &version, const QString &error);
    
    // 查询状态信号
    void queryStarted(const QString &sql);
    void queryCompleted(const QString &sql, qint64 elapsed);
    void queryFailed(const QString &sql, const QString &error);
    
    // 数据变更信号
    void dataRecordAdded(const QString &id);
    void dataRecordUpdated(const QString &id);
    void dataRecordDeleted(const QString &id);
    void userAdded(const QString &id);
    void userUpdated(const QString &id);
    void userDeleted(const QString &id);
    
    // 错误信号
    void databaseError(const QString &error);
    void connectionError(const QString &error);

private slots:
    void checkConnection();
    void onAsyncQueryFinished();
    void updateStatistics();

private:
    // 初始化和配置
    bool setupConnection();
    bool testConnection();
    bool initializeDatabase();
    
    // 迁移管理
    bool createMigrationTable();
    bool getCurrentVersion(QString &version);
    bool setCurrentVersion(const QString &version);
    bool applyMigration(const Migration &migration);
    QList<Migration> getAvailableMigrations() const;
    
    // 表创建
    bool createDataRecordTable();
    bool createUserTable();
    bool createCommentTable();
    bool createDataSharingTable();
    bool createPermissionTable();
    bool createRoleTable();
    bool createProjectTable();
    bool createAllTables();
    
    // 索引创建
    bool createDataRecordIndexes();
    bool createUserIndexes();
    bool createCommentIndexes();
    bool createDataSharingIndexes();
    bool createPermissionIndexes();
    bool createRoleIndexes();
    bool createProjectIndexes();
    bool createAllIndexes();
    
    // 查询辅助
    QSqlQuery prepareQuery(const QString &sql, const QVariantMap &params = {});
    QVariantMap recordToMap(const QSqlRecord &record);
    QString buildSelectQuery(const QString &table, const QStringList &fields = {}, 
                             const QString &where = "", const QString &orderBy = "", 
                             int limit = 0, int offset = 0);
    QString buildInsertQuery(const QString &table, const QVariantMap &values);
    QString buildUpdateQuery(const QString &table, const QVariantMap &values, const QString &where);
    QString buildDeleteQuery(const QString &table, const QString &where);
    
    // 缓存管理
    void addToCache(const QString &key, const QVariant &value);
    QVariant getFromCache(const QString &key);
    void removeFromCache(const QString &key);
    
    // 统计和监控
    void recordQueryStart(const QString &sql);
    void recordQueryEnd(const QString &sql, bool success, qint64 elapsed);
    
    // 连接池管理
    QSqlDatabase getConnection();
    void returnConnection(const QString &connectionName);
    QString generateConnectionName();
    
    // 成员变量
    DatabaseConfig m_config;
    QMap<QString, QSqlDatabase> m_connections;
    QMap<QString, QSqlDatabase> m_availableConnections;
    
    // 缓存
    QMap<QString, QVariant> m_cache;
    mutable QMutex m_cacheMutex;
    
    // 统计
    DatabaseStats m_stats;
    mutable QMutex m_statsMutex;
    
    // 异步查询
    QMap<QString, QFuture<void>> m_asyncQueries;
    QMap<QString, QueryCallback> m_asyncCallbacks;
    QMap<QString, ErrorCallback> m_asyncErrorCallbacks;
    
    // 定时器
    QTimer* m_connectionTimer;
    QTimer* m_statsTimer;
    
    // 线程安全
    mutable QMutex m_connectionMutex;
    mutable QMutex m_mutex;
    
    // 原子变量
    std::atomic<bool> m_connected{false};
    std::atomic<bool> m_initialized{false};
    
    // 线程池
    QThreadPool* m_threadPool;
    
    // 当前数据库版本
    QString m_currentVersion = "1.0.0";
};

} // namespace Services
} // namespace BondForge

#endif // BONDFORGE_DATABASESERVICE_H