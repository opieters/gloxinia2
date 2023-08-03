#include "gcreadoutdialog.h"
#include "ui_gcreadoutdialog.h"

GCReadoutDialog::GCReadoutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GCReadoutDialog)
{
    ui->setupUi(this);

    connect(ui->autoBox, &QCheckBox::stateChanged, this, &GCReadoutDialog::updateBoxes);
    connect(ui->confirmBox, &QDialogButtonBox::accepted, this, &GCReadoutDialog::readData);

    ui->autoBox->setCheckState(Qt::Checked);

    updateBoxes();
}

GCReadoutDialog::~GCReadoutDialog()
{
    delete ui;
}

void GCReadoutDialog::updateBoxes(void)
{
    // enable changing this box or not
    if(ui->autoBox->isChecked())
        ui->stopBox->setDisabled(true);
    else
        ui->stopBox->setDisabled(false);

    if(ui->startBox->value() < ADDRESS_START)
        ui->startBox->setValue(ADDRESS_START);


    if(ui->startBox->value() > ADDRESS_STOP)
        ui->startBox->setValue(ADDRESS_STOP);
    if(ui->stopBox->value() > ADDRESS_STOP)
        ui->stopBox->setValue(ADDRESS_STOP);

    if(ui->stopBox->value() < ui->startBox->value())
        ui->stopBox->setValue(ui->startBox->value());
}

void GCReadoutDialog::readData(void)
{

    int startAddress = ui->startBox->value();
    int stopAddress;
    if(ui->autoBox->isChecked())
        stopAddress = ADDRESS_AUTO_DETECT;
    else
        stopAddress = ui->stopBox->value() + 1;
}

GCReadoutDialog::Settings GCReadoutDialog::settings() const
{
    return {.startAddress = ui->startBox->value(), .stopAddress = ui->stopBox->value(), .autoDetect = ui->autoBox->isChecked()};
}
