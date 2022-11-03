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


GloxiniaConfigurator::GloxiniaConfigurator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GloxiniaConfigurator)
    , serial(new QSerialPort(this))
    , status(new QLabel)
    , systemSettings(new SettingsDialog)
    , sensorSettings(new SensorDialog)
    , configureSHT35Dialog(new ConfigureSHT35Dialog)
    , nodeDialog(new NodeDialog)
    , chart(new QChart)
    , messageModel(new QStringListModel)
{
    ui->setupUi(this);
    ui->statusbar->addWidget(status);

    const QStringList headers({tr("Title"), tr("Description")});

    this->treeModel = new TreeModel(this);

    // serial data readout trigger
    connect(serial, &QSerialPort::readyRead, this, &GloxiniaConfigurator::readData);

    // connect File menu to functions
    connect(ui->actionSave, &QAction::triggered, this, &GloxiniaConfigurator::saveToFile);
    connect(ui->actionOpen, &QAction::triggered, this, &GloxiniaConfigurator::loadFromFile);
    connect(ui->actionSelectDataFile, &QAction::triggered, this, &GloxiniaConfigurator::selectDataFile);
    connect(ui->actionClear, &QAction::triggered, this, &GloxiniaConfigurator::clearAll);


    // connect Edit menu to functions
    connect(ui->actionAddNode, &QAction::triggered, this, &GloxiniaConfigurator::addNode);
    connect(ui->actionAddSensor, &QAction::triggered, this, &GloxiniaConfigurator::addSensor);
    connect(ui->actionEditNode, &QAction::triggered, this, &GloxiniaConfigurator::editNode);
    connect(ui->actionEditSensor, &QAction::triggered, this, &GloxiniaConfigurator::editSensor);
    connect(ui->actionDelete, &QAction::triggered, this, &GloxiniaConfigurator::removeItems);
    connect(ui->actionPreferences, &QAction::triggered, this, &GloxiniaConfigurator::preferencesMenu);
    connect(ui->actionRunDiscovery, &QAction::triggered, this, &GloxiniaConfigurator::runDiscovery);

    // connect System menu to functions
    connect(ui->actionConnect, &QAction::triggered, this, &GloxiniaConfigurator::openSerialPort);
    //connect(ui->actionUpdate, &QAction::triggered, this, &GloxiniaConfigurator::);
    connect(ui->actionDisconnect, &QAction::triggered, this, &GloxiniaConfigurator::closeSerialPort);
    connect(ui->actionConfigure, &QAction::triggered, systemSettings, &SettingsDialog::show);
    connect(ui->actionRefreshPorts, &QAction::triggered, this, &GloxiniaConfigurator::updateSerialPortList);
    updateSerialPortList();


    ui->actionDisconnect->setEnabled(false);
    ui->actionUpdate->setEnabled(false);

    ui->messageView->setModel(this->messageModel);

    ui->systemOverview->setModel(this->treeModel);
    ui->systemOverview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //ui->systemOverview->setModel(this->model);
    //ui->systemOverview->setColumnCount(1);
    ui->systemOverview->setHeaderHidden(true);
    ui->systemOverview->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->systemOverview, &QTreeView::customContextMenuRequested, this, &GloxiniaConfigurator::showContextMenu);



    /*
     * for debugging, add some default system state
     */
    /*QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    this->treeModel->insertRow(index.row()+1, index.parent());
    QModelIndex child = this->treeModel->index(index.row() + 1, 0, index.parent());
    GCNode* data = new GCNode(0, GCNode::GCDicio, "logger");
    this->treeModel->setData(child, QVariant::fromValue(data), Qt::EditRole);

    index = ui->systemOverview->selectionModel()->currentIndex();
    this->treeModel->insertRow(index.row()+1, index.parent());
    child = this->treeModel->index(index.row() + 1, 0, index.parent());
    data = new GCNode(0, GCNode::GCDicio, "greenhouse 1");
    this->treeModel->setData(child, QVariant::fromValue(data), Qt::EditRole);

    index = ui->systemOverview->selectionModel()->currentIndex();
    this->treeModel->insertRow(index.row()+1, index.parent());
    child = this->treeModel->index(index.row() + 1, 0, index.parent());
    data = new GCNode(0, GCNode::GCDicio, "greenhouse 2");
    this->treeModel->setData(child, QVariant::fromValue(data), Qt::EditRole);*/

    //addNode(new GCNodeModel(GCNodeModel::DicioNode));
    QLineSeries *series = new QLineSeries();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setTitle("Some example plot");

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(10);
    axisX->setFormat("MMM yyyy");
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Sunspots count");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui->vLayout->addWidget(chartView);

    messageModel->insertRow(0);
    QModelIndex mIndex = messageModel->index(0, 0);
    messageModel->setData(mIndex, "Application started.");

    discoveryTimer = new QTimer();
    connect(discoveryTimer, &QTimer::timeout, this, &GloxiniaConfigurator::runDiscovery);
}

