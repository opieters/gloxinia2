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

    enum SerialReadoutState
    {
        FindStartByte,
        ReadIDH,
        ReadIDL,
        ReadRequest,
        ReadEIDU,
        ReadEIDH,
        ReadEIDL,
        ReadLength,
        ReadData,
        DetectStopByte
    };

    void openSerialPort(const QString& comPort);
    void closeSerialPort();

public slots:
    void process();
    void handleMessage(const GMessage& m);
    void readData();

signals:
    void serialOpenStatusChange(bool statusChanged);
    void receivedMessage(const GMessage m);

signals:
    void finished();
    void error(QString err);

private:
    QList<GMessage> mList;

    QSerialPort serial;

    quint8 data[9+8];

    QTimer timer;
};

#endif // GMESSAGEQUEUE_H
