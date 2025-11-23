#include "NetworkService.h"
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QFileInfo>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QWebSocket>
#include <QSslConfiguration>
#include <QSslError>
#include <QThread>
#include <QMutexLocker>
#include <QUuid>
#include <QRandomGenerator>
#include <QUrlQuery>

#include "../core/collaboration/User.h"
#include "../core/collaboration/DataSharing.h"
#include "../core/data/DataRecord.h"
#include "../utils/Logger.h"
#include "../utils/ConfigManager.h"

namespace BondForge {
namespace Services {

NetworkService::NetworkService(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_webSocket(nullptr)
    , m_apiKey("")
    , m_authToken("")
    , m_refreshToken("")
    , m_connected(false)
    , m_webSocketConnected(false)
    , m_connectionTimer(new QTimer(this))
    , m_retryTimer(new QTimer(this))
    , m_networkThread(new QThread(this))
    , m_bytesReceived(0)
    , m_bytesSent(0)
{
    // 初始化网络访问管理器
    setupNetworkAccessManager();
    
    // 设置定时器
    m_connectionTimer->setInterval(30000); // 30秒检查一次连接状态
    connect(m_connectionTimer, &QTimer::timeout, this, &NetworkService::checkConnectionStatus);
    m_connectionTimer->start();
    
    m_retryTimer->setSingleShot(true);
    connect(m_retryTimer, &QTimer::timeout, this, &NetworkService::processRequestQueue);
    
    // 设置WebSocket
    setupWebSocket();
    
    // 设置线程
    m_networkThread->start();
    
    Utils::Logger::info("NetworkService initialized");
}

NetworkService::~NetworkService()
{
    // 停止定时器
    m_connectionTimer->stop();
    m_retryTimer->stop();
    
    // 断开连接
    disconnectFromServer();
    disconnectWebSocket();
    
    // 停止线程
    m_networkThread->quit();
    m_networkThread->wait();
    
    // 清理资源
    clearPendingRequests();
    
    Utils::Logger::info("NetworkService destroyed");
}

void NetworkService::setConfig(const NetworkConfig &config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
    
    // 重新连接服务器
    if (config.autoConnect) {
        connectToServer();
    }
}

NetworkConfig NetworkService::config() const
{
    QMutexLocker locker(&m_mutex);
    return m_config;
}

bool NetworkService::connectToServer()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_connected) {
        return true;
    }
    
    // 测试连接
    if (!testConnection()) {
        emit connectionChanged(false);
        return false;
    }
    
    m_connected = true;
    emit connectionChanged(true);
    
    // 尝试连接WebSocket
    connectWebSocket();
    
    Utils::Logger::info("Connected to server");
    return true;
}

void NetworkService::disconnectFromServer()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_connected) {
        return;
    }
    
    m_connected = false;
    emit connectionChanged(false);
    
    // 断开WebSocket
    disconnectWebSocket();
    
    Utils::Logger::info("Disconnected from server");
}

bool NetworkService::isConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_connected;
}

QString NetworkService::get(const QString &endpoint, const ResponseCallback &callback)
{
    NetworkRequest request;
    request.type = RequestType::GET;
    request.endpoint = endpoint;
    request.callback = callback;
    request.requestId = generateRequestId();
    
    QMutexLocker locker(&m_requestMutex);
    m_requestQueue.enqueue(request);
    
    // 如果当前没有活跃请求，立即处理
    if (m_activeRequests.size() < m_config.maxConcurrentRequests) {
        processPendingRequest(request);
    }
    
    return request.requestId;
}

QString NetworkService::post(const QString &endpoint, const QByteArray &data, const ResponseCallback &callback)
{
    NetworkRequest request;
    request.type = RequestType::POST;
    request.endpoint = endpoint;
    request.data = data;
    request.callback = callback;
    request.requestId = generateRequestId();
    
    QMutexLocker locker(&m_requestMutex);
    m_requestQueue.enqueue(request);
    
    // 如果当前没有活跃请求，立即处理
    if (m_activeRequests.size() < m_config.maxConcurrentRequests) {
        processPendingRequest(request);
    }
    
    return request.requestId;
}

