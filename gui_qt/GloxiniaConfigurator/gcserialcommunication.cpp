#include "gloxiniaconfigurator.h"
#include "./ui_gloxiniaconfigurator.h"

#include <QMessageBox>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <gmessage.h>
#include <QSerialPortInfo>
#include <QInputDialog>
#include <configuresht35dialog.h>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QChartView>
#include <QValueAxis>
#include <QFileDialog>

// https://code.qt.io/cgit/qt/qtserialport.git/tree/examples/serialport/terminal/mainwindow.cpp?h=5.15
void GloxiniaConfigurator::openSerialPort()
{
    // const SettingsDialog::Settings p = m_settings->settings();
    serial->setPortName(serialPortName);
    serial->setBaudRate(500000);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite))
    {
        // m_console->setEnabled(true);
        // m_console->setLocalEchoEnabled(p.localEchoEnabled);
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionUpdate->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to ") + serialPortName);
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
    }
    // m_console->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    ui->actionUpdate->setEnabled(false);
    showStatusMessage(tr("Disconnected"));
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
            qInfo() << "Reading start byte";
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

            n_read = serial->read(data, 1); // read id H
            qInfo() << "Reading id byte H" << (int)data[0];
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

            n_read = serial->read(&data[1], 1); // read id L
            qInfo() << "Reading id byte L" << (int)data[1];
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

            n_read = serial->read(&data[2], 1); // read id
            qInfo() << "Reading command byte" << (int)data[2];
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

            n_read = serial->read(&data[3], 1); // read id
            qInfo() << "Reading request bit" << (int)data[3];
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

            n_read = serial->read(&data[4], 1); // read ext id H
            qInfo() << "Reading sensor id byte H" << (int)data[4];
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
            n_read = serial->read(&data[5], 1); // read ext id L
            qInfo() << "Reading ext id byte L" << (int)data[5];
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
            n_read = serial->read(&data[6], 1); // read length
            qInfo() << "Reading length byte" << (int)data[6];
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                return;
            }
            if (n_read == 1)
            {
                // we were able to read the length -> read data (if any)
                read_length = 0;
                if (data[6] == 0)
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
            qInfo() << "Reading data" << (int)data[6];
            n_read = serial->read(&data[7 + read_length], data[6] - read_length);
            if (n_read < 0)
            {
                readoutState = FindStartByte;
                return;
            }
            read_length += n_read;
            if (read_length == data[6])
            {
                readoutState = DetectStopByte;
            }
            else
            {
                break;
            }
        case DetectStopByte:
            qInfo() << "Reading stop byte";
            n_read = serial->read(&data[7 + read_length], 1);
            if (n_read < 0)
            {
                qInfo() << "ERROR";
                readoutState = FindStartByte;
                read_length = 0;
                return;
            }
            if (n_read == 1)
            {
                qInfo() << "Entire message received";
                if (data[7 + read_length] == GMessage::GMessageStopByte)
                {
                    quint8 *udata = (quint8 *)data;
                    qInfo() << "Processing message";
                    if (read_length == 0)
                    {
                        GMessage m((GMessage::Code)udata[2], (((uint16_t)udata[0] << 8) | udata[1]), ((uint16_t)udata[4] << 8) | udata[5], udata[3] == 0 ? false : true);
                        processIncomingGMessage(m);
                    }
                    else
                    {
                        GMessage m((GMessage::Code)udata[2], (((uint16_t)udata[0] << 8) | udata[1]), ((uint16_t)udata[4] << 8) | udata[5], udata[3] == 0 ? false : true, std::vector<quint8>(&data[7], &data[7 + read_length]));
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
                qInfo() << "No data received";
                break;
            }

            break;
        default:
            readoutState = FindStartByte;
            break;
        }
    } while (n_read > 0);
}

void GloxiniaConfigurator::updateSerialPortList(void)
{
    // TODO: add sort items

    auto list = QSerialPortInfo::availablePorts();

    // remove items
    for (QAction *i : serialPortActionList)
    {
        bool serialPortFound = false;
        for (const QSerialPortInfo &j : list)
        {
            if (j.portName() == i->text())
            {
                serialPortFound = true;
                break;
            }
        }
        if (!serialPortFound)
        {
            ui->menuPortSelection->removeAction(i);
        }
    }

    // add new items
    for (const QSerialPortInfo &i : list)
    {
        bool serialPortFound = false;
        for (QAction *j : serialPortActionList)
        {
            if (i.portName() == j->text())
            {
                serialPortFound = true;
                break;
            }
        }
        if (!serialPortFound)
        {
            QAction *action = ui->menuPortSelection->addAction(i.portName());
            action->setCheckable(true);
            connect(action, &QAction::triggered, this, &GloxiniaConfigurator::setSerialPort);
            serialPortActionList.append(action);
        }
    }
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
        serialPortName = button->text();
    }
}