GloxiniaConfigurator::~GloxiniaConfigurator()
{
    delete ui;
}

// https://code.qt.io/cgit/qt/qtserialport.git/tree/examples/serialport/terminal/mainwindow.cpp?h=5.15
void GloxiniaConfigurator::openSerialPort()
{
    //const SettingsDialog::Settings p = m_settings->settings();
    serial->setPortName(serialPortName);
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
        showStatusMessage(tr("Connected to ") + serialPortName);
        /*showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));*/

        // TODO make timer run/stop based on settings
        discoveryTimer->start(30*1000); // run every 30 seconds
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}

void GloxiniaConfigurator::closeSerialPort()
{
    discoveryTimer->stop();

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
                } else {
                    break;
                }
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
                } else {
                    break;
                }
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
                } else {
                    break;
                }
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
                } else {
                    break;
                }
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
                } else {
                    break;
                }
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
                        break;
                    } else {
                        readoutState++;
                    }
                } else {
                    break;
                }
            case 6:
                n_read = serial->read(&data[4+read_length], data[4]-read_length);
                if(n_read < 0){
                    readoutState = 0;
                    return;
                }
                read_length += n_read;
                if(read_length == data[4]){
                    readoutState++;
                } else {
                    break;
                }
            case 7:
                n_read = serial->read(&data[4+read_length], 1);
                if(n_read < 0){
                    readoutState = 0;
                    n_read = 0;
                    read_length = 0;
                    return;
                }
                if(n_read == 1) {
                    if(data[4+read_length] == GMessage::GMessageStopByte){
                        GMessage m((GMessage::Code) data[0], data[1], ((uint16_t) data[2] << 8) | data[3], &data[4], read_length);

                        processIncomingGMessage(m);

                        /*if(model->insertRow(model->rowCount())){
                            QModelIndex index = model->index(model->rowCount() - 1, 0);
                            model->setData(index, "Data received!");
                        }*/

                    } else {
                        // incorrect data byte received -> reset state
                    }
                    readoutState = 0;
                    n_read = 0;
                    read_length = 0;
                    break;
                } else {
                    break;
                }

                break;
            default:
                readoutState = 0;
                break;
        }
    } while(n_read > 0);
}
void GloxiniaConfigurator::processCANDiscoveryMessage(const GMessage& m)
{
    qInfo() << "Received discovery message";

    if(treeModel->checkUniqueNodeID(m.getMessageID()))
    {
        QModelIndex index = QModelIndex();
        bool success = this->treeModel->insertRow(treeModel->rowCount(), index);
        QModelIndex child = this->treeModel->index(treeModel->rowCount()-1, 0, index);
        // TODO: handle different types of nodes
        GCNode* data = new GCNode(m.getMessageID(), GCNode::GCDicio, "unlabeled node");
        this->treeModel->setData(child, QVariant::fromValue(data), Qt::EditRole);
    }
}

