#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include "mainwindow.h"
#include "database.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    // Enable remote debugging for Qt WebEngine
    // Access at chrome://inspect or edge://inspect in Chromium-based browsers
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "9222");
    
    QApplication app(argc, argv);
    app.setApplicationName("MapAddress");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("DataInquiry");
    
    // Set application icon
    QIcon appIcon;
    appIcon.addFile(":/icons/icons/app-icon-16.png");
    appIcon.addFile(":/icons/icons/app-icon-32.png");
    appIcon.addFile(":/icons/icons/app-icon-48.png");
    appIcon.addFile(":/icons/icons/app-icon-64.png");
    appIcon.addFile(":/icons/icons/app-icon-128.png");
    appIcon.addFile(":/icons/icons/app-icon-256.png");
    appIcon.addFile(":/icons/icons/app-icon-512.png");
    app.setWindowIcon(appIcon);
    
    // Initialize logger
    LOG_INFO("=== MapAddress Application Started ===");
    LOG_INFO(QString("Version: %1").arg(app.applicationVersion()));
    
    // Initialize database
    LOG_INFO("Initializing database...");
    if (!Database::instance().initialize()) {
        QString error = Database::instance().getLastError();
        LOG_ERROR("Database initialization failed: " + error);
        QMessageBox::critical(nullptr, "Database Error", 
            "Failed to initialize database: " + error);
        return 1;
    }
    LOG_INFO("Database initialized successfully");
    
    LOG_INFO("Creating main window...");
    MainWindow window;
    window.show();
    LOG_INFO("Main window displayed");
    
    int result = app.exec();
    LOG_INFO("=== MapAddress Application Closed ===");
    return result;
}
