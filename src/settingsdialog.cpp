#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "logger.h"
#include <QMessageBox>
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_settings(new QSettings("DataInquiry", "MapAddress", this))
{
    ui->setupUi(this);
    
    // Connect signals
    connect(ui->showGoogleKeyCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onShowGoogleKeyToggled);
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
            this, &SettingsDialog::onRestoreDefaults);
    
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::loadSettings()
{
    // Load API key
    QString apiKey = m_settings->value("Map/GoogleMapsApiKey", "").toString();
    ui->googleMapsApiKeyLineEdit->setText(apiKey);
    
    // Load default map provider
    QString defaultProvider = m_settings->value("Map/DefaultProvider", "osm").toString();
    if (defaultProvider == "google") {
        ui->googleMapsRadio->setChecked(true);
    } else {
        ui->osmRadio->setChecked(true);
    }
    
    // Load log level
    int logLevel = m_settings->value("General/LogLevel", 1).toInt(); // 1 = Info
    ui->logLevelComboBox->setCurrentIndex(logLevel);
    
    // Load startup settings
    bool loadLastList = m_settings->value("General/LoadLastList", true).toBool();
    ui->loadLastListCheckBox->setChecked(loadLastList);
}

void SettingsDialog::saveSettings()
{
    // Save API key
    m_settings->setValue("Map/GoogleMapsApiKey", ui->googleMapsApiKeyLineEdit->text());
    
    // Save default map provider
    QString provider = ui->googleMapsRadio->isChecked() ? "google" : "osm";
    m_settings->setValue("Map/DefaultProvider", provider);
    
    // Save log level
    m_settings->setValue("General/LogLevel", ui->logLevelComboBox->currentIndex());
    
    // Apply log level
    Logger::Level logLevel = static_cast<Logger::Level>(ui->logLevelComboBox->currentIndex());
    Logger::instance().setLogLevel(logLevel);
    LOG_INFO(QString("Log level changed to: %1").arg(ui->logLevelComboBox->currentText()));
    
    // Save startup settings
    m_settings->setValue("General/LoadLastList", ui->loadLastListCheckBox->isChecked());
    
    m_settings->sync();
    
    LOG_INFO("Settings saved");
}

void SettingsDialog::restoreDefaults()
{
    ui->googleMapsApiKeyLineEdit->clear();
    ui->osmRadio->setChecked(true);
    ui->logLevelComboBox->setCurrentIndex(1); // Info
    ui->loadLastListCheckBox->setChecked(true);
}

QString SettingsDialog::getGoogleMapsApiKey() const
{
    return ui->googleMapsApiKeyLineEdit->text();
}

QString SettingsDialog::getDefaultMapProvider() const
{
    return ui->googleMapsRadio->isChecked() ? "google" : "osm";
}

int SettingsDialog::getLogLevel() const
{
    return ui->logLevelComboBox->currentIndex();
}

bool SettingsDialog::getLoadLastList() const
{
    return ui->loadLastListCheckBox->isChecked();
}

void SettingsDialog::onShowGoogleKeyToggled(bool checked)
{
    ui->googleMapsApiKeyLineEdit->setEchoMode(
        checked ? QLineEdit::Normal : QLineEdit::Password
    );
}

void SettingsDialog::onRestoreDefaults()
{
    auto reply = QMessageBox::question(this, "Restore Defaults",
        "Are you sure you want to restore default settings?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        restoreDefaults();
    }
}

void SettingsDialog::accept()
{
    // Validate Google Maps API key if Google Maps is selected as default
    if (ui->googleMapsRadio->isChecked() && ui->googleMapsApiKeyLineEdit->text().isEmpty()) {
        auto reply = QMessageBox::warning(this, "Missing API Key",
            "Google Maps API key is empty. Google Maps may not work properly.\n\n"
            "Do you want to continue?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            return;
        }
    }
    
    saveSettings();
    QDialog::accept();
}
