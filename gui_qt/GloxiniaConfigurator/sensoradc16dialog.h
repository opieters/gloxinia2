#ifndef SENSORADC16DIALOG_H
#define SENSORADC16DIALOG_H

#include <QDialog>
#include <gcsensor.h>
#include <globalmeasurementpolicydialog.h>

namespace Ui {
class SensorADC16Dialog;
}

class SensorADC16Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorADC16Dialog(QWidget *parent = nullptr);
    ~SensorADC16Dialog();

    void updateUISettings(GCSensorADC16* sensor);
    void setPeriodDialog(GlobalMeasurementPolicyDialog* dialog);
    void apply(GCSensorADC16* sensor);

private slots:
    void useGlobalPeriodToggle(void);
    void editGlobalPeriodSettings(void);

private:
    Ui::SensorADC16Dialog *ui;

    GlobalMeasurementPolicyDialog *gDialog = nullptr;
};

#endif // SENSORADC16DIALOG_H
