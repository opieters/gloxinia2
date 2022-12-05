#ifndef SENSORMEASUREMENTDIALOG_H
#define SENSORMEASUREMENTDIALOG_H

#include <QDialog>
#include <globalmeasurementpolicydialog.h>

namespace Ui {
class SensorMeasurementDialog;
}

class SensorMeasurementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorMeasurementDialog(QWidget *parent = nullptr);
    ~SensorMeasurementDialog();

    quint16 getPeriod(void) const;
    void setPeriod(const quint16 value);
    quint16 getGlobalPeriod(void) const;
    void setGlobalPeriod(const quint16 value);
    bool getUseGlobalPeriod(void) const;
    void setUseGlobalPeriod(bool value);
    void setGlobalPeriodDialog(GlobalMeasurementPolicyDialog* dialog);

private:
    void apply(void);
    void editGlobalPeriodSettings(void);

private slots:
    void update(void);

private:
    Ui::SensorMeasurementDialog *ui;
    GlobalMeasurementPolicyDialog *gDialog = nullptr;

    quint16 period;
    quint16 globalPeriod;
    bool useGlobalPeriod;
};

#endif // SENSORMEASUREMENTDIALOG_H
