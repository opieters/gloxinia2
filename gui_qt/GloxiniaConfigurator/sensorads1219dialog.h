#ifndef SENSORADS1219DIALOG_H
#define SENSORADS1219DIALOG_H

#include <QDialog>
#include <gcsensor.h>
#include <globalmeasurementpolicydialog.h>

namespace Ui {
class SensorADS1219Dialog;
}

class SensorADS1219Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorADS1219Dialog(QWidget *parent = nullptr);
    ~SensorADS1219Dialog();

    void updateUISettings(GCSensorADS1219* sensor);
    void apply(GCSensorADS1219* sensor);
    void setPeriodDialog(GlobalMeasurementPolicyDialog* dialog);

private slots:
    void editGlobalPeriodSettings(void);
    void useGlobalPeriodToggle(void);

private:
    Ui::SensorADS1219Dialog *ui;
    GlobalMeasurementPolicyDialog *gDialog = nullptr;
};

#endif // SENSORADS1219DIALOG_H
