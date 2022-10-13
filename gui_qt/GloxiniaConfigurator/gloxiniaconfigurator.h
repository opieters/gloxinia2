#ifndef GLOXINIACONFIGURATOR_H
#define GLOXINIACONFIGURATOR_H

#include <QMainWindow>
#include <QSerialPort>
#include <QStringListModel>
#include <QLabel>
#include <array>
#include <gcnode.h>
#include <settingsdialog.h>
#include <sensordialog.h>
#include <configuresht35dialog.h>
#include <gcsystem.h>
#include <treemodel.h>
#include <moduledialog.h>

QT_BEGIN_NAMESPACE
namespace Ui { class GloxiniaConfigurator; }
QT_END_NAMESPACE

class GloxiniaConfigurator : public QMainWindow
{
    Q_OBJECT

public:
    GloxiniaConfigurator(QWidget *parent = nullptr);
    ~GloxiniaConfigurator();

    void removeNode(uint8_t id);

private slots:
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void setSerialPort();
    void addNode(void);
    void addSensor(void);
    //void showContextMenu(const QPoint &pos);
    //void setSensor();
    //void writeData(const QByteArray &data);
    //void handleError(QSerialPort::SerialPortError error);

private:
    Ui::GloxiniaConfigurator* ui;
    QSerialPort* serial;
    QLabel* status;

    TreeModel* treeModel;


    std::array<char, 0xff> rawDataBuffer;
    qint32 read_index = 0;
    qint32 write_index = 0;

    void showStatusMessage(const QString &message);
    void updateSerialPortList(void);
    void updateActions();

    //QIcon getSensorIcon(GCSensor* s);
    //QString getSensorLabel(GCSensor* s);
    //QIcon getNodeIcon(GCNodeModel* n);
    //QString getNodeLabel(GCNodeModel* n);

    QList<QAction*> serialPortActionList = QList<QAction*>();
    //QList<GCNodeModel*> nodeList;

    SettingsDialog* systemSettings = nullptr;
    SensorDialog* sensorSettings = nullptr;
    ConfigureSHT35Dialog* configureSHT35Dialog;
    QStringList messageList;
    ModuleDialog* moduleDialog = nullptr;


};
#endif // GLOXINIACONFIGURATOR_H
