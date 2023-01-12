#include "measurementsettingsdialog.h"
#include "ui_measurementsettingsdialog.h"
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>

MeasurementSettingsDialog::MeasurementSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MeasurementSettingsDialog)
{
    ui->setupUi(this);

    settings = {0, true, QString("myproject")};

    connect(ui->periodBox, &QDoubleSpinBox::valueChanged, this, &MeasurementSettingsDialog::update);

    update();
}

MeasurementSettingsDialog::~MeasurementSettingsDialog()
{
    delete ui;
}


measurement_settings_t MeasurementSettingsDialog::getSettings(void)
{
    return settings;
}

void MeasurementSettingsDialog::setSettings(measurement_settings_t& settings)
{
    this->settings = settings;

    if(settings.uniqueFolder){
        ui->folderBox->setCheckState(Qt::CheckState::Checked);
    } else {
        ui->folderBox->setCheckState(Qt::CheckState::Unchecked);
    }

    double period = (0.1*(settings.measurementPeriod+1));
    ui->periodBox->setValue(period);

    update();
}

void MeasurementSettingsDialog::update(void)
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

void MeasurementSettingsDialog::apply(void)
{
    double period = 10*ui->periodBox->value() - 1.0;
    settings.measurementPeriod = (uint16_t) period;

    settings.uniqueFolder = ui->folderBox->isChecked();

    QDir documents = QStandardPaths::displayName(QStandardPaths::DocumentsLocation);

    // create project directory and alert user if this is not possible
    if(!documents.mkpath(settings.projectName)){
        QMessageBox dirError;
        dirError.setText("Unable to create project directory `" + settings.projectName + "`");
        dirError.exec();
        return;
    }

    hide();
}
