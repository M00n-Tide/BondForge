#include "DatabaseService.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlDriver>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QMutexLocker>
#include <QDebug>
#include <QUuid>

#include "../core/data/DataRecord.h"
#include "../core/collaboration/User.h"
#include "../core/collaboration/DataSharing.h"
#include "../core/permissions/Permission.h"
#include "../core/permissions/Role.h"
#include "../utils/Logger.h"
#include "../utils/ConfigManager.h"

namespace BondForge {
namespace Services {

DatabaseService::DatabaseService(QObject *parent)
    : QObject(parent)
    , m_connectionTimer(new QTimer(this))
    , m_statsTimer(new QTimer(this))
    , m_threadPool(QThreadPool::globalInstance())
    , m_connected(false)
    , m_initialized(false)
{
    // 设置默认配置
    m_config.type = DatabaseType::SQLITE;
    m_config.databaseName = "bondforge.db";
    m_config.autoConnect = true;
    m_config.connectionTimeout = 5000;
    m_config.queryTimeout = 30000;
    m_config.maxConnections = 5;
    m_config.enableCache = true;
    m_config.cacheSize = 1000;
    
    // 连接定时器
    connect(m_connectionTimer, &QTimer::timeout, this, &DatabaseService::checkConnection);
    m_connectionTimer->start(60000); // 每分钟检查一次连接
    
    connect(m_statsTimer, &QTimer::timeout, this, &DatabaseService::updateStatistics);
    m_statsTimer->start(300000); // 每5分钟更新一次统计
    
    Utils::Logger::info("DatabaseService initialized");
}

DatabaseService::~DatabaseService()
{
    disconnect();
    
    // 清理连接池
    QMutexLocker locker(&m_connectionMutex);
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QSqlDatabase db = it.value();
        if (db.isOpen()) {
            db.close();
        }
    }
    m_connections.clear();
    m_availableConnections.clear();
    
    Utils::Logger::info("DatabaseService destroyed");
}

void DatabaseService::setConfig(const DatabaseConfig &config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
    
    // 如果已经连接，断开并重新连接
    if (m_connected) {
        disconnect();
        if (config.autoConnect) {
            connect();
        }
    }
}

DatabaseConfig DatabaseService::config() const
{
    QMutexLocker locker(&m_mutex);
    return m_config;
}

bool DatabaseService::connect()
{
    QMutexLocker locker(&m_connectionMutex);
    
    if (m_connected) {
        return true;
    }
    
    // 获取一个连接用于测试
    QSqlDatabase db = getConnection();
    
    if (!db.isOpen()) {
        Utils::Logger::error("Failed to open database connection");
        emit connectionChanged(false);
        return false;
    }
    
    m_connected = true;
    emit connectionChanged(true);
    
    // 初始化数据库
    if (!initializeDatabase()) {
        Utils::Logger::error("Failed to initialize database");
        m_connected = false;
        emit connectionChanged(false);
        return false;
    }
    
    Utils::Logger::info("Database connection established");
    return true;
}

bool DatabaseService::disconnect()
{
    QMutexLocker locker(&m_connectionMutex);
    
    if (!m_connected) {
        return true;
    }
    
    // 关闭所有连接
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QSqlDatabase db = it.value();
        if (db.isOpen()) {
            db.close();
        }
    }
    m_connections.clear();
    m_availableConnections.clear();
    
    m_connected = false;
    emit connectionChanged(false);
    
    Utils::Logger::info("Database connection closed");
    return true;
}

bool DatabaseService::reconnect()
{
    disconnect();
    return connect();
}

bool DatabaseService::isConnected() const
{
    return m_connected.load();
}

bool DatabaseService::isReady() const
{
    return m_connected.load() && m_initialized.load();
}

bool DatabaseService::createDatabase()
{
    QMutexLocker locker(&m_connectionMutex);
    
    if (!m_connected) {
        return false;
    }
    
    // SQLite不需要预先创建数据库，只需要创建表
    if (m_config.type == DatabaseType::SQLITE) {
        return createAllTables();
    }
    
    // 对于其他数据库，需要创建数据库
    // 这里暂时简化实现
    return true;
}

bool DatabaseService::initializeDatabase()
{
    if (m_initialized) {
        return true;
    }
    
    if (!createAllTables()) {
        return false;
    }
    
    if (!createAllIndexes()) {
        return false;
    }
    
    if (!createMigrationTable()) {
        return false;
    }
    
    // 检查并应用迁移
    QString currentVersion;
    if (!getCurrentVersion(currentVersion)) {
        // 如果没有版本信息，设置当前版本
        setCurrentVersion(m_currentVersion);
    } else if (currentVersion != m_currentVersion) {
        // 需要迁移
        if (!applyMigration(getCurrentMigration())) {
            return false;
        }
    }
    
    m_initialized = true;
    emit databaseInitialized();
    
    Utils::Logger::info("Database initialized successfully");
    return true;
}

bool DatabaseService::executeMigration(const QString &version)
{
    Q_UNUSED(version)
    // 这里可以实现特定版本的迁移
    // 暂时简化实现
    return true;
}

