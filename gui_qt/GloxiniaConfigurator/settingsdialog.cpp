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


GCProjectSettings SettingsDialog::getApplicationSettings(void)
{
    GCProjectSettings settings;

    settings.messageBufferSize = ui->mBufferEdit->value();
    settings.plotBufferWindow = ui->plotBufferEdit->value();

    return settings;
}

void SettingsDialog::setApplicationSettings(GCProjectSettings& settings)
{
    ui->mBufferEdit->setValue(settings.messageBufferSize);
    ui->plotBufferEdit->setValue(settings.plotBufferWindow);
}
