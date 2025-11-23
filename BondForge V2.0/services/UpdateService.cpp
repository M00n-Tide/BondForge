#include "UpdateService.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QMessageBox>
#include <QProcess>

namespace Services {

UpdateService::UpdateService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_checkTimer(new QTimer(this))
    , m_autoUpdateEnabled(true)
    , m_schedulerRunning(false)
{
    // 设置检查间隔为24小时
    m_checkTimer->setInterval(24 * 60 * 60 * 1000);
    connect(m_checkTimer, &QTimer::timeout, this, &UpdateService::onCheckTimer);
    
    // 设置路径
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_configPath = appDataPath + "/config/update_config.json";
    m_logPath = appDataPath + "/logs/update_log.txt";
    m_downloadPath = appDataPath + "/downloads";
    
    // 确保目录存在
    QDir dir;
    if (!dir.exists(m_downloadPath)) {
        dir.mkpath(m_downloadPath);
    }
    
    // 初始化全局进度
    m_globalProgress = {0, 0, "", ""};
}

UpdateService::~UpdateService()
{
    if (m_schedulerRunning) {
        stopAutoScheduler();
    }
    saveConfiguration();
}

bool UpdateService::initialize()
{
    // 加载配置
    loadConfiguration();
    
    // 如果没有数据源，加载默认配置
    if (m_dataSources.empty()) {
        loadDefaultDataSources();
    }
    
    // 设置自动更新调度器
    if (m_autoUpdateEnabled) {
        startAutoScheduler();
    }
    
    return true;
}

void UpdateService::setAutoUpdateEnabled(bool enabled)
{
    if (m_autoUpdateEnabled != enabled) {
        m_autoUpdateEnabled = enabled;
        
        if (enabled) {
            startAutoScheduler();
        } else {
            stopAutoScheduler();
        }
        
        emit autoUpdateChanged(enabled);
        logUpdateEvent("", "AutoUpdateChanged", enabled ? "Enabled" : "Disabled");
    }
}

bool UpdateService::isAutoUpdateEnabled() const
{
    return m_autoUpdateEnabled;
}

void UpdateService::setCheckInterval(const QString &sourceId, int days)
{
    if (hasDataSource(sourceId)) {
        m_dataSources[sourceId].checkIntervalDays = days;
        QDateTime nextCheck = QDateTime::currentDateTime().addDays(days);
        m_dataSources[sourceId].nextCheck = nextCheck;
        
        emit dataSourceModified(sourceId);
        logUpdateEvent(sourceId, "CheckIntervalChanged", QString::number(days));
    }
}

void UpdateService::setAutoUpdate(const QString &sourceId, bool enabled)
{
    if (hasDataSource(sourceId)) {
        m_dataSources[sourceId].autoUpdate = enabled;
        
        emit dataSourceModified(sourceId);
        logUpdateEvent(sourceId, "AutoUpdateChanged", enabled ? "Enabled" : "Disabled");
    }
}

void UpdateService::checkForUpdates(const QString &sourceId)
{
    if (sourceId.isEmpty()) {
        // 检查所有数据源
        for (const auto &pair : m_dataSources) {
            checkForUpdatesInternal(pair.first);
        }
    } else if (hasDataSource(sourceId)) {
        // 检查特定数据源
        checkForUpdatesInternal(sourceId);
    }
}

void UpdateService::updateAll()
{
    for (const auto &pair : m_dataSources) {
        updateSource(pair.first);
    }
}

void UpdateService::updateSource(const QString &sourceId)
{
    if (!hasDataSource(sourceId)) {
        return;
    }
    
    // 检查更新
    checkForUpdatesInternal(sourceId);
}

void UpdateService::addDataSource(const DataSourceInfo &source)
{
    m_dataSources[source.id] = source;
    
    // 设置默认状态
    if (source.nextCheck.isNull()) {
        m_dataSources[source.id].nextCheck = QDateTime::currentDateTime().addDays(source.checkIntervalDays);
    }
    
    if (source.lastUpdate.isNull()) {
        m_dataSources[source.id].lastUpdate = QDateTime::currentDateTime();
    }
    
    m_updateStatuses[source.id] = UpdateStatus::Idle;
    
    emit dataSourceAdded(source.id);
    logUpdateEvent(source.id, "Added", source.name);
}

void UpdateService::removeDataSource(const QString &sourceId)
{
    if (hasDataSource(sourceId)) {
        QString name = m_dataSources[sourceId].name;
        m_dataSources.erase(sourceId);
        m_updateStatuses.erase(sourceId);
        
        emit dataSourceRemoved(sourceId);
        logUpdateEvent(sourceId, "Removed", name);
    }
}

bool UpdateService::hasDataSource(const QString &sourceId) const
{
    return m_dataSources.find(sourceId) != m_dataSources.end();
}

std::vector<DataSourceInfo> UpdateService::getDataSources() const
{
    std::vector<DataSourceInfo> result;
    for (const auto &pair : m_dataSources) {
        result.push_back(pair.second);
    }
    return result;
}

DataSourceInfo UpdateService::getDataSource(const QString &sourceId) const
{
    if (hasDataSource(sourceId)) {
        return m_dataSources.at(sourceId);
    }
    return DataSourceInfo();
}

UpdateStatus UpdateService::getUpdateStatus(const QString &sourceId) const
{
    auto it = m_updateStatuses.find(sourceId);
    if (it != m_updateStatuses.end()) {
        return it->second;
    }
    return UpdateStatus::Idle;
}

UpdateProgress UpdateService::getUpdateProgress() const
{
    return m_globalProgress;
}

QDateTime UpdateService::getLastUpdateTime(const QString &sourceId) const
{
    if (hasDataSource(sourceId)) {
        return m_dataSources.at(sourceId).lastUpdate;
    }
    return QDateTime();
}

QDateTime UpdateService::getNextCheckTime(const QString &sourceId) const
{
    if (hasDataSource(sourceId)) {
        return m_dataSources.at(sourceId).nextCheck;
    }
    return QDateTime();
}

void UpdateService::saveConfiguration()
{
    QJsonObject config;
    config["autoUpdateEnabled"] = m_autoUpdateEnabled;
    
    QJsonArray sourcesArray;
    for (const auto &pair : m_dataSources) {
        const DataSourceInfo &source = pair.second;
        QJsonObject sourceObj;
        sourceObj["id"] = source.id;
        sourceObj["name"] = source.name;
        sourceObj["description"] = source.description;
        sourceObj["type"] = static_cast<int>(source.type);
        sourceObj["version"] = source.version;
        sourceObj["latestVersion"] = source.latestVersion;
        sourceObj["updateUrl"] = source.updateUrl.toString();
        sourceObj["checksumUrl"] = source.checksumUrl.toString();
        sourceObj["localPath"] = source.localPath;
        sourceObj["lastUpdate"] = source.lastUpdate.toString(Qt::ISODate);
        sourceObj["nextCheck"] = source.nextCheck.toString(Qt::ISODate);
        sourceObj["checkIntervalDays"] = source.checkIntervalDays;
        sourceObj["autoUpdate"] = source.autoUpdate;
        sourceObj["critical"] = source.critical;
        sourceObj["checksum"] = source.checksum;
        
        // 将元数据添加到配置中
        QJsonDocument metadataDoc(source.metadata);
        sourceObj["metadata"] = metadataDoc.toJson(QJsonDocument::Compact);
        
        sourcesArray.append(sourceObj);
    }
    config["dataSources"] = sourcesArray;
    
    // 保存配置到文件
    QFile configFile(m_configPath);
    if (configFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(config);
        configFile.write(doc.toJson());
    }
}

void UpdateService::loadConfiguration()
{
    QFile configFile(m_configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse update configuration:" << error.errorString();
        return;
    }
    
    QJsonObject config = doc.object();
    m_autoUpdateEnabled = config["autoUpdateEnabled"].toBool(true);
    
    QJsonArray sourcesArray = config["dataSources"].toArray();
    for (const QJsonValue &value : sourcesArray) {
        QJsonObject sourceObj = value.toObject();
        
        DataSourceInfo source;
        source.id = sourceObj["id"].toString();
        source.name = sourceObj["name"].toString();
        source.description = sourceObj["description"].toString();
        source.type = static_cast<DataSourceType>(sourceObj["type"].toInt());
        source.version = sourceObj["version"].toString();
        source.latestVersion = sourceObj["latestVersion"].toString();
        source.updateUrl = QUrl(sourceObj["updateUrl"].toString());
        source.checksumUrl = QUrl(sourceObj["checksumUrl"].toString());
        source.localPath = sourceObj["localPath"].toString();
        source.lastUpdate = QDateTime::fromString(sourceObj["lastUpdate"].toString(), Qt::ISODate);
        source.nextCheck = QDateTime::fromString(sourceObj["nextCheck"].toString(), Qt::ISODate);
        source.checkIntervalDays = sourceObj["checkIntervalDays"].toInt();
        source.autoUpdate = sourceObj["autoUpdate"].toBool(true);
        source.critical = sourceObj["critical"].toBool(false);
        source.checksum = sourceObj["checksum"].toString();
        
        // 解析元数据
        QJsonDocument metadataDoc = QJsonDocument::fromJson(sourceObj["metadata"].toString().toUtf8());
        if (!metadataDoc.isNull() && metadataDoc.isObject()) {
            source.metadata = metadataDoc.object();
        }
        
        m_dataSources[source.id] = source;
        m_updateStatuses[source.id] = UpdateStatus::Idle;
    }
}

QStringList UpdateService::getUpdateHistory(int limit) const
{
    int count = qMin(limit, m_updateHistory.size());
    return m_updateHistory.mid(m_updateHistory.size() - count);
}

void UpdateService::scheduleNextCheck(const QString &sourceId)
{
    if (hasDataSource(sourceId)) {
        QDateTime nextCheck = QDateTime::currentDateTime().addDays(m_dataSources[sourceId].checkIntervalDays);
        m_dataSources[sourceId].nextCheck = nextCheck;
        
        logUpdateEvent(sourceId, "NextCheckScheduled", nextCheck.toString(Qt::ISODate));
    }
}

void UpdateService::startAutoScheduler()
{
    if (!m_schedulerRunning) {
        m_schedulerRunning = true;
        m_checkTimer->start();
        
        emit schedulerStarted();
        logUpdateEvent("", "SchedulerStarted", "Auto-update scheduler started");
    }
}

void UpdateService::stopAutoScheduler()
{
    if (m_schedulerRunning) {
        m_schedulerRunning = false;
        m_checkTimer->stop();
        
        emit schedulerStopped();
        logUpdateEvent("", "SchedulerStopped", "Auto-update scheduler stopped");
    }
}

void UpdateService::onCheckTimer()
{
    QDateTime now = QDateTime::currentDateTime();
    
    for (const auto &pair : m_dataSources) {
        const QString &sourceId = pair.first;
        const DataSourceInfo &source = pair.second;
        
        if (source.nextCheck <= now) {
            checkForUpdatesInternal(sourceId);
            scheduleNextCheck(sourceId);
        }
    }
}

void UpdateService::onNetworkReplyFinished(QNetworkReply *reply)
{
    QString sourceId = reply->property("sourceId").toString();
    QString operation = reply->property("operation").toString();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorString = reply->errorString();
        setUpdateStatus(sourceId, UpdateStatus::Failed);
        
        logUpdateEvent(sourceId, operation + "Failed", errorString);
        emit updateFailed(sourceId, errorString);
        
        reply->deleteLater();
        return;
    }
    
