#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

struct GCProjectSettings {
    qint64 messageBufferSize;
    qint64 plotBufferWindow;

    bool success;
    QString projectName;
    QString projectDir;
    QString comPort;
    int baudrate;
    bool workOffline;
    bool runDiscovery;

    quint16 globalMeasurementPeriod;
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void updateApplicationSettings(GCProjectSettings& settings);
    void updateDialog(GCProjectSettings& settings);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
