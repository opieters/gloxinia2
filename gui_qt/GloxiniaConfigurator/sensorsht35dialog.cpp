#include "sensorsht35dialog.h"
#include "ui_sensorsht35dialog.h"
#include <string>

SensorSHT35Dialog::SensorSHT35Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorSHT35Dialog),
    mDialog(nullptr),
    sensor(new GCSensorSHT35())
{
    ui->setupUi(this);

    connect(ui->periodicityBox, &QComboBox::currentIndexChanged, this, &SensorSHT35Dialog::updatePeriodicity);
    connect(ui->confirmBox, &QDialogButtonBox::accepted, this, &SensorSHT35Dialog::apply);
    connect(ui->periodButton, &QPushButton::clicked, this, &SensorSHT35Dialog::updatePeriodSettings);

    updatePeriodicity();

    updateUISettings();
}

SensorSHT35Dialog::~SensorSHT35Dialog()
{
    delete sensor;
    delete ui;
}

void SensorSHT35Dialog::apply(void)
{
    sensor->setPeriodicity(ui->periodicityBox->currentIndex());
    sensor->setClockStretching(ui->clockStretchingBox->isChecked());
    sensor->setI2CAddress(ui->addressBox->currentText().toInt(nullptr, 0));
    sensor->setRate(ui->rateBox->currentIndex());
    sensor->setRepeatability(ui->repeatabilityBox->currentIndex());

    hide();
}

void SensorSHT35Dialog::updateUISettings(void)
{
    ui->periodicityBox->setCurrentIndex(sensor->getPeriodicity());
    ui->clockStretchingBox->setChecked(sensor->getClockStretching() != 0);
    ui->addressBox->setCurrentIndex(sensor->i2cAddressToInt(sensor->getI2CAddress()));
    ui->rateBox->setCurrentIndex(sensor->getRate());
    ui->repeatabilityBox->setCurrentIndex(sensor->getRepeatability());

    if(mDialog != nullptr)
    {
        ui->periodButton->setDisabled(false);
    } else {
        ui->periodButton->setDisabled(true);
    }
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

void SensorSHT35Dialog::updateSensor(GCSensorSHT35 *s)
{
    s->setRepeatability(sensor->getRepeatability());
    s->setClockStretching(sensor->getClockStretching());
    s->setRate(sensor->getRate());
    s->setPeriodicity(sensor->getPeriodicity());
}

void SensorSHT35Dialog::setSensorSettings(GCSensorSHT35 *s)
{
    delete sensor;
    sensor = new GCSensorSHT35(*s);
}

void SensorSHT35Dialog::setPeriodDialog(SensorMeasurementDialog* dialog)
{
    mDialog = dialog;

    updateUISettings();
}

void SensorSHT35Dialog::updatePeriodSettings(void)
{
    int result;
    if(mDialog == nullptr){
        return;
    }
    result = mDialog->exec();
    if(result == QDialog::Rejected){
        return;
    }
    sensor->setMeasurementPeriod(mDialog->getPeriod());
    sensor->setUseGlobalPeriodFlag(mDialog->getUseGlobalPeriod());
}

