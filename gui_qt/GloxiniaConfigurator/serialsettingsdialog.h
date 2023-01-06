#ifndef SERIALSETTINGSDIALOG_H
#define SERIALSETTINGSDIALOG_H

#include <QDialog>
#include "newprojectdialog.h"

namespace Ui {
class SerialSettingsDialog;
}

class SerialSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SerialSettingsDialog(QWidget *parent = nullptr);
    ~SerialSettingsDialog();

private slots:
    void cancel(void);
    void apply(void);
    void updateCOM(void);
    void updateBaudrate(void);

private:
    void updateCOMList(void);

    Ui::SerialSettingsDialog *ui;

    NewProjectDialog::ProjectSettings settings;
};

#endif // SERIALSETTINGSDIALOG_H