void GloxiniaConfigurator::processIncomingGMessage(const GMessage& m)
{
    switch(m.getCode()){
    case GMessage::Code::CAN_REQUEST_ADDRESS_AVAILABLE:
        break;
    case GMessage::Code::CAN_ADDRESS_TAKEN:
        break;
    case GMessage::Code::CAN_UPDATE_ADDRESS:
        break;
    case GMessage::Code::CAN_DISCOVERY:
        // check if node exists, if not create one and add to model
        processCANDiscoveryMessage(m);
        break;
    case GMessage::Code::startMeasurement:
    case GMessage::Code::stopMeasurement:
    case GMessage::Code::activate_sensor:
    case GMessage::Code::deactivate_sensor:
    case GMessage::Code::reset_node:
    case GMessage::Code::reset_system:
    case GMessage::Code::text_message:
    case GMessage::Code::sensor_data:
    case GMessage::Code::sensor_status:
    case GMessage::Code::measurement_period:
    case GMessage::Code::error_message:
    case GMessage::Code::loopback_message:
    case GMessage::Code::actuator_status:
    case GMessage::Code::hello_message:
    case GMessage::Code::init_sampling:
    case GMessage::Code::init_sensors:
    case GMessage::Code::sensor_error:
    case GMessage::Code::lia_gain:
    case GMessage::Code::unknown:
    case GMessage::Code::meas_trigger:
    case GMessage::Code::sensor_config:
    case GMessage::Code::actuator_data:
    case GMessage::Code::actuator_error:
    case GMessage::Code::actuator_trigger:
    case GMessage::Code::actuator_gc_temp:
    case GMessage::Code::actuator_gc_rh:
    case GMessage::Code::sensor_start:
    case GMessage::Code::actuator_relay:
    case GMessage::Code::sensor_actuator_enable:
    case GMessage::Code::actuator_relay_now:
    default:
        // LOG unknown message to log;
        break;
    }

    messageModel->insertRow(0);
    QModelIndex mIndex = messageModel->index(0, 0);
    messageModel->setData(mIndex, m.toLogString());

    qInfo() << "Processing" << m.toString();
}

void GloxiniaConfigurator::updateSerialPortList(void)
{
    // TODO: add sort items

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
            ui->menuPortSelection->removeAction(i);
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
            QAction* action = ui->menuPortSelection->addAction(i.portName());
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
            } else {
                i->setChecked(true);
            }
        }

        // update configuration and the connected port
        serialPortName = button->text();
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

void GloxiniaConfigurator::runDiscovery()
{
    qInfo() << "Running discovery broadcast";

    GMessage m(GMessage::Code::CAN_DISCOVERY, 0, 0);
    char rawData[32];
    unsigned int length = m.toBytes(rawData, 32);
    serial->write(rawData, length);
}

void GloxiniaConfigurator::addNode()
{
    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();

    auto parent = index.parent();

    if(parent.isValid()){
        QMessageBox msgBox;
        msgBox.setText("Nodes cannot be added to other nodes.");
        msgBox.exec();
        return;
    }

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    updateActions();

    const QModelIndex child = model->index(index.row() + 1, 0, index.parent());
    nodeDialog->setWindowModality(Qt::ApplicationModal);
    nodeDialog->exec();
    GCNode* data = nodeDialog->getNode();
    model->setData(child, QVariant::fromValue(data), Qt::EditRole);

    //ui->systemOverview->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

    //updateActions();
}

bool GloxiniaConfigurator::removeNode(const QModelIndex& index)
{

    QAbstractItemModel *model = ui->systemOverview->model();

    if(index.isValid() && !index.parent().isValid()){
        return model->removeRow(index.row(), index.parent());
    }

    messageModel->insertRow(0);
    QModelIndex mIndex = messageModel->index(0, 0);
    messageModel->setData(mIndex, "Could not remove node.");


    return false;


    // get item at selected position:
    QItemSelectionModel* selectionModel = this->ui->systemOverview->selectionModel();

    const QModelIndexList indexes = selectionModel->selectedIndexes();

    GCSensor* s;
    GCNode* n;
    bool containsNode = false, containsSensor = false;

    for (const QModelIndex &index : indexes) {
        QString text = QString("(%1,%2)").arg(index.row()).arg(index.column());
        QVariant data = ui->systemOverview->model()->data(index, Qt::EditRole);
        s = data.value<GCSensor*>();
        n = data.value<GCNode*>();
        if(s != nullptr){ containsSensor = true; }
        if(n != nullptr){ containsNode = true; }
    }

    // https://stackoverflow.com/questions/14237020/qtreewidget-right-click-menu

    //qInfo() << "Index is " << this->ui->systemOverview->indexFromItem(nd);
}

void GloxiniaConfigurator::editNode()
{

    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();

    // node is selected -> run menu
    if(index.isValid() && !index.parent().isValid()){
        QVariant data = model->data(index, Qt::EditRole);
        GCNode* nodeData = data.value<GCNode*>();

        nodeDialog->setNodeSettings(nodeData);
        nodeDialog->setWindowModality(Qt::ApplicationModal);
        nodeDialog->exec();
        delete nodeData;
        model->setData(index, QVariant::fromValue(nodeDialog->getNode()), Qt::EditRole);
    }
}