    if (operation == "check") {
        // 处理检查更新结果
        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            QString errorString = "Failed to parse update info: " + error.errorString();
            setUpdateStatus(sourceId, UpdateStatus::Failed);
            
            logUpdateEvent(sourceId, "CheckFailed", errorString);
            emit updateFailed(sourceId, errorString);
        } else {
            QJsonObject updateInfo = doc.object();
            QString latestVersion = updateInfo["version"].toString();
            QString checksum = updateInfo["checksum"].toString();
            QUrl downloadUrl = updateInfo["downloadUrl"].toString();
            
            if (latestVersion != m_dataSources[sourceId].version) {
                // 有更新可用
                m_dataSources[sourceId].latestVersion = latestVersion;
                m_dataSources[sourceId].checksum = checksum;
                m_dataSources[sourceId].updateUrl = downloadUrl;
                
                emit updateAvailable(sourceId, m_dataSources[sourceId].version, latestVersion);
                logUpdateEvent(sourceId, "UpdateAvailable", latestVersion);
                
                // 如果启用了自动更新，开始下载
                if (m_dataSources[sourceId].autoUpdate) {
                    downloadUpdate(sourceId);
                }
            } else {
                // 没有更新
                emit noUpdateAvailable(sourceId);
                logUpdateEvent(sourceId, "NoUpdateAvailable", latestVersion);
            }
        }
    } else if (operation == "download") {
        // 处理下载结果
        QString filePath = reply->property("filePath").toString();
        QFile file(filePath);
        
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            
            // 验证校验和
            QString expectedChecksum = m_dataSources[sourceId].checksum;
            QString actualChecksum = generateChecksum(filePath);
            
            if (expectedChecksum.isEmpty() || actualChecksum == expectedChecksum) {
                // 校验和验证通过，开始安装
                installUpdate(sourceId);
            } else {
                // 校验和不匹配
                QString errorString = "Checksum verification failed";
                setUpdateStatus(sourceId, UpdateStatus::Failed);
                
                logUpdateEvent(sourceId, "ChecksumFailed", errorString);
                emit updateFailed(sourceId, errorString);
            }
        } else {
            QString errorString = "Failed to save downloaded file";
            setUpdateStatus(sourceId, UpdateStatus::Failed);
            
            logUpdateEvent(sourceId, "DownloadFailed", errorString);
            emit updateFailed(sourceId, errorString);
        }
    }
    
    reply->deleteLater();
}

