#include "sensorsht35dialog.h"
#include "ui_sensorsht35dialog.h"
#include <string>

SensorSHT35Dialog::SensorSHT35Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorSHT35Dialog),
    gDialog(nullptr),
    sensor(new GCSensorSHT35())
{
    ui->setupUi(this);

    connect(ui->periodicityBox, &QComboBox::currentIndexChanged, this, &SensorSHT35Dialog::updatePeriodicity);
    connect(ui->confirmBox, &QDialogButtonBox::accepted, this, &SensorSHT35Dialog::apply);
    connect(ui->gperiodButton, &QPushButton::clicked, this, &SensorSHT35Dialog::editGlobalPeriodSettings);

    updatePeriodicity();

    updateUISettings();
}

SensorSHT35Dialog::~SensorSHT35Dialog()
{
    delete sensor;
    delete ui;
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

    globalPeriod = gDialog->getPeriod();
    if(useGlobalPeriod)
        period = globalPeriod;

    update();
}

void SensorSHT35Dialog::apply(void)
{
    sensor->setPeriodicity(ui->periodicityBox->currentIndex());
    sensor->setClockStretching(ui->clockStretchingBox->isChecked());
    sensor->setI2CAddress(ui->addressBox->currentText().toInt(nullptr, 0));
    sensor->setRate(ui->rateBox->currentIndex());
    sensor->setRepeatability(ui->repeatabilityBox->currentIndex());

    if(useGlobalPeriod){
        period = globalPeriod;
    } else {
        period = round(ui->periodBox->value()*10) - 1;
    }

    hide();
}

void SensorSHT35Dialog::updateUISettings(void)
{
    ui->periodicityBox->setCurrentIndex(sensor->getPeriodicity());
    ui->clockStretchingBox->setChecked(sensor->getClockStretching() != 0);
    ui->addressBox->setCurrentIndex(sensor->i2cAddressToInt(sensor->getI2CAddress()));
    ui->rateBox->setCurrentIndex(sensor->getRate());
    ui->repeatabilityBox->setCurrentIndex(sensor->getRepeatability());

    if(gDialog != nullptr)
    {
        ui->gperiodButton->setDisabled(false);
    } else {
        ui->gperiodButton->setDisabled(true);
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
    apply();

    s->setRepeatability(sensor->getRepeatability());
    s->setClockStretching(sensor->getClockStretching());
    s->setRate(sensor->getRate());
    s->setPeriodicity(sensor->getPeriodicity());
    s->setMeasurementPeriod(period);
    s->setUseGlobalPeriodFlag(useGlobalPeriod);
}

void SensorSHT35Dialog::setSensorSettings(GCSensorSHT35 *s)
{
    delete sensor;
    sensor = new GCSensorSHT35(*s);
}

void SensorSHT35Dialog::setPeriodDialog(GlobalMeasurementPolicyDialog* dialog)
{
    gDialog = dialog;

    updateUISettings();
}

