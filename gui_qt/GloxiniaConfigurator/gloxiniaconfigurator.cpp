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


GloxiniaConfigurator::GloxiniaConfigurator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GloxiniaConfigurator)
    , serial(new QSerialPort(this))
    , status(new QLabel)
    , systemSettings(new SettingsDialog)
    , sensorSettings(new SensorDialog)
    , configureSHT35Dialog(new ConfigureSHT35Dialog)
    , moduleDialog(new ModuleDialog)
{
    ui->setupUi(this);
    ui->statusbar->addWidget(status);

    const QStringList headers({tr("Title"), tr("Description")});

    QFile file(":/empty.txt");
    file.open(QIODevice::ReadOnly);

    this->treeModel = new TreeModel(file.readAll(), this);

    connect(serial, &QSerialPort::readyRead, this, &GloxiniaConfigurator::readData);
    connect(ui->actionConnect, &QAction::triggered, this, &GloxiniaConfigurator::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &GloxiniaConfigurator::closeSerialPort);
    connect(ui->actionConfigure, &QAction::triggered, systemSettings, &SettingsDialog::show);
    //connect(ui->actionAdd_sensor, &QAction::triggered, sensorSettings, &SensorDialog::show);
    connect(ui->actionAddSensor, &QAction::triggered, this, &GloxiniaConfigurator::addSensor);
    connect(ui->actionAddNode, &QAction::triggered, this, &GloxiniaConfigurator::addNode);
    ui->actionDisconnect->setEnabled(false);
    ui->actionUpdate->setEnabled(false);

    ui->messageView->setModel(this->treeModel);

    ui->systemOverview->setModel(this->treeModel);
    //ui->systemOverview->setModel(this->model);
    //ui->systemOverview->setColumnCount(1);
    ui->systemOverview->setHeaderHidden(true);
    ui->systemOverview->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(ui->systemOverview, &QTreeView::customContextMenuRequested,this, &GloxiniaConfigurator::showContextMenu);

    updateSerialPortList();

    //addNode(new GCNodeModel(GCNodeModel::DicioNode));
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
        ui->actionUpdate->setEnabled(true);
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
    ui->actionUpdate->setEnabled(false);
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

                        /*if(model->insertRow(model->rowCount())){
                            QModelIndex index = model->index(model->rowCount() - 1, 0);
                            model->setData(index, "Data received!");
                        }*/

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
            connect(action,  &QAction::triggered, this, &GloxiniaConfigurator::setSerialPort);
            serialPortActionList.append(action);
        }
    }
}

void GloxiniaConfigurator::setSerialPort(void)
{
    QAction* button = qobject_cast<QAction*>(sender());
    if( button != NULL )
    {
        // disable other buttons
        for(QAction* i : serialPortActionList){
            if(i != button){
                i->setChecked(false);
            }
        }
    }
}

void GloxiniaConfigurator::showStatusMessage(const QString &message)
{
    status->setText(message);
}
/*
QIcon GloxiniaConfigurator::getSensorIcon(GCSensor* s)
{
    if(s == nullptr){
        return QIcon(":/images/unknown-sensor.png");
    } else {
        switch(s->getSensorType()){
        case GCSensor::Disabeled:
            return QIcon(":/images/general-sensor.png");
            break;
        case GCSensor::SHT35:
            return QIcon(":/images/temperature-rh-sensor.png");
            break;
        case GCSensor::APDS9306:
            return QIcon(":/images/light-sensor.png");
            break;
        }
    }

    return QIcon(":/images/unknown-sensor.png");
}*/

/*
QIcon GloxiniaConfigurator::getNodeIcon(GCNodeModel* n)
{
    // TODO: custom icons

    if(n == nullptr){
        return QIcon(":/images/unknown-sensor.png");
    } else {
        switch(n->getType()){
        case GCNodeModel::DicioNode:
            return QIcon(":/images/node.png");
            break;
        case GCNodeModel::PlanaltaNode:
            return QIcon(":/images/node.png");
            break;
        case GCNodeModel::SylvaticaNode:
            return QIcon(":/images/node.png");
            break;
        }
    }

    return QIcon(":/images/unknown-sensor.png");
}

QString GloxiniaConfigurator::getNodeLabel(GCNodeModel* n){
    QString label;
    label += tr("Node ");
    label += QString::number(n->getID());

    return label;
}

QString GloxiniaConfigurator::getSensorLabel(GCSensor* s){
    QString label;
    if(s == nullptr){
        label += tr("Unconnected port ");

    } else {
        label += GCSensor::sensorTypeToString(s->getSensorType());
    }

    return label;
}*/

