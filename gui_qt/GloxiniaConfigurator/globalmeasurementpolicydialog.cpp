#include "globalmeasurementpolicydialog.h"
#include "ui_globalmeasurementpolicydialog.h"
#include <math.h>

GlobalMeasurementPolicyDialog::GlobalMeasurementPolicyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalMeasurementPolicyDialog)
{
    ui->setupUi(this);

    connect(ui->periodBox, &QDoubleSpinBox::valueChanged, this, &GlobalMeasurementPolicyDialog::updateUI);

    ui->periodBox->setValue(1.0);
}

GlobalMeasurementPolicyDialog::~GlobalMeasurementPolicyDialog()
{
    delete ui;
}

void GlobalMeasurementPolicyDialog::updateUI(void)
{
    double value = 1/ui->periodBox->value();
    QString freq;

    if(value >= 1.0){
        freq.append("Hz");
        freq.prepend(QString::number(value, 'f', 1));
    } else{
        if(value < 1/(60*60))
        {
            value *= 60*60*24;
            freq.append("samples/day");
        } else if(value < 1/60)
        {
            value *= 60*60;
            freq.append("samples/hour");
        } else {
            value *= 60;
            freq.append("samples/minute");
        }
        freq.prepend(QString::number(value, 'g', 1));
    }

    ui->frequencyValue->setText(freq);
}

void GlobalMeasurementPolicyDialog::apply(quint16 period)
{
    ui->periodBox->setValue((period+1.0)/10);
}

void GlobalMeasurementPolicyDialog::updatePeriod(quint16& value)
{
    double period = ui->periodBox->value()*10-1;
    value = round(period);
}

double GlobalMeasurementPolicyDialog::getPeriod(void) const
{
    return ui->periodBox->value();
}