QString NetworkService::postJson(const QString &endpoint, const QJsonObject &json, const ResponseCallback &callback)
{
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    return post(endpoint, data, callback);
}

QString NetworkService::put(const QString &endpoint, const QByteArray &data, const ResponseCallback &callback)
{
    NetworkRequest request;
    request.type = RequestType::PUT;
    request.endpoint = endpoint;
    request.data = data;
    request.callback = callback;
    request.requestId = generateRequestId();
    
    QMutexLocker locker(&m_requestMutex);
    m_requestQueue.enqueue(request);
    
    // 如果当前没有活跃请求，立即处理
    if (m_activeRequests.size() < m_config.maxConcurrentRequests) {
        processPendingRequest(request);
    }
    
    return request.requestId;
}

QString NetworkService::putJson(const QString &endpoint, const QJsonObject &json, const ResponseCallback &callback)
{
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    return put(endpoint, data, callback);
}

QString NetworkService::deleteResource(const QString &endpoint, const ResponseCallback &callback)
{
    NetworkRequest request;
    request.type = RequestType::DELETE;
    request.endpoint = endpoint;
    request.callback = callback;
    request.requestId = generateRequestId();
    
    QMutexLocker locker(&m_requestMutex);
    m_requestQueue.enqueue(request);
    
    // 如果当前没有活跃请求，立即处理
    if (m_activeRequests.size() < m_config.maxConcurrentRequests) {
        processPendingRequest(request);
    }
    
    return request.requestId;
}

QString NetworkService::uploadFile(const QString &endpoint, const QString &filePath, const QString &fieldName,
                                   const QMap<QString, QString> &formData, const ResponseCallback &callback)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (callback) {
            HttpResponse response;
            response.status = ResponseStatus::ERROR;
            response.errorString = tr("Could not open file: %1").arg(filePath);
            callback(response);
        }
        return "";
    }
    
    QByteArray fileData = file.readAll();
    file.close();
    
    // 创建多部分表单
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    // 添加文件
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                      QString("form-data; name=\"%1\"; filename=\"%2\"").arg(fieldName).arg(QFileInfo(filePath).fileName()));
    filePart.setBody(fileData);
    multiPart->append(filePart);
    
    // 添加表单数据
    for (auto it = formData.constBegin(); it != formData.constEnd(); ++it) {
        QHttpPart dataPart;
        dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                          QString("form-data; name=\"%1\"").arg(it.key()));
        dataPart.setBody(it.value().toUtf8());
        multiPart->append(dataPart);
    }
    
    NetworkRequest request;
    request.type = RequestType::POST;
    request.endpoint = endpoint;
    request.callback = [callback, multiPart](const HttpResponse &response) {
        delete multiPart; // 清理多部分数据
        if (callback) {
            callback(response);
        }
    };
    request.requestId = generateRequestId();
    
    // 调整请求头以适应多部分内容
    request.headers["Content-Type"] = "multipart/form-data; boundary=" + multiPart->boundary();
    
    QMutexLocker locker(&m_requestMutex);
    m_requestQueue.enqueue(request);
    
    // 如果当前没有活跃请求，立即处理
    if (m_activeRequests.size() < m_config.maxConcurrentRequests) {
        processPendingRequest(request);
    }
    
    return request.requestId;
}

QString NetworkService::downloadFile(const QString &endpoint, const QString &filePath, const ResponseCallback &callback,
                                    const ProgressCallback &progressCallback)
{
    NetworkRequest request;
    request.type = RequestType::GET;
    request.endpoint = endpoint;
    request.callback = callback;
    request.progressCallback = progressCallback;
    request.requestId = generateRequestId();
    
    QMutexLocker locker(&m_requestMutex);
    m_requestQueue.enqueue(request);
    
    // 如果当前没有活跃请求，立即处理
    if (m_activeRequests.size() < m_config.maxConcurrentRequests) {
        processPendingRequest(request);
    }
    
    return request.requestId;
}

