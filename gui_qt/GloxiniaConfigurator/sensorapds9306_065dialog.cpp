#include "sensorapds9306_065dialog.h"
#include "ui_sensorapds9306_065dialog.h"

SensorAPDS9306_065Dialog::SensorAPDS9306_065Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorAPDS9306_065Dialog),
    sensor(new GCSensorAPDS9306())
{
    ui->setupUi(this);

    // disable address selection since only one option
    ui->addressBox->setDisabled(true);

    connect(ui->confirmBox, &QDialogButtonBox::accepted, this, &SensorAPDS9306_065Dialog::apply);

    updateUISettings();
}

SensorAPDS9306_065Dialog::~SensorAPDS9306_065Dialog()
{
    delete sensor;
    delete ui;
}

void SensorAPDS9306_065Dialog::updateUISettings()
{
    ui->rateBox->setCurrentIndex(sensor->getAlsMeasurementRate());
    ui->resolutionBox->setCurrentIndex(sensor->getAlsResolution());
    ui->gainBox->setCurrentIndex(sensor->getAlsGain());
}
void SensorAPDS9306_065Dialog::setSensorSettings(GCSensorAPDS9306* s)
{
    delete sensor;
    sensor = new GCSensorAPDS9306(*s);
}
void SensorAPDS9306_065Dialog::apply()
{
    sensor->setI2CAddress(ui->addressBox->currentText().toInt(nullptr, 0));
    sensor->setAlsMeasurementRate(ui->rateBox->currentIndex());
    sensor->setAlsResolution(ui->resolutionBox->currentIndex());
    sensor->setAlsGain(ui->gainBox->currentIndex());

    // options that are currently not used and set to default values
    sensor->setAlsIVCount(0);
    sensor->setAlsTHHigh(0);
    sensor->setAlsTHLow(0);

    hide();
}
void SensorAPDS9306_065Dialog::updateSensor(GCSensorAPDS9306* s)
{
    s->setI2CAddress(sensor->getI2CAddress());
    s->setMeasurementPeriod(sensor->getMeasurementPeriod());

    s->setAlsMeasurementRate(sensor->getAlsMeasurementRate());
    s->setAlsResolution(sensor->getAlsResolution());
    s->setAlsGain(sensor->getAlsGain());
    s->setAlsIVCount(sensor->getAlsIVCount());
    s->setAlsTHHigh(sensor->getAlsTHHigh());
    s->setAlsTHLow(sensor->getAlsTHLow());
}
