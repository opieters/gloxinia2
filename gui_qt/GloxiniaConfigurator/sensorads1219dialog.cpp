#include "sensorads1219dialog.h"
#include "ui_sensorads1219dialog.h"

SensorADS1219Dialog::SensorADS1219Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorADS1219Dialog)
{
    ui->setupUi(this);

    ui->addressBox->setValue(0x40);

    connect(ui->gperiodButton, &QPushButton::clicked, this, &SensorADS1219Dialog::editGlobalPeriodSettings);
    connect(ui->gperiodBox, &QCheckBox::stateChanged, this, &SensorADS1219Dialog::useGlobalPeriodToggle);

    useGlobalPeriodToggle();
}

SensorADS1219Dialog::~SensorADS1219Dialog()
{
    delete ui;
}

void SensorADS1219Dialog::editGlobalPeriodSettings(void)
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


void SensorADS1219Dialog::updateUISettings(GCSensorADS1219* sensor)
{
    ui->addressBox->setValue(sensor->getI2CAddress());

    auto channelSelection = sensor->getEnabledChannels();
    if((channelSelection & 0b00000001) != 0)
        ui->ch1Box->setCheckState(Qt::Checked);
    else
        ui->ch1Box->setCheckState(Qt::Unchecked);
    if((channelSelection & 0b00000010) != 0)
        ui->ch1CalBox->setCheckState(Qt::Checked);
    else
        ui->ch1CalBox->setCheckState(Qt::Unchecked);
    if((channelSelection & 0b00000100) != 0)
        ui->ch2Box->setCheckState(Qt::Checked);
    else
        ui->ch2Box->setCheckState(Qt::Unchecked);
    if((channelSelection & 0b00001000) != 0)
        ui->ch2CalBox->setCheckState(Qt::Checked);
    else
        ui->ch2CalBox->setCheckState(Qt::Unchecked);
    if((channelSelection & 0b00010000) != 0)
        ui->ch3Box->setCheckState(Qt::Checked);
    else
        ui->ch3Box->setCheckState(Qt::Unchecked);
    if((channelSelection & 0b00100000) != 0)
        ui->ch3CalBox->setCheckState(Qt::Checked);
    else
        ui->ch3CalBox->setCheckState(Qt::Unchecked);
    if((channelSelection & 0b01000000) != 0)
        ui->ch4Box->setCheckState(Qt::Checked);
    else
        ui->ch4Box->setCheckState(Qt::Unchecked);
    if((channelSelection & 0b10000000) != 0)
        ui->ch4CalBox->setCheckState(Qt::Checked);
    else
        ui->ch4CalBox->setCheckState(Qt::Unchecked);

    ui->gainBox->setCurrentIndex(sensor->getGain());
    ui->vrefBox->setCurrentIndex(sensor->getReferenceVoltage());
    ui->periodBox->setValue((sensor->getMeasurementPeriod()+1.0) / 10);

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

void SensorADS1219Dialog::apply(GCSensorADS1219* sensor)
{
    sensor->setI2CAddress(ui->addressBox->value());

    quint8 channelSelection = 0;
    if(ui->ch1Box->isChecked())
        channelSelection |= 0b00000001;
    if(ui->ch1CalBox->isChecked())
        channelSelection |= 0b00000010;
    if(ui->ch2Box->isChecked())
        channelSelection |= 0b00000100;
    if(ui->ch2CalBox->isChecked())
        channelSelection |= 0b00001000;
    if(ui->ch3Box->isChecked())
        channelSelection |= 0b00010000;
    if(ui->ch3CalBox->isChecked())
        channelSelection |= 0b00100000;
    if(ui->ch4Box->isChecked())
        channelSelection |= 0b01000000;
    if(ui->ch4CalBox->isChecked())
        channelSelection |= 0b10000000;

    sensor->setEnabledChannels(channelSelection);
    sensor->setGain(ui->gainBox->currentIndex());
    sensor->setConversionMode(GCSensorADS1219::ConversionMode::SingleShot);
    sensor->setConversionRate(GCSensorADS1219::ConversionRate::SPS_90);
    sensor->setReferenceVoltage(ui->vrefBox->currentIndex());

    quint16 period = round(ui->periodBox->value()*10) - 1;
    sensor->setMeasurementPeriod(period);
    sensor->setUseGlobalPeriodFlag(ui->gperiodBox->isChecked());

    hide();
}



void SensorADS1219Dialog::setPeriodDialog(GlobalMeasurementPolicyDialog* dialog)
{
    gDialog = dialog;
}

void SensorADS1219Dialog::useGlobalPeriodToggle()
{
    if(ui->gperiodBox->isChecked()){
        if(gDialog != nullptr)
            ui->periodBox->setValue(gDialog->getPeriod());
        ui->periodBox->setEnabled(false);
    } else {
        ui->periodBox->setEnabled(true);
    }
}

