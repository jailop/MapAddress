#include "logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QMutexLocker>
#include <iostream>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : m_logLevel(Info) {
    // Default log file location
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appData);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QString logPath = appData + "/mapaddress.log";
    setLogFile(logPath);
}

Logger::~Logger() {
    if (m_logFile.isOpen()) {
        m_stream.flush();
        m_logFile.close();
    }
}

void Logger::setLogFile(const QString& filePath) {
    QMutexLocker locker(&m_mutex);
    
    if (m_logFile.isOpen()) {
        m_stream.flush();
        m_logFile.close();
    }
    
    m_logFile.setFileName(filePath);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_stream.setDevice(&m_logFile);
    }
}

void Logger::setLogLevel(Level level) {
    m_logLevel = level;
}

void Logger::log(Level level, const QString& message) {
    if (level < m_logLevel) {
        return;
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString levelStr = levelToString(level);
    QString logMessage = QString("[%1] [%2] %3").arg(timestamp).arg(levelStr).arg(message);
    
    // Write to console
    if (level >= Warning) {
        std::cerr << logMessage.toStdString() << std::endl;
    } else {
        std::cout << logMessage.toStdString() << std::endl;
    }
    
    // Write to file
    writeToFile(logMessage);
}

void Logger::debug(const QString& message) {
    log(Debug, message);
}

void Logger::info(const QString& message) {
    log(Info, message);
}

void Logger::warning(const QString& message) {
    log(Warning, message);
}

void Logger::error(const QString& message) {
    log(Error, message);
}

QString Logger::levelToString(Level level) const {
    switch (level) {
        case Debug: return "DEBUG";
        case Info: return "INFO";
        case Warning: return "WARNING";
        case Error: return "ERROR";
        default: return "UNKNOWN";
    }
}

void Logger::writeToFile(const QString& message) {
    QMutexLocker locker(&m_mutex);
    
    if (m_logFile.isOpen()) {
        m_stream << message << "\n";
        m_stream.flush();
    }
}