void UpdateService::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int progress = static_cast<int>((bytesReceived * 100) / bytesTotal);
        setUpdateProgress(100, progress, "Downloading", 
                        QString("%1 / %2").arg(formatFileSize(bytesReceived)).arg(formatFileSize(bytesTotal)));
    }
}

void UpdateService::checkForUpdatesInternal(const QString &sourceId)
{
    if (!hasDataSource(sourceId)) {
        return;
    }
    
    DataSourceInfo &source = m_dataSources[sourceId];
    
    setUpdateStatus(sourceId, UpdateStatus::Checking);
    setUpdateProgress(100, 0, "Checking for updates", "");
    
    // 创建网络请求
    QNetworkRequest request(source.updateUrl);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    
    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("sourceId", sourceId);
    reply->setProperty("operation", "check");
    
    connect(reply, &QNetworkReply::finished, this, &UpdateService::onNetworkReplyFinished);
    
    emit updateStarted(sourceId);
    logUpdateEvent(sourceId, "CheckStarted", source.updateUrl.toString());
}

void UpdateService::downloadUpdate(const QString &sourceId)
{
    if (!hasDataSource(sourceId)) {
        return;
    }
    
    DataSourceInfo &source = m_dataSources[sourceId];
    
    setUpdateStatus(sourceId, UpdateStatus::Downloading);
    setUpdateProgress(100, 0, "Preparing download", "");
    
    // 创建下载文件路径
    QString fileName = source.updateUrl.fileName();
    if (fileName.isEmpty()) {
        fileName = sourceId + "_update.zip";
    }
    QString filePath = m_downloadPath + "/" + fileName;
    
    // 创建网络请求
    QNetworkRequest request(source.updateUrl);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    
    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("sourceId", sourceId);
    reply->setProperty("operation", "download");
    reply->setProperty("filePath", filePath);
    
    connect(reply, &QNetworkReply::finished, this, &UpdateService::onNetworkReplyFinished);
    connect(reply, &QNetworkReply::downloadProgress, this, &UpdateService::onDownloadProgress);
    
    m_currentDownloadingSource = sourceId;
    
    logUpdateEvent(sourceId, "DownloadStarted", source.updateUrl.toString());
}

