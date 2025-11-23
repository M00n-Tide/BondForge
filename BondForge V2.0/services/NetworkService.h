#ifndef BONDFORGE_NETWORKSERVICE_H
#define BONDFORGE_NETWORKSERVICE_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <memory>
#include <functional>
#include <map>
#include <queue>
#include <condition_variable>
#include <atomic>

// 前向声明
namespace BondForge {
    namespace Core {
        namespace Collaboration {
            class User;
            class DataSharing;
        }
        namespace Data {
            class DataRecord;
        }
    }
}

namespace BondForge {
namespace Services {

// 请求类型枚举
enum class RequestType {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH
};

// 响应状态枚举
enum class ResponseStatus {
    SUCCESS,
    ERROR,
    TIMEOUT,
    NETWORK_ERROR,
    SERVER_ERROR,
    UNAUTHORIZED,
    FORBIDDEN,
    NOT_FOUND,
    VALIDATION_ERROR
};

// HTTP响应结构
struct HttpResponse {
    ResponseStatus status;
    int statusCode;
    QByteArray data;
    QJsonObject jsonData;
    QMap<QString, QString> headers;
    QString errorString;
};

// WebSocket消息类型
enum class WebSocketMessageType {
    TEXT,
    BINARY,
    PING,
    PONG,
    CLOSE
};

// 网络服务配置
struct NetworkConfig {
    QString serverUrl = "http://localhost:8080/api";
    QString websocketUrl = "ws://localhost:8080/ws";
    QString apiKey = "";
    int timeoutMs = 30000;
    int retryCount = 3;
    int retryDelayMs = 1000;
    bool enableCompression = true;
    bool enableSslVerification = true;
    int maxConcurrentRequests = 10;
};

// 请求回调函数类型
using ResponseCallback = std::function<void(const HttpResponse&)>;
using ProgressCallback = std::function<void(qint64, qint64)>;

// WebSocket消息回调
using WebSocketMessageCallback = std::function<void(const QString&, const QByteArray&, WebSocketMessageType)>;

// 网络请求信息
struct NetworkRequest {
    RequestType type = RequestType::GET;
    QString endpoint;
    QByteArray data;
    QMap<QString, QString> headers;
    ResponseCallback callback;
    ProgressCallback progressCallback;
    int timeoutMs = 0;
    int retryCount = 0;
    QString requestId;
    bool priority = false;
};

class NetworkService : public QObject
{
    Q_OBJECT

public:
    explicit NetworkService(QObject *parent = nullptr);
    ~NetworkService();

    // 配置管理
    void setConfig(const NetworkConfig &config);
    NetworkConfig config() const;
    
    // 连接管理
    bool connectToServer();
    void disconnectFromServer();
    bool isConnected() const;
    
    // HTTP请求
    QString get(const QString &endpoint, const ResponseCallback &callback = nullptr);
    QString post(const QString &endpoint, const QByteArray &data, const ResponseCallback &callback = nullptr);
    QString postJson(const QString &endpoint, const QJsonObject &json, const ResponseCallback &callback = nullptr);
    QString put(const QString &endpoint, const QByteArray &data, const ResponseCallback &callback = nullptr);
    QString putJson(const QString &endpoint, const QJsonObject &json, const ResponseCallback &callback = nullptr);
    QString deleteResource(const QString &endpoint, const ResponseCallback &callback = nullptr);
    
    // 文件上传和下载
    QString uploadFile(const QString &endpoint, const QString &filePath, const QString &fieldName = "file",
                       const QMap<QString, QString> &formData = {}, const ResponseCallback &callback = nullptr);
    QString downloadFile(const QString &endpoint, const QString &filePath, const ResponseCallback &callback = nullptr,
                         const ProgressCallback &progressCallback = nullptr);
    
    // 批量请求
    QString executeBatch(const QList<NetworkRequest> &requests, const std::function<void(const QList<HttpResponse>&)> &callback);
    
