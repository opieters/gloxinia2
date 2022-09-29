#ifndef GLOXINIACONFIGURATOR_H
#define GLOXINIACONFIGURATOR_H

#include <QMainWindow>
#include <gcmodel.h>

QT_BEGIN_NAMESPACE
namespace Ui { class GloxiniaConfigurator; }
QT_END_NAMESPACE

class GloxiniaConfigurator : public QMainWindow
{
    Q_OBJECT

public:
    GloxiniaConfigurator(QWidget *parent = nullptr);
    ~GloxiniaConfigurator();

private slots:
    void addNode();

private:
    Ui::GloxiniaConfigurator *ui;

    GCModel systemModel;
};
#endif // GLOXINIACONFIGURATOR_H
