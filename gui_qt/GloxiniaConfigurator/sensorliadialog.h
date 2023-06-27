#ifndef SENSORLIADIALOG_H
#define SENSORLIADIALOG_H

#include <QDialog>
#include <gcsensor.h>
#include <globalmeasurementpolicydialog.h>
#include <liaenginedialog.h>


namespace Ui {
class SensorLIADialog;
}

class SensorLIADialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorLIADialog(QWidget *parent = nullptr);

    ~SensorLIADialog();

    void updateUISettings(GCSensorLIA* sensor);
    void setPeriodDialog(GlobalMeasurementPolicyDialog* dialog);
    void setLIAEngineDialog(LIAEngineDialog* dialog);
    void apply(GCSensorLIA* sensor);

private slots:
    void useGlobalPeriodToggle(void);
    void editGlobalPeriodSettings(void);
    void editLIAEngineSettings(void);

private:
    Ui::SensorLIADialog *ui;
    GlobalMeasurementPolicyDialog *gDialog = nullptr;
    LIAEngineDialog* liaEngineDialog = nullptr;

    GLIAEngine liaEngine;
};

#endif // SENSORLIADIALOG_H