void GloxiniaConfigurator::addNode()
{
    // add node to the list
    /*nodeList.append(node);

    // create tree item
    QTreeWidgetItem* itemNode = new QTreeWidgetItem(ui->systemOverview);
    QIcon nodeIcon = getNodeIcon(node);
    QString nodeLabel = getNodeLabel(node);
    itemNode->setText(0, nodeLabel);
    itemNode->setIcon(0, nodeIcon);
    ui->systemOverview->insertTopLevelItem(node->getID(), itemNode);

    // create sensors
    for(int i = 0; i < node->rowCount(); i++){
        QTreeWidgetItem* item = new QTreeWidgetItem(itemNode);
        GCSensor* sensor = node->getSensor(i);
        QString sensorLabel = getSensorLabel(sensor) + "(" + QString::number(i) + ")";
        QIcon sensorIcon = getSensorIcon(sensor);

        item->setText(0, sensorLabel);
        item->setIcon(0, sensorIcon);
        itemNode->addChild(item);

    }*/
    //const QModelIndex index = ui->systemOverview->selectionModel();//)->currentIndex();
    //auto sm = ui->systemOverview->selectionModel();
    //qInfo() << sm;
    //QAbstractItemModel *model = ui->systemOverview->model();

    //updateActions();
    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    updateActions();

    const QModelIndex child = model->index(index.row() + 1, 0, index.parent());
    moduleDialog->setWindowModality(Qt::ApplicationModal);
    moduleDialog->exec();
    const GCNode data = moduleDialog->getNode();
    model->setData(child, QVariant::fromValue(data), Qt::EditRole);
}


void GloxiniaConfigurator::addSensor()
{
    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();

    /*if (model->columnCount(index) == 0)
    {
        if (!model->insertColumn(0, index))
        {
            return;
        }
    }*/

    auto parent = index.parent();
    bool cond1 = !index.parent().isValid();
    QVariant data = model->data(index, Qt::EditRole);

    bool cond2 = !model->insertRow(0, index);

    if (cond1 || cond2)
    {
        QMessageBox msgBox;
        msgBox.setText("Cannot add a sensor here. Select/add a node instead.");
        msgBox.exec();
        return;
    }

    for (int column = 0; column < model->columnCount(index); ++column)
    {
        const QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant(tr("[No data]")), Qt::EditRole);
    }

    ui->systemOverview->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);
    updateActions();
}




void GloxiniaConfigurator::removeNode(uint8_t id)
{
    // TODO
}

/*
void GloxiniaConfigurator::showContextMenu(const QPoint &pos)
{
    QMenu m(tr("Context menu"), this);

    QAction a("Delete", this);
    connect(&a, &QAction::triggered, this, &GloxiniaConfigurator::removeNode);

    // get item at selected position:
    QTreeWidgetItem *nd = this->ui->systemOverview->itemAt( pos );
    // https://stackoverflow.com/questions/14237020/qtreewidget-right-click-menu

    qInfo() << "Index is " << this->ui->systemOverview->indexFromItem(nd);

    m.addAction(&a);

    m.exec(ui->systemOverview->mapToGlobal(pos));
}*/

/*
void GloxiniaConfigurator::setSensor(void)
{
    QStringList nodeList;
    for(GCNodeModel* i : this->nodeList){
        QString label = tr("Node ");
        label.append(QString::number(i->getID()));
        nodeList << label;
    }

    // select node
    bool ok;
    QString item = QInputDialog::getItem(this, tr("Select node"), tr("Node:"), nodeList, 0, false, &ok);
    int nodeIndex = nodeList.indexOf(item);

    if(ok && !item.isEmpty()){
        // all OK, we go to next screen
    } else {
        // TODO: add error dialog?
        return;
    }

    GCNodeModel* node = this->nodeList[nodeIndex];

    // select port on the node
    // TODO
    int port = 0;
    GCSensor* sensor = node->getSensor(port);

    QStringList items;
    //items << "-- select sensor--";
    items << "SHT35";
    items << "APDS9306";

    item = QInputDialog::getItem(this, tr("Select sensor type"), tr("Sensor:"), items, 0, false, &ok);
    int sensorIndex = items.indexOf(item);
    if(ok && !item.isEmpty()){
        switch(sensorIndex){
            case 0:
                if((sensor != nullptr) && (sensor->getSensorType() == GCSensor::SensorType::SHT35)){
                    GCSensorSHT35* specific_sensor = static_cast<GCSensorSHT35*>(sensor);
                    configureSHT35Dialog->setSensorSettings(*specific_sensor);
                    //free(specific_sensor);
                }
                configureSHT35Dialog->show();
                sensor = new GCSensorSHT35(configureSHT35Dialog->getSensor());
                sensor = node->replaceSensor(port, sensor);
                if(sensor != nullptr){
                    free(sensor);
                }

                // add sensor to layout
                QTreeWidgetItem* nodeTreeItem = ui->systemOverview->itemAt(0, node->getID());
                QTreeWidgetItem* item = new QTreeWidgetItem(nodeTreeItem);
                QIcon sensorIcon(":/images/temperature-rh-sensor.png");
                item->setText(0, "SHT35");
                item->setIcon(0, sensorIcon);




                break;
        }
    }
}*/

void GloxiniaConfigurator::updateActions()
{
    const bool hasSelection = !ui->systemOverview->selectionModel()->selection().isEmpty();
    //removeRowAction->setEnabled(hasSelection);
    //removeColumnAction->setEnabled(hasSelection);

    const bool hasCurrent = ui->systemOverview->selectionModel()->currentIndex().isValid();
    //insertRowAction->setEnabled(hasCurrent);
    //insertColumnAction->setEnabled(hasCurrent);

    if (hasCurrent) {
        ui->systemOverview->closePersistentEditor(ui->systemOverview->selectionModel()->currentIndex());

        const int row = ui->systemOverview->selectionModel()->currentIndex().row();
        const int column = ui->systemOverview->selectionModel()->currentIndex().column();
        if (ui->systemOverview->selectionModel()->currentIndex().parent().isValid())
            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        else
            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
    }
}