void GloxiniaConfigurator::addSensor()
{
    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();

    // node is selected -> add sensor as child
    if(index.isValid() && !index.parent().isValid()){

        if (!model->insertRow(0, index))
        {
            QMessageBox msgBox;
            msgBox.setText("Error inserting the sensor into the system.");
            msgBox.exec();
            return;
        }

        for (int column = 0; column < model->columnCount(index); ++column)
        {
            const QModelIndex child = model->index(0, column, index);
            GCSensor* sensor = selectSensor();
            model->setData(child, QVariant::fromValue(sensor), Qt::EditRole);
            //model->setData(child, QVariant::fromValue(GCNode()), Qt::EditRole);
        }

        ui->systemOverview->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                                QItemSelectionModel::ClearAndSelect);
        updateActions();

        return;

    }

    // sensor is selected -> add sensor as new row
    if(index.isValid() && index.parent().isValid()){
        if (!model->insertRow(index.row()+1, index.parent()))
            return;

        updateActions();

        const QModelIndex child = model->index(index.row() + 1, 0, index.parent());
        selectSensor();
        GCSensor* sensor = selectSensor();
        model->setData(child, QVariant::fromValue(sensor), Qt::EditRole);
        //model->setData(child, QVariant::fromValue(GCNode()), Qt::EditRole);

        ui->systemOverview->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                                QItemSelectionModel::ClearAndSelect);
        updateActions();

        return;
    }

    QMessageBox msgBox;
    msgBox.setText("Sensors can only be added to nodes.");
    msgBox.exec();
    return;
}


bool GloxiniaConfigurator::removeSensor(const QModelIndex& index)
{
    QAbstractItemModel *model = ui->systemOverview->model();

    if(index.isValid() && index.parent().isValid()){
        return model->removeRow(index.row(), index.parent());
    }

    return false;
}

void GloxiniaConfigurator::removeItems()
{
    QItemSelectionModel *selectionModel = ui->systemOverview->selectionModel();
    QAbstractItemModel *model = ui->systemOverview->model();

    const QModelIndexList indexes = selectionModel->selectedIndexes();
    QModelIndexList nodeIndices;

    for (const QModelIndex &index : indexes)
    {
        if(!removeSensor(index))
        {
            nodeIndices.append(index);
        }
    }

    for(const QModelIndex &index : nodeIndices)
    {
        removeNode(index);
    }
}

void GloxiniaConfigurator::editSensor()
{
    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();

    // sensor is selected -> run menu
    if(index.isValid() && index.parent().isValid()){
        QVariant data = model->data(index, Qt::EditRole);
        GCSensorSHT35* sensorSHT35 = data.value<GCSensorSHT35*>();
        GCSensorAPDS9306* sensorAPDS9306 = data.value<GCSensorAPDS9306*>();

        if(sensorSHT35 != nullptr)
        {
            configureSHT35Dialog->setSensorSettings(sensorSHT35);
            configureSHT35Dialog->setWindowModality(Qt::ApplicationModal);
            configureSHT35Dialog->exec();
            delete sensorSHT35;
            model->setData(index, QVariant::fromValue(configureSHT35Dialog->getSensor()), Qt::EditRole);
        }

        if(sensorAPDS9306 != nullptr)
        {
            // TODO
        }
    }
}

void GloxiniaConfigurator::preferencesMenu(void)
{

}

