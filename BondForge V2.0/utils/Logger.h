#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <sstream>
#include <queue>

namespace BondForge {
namespace Utils {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    Trace,      // 跟踪 - 最详细的日志信息
    Debug,      // 调试 - 开发者调试信息
    Info,       // 信息 - 常规信息
    Warning,    // 警告 - 潜在问题
    Error,      // 错误 - 错误但不致命
    Critical    // 严重 - 致命错误
};

/**
 * @brief 日志记录
 */
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;   // 时间戳
    LogLevel level;                                   // 日志级别
    std::string category;                               // 日志类别
    std::string message;                                // 日志消息
    std::string function;                                // 函数名
    std::string file;                                    // 文件名
    int line;                                           // 行号
    std::thread::id threadId;                           // 线程ID
};

/**
 * @brief 日志格式化器接口
 */
class ILogFormatter {
public:
    virtual ~ILogFormatter() = default;
    
    /**
     * @brief 格式化日志记录
     * 
     * @param entry 日志记录
     * @return 格式化后的字符串
     */
    virtual std::string format(const LogEntry& entry) = 0;
};

/**
 * @brief 控制台日志格式化器
 */
class ConsoleLogFormatter : public ILogFormatter {
private:
    bool m_useColors;
    
    /**
     * @brief 获取日志级别颜色代码
     * 
     * @param level 日志级别
     * @return 颜色代码
     */
    std::string getColorCode(LogLevel level);
    
    /**
     * @brief 获取日志级别名称
     * 
     * @param level 日志级别
     * @return 级别名称
     */
    std::string getLevelName(LogLevel level);
    
public:
    explicit ConsoleLogFormatter(bool useColors = true);
    
    std::string format(const LogEntry& entry) override;
};

/**
 * @brief 文件日志格式化器
 */
class FileLogFormatter : public ILogFormatter {
public:
    std::string format(const LogEntry& entry) override;
};

/**
 * @brief 日志输出器接口
 */
class ILogOutput {
public:
    virtual ~ILogOutput() = default;
    
    /**
     * @brief 输出日志记录
     * 
     * @param message 格式化后的日志消息
     */
    virtual void write(const std::string& message) = 0;
    
    /**
     * @brief 刷新输出缓冲区
     */
    virtual void flush() = 0;
};

/**
 * @brief 控制台日志输出器
 */
class ConsoleLogOutput : public ILogOutput {
public:
    void write(const std::string& message) override;
    void flush() override;
};

/**
 * @brief 文件日志输出器
 */
class FileLogOutput : public ILogOutput {
private:
    std::string m_filePath;
    std::mutex m_fileMutex;
    
public:
    explicit FileLogOutput(const std::string& filePath);
    
    void write(const std::string& message) override;
    void flush() override;
};

/**
 * @brief 异步文件日志输出器
 */
class AsyncFileLogOutput : public ILogOutput {
private:
    std::string m_filePath;
    std::mutex m_queueMutex;
    std::mutex m_fileMutex;
    std::queue<std::string> m_messageQueue;
    std::unique_ptr<std::thread> m_writerThread;
    std::atomic<bool> m_running;
    
    /**
     * @brief 后台写入线程
     */
    void writerThread();
    
public:
    explicit AsyncFileLogOutput(const std::string& filePath);
    ~AsyncFileLogOutput();
    
    void write(const std::string& message) override;
    void flush() override;
};

/**
 * @brief 日志器接口
 */
class ILogger {
public:
    virtual ~ILogger() = default;
    
    /**
     * @brief 记录日志
     * 
     * @param level 日志级别
     * @param message 日志消息
     * @param category 日志类别
     * @param function 函数名
     * @param file 文件名
     * @param line 行号
     */
    virtual void log(
        LogLevel level,
        const std::string& message,
        const std::string& category = "",
        const std::string& function = "",
        const std::string& file = "",
        int line = 0) = 0;
    
    /**
     * @brief 设置最低日志级别
     * 
     * @param level 最低级别
     */
    virtual void setLogLevel(LogLevel level) = 0;
    
