#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

struct GCAppSettings {
    qint64 messageBufferSize;
    qint64 plotBufferWindow;
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    GCAppSettings getApplicationSettings(void);
    void setApplicationSettings(GCAppSettings& settings);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
