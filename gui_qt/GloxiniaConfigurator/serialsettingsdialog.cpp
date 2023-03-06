#include "serialsettingsdialog.h"
#include "ui_serialsettingsdialog.h"
#include <QSerialPortInfo>

SerialSettingsDialog::SerialSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialSettingsDialog)
{

    connect(ui->okBox, &QDialogButtonBox::rejected, this, &SerialSettingsDialog::cancel);
    connect(ui->okBox, &QDialogButtonBox::accepted, this, &SerialSettingsDialog::apply);
    connect(ui->comBox, &QComboBox::currentIndexChanged, this,  &SerialSettingsDialog::updateCOM);
    connect(ui->baudrateBox, &QSpinBox::valueChanged, this, &SerialSettingsDialog::updateBaudrate);

    settings.comPort = "";
    settings.baudrate = 500000;

    ui->setupUi(this);
}

SerialSettingsDialog::~SerialSettingsDialog()
{
    delete ui;
}

void SerialSettingsDialog::cancel(void)
{
    reject();
}

void SerialSettingsDialog::apply(void)
{
    settings.comPort = ui->comBox->currentText();
    settings.baudrate = ui->baudrateBox->value();

    if(settings.projectDir.isEmpty())
        settings.success = false;
    if(settings.projectName.isEmpty())
        settings.success = false;
    if((!settings.workOffline) && (ui->comBox->currentIndex() ))
        settings.success = false;

    if(!settings.comPort.isEmpty())
        accept();
    else
        reject();
}

void SerialSettingsDialog::updateCOM(void)
{
    settings.comPort = ui->comBox->currentText();
}

void SerialSettingsDialog::updateCOMList()
{
    QStringList comList;
    auto list = QSerialPortInfo::availablePorts();

    // store currently selected item
    QString selectedCOM = ui->comBox->currentText();

    // clear and add new items
    comList.clear();
    for (const QSerialPortInfo &i : list)
    {
        if(i.hasVendorIdentifier() && i.hasProductIdentifier())
        {
            if((i.vendorIdentifier() == 0x0403) && (i.productIdentifier() == 0x6001))
                comList.append(i.portName());
        }
    }

    // find index of selected COM port in list
    int index = comList.indexOf(selectedCOM);

    comList.sort();
    ui->comBox->clear();
    ui->comBox->addItems(comList);
    ui->comBox->setCurrentIndex(index);
}

void SerialSettingsDialog::updateBaudrate(void)
{
    settings.baudrate = ui->baudrateBox->value();

    update();
}