    /**
     * @brief 获取最低日志级别
     * 
     * @return 最低级别
     */
    virtual LogLevel getLogLevel() const = 0;
    
    /**
     * @brief 添加日志输出器
     * 
     * @param output 输出器
     */
    virtual void addOutput(std::shared_ptr<ILogOutput> output) = 0;
    
    /**
     * @brief 移除日志输出器
     * 
     * @param output 输出器
     */
    virtual void removeOutput(std::shared_ptr<ILogOutput> output) = 0;
    
    /**
     * @brief 设置日志格式化器
     * 
     * @param formatter 格式化器
     */
    virtual void setFormatter(std::shared_ptr<ILogFormatter> formatter) = 0;
    
    /**
     * @brief 刷新所有输出器
     */
    virtual void flushAll() = 0;
};

/**
 * @brief 日志器实现类
 */
class Logger : public ILogger {
private:
    LogLevel m_minLevel;
    std::vector<std::shared_ptr<ILogOutput>> m_outputs;
    std::shared_ptr<ILogFormatter> m_formatter;
    std::mutex m_mutex;
    
public:
    Logger();
    ~Logger();
    
    void log(
        LogLevel level,
        const std::string& message,
        const std::string& category = "",
        const std::string& function = "",
        const std::string& file = "",
        int line = 0) override;
    
    void setLogLevel(LogLevel level) override;
    LogLevel getLogLevel() const override;
    
    void addOutput(std::shared_ptr<ILogOutput> output) override;
    void removeOutput(std::shared_ptr<ILogOutput> output) override;
    void setFormatter(std::shared_ptr<ILogFormatter> formatter) override;
    void flushAll() override;
};

/**
 * @brief 日志管理器
 */
class LogManager {
private:
    static std::shared_ptr<Logger> s_instance;
    static std::mutex s_mutex;
    
public:
    /**
     * @brief 获取全局日志器实例
     * 
     * @return 日志器实例
     */
    static std::shared_ptr<Logger> getLogger();
    
    /**
     * @brief 设置全局日志器实例
     * 
     * @param logger 日志器实例
     */
    static void setLogger(std::shared_ptr<Logger> logger);
    
    /**
     * @brief 初始化默认日志器
     * 
     * @param logToFile 是否记录到文件
     * @param logFilePath 日志文件路径
     */
    static void initialize(bool logToFile = false, const std::string& logFilePath = "");
};

/**
 * @brief 日志宏定义
 */
#define LOG_TRACE(message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Trace, message, "", __FUNCTION__, __FILE__, __LINE__)

#define LOG_DEBUG(message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Debug, message, "", __FUNCTION__, __FILE__, __LINE__)

#define LOG_INFO(message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Info, message, "", __FUNCTION__, __FILE__, __LINE__)

#define LOG_WARNING(message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Warning, message, "", __FUNCTION__, __FILE__, __LINE__)

#define LOG_ERROR(message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Error, message, "", __FUNCTION__, __FILE__, __LINE__)

#define LOG_CRITICAL(message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Critical, message, "", __FUNCTION__, __FILE__, __LINE__)

#define LOG_CATEGORY_TRACE(category, message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Trace, message, category, __FUNCTION__, __FILE__, __LINE__)

#define LOG_CATEGORY_DEBUG(category, message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Debug, message, category, __FUNCTION__, __FILE__, __LINE__)

#define LOG_CATEGORY_INFO(category, message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Info, message, category, __FUNCTION__, __FILE__, __LINE__)

#define LOG_CATEGORY_WARNING(category, message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Warning, message, category, __FUNCTION__, __FILE__, __LINE__)

#define LOG_CATEGORY_ERROR(category, message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Error, message, category, __FUNCTION__, __FILE__, __LINE__)

#define LOG_CATEGORY_CRITICAL(category, message) \
    BondForge::Utils::LogManager::getLogger()->log( \
        BondForge::Utils::LogLevel::Critical, message, category, __FUNCTION__, __FILE__, __LINE__)

} // namespace Utils
} // namespace BondForge