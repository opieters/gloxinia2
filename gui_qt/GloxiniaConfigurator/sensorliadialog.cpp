#include "sensorliadialog.h"
#include "ui_sensorliadialog.h"

SensorLIADialog::SensorLIADialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorLIADialog),
    gDialog(nullptr)
{
    ui->setupUi(this);

    connect(ui->gPeriodButton, &QPushButton::clicked, this, &SensorLIADialog::editGlobalPeriodSettings);
    connect(ui->gPeriodBox, &QCheckBox::stateChanged, this, &SensorLIADialog::useGlobalPeriodToggle);
    connect(ui->liaSettingsButton, &QPushButton::clicked, this, &SensorLIADialog::editLIAEngineSettings);

    ui->periodBox->setValue(1.0);

    useGlobalPeriodToggle();
}

SensorLIADialog::~SensorLIADialog()
{
    delete ui;
}


void SensorLIADialog::useGlobalPeriodToggle(void)
{
    if(ui->gPeriodBox->isChecked()){
        if(gDialog != nullptr)
            ui->periodBox->setValue(gDialog->getPeriod());
        ui->periodBox->setEnabled(false);
    } else {
        ui->periodBox->setEnabled(true);
    }
}

void SensorLIADialog::editGlobalPeriodSettings(void)
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

void SensorLIADialog::editLIAEngineSettings(void)
{
    if(liaEngineDialog == nullptr)
        return;

    int result = liaEngineDialog->exec();
    if (result == QDialog::Rejected)
    {
        return;
    }

    liaEngineDialog->apply(&this->liaEngine);
}

void SensorLIADialog::apply(GCSensorLIA* sensor)
{
    quint16 period = round(ui->periodBox->value()*10) - 1;
    sensor->setMeasurementPeriod(period);
    sensor->setUseGlobalPeriodFlag(ui->gPeriodBox->isChecked());

    GCNodePlanalta *node = static_cast<GCNodePlanalta*>(sensor->getNode());
    node->setLIAEngine(liaEngine);

    hide();
}

void SensorLIADialog::updateUISettings(GCSensorLIA* sensor)
{
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

    GCNodePlanalta *node = static_cast<GCNodePlanalta*>(sensor->getNode());
    liaEngine = node->getLIAEngine();

    useGlobalPeriodToggle();
}


void SensorLIADialog::setPeriodDialog(GlobalMeasurementPolicyDialog* dialog)
{
    gDialog = dialog;
}

void SensorLIADialog::setLIAEngineDialog(LIAEngineDialog* dialog)
{
    liaEngineDialog = dialog;
}
