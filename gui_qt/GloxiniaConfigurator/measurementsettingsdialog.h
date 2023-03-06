#ifndef MEASUREMENTSETTINGSDIALOG_H
#define MEASUREMENTSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class MeasurementSettingsDialog;
}

typedef struct {
    uint16_t measurementPeriod;
    bool uniqueFolder;
    QString projectName;
} measurement_settings_t;

class MeasurementSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MeasurementSettingsDialog(QWidget *parent = nullptr);
    ~MeasurementSettingsDialog();

    measurement_settings_t getSettings(void);
    void setSettings(measurement_settings_t& settings);

private:
    Ui::MeasurementSettingsDialog *ui;

    void apply(void);
    void update(void);

    measurement_settings_t settings;
};

#endif // MEASUREMENTSETTINGSDIALOG_H