void UpdateService::installUpdate(const QString &sourceId)
{
    if (!hasDataSource(sourceId)) {
        return;
    }
    
    DataSourceInfo &source = m_dataSources[sourceId];
    
    setUpdateStatus(sourceId, UpdateStatus::Installing);
    setUpdateProgress(100, 0, "Installing update", "");
    
    // 备份当前数据
    backupCurrentData(sourceId);
    
    // 根据数据源类型执行不同的安装过程
    // 这里简化实现，实际应根据数据源类型进行处理
    
    // 更新版本信息
    source.version = source.latestVersion;
    source.lastUpdate = QDateTime::currentDateTime();
    scheduleNextCheck(sourceId);
    
    setUpdateStatus(sourceId, UpdateStatus::Success);
    setUpdateProgress(100, 100, "Update completed", "");
    
    emit updateCompleted(sourceId, true, "Update installed successfully");
    logUpdateEvent(sourceId, "UpdateCompleted", source.version);
    
    // 重新加载配置
    saveConfiguration();
}

void UpdateService::verifyChecksum(const QString &filePath, const QString &checksum)
{
    QString actualChecksum = generateChecksum(filePath);
    if (actualChecksum != checksum) {
        throw std::runtime_error("Checksum verification failed");
    }
}

QString UpdateService::generateChecksum(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(&file);
    file.close();
    
    return hash.result().toHex();
}

