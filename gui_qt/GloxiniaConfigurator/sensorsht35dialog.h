#ifndef SENSORSHT35DIALOG_H
#define SENSORSHT35DIALOG_H

#include <QDialog>
#include <gcsensor.h>
#include <sensormeasurementdialog.h>

namespace Ui
{
    class SensorSHT35Dialog;
}

class SensorSHT35Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorSHT35Dialog(QWidget *parent = nullptr);
    ~SensorSHT35Dialog();

    void updateUISettings();
    void setSensorSettings(GCSensorSHT35 *s);
    void setPeriodDialog(SensorMeasurementDialog* dialog);
    void apply(void);

    void updateSensor(GCSensorSHT35 *s);
private slots:
    void updatePeriodicity(void);
    void updatePeriodSettings(void);

private:
    Ui::SensorSHT35Dialog *ui;
    SensorMeasurementDialog *mDialog = nullptr;

    GCSensorSHT35 sensor = GCSensorSHT35();
};

#endif // SENSORSHT35DIALOG_H
