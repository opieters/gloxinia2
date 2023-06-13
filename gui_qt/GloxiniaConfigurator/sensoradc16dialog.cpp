#include "sensoradc16dialog.h"
#include "ui_sensoradc16dialog.h"

SensorADC16Dialog::SensorADC16Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorADC16Dialog),
    gDialog(nullptr)
{
    ui->setupUi(this);

    connect(ui->gPeriodButton, &QPushButton::clicked, this, &SensorADC16Dialog::editGlobalPeriodSettings);
    connect(ui->gPeriodBox, &QCheckBox::stateChanged, this, &SensorADC16Dialog::useGlobalPeriodToggle);

    ui->periodBox->setValue(1.0);

    useGlobalPeriodToggle();
}

SensorADC16Dialog::~SensorADC16Dialog()
{
    delete ui;
}


void SensorADC16Dialog::useGlobalPeriodToggle(void)
{
    if(ui->gPeriodBox->isChecked()){
        if(gDialog != nullptr)
            ui->periodBox->setValue(gDialog->getPeriod());
        ui->periodBox->setEnabled(false);
    } else {
        ui->periodBox->setEnabled(true);
    }
}

void SensorADC16Dialog::editGlobalPeriodSettings(void)
{
    int result;

    if(gDialog == nullptr){
        return;
    }

    result = gDialog->exec();
    if(result == QDialog::Rejected)
        return;

    if(ui->gPeriodBox->isChecked()){
        ui->periodBox->setValue(gDialog->getPeriod());
    }
}

void SensorADC16Dialog::apply(GCSensorADC16* sensor)
{
    sensor->setAverage(ui->averageBox->isChecked());

    quint16 period = round(ui->periodBox->value()*10) - 1;
    sensor->setMeasurementPeriod(period);
    sensor->setUseGlobalPeriodFlag(ui->gPeriodBox->isChecked());

    switch(ui->gainBox->currentIndex())
    {
    case 0:
        // for now, auto defaults to 1. This might change in the future
        sensor->setGain(GCSensorADC16::PGA_GAIN_1);
        break;
    case 1:
        sensor->setGain(GCSensorADC16::PGA_GAIN_1);
        break;
    case 2:
        sensor->setGain(GCSensorADC16::PGA_GAIN_2);
        break;
    case 3:
        sensor->setGain(GCSensorADC16::PGA_GAIN_5);
        break;
    case 4:
        sensor->setGain(GCSensorADC16::PGA_GAIN_10);
        break;
    case 5:
        sensor->setGain(GCSensorADC16::PGA_GAIN_20);
        break;
    case 6:
        sensor->setGain(GCSensorADC16::PGA_GAIN_50);
        break;
    case 7:
        sensor->setGain(GCSensorADC16::PGA_GAIN_100);
        break;
    case 8:
        sensor->setGain(GCSensorADC16::PGA_GAIN_200);
        break;
    default:
        sensor->setGain(GCSensorADC16::PGA_GAIN_1);
    }

    hide();
}

void SensorADC16Dialog::updateUISettings(GCSensorADC16* sensor)
{
    ui->averageBox->setChecked(sensor->getAverage());
    ui->periodBox->setValue((sensor->getMeasurementPeriod()+1.0)/10);
    if(sensor->getUseGlobalPeriodFlag())
        ui->gPeriodBox->setCheckState(Qt::Checked);
    else
        ui->gPeriodBox->setCheckState(Qt::Unchecked);

    if(gDialog != nullptr)
    {
        ui->gPeriodButton->setDisabled(false);
        ui->gPeriodBox->setDisabled(false);
    } else {
        ui->gPeriodButton->setDisabled(true);
        ui->gPeriodBox->setDisabled(true);
    }

    ui->gainBox->setCurrentIndex(sensor->getGain()+1); // +1 to skip "auto"

    useGlobalPeriodToggle();
}

void SensorADC16Dialog::setPeriodDialog(GlobalMeasurementPolicyDialog* dialog)
{
    gDialog = dialog;
}