QString NetworkService::executeBatch(const QList<NetworkRequest> &requests, const std::function<void(const QList<HttpResponse>&)> &callback)
{
    QString batchId = generateRequestId();
    
    // 批量请求实现
    // 这里可以通过并行或串行方式执行多个请求
    // 为简化实现，我们使用串行方式
    
    auto responses = std::make_shared<QList<HttpResponse>>();
    auto completed = std::make_shared<int>(0);
    
    for (int i = 0; i < requests.size(); ++i) {
        NetworkRequest request = requests[i];
        
        // 设置回调以处理批量响应
        ResponseCallback originalCallback = request.callback;
        request.callback = [this, batchId, i, responses, completed, callback, originalCallback](const HttpResponse &response) {
            (*responses) << response;
            (*completed)++;
            
            // 调用原始回调
            if (originalCallback) {
                originalCallback(response);
            }
            
            // 当所有请求完成时，调用批量回调
            if (*completed == requests.size() && callback) {
                callback(*responses);
            }
        };
        
        // 如果是第一个请求，使用现有的get/post/put/delete方法
        switch (request.type) {
            case RequestType::GET:
                get(request.endpoint, request.callback);
                break;
            case RequestType::POST:
                post(request.endpoint, request.data, request.callback);
                break;
            case RequestType::PUT:
                put(request.endpoint, request.data, request.callback);
                break;
            case RequestType::DELETE:
                deleteResource(request.endpoint, request.callback);
                break;
            case RequestType::PATCH:
                // 需要实现PATCH方法
                break;
        }
    }
    
    return batchId;
}

bool NetworkService::connectWebSocket()
{
    QMutexLocker locker(&m_webSocketMutex);
    
    if (m_webSocketConnected) {
        return true;
    }
    
    if (!m_webSocket) {
        setupWebSocket();
    }
    
    m_webSocket->open(QUrl(m_config.websocketUrl));
    
    return true;
}

void NetworkService::disconnectWebSocket()
{
    QMutexLocker locker(&m_webSocketMutex);
    
    if (!m_webSocketConnected || !m_webSocket) {
        return;
    }
    
    m_webSocket->close();
}

bool NetworkService::isWebSocketConnected() const
{
    QMutexLocker locker(&m_webSocketMutex);
    return m_webSocketConnected;
}

void NetworkService::sendWebSocketMessage(const QString &channel, const QByteArray &message)
{
    QMutexLocker locker(&m_webSocketMutex);
    
    if (!m_webSocketConnected || !m_webSocket) {
        return;
    }
    
    QJsonObject json;
    json["channel"] = channel;
    json["message"] = QString::fromUtf8(message);
    
    QJsonDocument doc(json);
    m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}

void NetworkService::subscribeToChannel(const QString &channel, const WebSocketMessageCallback &callback)
{
    QMutexLocker locker(&m_webSocketMutex);
    m_webSocketSubscriptions[channel] = callback;
    
    // 如果已连接，发送订阅消息
    if (m_webSocketConnected && m_webSocket) {
        QJsonObject json;
        json["type"] = "subscribe";
        json["channel"] = channel;
        
        QJsonDocument doc(json);
        m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }
}

void NetworkService::unsubscribeFromChannel(const QString &channel)
{
    QMutexLocker locker(&m_webSocketMutex);
    m_webSocketSubscriptions.remove(channel);
    
    // 如果已连接，发送取消订阅消息
    if (m_webSocketConnected && m_webSocket) {
        QJsonObject json;
        json["type"] = "unsubscribe";
        json["channel"] = channel;
        
        QJsonDocument doc(json);
        m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }
}

