#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class NewProjectDialog;
}


class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    struct ProjectSettings {
        bool success;
        QString projectName;
        QString projectDir;
        QString comPort;
        int baudrate;
        bool workOffline;
        bool runDiscovery;
    };

    explicit NewProjectDialog(QWidget *parent = nullptr);
    ~NewProjectDialog();

    ProjectSettings getProject(void) const;
    void setProject(ProjectSettings& settings);

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

    ProjectSettings settings;
};

#endif // NEWPROJECTDIALOG_H
