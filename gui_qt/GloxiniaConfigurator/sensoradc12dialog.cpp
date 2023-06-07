#include "sensoradc12dialog.h"
#include "ui_SensorADC12Dialog.h"
#include <string>

SensorADC12Dialog::SensorADC12Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorADC12Dialog),
    gDialog(nullptr)
{
    ui->setupUi(this);

    connect(ui->gperiodButton, &QPushButton::clicked, this, &SensorADC12Dialog::editGlobalPeriodSettings);
    connect(ui->gperiodBox, &QCheckBox::stateChanged, this, &SensorADC12Dialog::useGlobalPeriodToggle);

    ui->periodBox->setValue(1.0);

    useGlobalPeriodToggle();
}

SensorADC12Dialog::~SensorADC12Dialog()
{
    delete ui;
}


void SensorADC12Dialog::useGlobalPeriodToggle(void)
{
    if(ui->gperiodBox->isChecked()){
        if(gDialog != nullptr)
            ui->periodBox->setValue(gDialog->getPeriod());
        ui->periodBox->setEnabled(false);
    } else {
        ui->periodBox->setEnabled(true);
    }
}

void SensorADC12Dialog::editGlobalPeriodSettings(void)
{
    int result;

    if(gDialog == nullptr){
        return;
    }

    result = gDialog->exec();
    if(result == QDialog::Rejected)
        return;

    if(ui->gperiodBox->isChecked()){
    ui->periodBox->setValue(gDialog->getPeriod());
    }
}

void SensorADC12Dialog::apply(GCSensorADC12* sensor)
{
    sensor->setAverage(ui->enableBox->isChecked());

    quint16 period = round(ui->periodBox->value()*10) - 1;
    sensor->setMeasurementPeriod(period);
    sensor->setUseGlobalPeriodFlag(ui->gperiodBox->isChecked());

    hide();
}

void SensorADC12Dialog::updateUISettings(GCSensorADC12* sensor)
{
    ui->enableBox->setChecked(sensor->getAverage());
    ui->periodBox->setValue((sensor->getMeasurementPeriod()+1.0)/10);
    if(sensor->getUseGlobalPeriodFlag())
        ui->gperiodBox->setCheckState(Qt::Checked);
    else
        ui->gperiodBox->setCheckState(Qt::Unchecked);

    if(gDialog != nullptr)
    {
        ui->gperiodButton->setDisabled(false);
        ui->gperiodBox->setDisabled(false);
    } else {
        ui->gperiodButton->setDisabled(true);
        ui->gperiodBox->setDisabled(true);
    }

    useGlobalPeriodToggle();
}

void SensorADC12Dialog::setPeriodDialog(GlobalMeasurementPolicyDialog* dialog)
{
    gDialog = dialog;
}

