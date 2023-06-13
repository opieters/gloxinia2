#ifndef GMESSAGEQUEUE_H
#define GMESSAGEQUEUE_H

#include <QObject>
#include <gmessage.h>
#include <QSerialPort>
#include <QTimer>


class GDeviceCommunication : public QObject
{
    Q_OBJECT

public:
    GDeviceCommunication();
    ~GDeviceCommunication();

    void setSerialPort(QSerialPort* port);

public slots:
    void process();
    void handleMessage(const GMessage& m);

signals:
    void queueMessage(const GMessage& m);

signals:
    void finished();
    void error(QString err);

private:
    QList<GMessage> mList;

    QSerialPort *serial;

    quint8 data[8+8];

    QTimer timer;
};

#endif // GMESSAGEQUEUE_H
