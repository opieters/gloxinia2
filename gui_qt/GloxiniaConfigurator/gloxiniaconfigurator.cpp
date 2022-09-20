#include "gloxiniaconfigurator.h"
#include "./ui_gloxiniaconfigurator.h"

#include <QMessageBox>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <gmessage.h>
#include <QSerialPortInfo>

GloxiniaConfigurator::GloxiniaConfigurator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GloxiniaConfigurator)
    , serial(new QSerialPort(this))
    , model(new QStringListModel(this))
    , status(new QLabel)
{
    ui->setupUi(this);
    ui->statusbar->addWidget(status);

    connect(serial, &QSerialPort::readyRead, this, &GloxiniaConfigurator::readData);
    connect(ui->actionConnect, &QAction::triggered, this, &GloxiniaConfigurator::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &GloxiniaConfigurator::closeSerialPort);

    QStringList list;
    list << "Example data";
    model->setStringList(list);

    ui->listView->setModel(model);

    updateSerialPortList();
}

GloxiniaConfigurator::~GloxiniaConfigurator()
{
    delete ui;
}

// https://code.qt.io/cgit/qt/qtserialport.git/tree/examples/serialport/terminal/mainwindow.cpp?h=5.15
void GloxiniaConfigurator::openSerialPort()
{
    //const SettingsDialog::Settings p = m_settings->settings();
    serial->setPortName("COM12");
    serial->setBaudRate(500000);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        //m_console->setEnabled(true);
        //m_console->setLocalEchoEnabled(p.localEchoEnabled);
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to COM12"));
        /*showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));*/
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}

void GloxiniaConfigurator::closeSerialPort()
{
    if (serial->isOpen()){
        serial->close();
    }
    //m_console->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}

void GloxiniaConfigurator::readData()
{
    static short readoutState = 0;
    short n_read = 0;
    static short read_length = 0;
    static char data[255+5];

    do {
        switch(readoutState){
            case 0:
                n_read = serial->read(data, 1);
                if(n_read != 1){
                    return;
                }
                if(data[0] == GMessage::GMessageStartByte){
                    readoutState++;
                }
                break;
            case 1:
                n_read = serial->read(data, 1); // read code
                if(n_read < 0){
                    // an error occurred -> return to initial state
                    readoutState = 0;
                    return;
                }
                if(n_read == 1){
                    // we were able to read code -> read id H
                    readoutState++;
                }
                break;
            case 2:
                n_read = serial->read(&data[1], 1); // read id H
                if(n_read < 0){
                    readoutState = 0;
                    return;
                }
                if(n_read == 1){
                    // we were able to read -> read id M
                    readoutState++;
                    read_length = 0;
                }
                break;
            case 3:
                n_read = serial->read(&data[1], 1); // read id M
                if(n_read < 0){
                    readoutState = 0;
                    return;
                }
                if(n_read == 1){
                    // we were able to read -> read id L
                    readoutState++;
                    read_length = 0;
                }
                break;
            case 4:
                n_read = serial->read(&data[2], 1); // read id L
                if(n_read < 0){
                    readoutState = 0;
                    return;
                }
                if(n_read == 1){
                    // we were able to read -> read length
                    readoutState++;
                    read_length = 0;
                }
                break;
            case 5:
                n_read = serial->read(&data[3], 1); // read length
                if(n_read < 0){
                    readoutState = 0;
                    return;
                }
                if(n_read == 1){
                    // we were able to read the length -> read data (if any)
                    read_length = 0;
                    if(data[3] == 0){
                        readoutState += 2;
                    } else {
                        readoutState++;
                    }
                }
                break;
            case 6:
                n_read = serial->read(&data[4+read_length], data[4]-read_length);
                if(n_read < 0){
                    readoutState = 0;
                    return;
                }
                read_length += n_read;
                if(read_length == data[3]){
                    readoutState++;
                }
                break;
            case 7:
                n_read = serial->read(&data[4+read_length], 1);
                if(n_read < 0){
                    readoutState = 0;
                    return;
                }
                if(n_read == 1) {
                    if(data[4+read_length] == GMessage::GMessageStopByte){
                        GMessage((GMessageCode) data[0], data[1], ((uint16_t) data[2] << 8) | data[3], &data[4], read_length);
                        // TODO: process message

                        if(model->insertRow(model->rowCount())){
                            QModelIndex index = model->index(model->rowCount() - 1, 0);
                            model->setData(index, "Data received!");
                        }

                    } else {
                        // incorrect data byte received -> reset state
                    }
                    readoutState = 0;
                }

                break;
            default:
                readoutState = 0;
                break;
        }
    } while(n_read > 0);
}

void GloxiniaConfigurator::updateSerialPortList(void)
{
    auto list = QSerialPortInfo::availablePorts();

    // remove items
     for(QAction* i : serialPortActionList)
     {
        bool serialPortFound = false;
        for(const QSerialPortInfo &j : list)
        {
            if(j.portName() == i->text())
            {
                serialPortFound = true;
                break;
            }
        }
        if(!serialPortFound)
        {
            ui->menuPort_selection->removeAction(i);
        }
    }

    // add new items
    for(const QSerialPortInfo &i : list)
    {
        bool serialPortFound = false;
        for(QAction* j : serialPortActionList)
        {
            if(i.portName() == j->text()){
                serialPortFound = true;
                break;
            }
        }
        if(!serialPortFound)
        {
            QAction* action = ui->menuPort_selection->addAction(i.portName());
            action->setCheckable(true);
            connect(action, SIGNAL(triggered()), this, SLOT(GloxiniaConfigurator::setSerialPort()));
            serialPortActionList.append(action);
        }
    }
}

void GloxiniaConfigurator::setSerialPort(void)
{
    QAction* button = qobject_cast<QAction*>(sender());
    if( button != NULL )
    {

    }
}

void GloxiniaConfigurator::showStatusMessage(const QString &message)
{
    status->setText(message);
}

