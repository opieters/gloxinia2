#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


GCAppSettings SettingsDialog::getApplicationSettings(void)
{
    GCAppSettings settings;

    settings.messageBufferSize = ui->mBufferEdit->value();
    settings.plotBufferSize = ui->plotBufferEdit->value();

    return settings;
}

void SettingsDialog::setApplicationSettings(GCAppSettings& settings)
{
    ui->mBufferEdit->setValue(settings.messageBufferSize);
    ui->plotBufferEdit->setValue(settings.plotBufferSize);
}