QString UpdateService::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes < KB) {
        return QString("%1 B").arg(bytes);
    } else if (bytes < MB) {
        return QString("%1 KB").arg(static_cast<double>(bytes) / KB, 0, 'f', 2);
    } else if (bytes < GB) {
        return QString("%1 MB").arg(static_cast<double>(bytes) / MB, 0, 'f', 2);
    } else {
        return QString("%1 GB").arg(static_cast<double>(bytes) / GB, 0, 'f', 2);
    }
}

QString UpdateService::getUpdateConfigPath()
{
    return m_configPath;
}

QString UpdateService::getUpdateLogPath()
{
    return m_logPath;
}

QString UpdateService::getDownloadPath()
{
    return m_downloadPath;
}

void UpdateService::backupCurrentData(const QString &sourceId)
{
    if (!hasDataSource(sourceId)) {
        return;
    }
    
    DataSourceInfo &source = m_dataSources[sourceId];
    QString localPath = source.localPath;
    
    if (QFile::exists(localPath)) {
        // 创建备份文件路径
        QDateTime now = QDateTime::currentDateTime();
        QString timestamp = now.toString("yyyyMMdd_hhmmss");
        QString backupPath = localPath + ".backup_" + timestamp;
        
        // 复制文件到备份路径
        if (QFile::copy(localPath, backupPath)) {
            logUpdateEvent(sourceId, "BackupCreated", backupPath);
        } else {
            logUpdateEvent(sourceId, "BackupFailed", backupPath);
        }
    }
}

void UpdateService::restoreFromBackup(const QString &sourceId)
{
    // 实现恢复功能
    // 这里简化实现
}

bool UpdateService::createBackup(const QString &source, const QString &backupPath)
{
    // 简化实现，实际应根据source类型进行备份
    return QFile::copy(source, backupPath);
}

