#ifndef GLOXINIACONFIGURATOR_H
#define GLOXINIACONFIGURATOR_H

#include <QMainWindow>
#include <QSerialPort>
#include <QStringListModel>
#include <QLabel>
#include <array>

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
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void setSerialPort();
    //void writeData(const QByteArray &data);
    //void handleError(QSerialPort::SerialPortError error);

private:
    Ui::GloxiniaConfigurator* ui;
    QSerialPort* serial;
    QLabel* status;

    QStringListModel* model;

    std::array<char, 0xff> rawDataBuffer;
    qint32 read_index = 0;
    qint32 write_index = 0;

    void showStatusMessage(const QString &message);
    void updateSerialPortList(void);

    QList<QAction*> serialPortActionList = QList<QAction*>();


};
#endif // GLOXINIACONFIGURATOR_H