    // WebSocket连接
    bool connectWebSocket();
    void disconnectWebSocket();
    bool isWebSocketConnected() const;
    void sendWebSocketMessage(const QString &channel, const QByteArray &message);
    void subscribeToChannel(const QString &channel, const WebSocketMessageCallback &callback);
    void unsubscribeFromChannel(const QString &channel);
    
    // 用户认证
    QString login(const QString &username, const QString &password, const ResponseCallback &callback = nullptr);
    QString logout(const ResponseCallback &callback = nullptr);
    QString refreshToken(const ResponseCallback &callback = nullptr);
    bool isAuthenticated() const;
    
    // 协作功能
    QString shareData(const Core::Data::DataRecord &record, const Core::Collaboration::DataSharing &sharing, 
                      const ResponseCallback &callback = nullptr);
    QString getSharedData(const QString &sharingId, const ResponseCallback &callback = nullptr);
    QString addComment(const QString &recordId, const QString &comment, const ResponseCallback &callback = nullptr);
    QString getComments(const QString &recordId, const ResponseCallback &callback = nullptr);
    
    // 系统信息
    QString getServerInfo(const ResponseCallback &callback = nullptr);
    QString getSystemStatus(const ResponseCallback &callback = nullptr);
    
    // 请求队列管理
    void clearPendingRequests();
    int pendingRequestCount() const;
    
    // 网络统计
    quint64 bytesReceived() const;
    quint64 bytesSent() const;
    int activeRequestCount() const;

signals:
    // 连接状态信号
    void connectionChanged(bool connected);
    void authenticationChanged(bool authenticated);
    void serverUrlChanged(const QString &url);
    
    // 请求状态信号
    void requestStarted(const QString &requestId);
    void requestFinished(const QString &requestId, const HttpResponse &response);
    void requestFailed(const QString &requestId, const QString &error);
    
    // WebSocket信号
    void webSocketConnected();
    void webSocketDisconnected();
    void webSocketError(const QString &error);
    void webSocketMessageReceived(const QString &channel, const QByteArray &message);
    
    // 错误信号
    void networkError(const QString &error);
    void sslError(const QString &error);

private slots:
    void onRequestFinished();
    void onRequestProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onRequestError(QNetworkReply::NetworkError error);
    void onSslErrors(const QList<QSslError> &errors);
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketError(QAbstractSocket::SocketError error);
    void onWebSocketTextMessageReceived(const QString &message);
    void onWebSocketBinaryMessageReceived(const QByteArray &message);
    void processRequestQueue();
    void checkConnectionStatus();

private:
    void setupNetworkAccessManager();
    void setupWebSocket();
    void processPendingRequest(const NetworkRequest &request);
    QNetworkReply* executeRequest(const NetworkRequest &request);
    QString generateRequestId();
    QByteArray prepareRequestData(const QByteArray &data, const QMap<QString, QString> &headers);
    HttpResponse parseResponse(QNetworkReply *reply);
    void handleRequestTimeout(const QString &requestId);
    void retryRequest(const QString &requestId);
    
    // HTTP请求
    QNetworkAccessManager* m_networkManager;
    QMap<QString, QNetworkReply*> m_activeRequests;
    QQueue<NetworkRequest> m_requestQueue;
    
    // WebSocket
    QWebSocket* m_webSocket;
    QMap<QString, WebSocketMessageCallback> m_webSocketSubscriptions;
    
    // 配置和状态
    NetworkConfig m_config;
    QString m_apiKey;
    QString m_authToken;
    QString m_refreshToken;
    QAtomicInt m_authenticated{false};
    QAtomicInt m_connected{false};
    QAtomicInt m_webSocketConnected{false};
    
    // 定时器
    QTimer* m_connectionTimer;
    QTimer* m_retryTimer;
    
    // 线程安全
    mutable QMutex m_mutex;
    QMutex m_requestMutex;
    QMutex m_webSocketMutex;
    
    // 统计信息
    quint64 m_bytesReceived = 0;
    quint64 m_bytesSent = 0;
    
    // 线程池
    QThread* m_networkThread;
};

} // namespace Services
} // namespace BondForge

#endif // BONDFORGE_NETWORKSERVICE_H