bool DatabaseService::rollbackMigration(const QString &version)
{
    Q_UNUSED(version)
    // 这里可以实现特定版本的回滚
    // 暂时简化实现
    return true;
}

QStringList DatabaseService::pendingMigrations() const
{
    // 这里可以实现待迁移的版本列表
    // 暂时简化实现
    return QStringList();
}

// 数据记录管理
bool DatabaseService::saveDataRecord(const Core::Data::DataRecord &record)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    // 检查记录是否已存在
    query.prepare("SELECT COUNT(*) FROM data_records WHERE id = ?");
    query.addBindValue(QString::fromStdString(record.id));
    
    if (!query.exec() || !query.next()) {
        Utils::Logger::error("Failed to check data record existence: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    bool exists = query.value(0).toInt() > 0;
    
    if (exists) {
        // 更新现有记录
        query.prepare(R"(
            UPDATE data_records SET 
                name = ?, format = ?, category = ?, content = ?, 
                modified_at = ?, metadata = ?
            WHERE id = ?
        )");
        
        query.addBindValue(QString::fromStdString(record.name));
        query.addBindValue(QString::fromStdString(record.format));
        query.addBindValue(QString::fromStdString(record.category));
        query.addBindValue(QString::fromStdString(record.content));
        query.addBindValue(static_cast<qint64>(record.modifiedAt));
        query.addBindValue(QString::fromStdString(record.metadataToJson()));
        query.addBindValue(QString::fromStdString(record.id));
    } else {
        // 插入新记录
        query.prepare(R"(
            INSERT INTO data_records (
                id, name, format, category, content, 
                created_at, modified_at, metadata
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        )");
        
        query.addBindValue(QString::fromStdString(record.id));
        query.addBindValue(QString::fromStdString(record.name));
        query.addBindValue(QString::fromStdString(record.format));
        query.addBindValue(QString::fromStdString(record.category));
        query.addBindValue(QString::fromStdString(record.content));
        query.addBindValue(static_cast<qint64>(record.createdAt));
        query.addBindValue(static_cast<qint64>(record.modifiedAt));
        query.addBindValue(QString::fromStdString(record.metadataToJson()));
    }
    
    if (!query.exec()) {
        Utils::Logger::error("Failed to save data record: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    
    if (exists) {
        emit dataRecordUpdated(QString::fromStdString(record.id));
    } else {
        emit dataRecordAdded(QString::fromStdString(record.id));
    }
    
    return true;
}

bool DatabaseService::loadDataRecord(const QString &id, Core::Data::DataRecord &record)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT * FROM data_records WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec() || !query.next()) {
        Utils::Logger::error("Failed to load data record: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    QSqlRecord rec = query.record();
    
    record.id = rec.value("id").toString().toStdString();
    record.name = rec.value("name").toString().toStdString();
    record.format = rec.value("format").toString().toStdString();
    record.category = rec.value("category").toString().toStdString();
    record.content = rec.value("content").toString().toStdString();
    record.createdAt = rec.value("created_at").toULongLong();
    record.modifiedAt = rec.value("modified_at").toULongLong();
    
    QString metadataJson = rec.value("metadata").toString();
    if (!metadataJson.isEmpty()) {
        record.metadataFromJson(metadataJson.toStdString());
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::updateDataRecord(const Core::Data::DataRecord &record)
{
    return saveDataRecord(record); // saveDataRecord 已经处理更新逻辑
}

bool DatabaseService::deleteDataRecord(const QString &id)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("DELETE FROM data_records WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        Utils::Logger::error("Failed to delete data record: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    emit dataRecordDeleted(id);
    
    return true;
}

QList<Core::Data::DataRecord> DatabaseService::findDataRecords(const QString &filter, const QString &orderBy, int limit)
{
    QList<Core::Data::DataRecord> records;
    
    if (!isReady()) {
        return records;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return records;
    }
    
    QString sql = "SELECT * FROM data_records";
    
    if (!filter.isEmpty()) {
        sql += " WHERE " + filter;
    }
    
    if (!orderBy.isEmpty()) {
        sql += " ORDER BY " + orderBy;
    }
    
    if (limit > 0) {
        sql += " LIMIT " + QString::number(limit);
    }
    
    QSqlQuery query(db);
    query.prepare(sql);
    
    if (!query.exec()) {
        Utils::Logger::error("Failed to find data records: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return records;
    }
    
    while (query.next()) {
        Core::Data::DataRecord record;
        QSqlRecord rec = query.record();
        
        record.id = rec.value("id").toString().toStdString();
        record.name = rec.value("name").toString().toStdString();
        record.format = rec.value("format").toString().toStdString();
        record.category = rec.value("category").toString().toStdString();
        record.content = rec.value("content").toString().toStdString();
        record.createdAt = rec.value("created_at").toULongLong();
        record.modifiedAt = rec.value("modified_at").toULongLong();
        
        QString metadataJson = rec.value("metadata").toString();
        if (!metadataJson.isEmpty()) {
            record.metadataFromJson(metadataJson.toStdString());
        }
        
        records.append(record);
    }
    
    returnConnection(db.connectionName());
    return records;
}

QList<Core::Data::DataRecord> DatabaseService::findDataRecordsByCategory(const QString &category)
{
    return findDataRecords("category = '" + category + "'", "name");
}

QList<Core::Data::DataRecord> DatabaseService::findDataRecordsByFormat(const QString &format)
{
    return findDataRecords("format = '" + format + "'", "name");
}

QList<Core::Data::DataRecord> DatabaseService::searchDataRecords(const QString &searchTerm)
{
    QString filter = QString("name LIKE '%%1%' OR content LIKE '%%1%'").arg(searchTerm);
    return findDataRecords(filter, "name");
}

int DatabaseService::getDataRecordsCount(const QString &filter)
{
    if (!isReady()) {
        return 0;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return 0;
    }
    
    QString sql = "SELECT COUNT(*) FROM data_records";
    
    if (!filter.isEmpty()) {
        sql += " WHERE " + filter;
    }
    
    QSqlQuery query(db);
    query.prepare(sql);
    
    if (!query.exec() || !query.next()) {
        Utils::Logger::error("Failed to get data records count: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return 0;
    }
    
    int count = query.value(0).toInt();
    
    returnConnection(db.connectionName());
    return count;
}

// 用户管理
bool DatabaseService::saveUser(const Core::Collaboration::User &user)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    // 检查用户是否已存在
    query.prepare("SELECT COUNT(*) FROM users WHERE id = ?");
    query.addBindValue(QString::fromStdString(user.id));
    
    if (!query.exec() || !query.next()) {
        Utils::Logger::error("Failed to check user existence: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    bool exists = query.value(0).toInt() > 0;
    
    if (exists) {
        // 更新现有用户
        query.prepare(R"(
            UPDATE users SET 
                username = ?, name = ?, email = ?, role = ?, status = ?, 
                department = ?, modified_at = ?
            WHERE id = ?
        )");
        
        query.addBindValue(QString::fromStdString(user.username));
        query.addBindValue(QString::fromStdString(user.name));
        query.addBindValue(QString::fromStdString(user.email));
        query.addBindValue(QString::fromStdString(user.role));
        query.addBindValue(static_cast<int>(user.status));
        query.addBindValue(QString::fromStdString(user.department));
        query.addBindValue(static_cast<qint64>(user.modifiedAt));
        query.addBindValue(QString::fromStdString(user.id));
    } else {
        // 插入新用户
        query.prepare(R"(
            INSERT INTO users (
                id, username, name, email, password_hash, role, status, 
                department, created_at, modified_at
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )");
        
        query.addBindValue(QString::fromStdString(user.id));
        query.addBindValue(QString::fromStdString(user.username));
        query.addBindValue(QString::fromStdString(user.name));
        query.addBindValue(QString::fromStdString(user.email));
        query.addBindValue(QString::fromStdString(user.passwordHash));
        query.addBindValue(QString::fromStdString(user.role));
        query.addBindValue(static_cast<int>(user.status));
        query.addBindValue(QString::fromStdString(user.department));
        query.addBindValue(static_cast<qint64>(user.createdAt));
        query.addBindValue(static_cast<qint64>(user.modifiedAt));
    }
    
    if (!query.exec()) {
        Utils::Logger::error("Failed to save user: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    
    if (exists) {
        emit userUpdated(QString::fromStdString(user.id));
    } else {
        emit userAdded(QString::fromStdString(user.id));
    }
    
    return true;
}

bool DatabaseService::loadUser(const QString &id, Core::Collaboration::User &user)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec() || !query.next()) {
        Utils::Logger::error("Failed to load user: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    QSqlRecord rec = query.record();
    
    user.id = rec.value("id").toString().toStdString();
    user.username = rec.value("username").toString().toStdString();
    user.name = rec.value("name").toString().toStdString();
    user.email = rec.value("email").toString().toStdString();
    user.passwordHash = rec.value("password_hash").toString().toStdString();
    user.role = rec.value("role").toString().toStdString();
    user.status = static_cast<Core::Collaboration::UserStatus>(rec.value("status").toInt());
    user.department = rec.value("department").toString().toStdString();
    user.createdAt = rec.value("created_at").toULongLong();
    user.modifiedAt = rec.value("modified_at").toULongLong();
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::loadUserByUsername(const QString &username, Core::Collaboration::User &user)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (!query.exec() || !query.next()) {
        Utils::Logger::error("Failed to load user by username: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    QSqlRecord rec = query.record();
    
    user.id = rec.value("id").toString().toStdString();
    user.username = rec.value("username").toString().toStdString();
    user.name = rec.value("name").toString().toStdString();
    user.email = rec.value("email").toString().toStdString();
    user.passwordHash = rec.value("password_hash").toString().toStdString();
    user.role = rec.value("role").toString().toStdString();
    user.status = static_cast<Core::Collaboration::UserStatus>(rec.value("status").toInt());
    user.department = rec.value("department").toString().toStdString();
    user.createdAt = rec.value("created_at").toULongLong();
    user.modifiedAt = rec.value("modified_at").toULongLong();
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::updateUser(const Core::Collaboration::User &user)
{
    return saveUser(user); // saveUser 已经处理更新逻辑
}

bool DatabaseService::deleteUser(const QString &id)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("DELETE FROM users WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        Utils::Logger::error("Failed to delete user: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    emit userDeleted(id);
    
    return true;
}

QList<Core::Collaboration::User> DatabaseService::getAllUsers()
{
    QList<Core::Collaboration::User> users;
    
    if (!isReady()) {
        return users;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return users;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users ORDER BY name");
    
    if (!query.exec()) {
        Utils::Logger::error("Failed to get all users: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return users;
    }
    
    while (query.next()) {
        Core::Collaboration::User user;
        QSqlRecord rec = query.record();
        
        user.id = rec.value("id").toString().toStdString();
        user.username = rec.value("username").toString().toStdString();
        user.name = rec.value("name").toString().toStdString();
        user.email = rec.value("email").toString().toStdString();
        user.passwordHash = rec.value("password_hash").toString().toStdString();
        user.role = rec.value("role").toString().toStdString();
        user.status = static_cast<Core::Collaboration::UserStatus>(rec.value("status").toInt());
        user.department = rec.value("department").toString().toStdString();
        user.createdAt = rec.value("created_at").toULongLong();
        user.modifiedAt = rec.value("modified_at").toULongLong();
        
        users.append(user);
    }
    
    returnConnection(db.connectionName());
    return users;
}

// 原始SQL查询
QueryResult DatabaseService::executeQuery(const QString &sql, const QVariantMap &params)
{
    QueryResult result;
    
    if (!isReady()) {
        result.errorString = "Database not ready";
        return result;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        result.errorString = "Failed to get database connection";
        return result;
    }
    
    QSqlQuery query(db);
    query.prepare(sql);
    
    // 绑定参数
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(it.key(), it.value());
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (!query.exec()) {
        result.errorString = query.lastError().text();
        recordQueryEnd(sql, false, 0);
        returnConnection(db.connectionName());
        return result;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    
    result.success = true;
    result.query = query;
    result.affectedRows = query.numRowsAffected();
    
    if (query.driver()->hasFeature(QSqlDriver::QuerySize)) {
        result.affectedRows = query.size();
    }
    
    recordQueryEnd(sql, true, duration);
    returnConnection(db.connectionName());
    
    return result;
}

QueryResult DatabaseService::executeSelectQuery(const QString &sql, const QVariantMap &params)
{
    QueryResult result = executeQuery(sql, params);
    
    if (result.success && result.query.lastError().type() != QSqlError::NoError) {
        result.success = false;
        result.errorString = result.query.lastError().text();
    }
    
    return result;
}

QueryResult DatabaseService::executeInsertQuery(const QString &sql, const QVariantMap &params)
{
    QueryResult result = executeQuery(sql, params);
    
    if (result.success) {
        result.lastInsertId = result.query.lastInsertId();
    }
    
    return result;
}

QueryResult DatabaseService::executeUpdateQuery(const QString &sql, const QVariantMap &params)
{
    return executeQuery(sql, params);
}

QueryResult DatabaseService::executeDeleteQuery(const QString &sql, const QVariantMap &params)
{
    return executeQuery(sql, params);
}

// 事务管理
bool DatabaseService::beginTransaction()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    if (!db.transaction()) {
        Utils::Logger::error("Failed to begin transaction: " + db.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::commitTransaction()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    if (!db.commit()) {
        Utils::Logger::error("Failed to commit transaction: " + db.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::rollbackTransaction()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    if (!db.rollback()) {
        Utils::Logger::error("Failed to rollback transaction: " + db.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

// 数据库优化
bool DatabaseService::vacuumDatabase()
{
    if (!isReady()) {
        return false;
    }
    
    if (m_config.type != DatabaseType::SQLITE) {
        Utils::Logger::warning("VACUUM is only supported for SQLite databases");
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    if (!query.exec("VACUUM")) {
        Utils::Logger::error("Failed to vacuum database: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    Utils::Logger::info("Database vacuumed successfully");
    return true;
}

bool DatabaseService::analyzeDatabase()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    if (m_config.type == DatabaseType::SQLITE) {
        if (!query.exec("ANALYZE")) {
            Utils::Logger::error("Failed to analyze database: " + query.lastError().text().toStdString());
            returnConnection(db.connectionName());
            return false;
        }
    } else {
        // 对于其他数据库，需要使用特定的ANALYZE语句
        // 这里暂时简化实现
    }
    
    returnConnection(db.connectionName());
    Utils::Logger::info("Database analyzed successfully");
    return true;
}

bool DatabaseService::reindexDatabase()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    if (m_config.type == DatabaseType::SQLITE) {
        if (!query.exec("REINDEX")) {
            Utils::Logger::error("Failed to reindex database: " + query.lastError().text().toStdString());
            returnConnection(db.connectionName());
            return false;
        }
    } else {
        // 对于其他数据库，需要使用特定的REINDEX语句
        // 这里暂时简化实现
    }
    
    returnConnection(db.connectionName());
    Utils::Logger::info("Database reindexed successfully");
    return true;
}

bool DatabaseService::optimizeDatabase()
{
    bool success = true;
    
    if (!vacuumDatabase()) {
        success = false;
    }
    
    if (!analyzeDatabase()) {
        success = false;
    }
    
    if (!reindexDatabase()) {
        success = false;
    }
    
    return success;
}

// 数据库统计
DatabaseStats DatabaseService::getStatistics() const
{
    QMutexLocker locker(&m_statsMutex);
    return m_stats;
}

void DatabaseService::resetStatistics()
{
    QMutexLocker locker(&m_statsMutex);
    m_stats = DatabaseStats();
}

QString DatabaseService::getDatabaseInfo() const
{
    if (!isReady()) {
        return "Database not ready";
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return "Failed to get database connection";
    }
    
    QString info;
    info += QString("Database Type: %1\n").arg(static_cast<int>(m_config.type));
    info += QString("Database Name: %1\n").arg(m_config.databaseName);
    info += QString("Host: %1\n").arg(m_config.host);
    info += QString("Port: %1\n").arg(m_config.port);
    info += QString("Driver: %1\n").arg(db.driverName());
    info += QString("Connection Options: %1\n").arg(m_config.connectionOptions);
    
    returnConnection(db.connectionName());
    return info;
}

QString DatabaseService::getTableInfo(const QString &tableName) const
{
    if (!isReady()) {
        return "Database not ready";
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return "Failed to get database connection";
    }
    
    QSqlRecord record = db.record(tableName);
    QString info;
    info += QString("Table: %1\n").arg(tableName);
    info += QString("Columns: %1\n\n").arg(record.count());
    
    for (int i = 0; i < record.count(); ++i) {
        QSqlField field = record.field(i);
        info += QString("%1: %2\n").arg(field.name(), field.typeID());
    }
    
    returnConnection(db.connectionName());
    return info;
}

// 缓存管理
void DatabaseService::clearCache()
{
    QMutexLocker locker(&m_cacheMutex);
    m_cache.clear();
}

void DatabaseService::setCacheSize(int size)
{
    QMutexLocker locker(&m_cacheMutex);
    m_config.cacheSize = size;
    
    // 如果当前缓存大小超过了新的大小，删除一些条目
    while (m_cache.size() > size) {
        m_cache.erase(m_cache.begin());
    }
}

int DatabaseService::getCacheSize() const
{
    QMutexLocker locker(&m_cacheMutex);
    return m_config.cacheSize;
}

float DatabaseService::getCacheHitRatio() const
{
    QMutexLocker locker(&m_statsMutex);
    
    if (m_stats.cacheHits + m_stats.cacheMisses == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(m_stats.cacheHits) / (m_stats.cacheHits + m_stats.cacheMisses);
}

// 私有方法
void DatabaseService::setupConnection()
{
    // 在连接池中创建新连接
    QString connectionName = generateConnectionName();
    
    QSqlDatabase db;
    
    if (m_config.type == DatabaseType::SQLITE) {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        
        // 设置SQLite特定选项
        QString dbPath = m_config.databaseName;
        if (QDir::isRelativePath(dbPath)) {
            QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            dbPath = QDir(appDataPath).filePath(dbPath);
        }
        
        db.setDatabaseName(dbPath);
        
        // 启用WAL和外键约束
        if (m_config.enableWAL) {
            db.exec("PRAGMA journal_mode=WAL");
        }
        
        if (m_config.enableForeignKeys) {
            db.exec("PRAGMA foreign_keys=ON");
        }
    } else {
        // 其他数据库类型的连接设置
        QString driverName;
        
        switch (m_config.type) {
            case DatabaseType::MYSQL:
                driverName = "QMYSQL";
                break;
            case DatabaseType::POSTGRESQL:
                driverName = "QPSQL";
                break;
            case DatabaseType::ORACLE:
                driverName = "QOCI";
                break;
            case DatabaseType::SQLSERVER:
                driverName = "QODBC";
                break;
            default:
                driverName = "QSQLITE";
                break;
        }
        
        db = QSqlDatabase::addDatabase(driverName, connectionName);
        db.setHostName(m_config.host);
        db.setPort(m_config.port);
        db.setDatabaseName(m_config.databaseName);
        db.setUserName(m_config.username);
        db.setPassword(m_config.password);
        
        if (!m_config.connectionOptions.isEmpty()) {
            db.setConnectOptions(m_config.connectionOptions);
        }
    }
    
    // 添加到连接池
    m_connections[connectionName] = db;
    
    Utils::Logger::info("Created database connection: " + connectionName.toStdString());
}

bool DatabaseService::testConnection()
{
    QString connectionName = generateConnectionName();
    
    QSqlDatabase db;
    
    if (m_config.type == DatabaseType::SQLITE) {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        
        QString dbPath = m_config.databaseName;
        if (QDir::isRelativePath(dbPath)) {
            QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            dbPath = QDir(appDataPath).filePath(dbPath);
        }
        
        db.setDatabaseName(dbPath);
    } else {
        QString driverName;
        
        switch (m_config.type) {
            case DatabaseType::MYSQL:
                driverName = "QMYSQL";
                break;
            case DatabaseType::POSTGRESQL:
                driverName = "QPSQL";
                break;
            case DatabaseType::ORACLE:
                driverName = "QOCI";
                break;
            case DatabaseType::SQLSERVER:
                driverName = "QODBC";
                break;
            default:
                driverName = "QSQLITE";
                break;
        }
        
        db = QSqlDatabase::addDatabase(driverName, connectionName);
        db.setHostName(m_config.host);
        db.setPort(m_config.port);
        db.setDatabaseName(m_config.databaseName);
        db.setUserName(m_config.username);
        db.setPassword(m_config.password);
        
        if (!m_config.connectionOptions.isEmpty()) {
            db.setConnectOptions(m_config.connectionOptions);
        }
    }
    
    bool success = db.open();
    
    if (success) {
        db.close();
    }
    
    QSqlDatabase::removeDatabase(connectionName);
    
    return success;
}

QSqlDatabase DatabaseService::getConnection()
{
    QMutexLocker locker(&m_connectionMutex);
    
    // 首先检查是否有可用的连接
    if (!m_availableConnections.isEmpty()) {
        QString connectionName = m_availableConnections.takeFirst();
        return QSqlDatabase::database(connectionName, false);
    }
    
    // 如果没有可用连接，创建新连接
    if (m_connections.size() < m_config.maxConnections) {
        setupConnection();
        
        // 获取最后添加的连接
        QString connectionName = m_connections.keys().last();
        return QSqlDatabase::database(connectionName, false);
    }
    
    // 如果所有连接都在使用中，返回一个空数据库
    Utils::Logger::warning("All database connections are in use");
    return QSqlDatabase();
}

void DatabaseService::returnConnection(const QString &connectionName)
{
    if (connectionName.isEmpty()) {
        return;
    }
    
    QMutexLocker locker(&m_connectionMutex);
    
    if (m_connections.contains(connectionName)) {
        m_availableConnections.append(connectionName);
    }
}

QString DatabaseService::generateConnectionName()
{
    return QString("BondForgeDB_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

void DatabaseService::recordQueryStart(const QString &sql)
{
    Q_UNUSED(sql)
    // 这里可以添加查询开始的记录
}

void DatabaseService::recordQueryEnd(const QString &sql, bool success, qint64 elapsed)
{
    QMutexLocker locker(&m_statsMutex);
    
    m_stats.totalQueries++;
    
    if (success) {
        m_stats.successfulQueries++;
    } else {
        m_stats.failedQueries++;
    }
    
    m_stats.totalQueryTime += elapsed;
    m_stats.avgQueryTime = m_stats.totalQueryTime / m_stats.totalQueries;
}

void DatabaseService::checkConnection()
{
    if (!m_connected) {
        return;
    }
    
    // 测试连接
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        Utils::Logger::warning("Database connection lost, attempting to reconnect...");
        m_connected = false;
        emit connectionChanged(false);
        
        // 尝试重新连接
        QTimer::singleShot(1000, this, [this]() {
            reconnect();
        });
    } else {
        returnConnection(db.connectionName());
    }
}

void DatabaseService::updateStatistics()
{
    if (!isReady()) {
        return;
    }
    
    QMutexLocker locker(&m_statsMutex);
    
    // 更新活跃连接数
    m_stats.activeConnections = m_connections.size() - m_availableConnections.size();
    m_stats.totalConnections = m_connections.size();
    
    // 发出统计更新信号
    // 这里可以添加信号发射
}

// 表创建方法
bool DatabaseService::createAllTables()
{
    return createDataRecordTable() &&
           createUserTable() &&
           createCommentTable() &&
           createDataSharingTable() &&
           createPermissionTable() &&
           createRoleTable() &&
           createProjectTable();
}

bool DatabaseService::createDataRecordTable()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS data_records (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            format TEXT NOT NULL,
            category TEXT NOT NULL,
            content TEXT NOT NULL,
            created_at INTEGER NOT NULL,
            modified_at INTEGER NOT NULL,
            metadata TEXT
        )
    )";
    
    if (!query.exec(sql)) {
        Utils::Logger::error("Failed to create data_records table: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createUserTable()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            email TEXT UNIQUE,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL,
            status INTEGER NOT NULL DEFAULT 0,
            department TEXT,
            created_at INTEGER NOT NULL,
            modified_at INTEGER NOT NULL
        )
    )";
    
    if (!query.exec(sql)) {
        Utils::Logger::error("Failed to create users table: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createCommentTable()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS comments (
            id TEXT PRIMARY KEY,
            record_id TEXT NOT NULL,
            user_id TEXT NOT NULL,
            content TEXT NOT NULL,
            parent_id TEXT,
            created_at INTEGER NOT NULL,
            modified_at INTEGER NOT NULL,
            FOREIGN KEY (record_id) REFERENCES data_records (id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE,
            FOREIGN KEY (parent_id) REFERENCES comments (id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(sql)) {
        Utils::Logger::error("Failed to create comments table: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createDataSharingTable()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS data_sharing (
            id TEXT PRIMARY KEY,
            record_id TEXT NOT NULL,
            owner_id TEXT NOT NULL,
            sharee_id TEXT NOT NULL,
            permission TEXT NOT NULL,
            expires_at INTEGER,
            created_at INTEGER NOT NULL,
            modified_at INTEGER NOT NULL,
            FOREIGN KEY (record_id) REFERENCES data_records (id) ON DELETE CASCADE,
            FOREIGN KEY (owner_id) REFERENCES users (id) ON DELETE CASCADE,
            FOREIGN KEY (sharee_id) REFERENCES users (id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(sql)) {
        Utils::Logger::error("Failed to create data_sharing table: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createPermissionTable()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS permissions (
            id TEXT PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            description TEXT,
            resource_type TEXT NOT NULL,
            action TEXT NOT NULL,
            conditions TEXT,
            created_at INTEGER NOT NULL,
            modified_at INTEGER NOT NULL
        )
    )";
    
    if (!query.exec(sql)) {
        Utils::Logger::error("Failed to create permissions table: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createRoleTable()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS roles (
            id TEXT PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            description TEXT,
            permissions TEXT,
            created_at INTEGER NOT NULL,
            modified_at INTEGER NOT NULL
        )
    )";
    
    if (!query.exec(sql)) {
        Utils::Logger::error("Failed to create roles table: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createProjectTable()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS projects (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            owner_id TEXT NOT NULL,
            created_at INTEGER NOT NULL,
            modified_at INTEGER NOT NULL,
            FOREIGN KEY (owner_id) REFERENCES users (id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(sql)) {
        Utils::Logger::error("Failed to create projects table: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

// 索引创建方法
bool DatabaseService::createAllIndexes()
{
    return createDataRecordIndexes() &&
           createUserIndexes() &&
           createCommentIndexes() &&
           createDataSharingIndexes() &&
           createPermissionIndexes() &&
           createRoleIndexes() &&
           createProjectIndexes();
}

bool DatabaseService::createDataRecordIndexes()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_data_records_name ON data_records (name)",
        "CREATE INDEX IF NOT EXISTS idx_data_records_format ON data_records (format)",
        "CREATE INDEX IF NOT EXISTS idx_data_records_category ON data_records (category)",
        "CREATE INDEX IF NOT EXISTS idx_data_records_created_at ON data_records (created_at)",
        "CREATE INDEX IF NOT EXISTS idx_data_records_modified_at ON data_records (modified_at)"
    };
    
    for (const QString &sql : indexes) {
        if (!query.exec(sql)) {
            Utils::Logger::error("Failed to create data_record index: " + query.lastError().text().toStdString());
            returnConnection(db.connectionName());
            return false;
        }
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createUserIndexes()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_users_username ON users (username)",
        "CREATE INDEX IF NOT EXISTS idx_users_email ON users (email)",
        "CREATE INDEX IF NOT EXISTS idx_users_role ON users (role)",
        "CREATE INDEX IF NOT EXISTS idx_users_status ON users (status)",
        "CREATE INDEX IF NOT EXISTS idx_users_created_at ON users (created_at)"
    };
    
    for (const QString &sql : indexes) {
        if (!query.exec(sql)) {
            Utils::Logger::error("Failed to create user index: " + query.lastError().text().toStdString());
            returnConnection(db.connectionName());
            return false;
        }
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createCommentIndexes()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_comments_record_id ON comments (record_id)",
        "CREATE INDEX IF NOT EXISTS idx_comments_user_id ON comments (user_id)",
        "CREATE INDEX IF NOT EXISTS idx_comments_parent_id ON comments (parent_id)",
        "CREATE INDEX IF NOT EXISTS idx_comments_created_at ON comments (created_at)"
    };
    
    for (const QString &sql : indexes) {
        if (!query.exec(sql)) {
            Utils::Logger::error("Failed to create comment index: " + query.lastError().text().toStdString());
            returnConnection(db.connectionName());
            return false;
        }
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createDataSharingIndexes()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_data_sharing_record_id ON data_sharing (record_id)",
        "CREATE INDEX IF NOT EXISTS idx_data_sharing_owner_id ON data_sharing (owner_id)",
        "CREATE INDEX IF NOT EXISTS idx_data_sharing_sharee_id ON data_sharing (sharee_id)",
        "CREATE INDEX IF NOT EXISTS idx_data_sharing_permission ON data_sharing (permission)",
        "CREATE INDEX IF NOT EXISTS idx_data_sharing_expires_at ON data_sharing (expires_at)"
    };
    
    for (const QString &sql : indexes) {
        if (!query.exec(sql)) {
            Utils::Logger::error("Failed to create data_sharing index: " + query.lastError().text().toStdString());
            returnConnection(db.connectionName());
            return false;
        }
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createPermissionIndexes()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_permissions_name ON permissions (name)",
        "CREATE INDEX IF NOT EXISTS idx_permissions_resource_type ON permissions (resource_type)",
        "CREATE INDEX IF NOT EXISTS idx_permissions_action ON permissions (action)"
    };
    
    for (const QString &sql : indexes) {
        if (!query.exec(sql)) {
            Utils::Logger::error("Failed to create permission index: " + query.lastError().text().toStdString());
            returnConnection(db.connectionName());
            return false;
        }
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createRoleIndexes()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_roles_name ON roles (name)"
    };
    
    for (const QString &sql : indexes) {
        if (!query.exec(sql)) {
            Utils::Logger::error("Failed to create role index: " + query.lastError().text().toStdString());
            returnConnection(db.connectionName());
            return false;
        }
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::createProjectIndexes()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_projects_name ON projects (name)",
        "CREATE INDEX IF NOT EXISTS idx_projects_owner_id ON projects (owner_id)",
        "CREATE INDEX IF NOT EXISTS idx_projects_created_at ON projects (created_at)"
    };
    
    for (const QString &sql : indexes) {
        if (!query.exec(sql)) {
            Utils::Logger::error("Failed to create project index: " + query.lastError().text().toStdString());
            returnConnection(db.connectionName());
            return false;
        }
    }
    
    returnConnection(db.connectionName());
    return true;
}

// 迁移管理
bool DatabaseService::createMigrationTable()
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS migrations (
            version TEXT PRIMARY KEY,
            applied_at INTEGER NOT NULL,
            description TEXT
        )
    )";
    
    if (!query.exec(sql)) {
        Utils::Logger::error("Failed to create migrations table: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::getCurrentVersion(QString &version)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT version FROM migrations ORDER BY applied_at DESC LIMIT 1");
    
    if (!query.exec()) {
        Utils::Logger::error("Failed to get current version: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    if (query.next()) {
        version = query.value(0).toString();
    } else {
        version = "";
    }
    
    returnConnection(db.connectionName());
    return true;
}

bool DatabaseService::setCurrentVersion(const QString &version)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("INSERT OR REPLACE INTO migrations (version, applied_at, description) VALUES (?, ?, ?)");
    query.addBindValue(version);
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(QString("Current version: %1").arg(version));
    
    if (!query.exec()) {
        Utils::Logger::error("Failed to set current version: " + query.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    return true;
}

Migration DatabaseService::getCurrentMigration() const
{
    // 这里暂时返回一个空的迁移对象
    // 实际实现中，这里应该返回当前版本的迁移对象
    Migration migration;
    migration.version = m_currentVersion;
    migration.description = "Current version";
    migration.upSql = "";
    migration.downSql = "";
    
    return migration;
}

QList<Migration> DatabaseService::getAvailableMigrations() const
{
    // 这里暂时返回一个空的迁移列表
    // 实际实现中，这里应该返回所有可用的迁移对象
    return QList<Migration>();
}

bool DatabaseService::applyMigration(const Migration &migration)
{
    if (!isReady()) {
        return false;
    }
    
    QSqlDatabase db = getConnection();
    if (!db.isOpen()) {
        return false;
    }
    
    QSqlQuery query(db);
    
    // 开始事务
    if (!db.transaction()) {
        Utils::Logger::error("Failed to begin transaction for migration: " + db.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    // 执行迁移SQL
    if (!query.exec(migration.upSql)) {
        Utils::Logger::error("Failed to execute migration SQL: " + query.lastError().text().toStdString());
        db.rollback();
        returnConnection(db.connectionName());
        return false;
    }
    
    // 记录迁移
    query.prepare("INSERT INTO migrations (version, applied_at, description) VALUES (?, ?, ?)");
    query.addBindValue(migration.version);
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(migration.description);
    
    if (!query.exec()) {
        Utils::Logger::error("Failed to record migration: " + query.lastError().text().toStdString());
        db.rollback();
        returnConnection(db.connectionName());
        return false;
    }
    
    // 提交事务
    if (!db.commit()) {
        Utils::Logger::error("Failed to commit migration transaction: " + db.lastError().text().toStdString());
        returnConnection(db.connectionName());
        return false;
    }
    
    returnConnection(db.connectionName());
    
    Utils::Logger::info("Applied migration: " + migration.version.toStdString());
    emit migrationStarted(migration.version);
    emit migrationCompleted(migration.version);
    
    return true;
}

// 缓存管理
void DatabaseService::addToCache(const QString &key, const QVariant &value)
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (m_cache.size() >= m_config.cacheSize) {
        // 删除最旧的条目
        m_cache.erase(m_cache.begin());
    }
    
    m_cache[key] = value;
}

QVariant DatabaseService::getFromCache(const QString &key)
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (m_cache.contains(key)) {
        m_stats.cacheHits++;
        return m_cache[key];
    }
    
    m_stats.cacheMisses++;
    return QVariant();
}

void DatabaseService::removeFromCache(const QString &key)
{
    QMutexLocker locker(&m_cacheMutex);
    m_cache.remove(key);
}

} // namespace Services
} // namespace BondForge