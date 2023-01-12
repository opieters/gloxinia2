#ifndef SENSORSHT35DIALOG_H
#define SENSORSHT35DIALOG_H

#include <QDialog>
#include <gcsensor.h>
#include <globalmeasurementpolicydialog.h>

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
    void setPeriodDialog(GlobalMeasurementPolicyDialog* dialog);
    void apply(void);
    void editGlobalPeriodSettings(void);

    void updateSensor(GCSensorSHT35 *s);
private slots:
    void updatePeriodicity(void);

private:
    Ui::SensorSHT35Dialog *ui;
    GlobalMeasurementPolicyDialog *gDialog = nullptr;

    GCSensorSHT35* sensor;

    quint16 period = 9;
    quint16 globalPeriod = 9;
    bool useGlobalPeriod = false;
};

#endif // SENSORSHT35DIALOG_H