QString NetworkService::login(const QString &username, const QString &password, const ResponseCallback &callback)
{
    QJsonObject json;
    json["username"] = username;
    json["password"] = password;
    
    ResponseCallback loginCallback = [this, callback](const HttpResponse &response) {
        if (response.status == ResponseStatus::SUCCESS) {
            // 解析响应中的token
            QJsonObject jsonObj = response.jsonData;
            m_authToken = jsonObj["token"].toString();
            m_refreshToken = jsonObj["refreshToken"].toString();
            m_authenticated = true;
            
            emit authenticationChanged(true);
        } else {
            m_authenticated = false;
            emit authenticationChanged(false);
        }
        
        if (callback) {
            callback(response);
        }
    };
    
    return postJson("/auth/login", json, loginCallback);
}

QString NetworkService::logout(const ResponseCallback &callback)
{
    ResponseCallback logoutCallback = [this, callback](const HttpResponse &response) {
        m_authToken = "";
        m_refreshToken = "";
        m_authenticated = false;
        
        emit authenticationChanged(false);
        
        if (callback) {
            callback(response);
        }
    };
    
    return post("/auth/logout", QByteArray(), logoutCallback);
}

QString NetworkService::refreshToken(const ResponseCallback &callback)
{
    if (m_refreshToken.isEmpty()) {
        if (callback) {
            HttpResponse response;
            response.status = ResponseStatus::ERROR;
            response.errorString = tr("No refresh token available");
            callback(response);
        }
        return "";
    }
    
    QJsonObject json;
    json["refreshToken"] = m_refreshToken;
    
    ResponseCallback refreshCallback = [this, callback](const HttpResponse &response) {
        if (response.status == ResponseStatus::SUCCESS) {
            // 解析响应中的新token
            QJsonObject jsonObj = response.jsonData;
            m_authToken = jsonObj["token"].toString();
            m_refreshToken = jsonObj["refreshToken"].toString();
            m_authenticated = true;
            
            emit authenticationChanged(true);
        } else {
            m_authenticated = false;
            emit authenticationChanged(false);
        }
        
        if (callback) {
            callback(response);
        }
    };
    
    return postJson("/auth/refresh", json, refreshCallback);
}

bool NetworkService::isAuthenticated() const
{
    return m_authenticated.load();
}

QString NetworkService::shareData(const Core::Data::DataRecord &record, const Core::Collaboration::DataSharing &sharing, 
                                 const ResponseCallback &callback)
{
    QJsonObject json;
    json["recordId"] = QString::fromStdString(record.id);
    json["permission"] = QString::fromStdString(sharing.permission);
    json["shareeId"] = QString::fromStdString(sharing.shareeId);
    json["description"] = QString::fromStdString(sharing.description);
    
    if (sharing.expiresAt > 0) {
        json["expiresAt"] = static_cast<qint64>(sharing.expiresAt);
    }
    
    return postJson("/sharing/create", json, callback);
}

QString NetworkService::getSharedData(const QString &sharingId, const ResponseCallback &callback)
{
    return get("/sharing/" + sharingId, callback);
}

QString NetworkService::addComment(const QString &recordId, const QString &comment, const ResponseCallback &callback)
{
    QJsonObject json;
    json["recordId"] = recordId;
    json["content"] = comment;
    
    return postJson("/comments", json, callback);
}

QString NetworkService::getComments(const QString &recordId, const ResponseCallback &callback)
{
    return get("/comments/" + recordId, callback);
}

QString NetworkService::getServerInfo(const ResponseCallback &callback)
{
    return get("/server/info", callback);
}

QString NetworkService::getSystemStatus(const ResponseCallback &callback)
{
    return get("/system/status", callback);
}

void NetworkService::clearPendingRequests()
{
    QMutexLocker locker(&m_requestMutex);
    
    // 取消所有活跃请求
    for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it) {
        QNetworkReply* reply = it.value();
        if (reply) {
            reply->abort();
            reply->deleteLater();
        }
    }
    m_activeRequests.clear();
    
    // 清空请求队列
    m_requestQueue.clear();
}

