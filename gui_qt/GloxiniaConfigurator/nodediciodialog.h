#ifndef NODEDICIODIALOG_H
#define NODEDICIODIALOG_H

#include <QDialog>
#include "gcnode.h"

namespace Ui
{
    class NodeDicioDialog;
}

class NodeDicioDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NodeDicioDialog(QWidget *parent = nullptr);
    ~NodeDicioDialog();

    void updateUISettings();
    void setNodeSettings(GCNodeDicio *const n);

    void updateNode(GCNodeDicio *n) const;

private slots:
    void updateAutoID(void);
    void apply();

private:
    Ui::NodeDicioDialog *ui;

    GCNodeDicio *config;
};

#endif // NODEDICIODIALOG_H
