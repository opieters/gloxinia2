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

    updateUI();
}

// https://code.qt.io/cgit/qt/qtserialport.git/tree/examples/serialport/terminal/mainwindow.cpp?h=5.15
void GloxiniaConfigurator::openSerialPort()
{
    emit serialPortSelected(settings.comPort);
    qInfo() << "Opening serial port";

    ui->actionConnect->setEnabled(false);
    ui->actionDisconnect->setEnabled(true);
    ui->actionLoadDeviceConfig->setEnabled(true);
    ui->actionConfigureSerial->setEnabled(false);
}


void GloxiniaConfigurator::serialPortError(const QString errorMessage)
{
    QMessageBox::critical(this, tr("Error"), errorMessage);

    showStatusMessage(tr("Open error"));

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionLoadDeviceConfig->setEnabled(false);
    ui->actionConfigureSerial->setEnabled(true);
}

void GloxiniaConfigurator::setSerialOpenStatus(bool status)
{
    serialOpenStatus = status;

    updateUI();
}


void GloxiniaConfigurator::closeSerialPort()
{

    emit closeSerialPortEvent();

    discoveryTimer->stop();


    // m_console->setEnabled(false);
    /*ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigureSerial->setEnabled(true);
    ui->actionLoadDeviceConfig->setEnabled(false);*/

    updateUI();

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