int NetworkService::pendingRequestCount() const
{
    QMutexLocker locker(&m_requestMutex);
    return m_requestQueue.size() + m_activeRequests.size();
}

quint64 NetworkService::bytesReceived() const
{
    return m_bytesReceived;
}

quint64 NetworkService::bytesSent() const
{
    return m_bytesSent;
}

int NetworkService::activeRequestCount() const
{
    QMutexLocker locker(&m_requestMutex);
    return m_activeRequests.size();
}

void NetworkService::setupNetworkAccessManager()
{
    m_networkManager = new QNetworkAccessManager(this);
    
    // 连接信号
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &NetworkService::onRequestFinished);
    
    // 设置SSL配置
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    if (!m_config.enableSslVerification) {
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    }
    m_networkManager->setConfiguration(sslConfig);
}

void NetworkService::setupWebSocket()
{
    m_webSocket = new QWebSocket();
    
    connect(m_webSocket, &QWebSocket::connected,
            this, &NetworkService::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected,
            this, &NetworkService::onWebSocketDisconnected);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &NetworkService::onWebSocketError);
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &NetworkService::onWebSocketTextMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived,
            this, &NetworkService::onWebSocketBinaryMessageReceived);
    
    // 设置SSL配置
    if (!m_config.enableSslVerification) {
        QSslConfiguration sslConfig = m_webSocket->sslConfiguration();
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        m_webSocket->setSslConfiguration(sslConfig);
    }
}

bool NetworkService::testConnection()
{
    // 创建一个简单的测试请求
    QNetworkRequest request(QUrl(m_config.serverUrl + "/ping"));
    
    // 添加认证头
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }
    
    // 设置超时
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(m_config.connectionTimeout);
    
    QNetworkReply* reply = m_networkManager->get(request);
    
    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    bool success = false;
    if (timer.isActive()) {
        timer.stop();
        if (reply->error() == QNetworkReply::NoError) {
            success = true;
        }
    } else {
        reply->abort();
    }
    
    reply->deleteLater();
    return success;
}

QString NetworkService::generateRequestId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QNetworkReply* NetworkService::executeRequest(const NetworkRequest &request)
{
    QString url = m_config.serverUrl + request.endpoint;
    QNetworkRequest networkRequest(QUrl(url));
    
    // 添加认证头
    if (!m_authToken.isEmpty()) {
        networkRequest.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }
    
    // 添加API密钥
    if (!m_config.apiKey.isEmpty()) {
        networkRequest.setRawHeader("X-API-Key", m_config.apiKey.toUtf8());
    }
    
    // 添加自定义头
    for (auto it = request.headers.constBegin(); it != request.headers.constEnd(); ++it) {
        networkRequest.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    // 设置内容类型
    if (!request.data.isEmpty() && !request.headers.contains("Content-Type")) {
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    }
    
    // 设置超时
    int timeout = request.timeoutMs > 0 ? request.timeoutMs : m_config.timeoutMs;
    
    QNetworkReply* reply = nullptr;
    
    switch (request.type) {
        case RequestType::GET:
            reply = m_networkManager->get(networkRequest);
            break;
            
        case RequestType::POST:
            reply = m_networkManager->post(networkRequest, request.data);
            break;
            
        case RequestType::PUT:
            reply = m_networkManager->put(networkRequest, request.data);
            break;
            
        case RequestType::DELETE:
            reply = m_networkManager->deleteResource(networkRequest);
            break;
            
        case RequestType::PATCH:
            reply = m_networkManager->sendCustomRequest(networkRequest, "PATCH", request.data);
            break;
    }
    
    // 设置超时
    QTimer::singleShot(timeout, reply, &QNetworkReply::abort);
    
    // 连接进度信号
    if (request.progressCallback) {
        connect(reply, &QNetworkReply::downloadProgress, this, 
                [this, request](qint64 bytesReceived, qint64 bytesTotal) {
                    if (request.progressCallback) {
                        request.progressCallback(bytesReceived, bytesTotal);
                    }
                });
    }
    
    // 连接错误信号
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &NetworkService::onRequestError);
    
    // 连接SSL错误信号
    connect(reply, &QNetworkReply::sslErrors, this, &NetworkService::onSslErrors);
    
    return reply;
}

