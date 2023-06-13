#include "gdevicecommunication.h"
#include <QTimer>
#include <QDebug>

GDeviceCommunication::GDeviceCommunication() : serial(nullptr)
{
    timer.setTimerType(Qt::TimerType::CoarseTimer);
    timer.setInterval(25);
    connect(&timer, &QTimer::timeout, this, &GDeviceCommunication::process);
}

GDeviceCommunication::~GDeviceCommunication()
{
    emit finished();
}

void GDeviceCommunication::process()
{
    GMessage m = mList.at(0);
    mList.removeFirst();

    auto length = m.toBytes(data, 8+8);

    if((serial != nullptr) && (serial->isOpen())){
        serial->write(QByteArray((char *)data, length));
    }

    if(mList.empty())
    {
        timer.stop();
    }
}

void GDeviceCommunication::handleMessage(const GMessage& m)
{
    mList.append(m);
    if(!timer.isActive())
        timer.start();
}

void GDeviceCommunication::setSerialPort(QSerialPort* port)
{
    serial = port;
}
