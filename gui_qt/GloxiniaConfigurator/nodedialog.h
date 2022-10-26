#ifndef NODEDIALOG_H
#define NODEDIALOG_H

#include <QDialog>
#include "gcnode.h"

namespace Ui {
class NodeDialog;
}

class NodeDialog : public QDialog
{
    Q_OBJECT

private slots:
    void updateAutoID(void);
    void apply();

public:
    explicit NodeDialog(QWidget *parent = nullptr);
    ~NodeDialog();

    void updateUISettings();
    void setNodeSettings(GCNode* s);



    GCNode* getNode();

private:
    Ui::NodeDialog *ui;

    GCNode* nodeConfig;
};

#endif // NODEDIALOG_H
