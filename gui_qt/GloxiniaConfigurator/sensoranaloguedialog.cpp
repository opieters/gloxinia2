#include "sensoranaloguedialog.h"
#include "ui_sensoranaloguedialog.h"

SensorAnalogueDialog::SensorAnalogueDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorAnalogueDialog)
{
    ui->setupUi(this);
}

SensorAnalogueDialog::~SensorAnalogueDialog()
{
    delete ui;
}
