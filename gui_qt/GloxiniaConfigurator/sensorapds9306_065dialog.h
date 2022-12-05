#ifndef SENSORAPDS9306_065DIALOG_H
#define SENSORAPDS9306_065DIALOG_H

#include <QDialog>
#include <gcsensor.h>

namespace Ui {
class SensorAPDS9306_065Dialog;
}

class SensorAPDS9306_065Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorAPDS9306_065Dialog(QWidget *parent = nullptr);
    ~SensorAPDS9306_065Dialog();

    void updateUISettings();
    void setSensorSettings(GCSensorAPDS9306* s);
    void apply();
    void updateSensor(GCSensorAPDS9306* s);

private:
    Ui::SensorAPDS9306_065Dialog *ui;

    GCSensorAPDS9306 sensor;
};

#endif // SENSORAPDS9306_065DIALOG_H