void GloxiniaConfigurator::showContextMenu(const QPoint &pos)
{
    // only show menu when an item is selected

    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();

    bool nodeCurrent = false, sensorCurrent = false;

    // node is selected -> add sensor as child
    if(index.isValid() && !index.parent().isValid()){
        nodeCurrent = true;
    }

    // sensor is selected -> add sensor as new row
    if(index.isValid() && index.parent().isValid()){
        sensorCurrent = true;
    }

    QMenu m(tr("Context menu"), this);

    QAction mDelete("Delete", this);
    connect(&mDelete, &QAction::triggered, this, &GloxiniaConfigurator::removeItems);
    m.addAction(&mDelete);

    QAction mAddNode("Add node", this);
    connect(&mAddNode, &QAction::triggered, this, &GloxiniaConfigurator::addNode);
    m.addAction(&mAddNode);

    QAction mAddSensor("Add sensor", this);
    connect(&mAddSensor, &QAction::triggered, this, &GloxiniaConfigurator::addSensor);
    m.addAction(&mAddSensor);

    QAction mEditNode("Edit node", this);
    connect(&mEditNode, &QAction::triggered, this, &GloxiniaConfigurator::editNode);
    m.addAction(&mEditNode);

    QAction mEditSensor("Edit sensor", this);
    connect(&mEditSensor, &QAction::triggered, this, &GloxiniaConfigurator::editSensor);
    m.addAction(&mEditSensor);

    if(sensorCurrent){
        mAddNode.setEnabled(false);
        mEditNode.setEnabled(false);
    }

    if(nodeCurrent){
        mEditSensor.setEnabled(false);
    }

    m.exec(ui->systemOverview->mapToGlobal(pos));
}


GCSensor* GloxiniaConfigurator::selectSensor(void)
{
    QStringList items;
    //items << "-- select sensor--";
    items << "SHT35";
    items << "APDS9306";

    bool ok;
    GCSensor* sensor = nullptr;

    QString item = QInputDialog::getItem(this, tr("Select sensor type"), tr("Sensor:"), items, 0, false, &ok);
    int sensorIndex = items.indexOf(item);
    if(ok && !item.isEmpty()){
        switch(sensorIndex){
        case 0:
            configureSHT35Dialog->setWindowModality(Qt::ApplicationModal);
            configureSHT35Dialog->exec();
            sensor = configureSHT35Dialog->getSensor();
            break;
        default:
            sensor = nullptr;
            break;
        }
    }
    return sensor;

}

void GloxiniaConfigurator::updateActions()
{
    const bool hasSelection = !ui->systemOverview->selectionModel()->selection().isEmpty();
    //removeRowAction->setEnabled(hasSelection);
    //removeColumnAction->setEnabled(hasSelection);

    const bool hasCurrent = ui->systemOverview->selectionModel()->currentIndex().isValid();
    //addNode->setEnabled(hasCurrent);
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


void GloxiniaConfigurator::saveToFile(void)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Configuration"), "", tr("Gloxinia Config File (*.gcf);;All Files (*)"));

    // save only if filename is non-empty
    if(fileName.isEmpty())
        return;

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    QTextStream out(&file); // QDataStream is also possible for serialised data
    //out.setVersion(QDataStream::Qt_6_3);

    QStringList textConfig = treeModel->toTextConfig();
    for(QString i : textConfig)
    {
        out << i << "\n";
    }

    // TODO: update status bar with message indicating the file was saved
}
void GloxiniaConfigurator::loadFromFile(void)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Gloxinia Configuration"), "", tr("Gloxinia Config File (*.gcf);;All Files (*)"));

    // load only if filename is non-empty
    if(fileName.isEmpty())
        return;

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    QTextStream in(&file); // QDataStream is also possible for serialised data
    //out.setVersion(QDataStream::Qt_6_3);

    QString textConfigFull = file.readAll();
    QStringList textConfig = textConfigFull.split("\n");

    clearAll();

    // TODO: read file version
    // TODO: read filenames where data is created

    bool ok = treeModel->fromTextConfig(textConfig);

    //if(ok)
    // TODO: update status bar with message indicating the file was saved

}

void GloxiniaConfigurator::clearAll(void)
{
    // remove all items from the tree
    qInfo() << "Number of rows:" << treeModel->rowCount();
    treeModel->removeRows(0, treeModel->rowCount());
}

void GloxiniaConfigurator::selectDataFile(void)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Open Data File"), "", tr("Comma Seperated Values (*.csv);;All Files (*)"));

    // save only if filename is non-empty
    if(fileName.isEmpty())
        return;

    if(dataFile != nullptr)
    {
        dataFile->close();
        delete dataFile;
    }

    if(dataStream != nullptr)
    {
        delete dataStream;
    }

    dataFile = new QFile(fileName);
    if(!dataFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), dataFile->errorString());
        return;
    }

    // create a new datastream
    dataStream = new QDataStream(dataFile);
    dataStream->setVersion(QDataStream::Qt_6_3);
}
