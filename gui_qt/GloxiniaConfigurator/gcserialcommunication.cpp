#include "gloxiniaconfigurator.h"
#include "./ui_gloxiniaconfigurator.h"

#include <QMessageBox>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <gmessage.h>
#include <QSerialPortInfo>
#include <QInputDialog>
#include <sensorsht35dialog.h>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QChartView>
#include <QValueAxis>
#include <QFileDialog>


void GloxiniaConfigurator::connectToDevice(void)
{
    // check if serial port is empty, if so request one from user
    if(settings.comPort.isEmpty())
    {

    }

    openSerialPort();
}

// https://code.qt.io/cgit/qt/qtserialport.git/tree/examples/serialport/terminal/mainwindow.cpp?h=5.15
void GloxiniaConfigurator::openSerialPort()
{
    // const SettingsDialog::Settings p = m_settings->settings();
    serial->setPortName(settings.comPort);
    serial->setBaudRate(500000);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::HardwareControl);
    if (serial->open(QIODevice::ReadWrite))
    {
        // m_console->setEnabled(true);
        // m_console->setLocalEchoEnabled(p.localEchoEnabled);
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionLoadDeviceConfig->setEnabled(true);
        ui->actionConfigureSerial->setEnabled(false);
        showStatusMessage(tr("Connected to ") + settings.comPort);
        /*showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));*/

        // TODO make timer run/stop based on settings
        discoveryTimer->start(30 * 1000); // run every 30 seconds

        runDiscovery();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}

void GloxiniaConfigurator::closeSerialPort()
{
    discoveryTimer->stop();

    if (serial->isOpen())
    {
        serial->close();
        showStatusMessage(tr("Disconnected"));
    }
    // m_console->setEnabled(false);
    /*ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigureSerial->setEnabled(true);
    ui->actionLoadDeviceConfig->setEnabled(false);*/

    updateUI();

}

void GloxiniaConfigurator::readData()
{
    static SerialReadoutState readoutState = FindStartByte;
    short n_read = 0;
    static short read_length = 0;
    static char data[255 + 5];

    do
    {
        switch (readoutState)
        {
        case FindStartByte:
            n_read = serial->read(data, 1);
            if (n_read != 1)
            {
                return;
            }
            if (data[0] == GMessage::GMessageStartByte)
            {
                readoutState = ReadIdH;
            }
            else
            {
                break;
            }
        case ReadIdH:

            n_read = serial->read(&data[1], 1); // read id H
            if (n_read < 0)
            {
                // an error occurred -> return to initial state
                readoutState = FindStartByte;
                return;
            }
            if (n_read == 1)
            {
                // we were able to read code -> read id L
                readoutState = ReadIdL;
            }
            else
            {
                break;
            }
        case ReadIdL:

            n_read = serial->read(&data[2], 1); // read id L
            if (n_read < 0)
            {
                // an error occurred -> return to initial state
                readoutState = FindStartByte;
                return;
            }
            if (n_read == 1)
            {
                // we were able to read code -> read command
                readoutState = ReadCommand;
            }
            else
            {
                break;
            }
        case ReadCommand:

            n_read = serial->read(&data[3], 1); // read id
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                return;
            }
            if (n_read == 1)
            {
                // we were able to read -> read sensor id H
                readoutState = ReadRequest;
                read_length = 0;
            }
            else
            {
                break;
            }
        case ReadRequest:

            n_read = serial->read(&data[4], 1); // read id
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                return;
            }
            if (n_read == 1)
            {
                // we were able to read -> read sensor id H
                readoutState = ReadSensorIdH;
                read_length = 0;
            }
            else
            {
                break;
            }
        case ReadSensorIdH:

            n_read = serial->read(&data[5], 1); // read ext id H
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                return;
            }
            if (n_read == 1)
            {
                // we were able to read -> read ext id L
                readoutState = ReadSensorIdL;
                read_length = 0;
            }
            else
            {
                break;
            }
        case ReadSensorIdL:
            n_read = serial->read(&data[6], 1); // read ext id L
            if (n_read < 0)
            {
                readoutState = FindStartByte;
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
            n_read = serial->read(&data[7], 1); // read length
            if (n_read < 0)
            {
                readoutState = FindStartByte;
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
            n_read = serial->read(&data[8 + read_length], data[7] - read_length);
            if (n_read < 0)
            {
                readoutState = FindStartByte;
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
        case DetectStopByte:
            n_read = serial->read(&data[8 + read_length], 1);
            if (n_read < 0)
            {
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
                        GMessage m((GMessage::Code)udata[3], (((uint16_t)udata[1] << 8) | udata[2]), ((uint16_t)udata[5] << 8) | udata[6], udata[4] == 0 ? false : true);
                        processIncomingGMessage(m);
                    }
                    else
                    {
                        GMessage m((GMessage::Code)udata[3], (((uint16_t)udata[1] << 8) | udata[2]), ((uint16_t)udata[5] << 8) | udata[6], udata[4] == 0 ? false : true, std::vector<quint8>(&data[8], &data[8 + read_length]));
                        processIncomingGMessage(m);
                    }

                    /*if(model->insertRow(model->rowCount())){
                        QModelIndex index = model->index(model->rowCount() - 1, 0);
                        model->setData(index, "Data received!");
                    }*/
                }
                else
                {
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

void GloxiniaConfigurator::setSerialPort(void)
{
    QAction *button = qobject_cast<QAction *>(sender());
    if (button != NULL)
    {
        // disable other buttons
        for (QAction *i : serialPortActionList)
        {
            if (i != button)
            {
                i->setChecked(false);
            }
            else
            {
                i->setChecked(true);
            }
        }

        // update configuration and the connected port
        settings.comPort = button->text();
    }
}
