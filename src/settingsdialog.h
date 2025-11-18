#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    
    // Getters for settings
    QString getGoogleMapsApiKey() const;
    QString getDefaultMapProvider() const;
    int getLogLevel() const;
    bool getLoadLastList() const;

private slots:
    void onShowGoogleKeyToggled(bool checked);
    void onRestoreDefaults();
    void accept() override;

private:
    Ui::SettingsDialog *ui;
    QSettings* m_settings;
    
    void loadSettings();
    void saveSettings();
    void restoreDefaults();
};

#endif // SETTINGSDIALOG_H
