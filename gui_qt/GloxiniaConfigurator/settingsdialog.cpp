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


void SettingsDialog::updateApplicationSettings(GCProjectSettings& settings)
{
    settings.messageBufferSize = ui->mBufferEdit->value();
    settings.plotBufferWindow = ui->plotBufferEdit->value();
}

void SettingsDialog::updateDialog(GCProjectSettings& settings)
{
    ui->mBufferEdit->setValue(settings.messageBufferSize);
    ui->plotBufferEdit->setValue(settings.plotBufferWindow);
}
