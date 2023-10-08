#include "gdevicecommunication.h"
#include <QTimer>
#include <QDebug>

GDeviceCommunication::GDeviceCommunication() : serial(QSerialPort(this))
{
    timer.setTimerType(Qt::TimerType::CoarseTimer);
    timer.setInterval(50);
    connect(&timer, &QTimer::timeout, this, &GDeviceCommunication::process);
    connect(&serial, &QSerialPort::readyRead, this, &GDeviceCommunication::readData);

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

    auto length = m.toBytes(data, 9+8);

    if(serial.isOpen()){
        serial.write(QByteArray((char *)data, length));
    }
}

void GDeviceCommunication::handleMessage(const GMessage& m)
{
    mList.append(m);
    qInfo() << "Sending to device:" << m.toString();
}

// https://code.qt.io/cgit/qt/qtserialport.git/tree/examples/serialport/terminal/mainwindow.cpp?h=5.15
void GDeviceCommunication::openSerialPort(const QString& comPort)
{
    // const SettingsDialog::Settings p = m_settings->settings();
    serial.setPortName(QString(comPort));
    serial.setBaudRate(50000);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::HardwareControl);
    if (serial.open(QIODevice::ReadWrite)){
        serial.clear();
        //m_console->setEnabled(true);
        // m_console->setLocalEchoEnabled(p.localEchoEnabled);

        /*showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));*/

        // TODO make timer run/stop based on settings
        //discoveryTimer->start(30 * 1000); // run every 30 seconds

        emit serialOpenStatusChange(true);
        qInfo() << "Serial port open, ready to receive data";
    } else
    {
        emit serialOpenStatusChange(false);
    }
}

void GDeviceCommunication::closeSerialPort()
{
    if (serial.isOpen())
    {
        serial.flush();
        serial.close();
    }

}

void GDeviceCommunication::readData()
{
    static SerialReadoutState readoutState = FindStartByte;
    short n_read = 0;
    static int read_length = 0;
    static char data[255 + 5];

    do
    {
        switch (readoutState)
        {
        case FindStartByte:
            n_read = serial.read(data, 1);
            if (n_read != 1)
            {
                return;
            }
            if (data[0] == GMessage::GMessageStartByte)
            {
                readoutState = ReadIDH;
            }
            else
            {
                break;
            }
        case ReadIDH:

            n_read = serial.read(&data[1], 1); // read id H
            if (n_read < 0)
            {
                // an error occurred -> return to initial state
                readoutState = FindStartByte;
                qInfo() << "Error reading id H";
                return;
            }
            if (n_read == 1)
            {
                // we were able to read code -> read id L
                readoutState = ReadIDL;
            }
            else
            {
                break;
            }
        case ReadIDL:

            n_read = serial.read(&data[2], 1); // read id
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                qInfo() << "Error reading command";
                return;
            }
            if (n_read == 1)
            {
                // we were able to read -> read request info
                readoutState = ReadRequest;
                read_length = 0;
            }
            else
            {
                break;
            }
        case ReadRequest:

            n_read = serial.read(&data[3], 1); // read id
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                qInfo() << "Error reading request byte";
                return;
            }
            if (n_read == 1)
            {
                // we were able to read -> read sensor id U
                readoutState = ReadEIDU;
                read_length = 0;
            }
            else
            {
                break;
            }
        case ReadEIDU:

            n_read = serial.read(&data[4], 1); // read ext id H
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                qInfo() << "Error reading sensor id H";
                return;
            }
            if (n_read == 1)
            {
                // we were able to read -> read ext id L
                readoutState = ReadEIDH;
                read_length = 0;
            }
            else
            {
                break;
            }
        case ReadEIDH:
            n_read = serial.read(&data[5], 1); // read ext id L
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                qInfo() << "Error reading sensor id L";
                return;
            }
            if (n_read == 1)
            {
                // we were able to read -> read length
                readoutState = ReadEIDL;
                read_length = 0;
            }
            else
            {
                break;
            }
        case ReadEIDL:
            n_read = serial.read(&data[6], 1); // read ext id L
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                qInfo() << "Error reading sensor id L";
                return;
            }
            if (n_read == 1)
            {
                // we were able to read -> read length
                readoutState = ReadLength;
                read_length = 0;
            }
            else
            {
                break;
            }
        case ReadLength:
            n_read = serial.read(&data[7], 1); // read length
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                qInfo() << "Error reading length";
                return;
            }
            if (n_read == 1)
            {
                // we were able to read the length -> read data (if any)
                read_length = 0;
                if (data[7] == 0)
                {
                    readoutState = DetectStopByte;
                    break;
                }
                else
                {
                    readoutState = ReadData;
                }
            }
            else
            {
                break;
            }
        case ReadData:
            if((data[7] - read_length) > 0){
                n_read = serial.read(&data[8 + read_length], data[7] - read_length);
                if (n_read < 0)
                {
                    readoutState = FindStartByte;
                    qInfo() << "Error reading data";
                    return;
                }
                read_length += n_read;
                if (read_length == data[7])
                {
                    readoutState = DetectStopByte;
                }
                else
                {
                    break;
                }
            } else {
                qInfo() << "Error reading data (2)";
                readoutState = FindStartByte;
                return;
            }
        case DetectStopByte:
            n_read = serial.read(&data[8 + read_length], 1);
            if (n_read < 0)
            {
                qInfo() << "Error reading stop";
                readoutState = FindStartByte;
                read_length = 0;
                return;
            }
            if (n_read == 1)
            {
                if (data[8 + read_length] == GMessage::GMessageStopByte)
                {
                    quint8 *udata = (quint8 *)data;
                    if (read_length == 0)
                    {
                        GMessage m((GMessage::Code) udata[5], (udata[1] << 8) | udata[2], udata[4], udata[6] >> 4, udata[6] & 0xf, udata[3] == 0 ? false : true);
                        qInfo() << m.toString();
                        emit receivedMessage(m);
                    }
                    else
                    {
                        GMessage m((GMessage::Code) udata[5], (udata[1] << 8) | udata[2], udata[4], udata[6] >> 4, udata[6] & 0xf, udata[3] == 0 ? false : true, std::vector<quint8>(&data[8], &data[8 + read_length]));
                        qInfo() << m.toString();
                        emit receivedMessage(m);
                    }

                    //if(model->insertRow(model->rowCount())){
                    //    QModelIndex index = model->index(model->rowCount() - 1, 0);
                    //   model->setData(index, "Data received!");
                    //}
                }
                else
                {
                    qInfo() << "Error incorrect stop byte received";
                    // incorrect data byte received -> reset state
                }
                readoutState = FindStartByte;
                read_length = 0;
                break;
            }
            else
            {
                break;
            }

            break;
        default:
            readoutState = FindStartByte;
            break;
        }
    } while (n_read > 0);
}