void UpdateService::logUpdateEvent(const QString &sourceId, const QString &event, const QString &details)
{
    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString logEntry = QString("[%1] [%2] %3: %4")
                        .arg(timestamp)
                        .arg(sourceId.isEmpty() ? "SYSTEM" : sourceId)
                        .arg(event)
                        .arg(details);
    
    // 添加到内存中的历史记录
    m_updateHistory.append(logEntry);
    
    // 限制历史记录数量
    if (m_updateHistory.size() > MAX_HISTORY_ENTRIES) {
        m_updateHistory.removeFirst();
    }
    
    // 写入日志文件
    QFile logFile(m_logPath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&logFile);
        stream << logEntry << Qt::endl;
        logFile.close();
    }
}

void UpdateService::setUpdateStatus(const QString &sourceId, UpdateStatus status)
{
    m_updateStatuses[sourceId] = status;
}

void UpdateService::setUpdateProgress(int total, int current, const QString &stage, const QString &details)
{
    m_globalProgress = {total, current, stage, details};
    
    if (!m_currentDownloadingSource.isEmpty()) {
        emit updateProgress(m_currentDownloadingSource, current, stage + ": " + details);
    }
}

void UpdateService::loadDefaultDataSources()
{
    // 分子数据源
    DataSourceInfo molecularData;
    molecularData.id = "molecular_database";
    molecularData.name = "Molecular Properties Database";
    molecularData.description = "Database of molecular properties and structures";
    molecularData.type = DataSourceType::MolecularData;
    molecularData.version = "2.0.0";
    molecularData.updateUrl = QUrl("https://api.bondforge.org/updates/molecular_database.json");
    molecularData.checksumUrl = QUrl("https://api.bondforge.org/updates/molecular_database.sha256");
    molecularData.localPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/molecular_data.db";
    molecularData.checkIntervalDays = 30;
    molecularData.autoUpdate = true;
    molecularData.critical = false;
    addDataSource(molecularData);
    
    // 机器学习模型数据源
    DataSourceInfo mlModels;
    mlModels.id = "ml_models";
    mlModels.name = "Machine Learning Models";
    mlModels.description = "Pre-trained ML models for chemical property prediction";
    mlModels.type = DataSourceType::MLModels;
    mlModels.version = "2.0.0";
    mlModels.updateUrl = QUrl("https://api.bondforge.org/updates/ml_models.json");
    mlModels.checksumUrl = QUrl("https://api.bondforge.org/updates/ml_models.sha256");
    mlModels.localPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/models/";
    mlModels.checkIntervalDays = 60;
    mlModels.autoUpdate = true;
    mlModels.critical = false;
    addDataSource(mlModels);
    
    // 化学数据库
    DataSourceInfo chemicalDb;
    chemicalDb.id = "chemical_database";
    chemicalDb.name = "Chemical Reactions Database";
    chemicalDb.description = "Database of chemical reactions and pathways";
    chemicalDb.type = DataSourceType::ChemicalDatabases;
    chemicalDb.version = "2.0.0";
    chemicalDb.updateUrl = QUrl("https://api.bondforge.org/updates/chemical_database.json");
    chemicalDb.checksumUrl = QUrl("https://api.bondforge.org/updates/chemical_database.sha256");
    chemicalDb.localPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/chemical_reactions.db";
    chemicalDb.checkIntervalDays = 90;
    chemicalDb.autoUpdate = true;
    chemicalDb.critical = false;
    addDataSource(chemicalDb);
    
    // 参考数据
    DataSourceInfo refData;
    refData.id = "reference_data";
    refData.name = "Reference Data";
    refData.description = "Reference data for chemical standards and regulations";
    refData.type = DataSourceType::ReferenceData;
    refData.version = "2.0.0";
    refData.updateUrl = QUrl("https://api.bondforge.org/updates/reference_data.json");
    refData.checksumUrl = QUrl("https://api.bondforge.org/updates/reference_data.sha256");
    refData.localPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/reference_data.json";
    refData.checkIntervalDays = 180;
    refData.autoUpdate = false;
    refData.critical = false;
    addDataSource(refData);
}

} // namespace Services