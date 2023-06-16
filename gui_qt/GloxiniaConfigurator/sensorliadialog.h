#ifndef SENSORLIADIALOG_H
#define SENSORLIADIALOG_H

#include <QDialog>

namespace Ui {
class SensorLIADialog;
}

class SensorLIADialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorLIADialog(QWidget *parent = nullptr);
    ~SensorLIADialog();

private:
    Ui::SensorLIADialog *ui;
};

#endif // SENSORLIADIALOG_H
