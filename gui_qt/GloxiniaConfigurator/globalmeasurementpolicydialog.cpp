#include "globalmeasurementpolicydialog.h"
#include "ui_globalmeasurementpolicydialog.h"
#include <math.h>

GlobalMeasurementPolicyDialog::GlobalMeasurementPolicyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalMeasurementPolicyDialog)
{
    ui->setupUi(this);

    connect(ui->periodBox, &QDoubleSpinBox::valueChanged, this, &GlobalMeasurementPolicyDialog::update);
    connect(ui->okBox, &QDialogButtonBox::accepted, this, &GlobalMeasurementPolicyDialog::apply);
}

GlobalMeasurementPolicyDialog::~GlobalMeasurementPolicyDialog()
{
    delete ui;
}

void GlobalMeasurementPolicyDialog::update(void)
{
    double value = 1/ui->periodBox->value();
    QString freq;

    if(value >= 1.0){
        freq.append("Hz");
    } else if(value < 1/(60*60))
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
    ui->frequencyValue->setText(freq);
}

void GlobalMeasurementPolicyDialog::apply(void)
{
    double value = ui->periodBox->value()*10;
    period = round(value);
    hide();
}

void GlobalMeasurementPolicyDialog::setPeriod(quint16 value)
{
    period = value;
}

quint16 GlobalMeasurementPolicyDialog::getPeriod(void) const
{
    return period;
}
