#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include <QStringList>
#include "settingsdialog.h"

namespace Ui {
class NewProjectDialog;
}


class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewProjectDialog(QWidget *parent = nullptr);
    ~NewProjectDialog();

    GCProjectSettings getProject(void) const;
    void setProject(GCProjectSettings& settings);

private:
    void updateBaudrate(void);
    void offlineTrigger(void);
    void selectRootDir(void);
    void updateCOM(void);
    void updateCOMList(void);
    void apply(void);
    void update(void);
    void cancel(void);

    Ui::NewProjectDialog *ui;

    QStringList comList;

    GCProjectSettings settings;
};

#endif // NEWPROJECTDIALOG_H
