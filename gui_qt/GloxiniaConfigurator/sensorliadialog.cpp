#include "sensorliadialog.h"
#include "ui_sensorliadialog.h"

SensorLIADialog::SensorLIADialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorLIADialog)
{
    ui->setupUi(this);
}

SensorLIADialog::~SensorLIADialog()
{
    delete ui;
}
