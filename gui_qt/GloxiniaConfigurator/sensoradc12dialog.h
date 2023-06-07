#ifndef SENSORADC12DIALOG_H
#define SENSORADC12DIALOG_H

#include <QDialog>
#include <gcsensor.h>
#include <globalmeasurementpolicydialog.h>

namespace Ui
{
    class SensorADC12Dialog;
}

class SensorADC12Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorADC12Dialog(QWidget *parent = nullptr);
    ~SensorADC12Dialog();

    void updateUISettings(GCSensorADC12* sensor);
    void setPeriodDialog(GlobalMeasurementPolicyDialog* dialog);
    void apply(GCSensorADC12* sensor);

private slots:
    void useGlobalPeriodToggle(void);
    void editGlobalPeriodSettings(void);

private:
    Ui::SensorADC12Dialog *ui;
    GlobalMeasurementPolicyDialog *gDialog = nullptr;
};

#endif // SENSORADC12DIALOG_H
