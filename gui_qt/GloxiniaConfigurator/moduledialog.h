#ifndef MODULEDIALOG_H
#define MODULEDIALOG_H

#include <QDialog>
#include "gcnode.h"

namespace Ui {
class ModuleDialog;
}

class ModuleDialog : public QDialog
{
    Q_OBJECT

private slots:
    void updateAutoID(void);
    void apply();

public:
    explicit ModuleDialog(QWidget *parent = nullptr);
    ~ModuleDialog();

    void updateUISettings();
    void setNodeSettings(GCNode& s);



    GCNode getNode();

private:
    Ui::ModuleDialog *ui;

    GCNode nodeConfig;
};

#endif // MODULEDIALOG_H