void NetworkService::processPendingRequest(const NetworkRequest &request)
{
    QNetworkReply* reply = executeRequest(request);
    
    if (reply) {
        QMutexLocker locker(&m_requestMutex);
        m_activeRequests[request.requestId] = reply;
        
        emit requestStarted(request.requestId);
    }
}

void NetworkService::processRequestQueue()
{
    QMutexLocker locker(&m_requestMutex);
    
    // 处理队列中的请求，直到达到最大并发数
    while (!m_requestQueue.isEmpty() && m_activeRequests.size() < m_config.maxConcurrentRequests) {
        NetworkRequest request = m_requestQueue.dequeue();
        processPendingRequest(request);
    }
}

HttpResponse NetworkService::parseResponse(QNetworkReply *reply)
{
    HttpResponse response;
    
    // 获取状态码
    response.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    // 获取头信息
    for (auto it = reply->rawHeaderPairs().begin(); it != reply->rawHeaderPairs().end(); ++it) {
        response.headers[it->first] = it->second;
    }
    
    // 获取数据
    response.data = reply->readAll();
    
    // 解析JSON数据
    if (response.headers.contains("Content-Type") && 
        response.headers["Content-Type"].contains("application/json")) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response.data, &parseError);
        
        if (parseError.error == QJsonParseError::NoError) {
            response.jsonData = doc.object();
        }
    }
    
    // 确定响应状态
    if (reply->error() == QNetworkReply::NoError) {
        if (response.statusCode >= 200 && response.statusCode < 300) {
            response.status = ResponseStatus::SUCCESS;
        } else if (response.statusCode == 401) {
            response.status = ResponseStatus::UNAUTHORIZED;
        } else if (response.statusCode == 403) {
            response.status = ResponseStatus::FORBIDDEN;
        } else if (response.statusCode == 404) {
            response.status = ResponseStatus::NOT_FOUND;
        } else {
            response.status = ResponseStatus::SERVER_ERROR;
        }
    } else {
        if (reply->error() == QNetworkReply::TimeoutError) {
            response.status = ResponseStatus::TIMEOUT;
        } else {
            response.status = ResponseStatus::NETWORK_ERROR;
        }
    }
    
    response.errorString = reply->errorString();
    
    // 更新统计
    m_bytesReceived += response.data.size();
    
    return response;
}

void NetworkService::handleRequestTimeout(const QString &requestId)
{
    QMutexLocker locker(&m_requestMutex);
    
    if (m_activeRequests.contains(requestId)) {
        QNetworkReply* reply = m_activeRequests[requestId];
        if (reply) {
            reply->abort();
        }
    }
}

void NetworkService::retryRequest(const QString &requestId)
{
    // 这里可以实现请求重试逻辑
    // 从m_activeRequests中移除失败的请求，然后重新加入队列
}

void NetworkService::onRequestFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 获取请求ID
    QString requestId;
    {
        QMutexLocker locker(&m_requestMutex);
        for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it) {
            if (it.value() == reply) {
                requestId = it.key();
                m_activeRequests.remove(requestId);
                break;
            }
        }
    }
    
    // 解析响应
    HttpResponse response = parseResponse(reply);
    
    // 调用回调
    NetworkRequest request;
    if (!requestId.isEmpty()) {
        // 从队列中查找原始请求以获取回调
        // 这里需要存储请求信息，简化实现，暂不处理
        
        emit requestFinished(requestId, response);
    }
    
    // 处理队列中的下一个请求
    processRequestQueue();
    
    // 清理
    reply->deleteLater();
}

