#include "sensorsht35dialog.h"
#include "ui_sensorsht35dialog.h"
#include <string>

SensorSHT35Dialog::SensorSHT35Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorSHT35Dialog),
    gDialog(nullptr)
{
    ui->setupUi(this);

    connect(ui->periodicityBox, &QComboBox::currentIndexChanged, this, &SensorSHT35Dialog::updatePeriodicity);
    connect(ui->gperiodButton, &QPushButton::clicked, this, &SensorSHT35Dialog::editGlobalPeriodSettings);
    connect(ui->gperiodBox, &QCheckBox::stateChanged, this, &SensorSHT35Dialog::useGlobalPeriodToggle);

    ui->periodBox->setValue(1.0);
    ui->clockStretchingBox->setCheckState(Qt::Unchecked);

    updatePeriodicity();
    useGlobalPeriodToggle();
}

SensorSHT35Dialog::~SensorSHT35Dialog()
{
    delete ui;
}


void SensorSHT35Dialog::useGlobalPeriodToggle(void)
{
    if(ui->gperiodBox->isChecked()){
        if(gDialog != nullptr)
            ui->periodBox->setValue(gDialog->getPeriod());
        ui->periodBox->setEnabled(false);
    } else {
        ui->periodBox->setEnabled(true);
    }
}

void SensorSHT35Dialog::editGlobalPeriodSettings(void)
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

void SensorSHT35Dialog::apply(GCSensorSHT35* sensor)
{
    sensor->setPeriodicity(ui->periodicityBox->currentIndex());
    sensor->setClockStretching(ui->clockStretchingBox->isChecked());
    sensor->setI2CAddress(ui->addressBox->currentText().toInt(nullptr, 0));
    sensor->setRate(ui->rateBox->currentIndex());
    sensor->setRepeatability(ui->repeatabilityBox->currentIndex());

    quint16 period = round(ui->periodBox->value()*10) - 1;
    sensor->setMeasurementPeriod(period);
    sensor->setUseGlobalPeriodFlag(ui->gperiodBox->isChecked());

    hide();
}

void SensorSHT35Dialog::updateUISettings(GCSensorSHT35* sensor)
{
    ui->periodicityBox->setCurrentIndex(sensor->getPeriodicity());
    ui->clockStretchingBox->setChecked(sensor->getClockStretching() != 0);
    ui->addressBox->setCurrentIndex(sensor->i2cAddressToInt(sensor->getI2CAddress()));
    ui->rateBox->setCurrentIndex(sensor->getRate());
    ui->repeatabilityBox->setCurrentIndex(sensor->getRepeatability());
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

void SensorSHT35Dialog::updatePeriodicity()
{
    int index = ui->periodicityBox->currentIndex();
    if (index == 0)
    {
        // disable sample rate selection
        ui->rateBox->setDisabled(true);
    }
    else
    {
        ui->rateBox->setDisabled(false);
    }
}

void SensorSHT35Dialog::setPeriodDialog(GlobalMeasurementPolicyDialog* dialog)
{
    gDialog = dialog;
}

