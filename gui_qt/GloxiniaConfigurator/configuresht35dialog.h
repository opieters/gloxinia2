#ifndef CONFIGURESHT35DIALOG_H
#define CONFIGURESHT35DIALOG_H

#include <QDialog>
#include <gcsensor.h>
#include <QFileSystemModel >

namespace Ui {
class ConfigureSHT35Dialog;
}

class ConfigureSHT35Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureSHT35Dialog(QWidget *parent = nullptr);
    ~ConfigureSHT35Dialog();

    void updateUISettings();
    void setSensorSettings(GCSensorSHT35* s);
    void apply();

    GCSensorSHT35* getSensor();
private slots:
    void updatePeriodicity(void);
private:
    Ui::ConfigureSHT35Dialog *ui;

    GCSensorSHT35* sensor;
};

#endif // CONFIGURESHT35DIALOG_H
