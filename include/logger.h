#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class Logger {
public:
    enum Level {
        Debug,
        Info,
        Warning,
        Error
    };

    static Logger& instance();
    
    void log(Level level, const QString& message);
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    
    void setLogFile(const QString& filePath);
    void setLogLevel(Level level);
    
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    QString levelToString(Level level) const;
    void writeToFile(const QString& message);
    
    QFile m_logFile;
    QTextStream m_stream;
    Level m_logLevel;
    QMutex m_mutex;
};

// Convenience macros
#define LOG_DEBUG(msg) Logger::instance().debug(msg)
#define LOG_INFO(msg) Logger::instance().info(msg)
#define LOG_WARNING(msg) Logger::instance().warning(msg)
#define LOG_ERROR(msg) Logger::instance().error(msg)

#endif // LOGGER_H
