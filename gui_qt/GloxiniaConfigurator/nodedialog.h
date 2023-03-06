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

private slots:
    void updateAutoID(void);
    void apply();

public:
    explicit NodeDicioDialog(QWidget *parent = nullptr);
    ~NodeDicioDialog();

    void updateUISettings();
    void setNodeSettings(GCNodeDicio *s);

    void updateNode(GCNodeDicio *node) const;

private:
    Ui::NodeDicioDialog *ui;

    GCNodeDicio *nodeConfig;
};

#endif // NODEDICIODIALOG_H
