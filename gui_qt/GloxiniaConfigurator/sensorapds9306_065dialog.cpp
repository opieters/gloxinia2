#include "sensorapds9306_065dialog.h"
#include "ui_sensorapds9306_065dialog.h"

SensorAPDS9306_065Dialog::SensorAPDS9306_065Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorAPDS9306_065Dialog),
    gDialog(nullptr)
{
    ui->setupUi(this);

    // disable address selection since only one option
    ui->addressBox->setDisabled(true);


    connect(ui->gperiodButton, &QPushButton::clicked, this, &SensorAPDS9306_065Dialog::editGlobalPeriodSettings);
    connect(ui->gperiodBox, &QCheckBox::stateChanged, this, &SensorAPDS9306_065Dialog::useGlobalPeriodToggle);

    useGlobalPeriodToggle();
}

SensorAPDS9306_065Dialog::~SensorAPDS9306_065Dialog()
{
    delete ui;
}

void SensorAPDS9306_065Dialog::useGlobalPeriodToggle()
{
    if(ui->gperiodBox->isChecked()){
        if(gDialog != nullptr)
            ui->periodBox->setValue(gDialog->getPeriod());
        ui->periodBox->setEnabled(false);
    } else {
        ui->periodBox->setEnabled(true);
    }
}

void SensorAPDS9306_065Dialog::editGlobalPeriodSettings(void)
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


void SensorAPDS9306_065Dialog::updateUISettings(GCSensorAPDS9306* sensor)
{
    ui->rateBox->setCurrentIndex(sensor->getAlsMeasurementRate());
    ui->resolutionBox->setCurrentIndex(sensor->getAlsResolution());
    ui->gainBox->setCurrentIndex(sensor->getAlsGain());
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

void SensorAPDS9306_065Dialog::apply(GCSensorAPDS9306* sensor)
{
    sensor->setI2CAddress(ui->addressBox->currentText().toInt(nullptr, 0));
    sensor->setAlsMeasurementRate(ui->rateBox->currentIndex());
    sensor->setAlsResolution(ui->resolutionBox->currentIndex());
    sensor->setAlsGain(ui->gainBox->currentIndex());

    // options that are currently not used and set to default values
    sensor->setAlsIVCount(0);
    sensor->setAlsTHHigh(0);
    sensor->setAlsTHLow(0);

    quint16 period = round(ui->periodBox->value()*10) - 1;
    sensor->setMeasurementPeriod(period);
    sensor->setUseGlobalPeriodFlag(ui->gperiodBox->isChecked());

    hide();
}



void SensorAPDS9306_065Dialog::setPeriodDialog(GlobalMeasurementPolicyDialog* dialog)
{
    gDialog = dialog;
}

