#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <thread>
#include <condition_variable>

namespace BondForge {
namespace Utils {

// ConsoleLogFormatter 实现
ConsoleLogFormatter::ConsoleLogFormatter(bool useColors) 
    : m_useColors(useColors) {}

std::string ConsoleLogFormatter::format(const LogEntry& entry) {
    std::stringstream ss;
    
    // 时间戳
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    // 日志级别
    if (m_useColors) {
        ss << " " << getColorCode(entry.level) << "[" << getLevelName(entry.level) << "]\033[0m";
    } else {
        ss << " [" << getLevelName(entry.level) << "]";
    }
    
    // 线程ID（仅多线程环境）
    if (entry.threadId != std::thread::id()) {
        ss << " [T" << entry.threadId << "]";
    }
    
    // 类别
    if (!entry.category.empty()) {
        ss << " [" << entry.category << "]";
    }
    
    // 消息
    ss << " " << entry.message;
    
    // 文件和行号（仅调试模式）
    if (entry.level <= LogLevel::Debug) {
        ss << " (" << entry.file << ":" << entry.line;
        if (!entry.function.empty()) {
            ss << " in " << entry.function;
        }
        ss << ")";
    }
    
    return ss.str();
}

std::string ConsoleLogFormatter::getColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:   return "\033[37m";      // 白色
        case LogLevel::Debug:   return "\033[36m";      // 青色
        case LogLevel::Info:    return "\033[32m";      // 绿色
        case LogLevel::Warning: return "\033[33m";      // 黄色
        case LogLevel::Error:   return "\033[31m";      // 红色
        case LogLevel::Critical:return "\033[35m";      // 紫色
        default:              return "\033[37m";      // 白色（默认）
    }
}

std::string ConsoleLogFormatter::getLevelName(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Critical:return "CRITICAL";
        default:              return "UNKNOWN";
    }
}

// FileLogFormatter 实现
std::string FileLogFormatter::format(const LogEntry& entry) {
    std::stringstream ss;
    
    // 时间戳
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    // 日志级别
    ss << " [" << getLevelName(entry.level) << "]";
    
    // 线程ID
    if (entry.threadId != std::thread::id()) {
        ss << " [T" << entry.threadId << "]";
    }
    
    // 类别
    if (!entry.category.empty()) {
        ss << " [" << entry.category << "]";
    }
    
    // 文件和行号
    if (!entry.file.empty()) {
        ss << " (" << entry.file;
        if (entry.line > 0) {
            ss << ":" << entry.line;
        }
        if (!entry.function.empty()) {
            ss << " in " << entry.function;
        }
        ss << ")";
    }
    
    // 消息
    ss << " " << entry.message;
    
    return ss.str();
}

// ConsoleLogOutput 实现
void ConsoleLogOutput::write(const std::string& message) {
    std::cout << message << std::endl;
}

void ConsoleLogOutput::flush() {
    std::cout.flush();
}

// FileLogOutput 实现
FileLogOutput::FileLogOutput(const std::string& filePath) 
    : m_filePath(filePath) {
    // 确保日志目录存在
    std::filesystem::path path(filePath);
    std::filesystem::path dir = path.parent_path();
    
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
}

void FileLogOutput::write(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_fileMutex);
    
    std::ofstream file(m_filePath, std::ios::app);
    if (file.is_open()) {
        file << message << std::endl;
        file.close();
    }
}

void FileLogOutput::flush() {
    // 文件流通常自动刷新，这里不做特殊处理
}

// AsyncFileLogOutput 实现
AsyncFileLogOutput::AsyncFileLogOutput(const std::string& filePath) 
    : m_filePath(filePath)
    , m_running(true) {
    
    // 确保日志目录存在
    std::filesystem::path path(filePath);
    std::filesystem::path dir = path.parent_path();
    
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    
    // 启动写入线程
    m_writerThread = std::make_unique<std::thread>(&AsyncFileLogOutput::writerThread, this);
}

AsyncFileLogOutput::~AsyncFileLogOutput() {
    // 停止写入线程
    m_running = false;
    
    // 唤醒线程
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        // 通过添加空消息唤醒线程
    }
    
    // 等待线程结束
    if (m_writerThread && m_writerThread->joinable()) {
        m_writerThread->join();
    }
}

void AsyncFileLogOutput::write(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_messageQueue.push(message);
}

void AsyncFileLogOutput::flush() {
    // 异步模式下不保证立即刷新
    // 实际应用中可以添加条件变量来确保消息被写入
}

void AsyncFileLogOutput::writerThread() {
    while (m_running) {
        std::string message;
        
        // 等待消息
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            if (m_messageQueue.empty()) {
                // 简化处理，实际应用中可以使用条件变量
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            message = m_messageQueue.front();
            m_messageQueue.pop();
        }
        
        // 写入文件
        {
            std::lock_guard<std::mutex> lock(m_fileMutex);
            std::ofstream file(m_filePath, std::ios::app);
            if (file.is_open()) {
                file << message << std::endl;
                file.close();
            }
        }
    }
}

// Logger 实现
Logger::Logger() 
    : m_minLevel(LogLevel::Info) 
    , m_formatter(std::make_shared<ConsoleLogFormatter>()) {
    
    // 默认添加控制台输出
    addOutput(std::make_shared<ConsoleLogOutput>());
}

Logger::~Logger() {
    flushAll();
}

void Logger::log(
    LogLevel level,
    const std::string& message,
    const std::string& category,
    const std::string& function,
    const std::string& file,
    int line) {
    
    // 检查日志级别
    if (level < m_minLevel) {
        return;
    }
    
    // 创建日志记录
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = level;
    entry.category = category;
    entry.message = message;
    entry.function = function;
    entry.file = file;
    entry.line = line;
    entry.threadId = std::this_thread::get_id();
    
    // 格式化日志消息
    if (m_formatter) {
        std::string formattedMessage = m_formatter->format(entry);
        
        // 输出到所有输出器
        for (auto& output : m_outputs) {
            output->write(formattedMessage);
        }
    }
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

LogLevel Logger::getLogLevel() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_minLevel;
}

void Logger::addOutput(std::shared_ptr<ILogOutput> output) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_outputs.push_back(output);
}

void Logger::removeOutput(std::shared_ptr<ILogOutput> output) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_outputs.erase(
        std::remove(m_outputs.begin(), m_outputs.end(), output),
        m_outputs.end()
    );
}

void Logger::setFormatter(std::shared_ptr<ILogFormatter> formatter) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_formatter = formatter;
}

void Logger::flushAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& output : m_outputs) {
        output->flush();
    }
}

// LogManager 实现
std::shared_ptr<Logger> LogManager::s_instance;
std::mutex LogManager::s_mutex;

std::shared_ptr<Logger> LogManager::getLogger() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (!s_instance) {
        s_instance = std::make_shared<Logger>();
    }
    
    return s_instance;
}

void LogManager::setLogger(std::shared_ptr<Logger> logger) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_instance = logger;
}

void LogManager::initialize(bool logToFile, const std::string& logFilePath) {
    auto logger = getLogger();
    
    if (logToFile && !logFilePath.empty()) {
        // 添加文件输出器
        logger->addOutput(std::make_shared<AsyncFileLogOutput>(logFilePath));
        
        // 设置文件格式化器
        logger->setFormatter(std::make_shared<FileLogFormatter>());
    }
    
    LOG_INFO("Logger initialized");
}

} // namespace Utils
} // namespace BondForge