#include "gdevicecommunication.h"
#include <QTimer>
#include <QDebug>

GDeviceCommunication::GDeviceCommunication() : serial(nullptr)
{
    timer.setTimerType(Qt::TimerType::CoarseTimer);
    timer.setInterval(50);
    connect(&timer, &QTimer::timeout, this, &GDeviceCommunication::process);

    timer.start();
}

GDeviceCommunication::~GDeviceCommunication()
{
    emit finished();
}

void GDeviceCommunication::process()
{
    if(mList.empty())
        return;

    GMessage m = mList.at(0);
    mList.removeFirst();

    auto length = m.toBytes(data, 8+8);

    if((serial != nullptr) && (serial->isOpen())){
        serial->write(QByteArray((char *)data, length));
    }
}

void GDeviceCommunication::handleMessage(const GMessage& m)
{
    mList.append(m);
}

void GDeviceCommunication::setSerialPort(QSerialPort* port)
{
    serial = port;
}
