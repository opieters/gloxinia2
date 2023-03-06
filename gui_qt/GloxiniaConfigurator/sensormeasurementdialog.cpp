#include "sensormeasurementdialog.h"
#include "ui_sensormeasurementdialog.h"

SensorMeasurementDialog::SensorMeasurementDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorMeasurementDialog),
    gDialog(nullptr)
{
    ui->setupUi(this);

    connect(ui->policyBox, &QCheckBox::stateChanged, this, &SensorMeasurementDialog::update);
    connect(ui->globalPeriodButton, &QPushButton::clicked, this, &SensorMeasurementDialog::editGlobalPeriodSettings);
    connect(ui->okBox, &QDialogButtonBox::accepted, this, &SensorMeasurementDialog::apply);

    update();
}

SensorMeasurementDialog::~SensorMeasurementDialog()
{
    delete ui;
}

void SensorMeasurementDialog::apply(void)
{
    useGlobalPeriod = ui->policyBox->isChecked();

    if(useGlobalPeriod){
        period = globalPeriod;
    } else {
        period = round(ui->periodBox->value()*10);
    }
}

quint16 SensorMeasurementDialog::getPeriod(void) const
{
    return period;
}

void SensorMeasurementDialog::setPeriod(const quint16 value)
{
    period = value;
}

void SensorMeasurementDialog::setGlobalPeriod(const quint16 value)
{
    globalPeriod = value;
}

bool SensorMeasurementDialog::getUseGlobalPeriod(void) const
{
    return useGlobalPeriod;
}

void SensorMeasurementDialog::setUseGlobalPeriod(bool value)
{
    useGlobalPeriod = value;
}

void SensorMeasurementDialog::setGlobalPeriodDialog(GlobalMeasurementPolicyDialog* dialog)
{
    gDialog = dialog;

    update();
}

void SensorMeasurementDialog::editGlobalPeriodSettings(void)
{
    int result;

    if(gDialog == nullptr){
        return;
    }

    result = gDialog->exec();
    if(result == QDialog::Rejected)
        return;

    globalPeriod = gDialog->getPeriod();

    update();
}

void SensorMeasurementDialog::update(void)
{
    if(gDialog != nullptr)
    {
        ui->globalPeriodButton->setDisabled(false);
    } else {
        ui->globalPeriodButton->setDisabled(true);
    }

    if(gDialog != nullptr)
    {
        globalPeriod = gDialog->getPeriod();
    }

    if(ui->policyBox->isChecked())
    {
        ui->periodBox->setValue(globalPeriod / 10.0);
        ui->periodBox->setDisabled(true);
    } else {
        ui->periodBox->setDisabled(false);
    }

    double value = 1/ui->periodBox->value();
    QString freq;

    if(value >= 1.0){
        freq.append(" Hz");
    } else if(value < 1/(60*60))
    {
        value *= 60*60*24;
        freq.append(" samples/day");
    } else if(value < 1/60)
    {
        value *= 60*60;
        freq.append(" samples/hour");
    } else {
        value *= 60;
        freq.append(" samples/minute");
    }
    freq.prepend(QString::number(value, 'd', 1));
    ui->frequencyValue->setText(freq);
}
