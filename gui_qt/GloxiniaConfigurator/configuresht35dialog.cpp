#include "configuresht35dialog.h"
#include "ui_configuresht35dialog.h"
#include <string>

ConfigureSHT35Dialog::ConfigureSHT35Dialog(QWidget *parent) : QDialog(parent),
                                                              ui(new Ui::ConfigureSHT35Dialog),
                                                              sensor(GCSensorSHT35())
{
    ui->setupUi(this);

    connect(ui->periodicityBox, &QComboBox::currentIndexChanged, this, &ConfigureSHT35Dialog::updatePeriodicity);
    connect(ui->confirmBox, &QDialogButtonBox::accepted, this, &ConfigureSHT35Dialog::apply);

    updatePeriodicity();

    updateUISettings();
}

ConfigureSHT35Dialog::~ConfigureSHT35Dialog()
{
    delete ui;
}

void ConfigureSHT35Dialog::apply(void)
{
    sensor.setPeriodicity(ui->periodicityBox->currentIndex());
    sensor.setClockStretching(ui->clockStretchingBox->isChecked());
    sensor.setI2CAddress(ui->addressBox->currentText().toInt(nullptr, 0));
    sensor.setRate(ui->rateBox->currentIndex());
    sensor.setRepeatability(ui->repeatabilityBox->currentIndex());

    hide();
}

void ConfigureSHT35Dialog::updateUISettings(void)
{
    ui->periodicityBox->setCurrentIndex(sensor.getPeriodicity());
    ui->clockStretchingBox->setChecked(sensor.getClockStretching() != 0);
    ui->addressBox->setCurrentIndex(sensor.i2cAddressToInt(sensor.getI2CAddress()));
    ui->rateBox->setCurrentIndex(sensor.getRate());
    ui->repeatabilityBox->setCurrentIndex(sensor.getRepeatability());
}

void ConfigureSHT35Dialog::updatePeriodicity()
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

void ConfigureSHT35Dialog::updateSensor(GCSensorSHT35 *s)
{
    s->setRepeatability(sensor.getRepeatability());
    s->setClockStretching(sensor.getClockStretching());
    s->setRate(sensor.getRate());
    s->setPeriodicity(sensor.getPeriodicity());
}

void ConfigureSHT35Dialog::setSensorSettings(GCSensorSHT35 *s)
{
    sensor = GCSensorSHT35(*s);
}
