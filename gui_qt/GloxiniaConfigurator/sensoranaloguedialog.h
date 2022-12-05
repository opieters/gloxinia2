#ifndef SENSORANALOGUEDIALOG_H
#define SENSORANALOGUEDIALOG_H

#include <QDialog>

namespace Ui {
class SensorAnalogueDialog;
}

class SensorAnalogueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorAnalogueDialog(QWidget *parent = nullptr);
    ~SensorAnalogueDialog();

private:
    Ui::SensorAnalogueDialog *ui;
};

#endif // SENSORANALOGUEDIALOG_H