void NetworkService::onRequestProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    // 更新进度
    m_bytesReceived += bytesReceived;
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 获取请求ID
    QString requestId;
    {
        QMutexLocker locker(&m_requestMutex);
        for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it) {
            if (it.value() == reply) {
                requestId = it.key();
                break;
            }
        }
    }
    
    // 发出进度信号
    if (!requestId.isEmpty()) {
        // 这里可以发出进度信号
    }
}

void NetworkService::onRequestError(QNetworkReply::NetworkError error)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 获取请求ID
    QString requestId;
    {
        QMutexLocker locker(&m_requestMutex);
        for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it) {
            if (it.value() == reply) {
                requestId = it.key();
                break;
            }
        }
    }
    
    // 发出错误信号
    if (!requestId.isEmpty()) {
        emit requestFailed(requestId, reply->errorString());
        emit networkError(reply->errorString());
    }
}

void NetworkService::onSslErrors(const QList<QSslError> &errors)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 如果禁用了SSL验证，忽略所有SSL错误
    if (!m_config.enableSslVerification) {
        reply->ignoreSslErrors();
        return;
    }
    
    // 发出SSL错误信号
    QStringList errorStrings;
    for (const QSslError &error : errors) {
        errorStrings << error.errorString();
    }
    
    emit sslError(errorStrings.join(", "));
}

void NetworkService::onWebSocketConnected()
{
    QMutexLocker locker(&m_webSocketMutex);
    m_webSocketConnected = true;
    
    // 重新订阅所有频道
    for (auto it = m_webSocketSubscriptions.constBegin(); it != m_webSocketSubscriptions.constEnd(); ++it) {
        const QString &channel = it.key();
        
        QJsonObject json;
        json["type"] = "subscribe";
        json["channel"] = channel;
        
        QJsonDocument doc(json);
        m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }
    
    emit webSocketConnected();
    Utils::Logger::info("WebSocket connected");
}

void NetworkService::onWebSocketDisconnected()
{
    QMutexLocker locker(&m_webSocketMutex);
    m_webSocketConnected = false;
    
    emit webSocketDisconnected();
    Utils::Logger::info("WebSocket disconnected");
}

void NetworkService::onWebSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    
    emit webSocketError(m_webSocket->errorString());
    Utils::Logger::error("WebSocket error: " + m_webSocket->errorString().toStdString());
}

void NetworkService::onWebSocketTextMessageReceived(const QString &message)
{
    // 解析消息
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        return;
    }
    
    QJsonObject json = doc.object();
    QString channel = json["channel"].toString();
    QString text = json["message"].toString();
    QByteArray data = text.toUtf8();
    
    // 调用订阅回调
    QMutexLocker locker(&m_webSocketMutex);
    if (m_webSocketSubscriptions.contains(channel)) {
        const WebSocketMessageCallback &callback = m_webSocketSubscriptions[channel];
        if (callback) {
            callback(channel, data, WebSocketMessageType::TEXT);
        }
    }
    
    emit webSocketMessageReceived(channel, data);
}

void NetworkService::onWebSocketBinaryMessageReceived(const QByteArray &message)
{
    // 解析二进制消息
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        return;
    }
    
    QJsonObject json = doc.object();
    QString channel = json["channel"].toString();
    QByteArray data = json["data"].toString().toUtf8();
    
    // 调用订阅回调
    QMutexLocker locker(&m_webSocketMutex);
    if (m_webSocketSubscriptions.contains(channel)) {
        const WebSocketMessageCallback &callback = m_webSocketSubscriptions[channel];
        if (callback) {
            callback(channel, data, WebSocketMessageType::BINARY);
        }
    }
    
    emit webSocketMessageReceived(channel, data);
}

void NetworkService::checkConnectionStatus()
{
    // 检查连接状态，如果断开则尝试重新连接
    if (!isConnected()) {
        connectToServer();
    }
}

} // namespace Services
} // namespace BondForge