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

    void updateUISettings(GCSensorSHT35* sensor);
    void setPeriodDialog(GlobalMeasurementPolicyDialog* dialog);
    void apply(GCSensorSHT35* sensor);

private slots:
    void updatePeriodicity(void);
    void useGlobalPeriodToggle(void);
    void editGlobalPeriodSettings(void);

private:
    Ui::SensorSHT35Dialog *ui;
    GlobalMeasurementPolicyDialog *gDialog = nullptr;
};

#endif // SENSORSHT35DIALOG_H
