#ifndef SENSORDIALOG_H
#define SENSORDIALOG_H

#include <QDialog>
#include <gcsensor.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class SensorDialog;
}
QT_END_NAMESPACE

class SensorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorDialog(QWidget *parent = nullptr);
    ~SensorDialog();

    GCSensor* add();
private:
    Ui::SensorDialog *ui;
    void updateSensorSelect();
};

#endif // SENSORDIALOG_H
