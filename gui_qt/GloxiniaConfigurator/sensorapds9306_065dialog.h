#ifndef SENSORAPDS9306_065DIALOG_H
#define SENSORAPDS9306_065DIALOG_H

#include <QDialog>
#include <gcsensor.h>
#include <globalmeasurementpolicydialog.h>

namespace Ui {
class SensorAPDS9306_065Dialog;
}

class SensorAPDS9306_065Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorAPDS9306_065Dialog(QWidget *parent = nullptr);
    ~SensorAPDS9306_065Dialog();

    void updateUISettings(GCSensorAPDS9306* s);
    void setPeriodDialog(GlobalMeasurementPolicyDialog* dialog);
    void apply(GCSensorAPDS9306* s);

private slots:
    void editGlobalPeriodSettings(void);
    void useGlobalPeriodToggle(void);

private:
    Ui::SensorAPDS9306_065Dialog *ui;
    GlobalMeasurementPolicyDialog *gDialog = nullptr;

};

#endif // SENSORAPDS9306_065DIALOG_H
