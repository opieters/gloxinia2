#include "gloxiniaconfigurator.h"
#include "./ui_gloxiniaconfigurator.h"
#include "qthread.h"

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
#include <QThread>

GloxiniaConfigurator::GloxiniaConfigurator(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::GloxiniaConfigurator),
      serial(new QSerialPort(this)),
      status(new QLabel),
      systemSettings(new SettingsDialog),
      sensorSettings(new SensorDialog),
      sensorMeasurementDialog(new SensorMeasurementDialog),
      globalMeasurementPolicyDialog(new GlobalMeasurementPolicyDialog),
      sensorAnalogueDialog(new SensorAnalogueDialog),
      sensorAPDS9306_065Dialog(new SensorAPDS9306_065Dialog),
      sensorSHT35Dialog(new SensorSHT35Dialog),
      nodeDicioDialog(new NodeDicioDialog),
      chart(new QChart),
      messageModel(new QStringListModel)
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
    // connect(ui->actionAddNode, &QAction::triggered, this, &GloxiniaConfigurator::addNode);
    //connect(ui->actionAddSensor, &QAction::triggered, this, &GloxiniaConfigurator::addSensor);
    connect(ui->actionEditNode, &QAction::triggered, this, &GloxiniaConfigurator::editNode);
    connect(ui->actionEditSensor, &QAction::triggered, this, &GloxiniaConfigurator::editSensor);
    connect(ui->actionDelete, &QAction::triggered, this, &GloxiniaConfigurator::removeItems);
    connect(ui->actionPreferences, &QAction::triggered, this, &GloxiniaConfigurator::preferencesMenu);
    connect(ui->actionRunDiscovery, &QAction::triggered, this, &GloxiniaConfigurator::runDiscovery);
    connect(ui->actionStartMeasuring, &QAction::triggered, this, &GloxiniaConfigurator::startMeasuring);
    connect(ui->actionStopMeasuring, &QAction::triggered, this, &GloxiniaConfigurator::stopMeasuring);

    // connect System menu to functions
    connect(ui->actionConnect, &QAction::triggered, this, &GloxiniaConfigurator::openSerialPort);
    // connect(ui->actionUpdate, &QAction::triggered, this, &GloxiniaConfigurator::);
    connect(ui->actionDisconnect, &QAction::triggered, this, &GloxiniaConfigurator::closeSerialPort);
    connect(ui->actionConfigure, &QAction::triggered, systemSettings, &SettingsDialog::show);
    connect(ui->actionRefreshPorts, &QAction::triggered, this, &GloxiniaConfigurator::updateSerialPortList);
    updateSerialPortList();

    ui->actionDisconnect->setEnabled(false);
    ui->actionUpdate->setEnabled(false);

    ui->messageView->setModel(this->messageModel);

    ui->systemOverview->setModel(this->treeModel);
    ui->systemOverview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // ui->systemOverview->setModel(this->model);
    // ui->systemOverview->setColumnCount(1);
    ui->systemOverview->setHeaderHidden(true);
    ui->systemOverview->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->systemOverview, &QTreeView::customContextMenuRequested, this, &GloxiniaConfigurator::showContextMenu);

    // connect sample period settings dialogs
    sensorMeasurementDialog->setGlobalPeriodDialog(globalMeasurementPolicyDialog);
    sensorSHT35Dialog->setPeriodDialog(sensorMeasurementDialog);


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

    // addNode(new GCNodeModel(GCNodeModel::DicioNode));
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
    // connect(discoveryTimer, &QTimer::timeout, this, &GloxiniaConfigurator::runDiscovery);

    serialPortName = "COM12";
    openSerialPort();
}

GloxiniaConfigurator::~GloxiniaConfigurator()
{
    delete ui;
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
    quint8 rawData[32];
    unsigned int length;

    // check if there already is a node in the system, if not send address update to finish first assignment
    /*if(treeModel->rowCount() == 0){
        qInfo() << "Assiging address of node conncted to computer.";
        std::vector<uint8_t> data = {(uint8_t) (GMessage::SearchStartAddress >> 8), (uint8_t) GMessage::SearchStartAddress};
        GMessage m(GMessage::Code::UPDATE_ADDRESS, GMessage::UnsetAddress, GMessage::NoSensorID, false, data);
        length = m.toBytes(rawData, 32);
    } else {*/
    qInfo() << "Running discovery broadcast";

    GMessage m(GMessage::Code::DISCOVERY, 0, 0, true, std::vector<quint8>());

    length = m.toBytes(rawData, 32);
    //}
    serial->write((char *)rawData, length);
}

/*void GloxiniaConfigurator::addNode()
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
    nodeDicioDialog->setWindowModality(Qt::ApplicationModal);
    nodeDicioDialog->exec();
    GCNode* data = nodeDicioDialog->getNode();
    model->setData(child, QVariant::fromValue(data), Qt::EditRole);

    //ui->systemOverview->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

    //updateActions();
}*/

void GloxiniaConfigurator::startMeasuring(void)
{
    // loop over all nodes and sensors and trigger measurement
    for(int i = 0; i < treeModel->rowCount(); i++)
    {
        QModelIndex index = treeModel->index(i, 0);
        QVariant data = treeModel->data(index, Qt::EditRole);
        GCNode* node = GCNode::fromQVariant(data);

        if(node == nullptr)
            continue;

        for(int j = 0; j < treeModel->rowCount(index); j++)
        {
            QModelIndex sIndex = treeModel->index(j, 0, index);
            QVariant sData = treeModel->data(sIndex, Qt::EditRole);
            GCSensor* sensor = GCSensor::fromQVariant(sData);

            if(sensor == nullptr)
                continue;

            if(sensor->startMeasurement())
            {
                GMessage mStart = sensor->getStartMessage();

                sendSerialMessage(mStart);
                qInfo() << "Sending sensor start" << mStart.toString();
                QThread::msleep(1); // TODO: needed??
            }

        }
    }
}
void GloxiniaConfigurator::stopMeasuring(void)
{
    // loop over all nodes and sensora and cancel measurement
    for(int i = 0; i < treeModel->rowCount(); i++)
    {
        QModelIndex index = treeModel->index(i, 0);
        QVariant data = treeModel->data(index, Qt::EditRole);
        GCNode* node = GCNode::fromQVariant(data);

        if(node == nullptr)
            continue;

        for(int j = 0; j < treeModel->rowCount(index); j++)
        {
            QModelIndex sIndex = treeModel->index(j, 0, index);
            QVariant sData = treeModel->data(sIndex, Qt::EditRole);
            GCSensor* sensor = GCSensor::fromQVariant(sData);

            if(sensor == nullptr)
                continue;

            if(sensor->startMeasurement())
            {
                GMessage mStart = sensor->getStartMessage();

                sendSerialMessage(mStart);
                qInfo() << "Sending sensor start" << mStart.toString();
                QThread::msleep(1); // TODO: needed??
            }

        }
    }
}

bool GloxiniaConfigurator::removeNode(const QModelIndex &index)
{

    QAbstractItemModel *model = ui->systemOverview->model();

    if (index.isValid() && !index.parent().isValid())
    {
        return model->removeRow(index.row(), index.parent());
    }

    messageModel->insertRow(0);
    QModelIndex mIndex = messageModel->index(0, 0);
    messageModel->setData(mIndex, "Could not remove node.");

    return false;

    // get item at selected position:
    QItemSelectionModel *selectionModel = this->ui->systemOverview->selectionModel();

    const QModelIndexList indexes = selectionModel->selectedIndexes();

    GCSensor *s;
    GCNode *n;
    bool containsNode = false, containsSensor = false;

    for (const QModelIndex &index : indexes)
    {
        QString text = QString("(%1,%2)").arg(index.row()).arg(index.column());
        QVariant data = ui->systemOverview->model()->data(index, Qt::EditRole);
        s = data.value<GCSensor *>();
        n = data.value<GCNode *>();
        if (s != nullptr)
        {
            containsSensor = true;
        }
        if (n != nullptr)
        {
            containsNode = true;
        }
    }

    // https://stackoverflow.com/questions/14237020/qtreewidget-right-click-menu

    // qInfo() << "Index is " << this->ui->systemOverview->indexFromItem(nd);
}

void GloxiniaConfigurator::editNode()
{

    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();
    int result;

    // node is selected -> run menu
    if (index.isValid() && !index.parent().isValid())
    {
        QVariant data = model->data(index, Qt::EditRole);
        GCNodeDicio *nDicio = data.value<GCNodeDicio *>();
        GCNodePlanalta *nPlanalta = data.value<GCNodePlanalta *>();
        GCNodeSylvatica *nSylvatica = data.value<GCNodeSylvatica *>();

        if (nDicio != nullptr)
        {
            nodeDicioDialog->setNodeSettings(nDicio);
            nodeDicioDialog->setWindowModality(Qt::ApplicationModal);
            result = nodeDicioDialog->exec();
            if(result == QDialog::Rejected){
                return;
            }
            nodeDicioDialog->updateNode(nDicio);
            model->setData(index, QVariant::fromValue(nDicio), Qt::EditRole);
            return;
        }

        if (nPlanalta != nullptr)
        {
            // TODO
        }

        if (nSylvatica != nullptr)
        {
            // TODO
        }
    }
}

void GloxiniaConfigurator::addSensor()
{
    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();

    // node is selected -> add sensor as child
    if (index.isValid() && !index.parent().isValid())
    {

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
            GCSensor *sensor = selectSensor();
            model->setData(child, QVariant::fromValue(sensor), Qt::EditRole);
            // model->setData(child, QVariant::fromValue(GCNode()), Qt::EditRole);
        }

        ui->systemOverview->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                                              QItemSelectionModel::ClearAndSelect);
        updateActions();

        return;
    }

    // sensor is selected -> add sensor as new row
    if (index.isValid() && index.parent().isValid())
    {
        if (!model->insertRow(index.row() + 1, index.parent()))
            return;

        updateActions();

        const QModelIndex child = model->index(index.row() + 1, 0, index.parent());
        selectSensor();
        GCSensor *sensor = selectSensor();
        model->setData(child, QVariant::fromValue(sensor), Qt::EditRole);
        // model->setData(child, QVariant::fromValue(GCNode()), Qt::EditRole);

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

bool GloxiniaConfigurator::removeSensor(const QModelIndex &index)
{
    QAbstractItemModel *model = ui->systemOverview->model();

    if (index.isValid() && index.parent().isValid())
    {
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
        if (!removeSensor(index))
        {
            nodeIndices.append(index);
        }
    }

    for (const QModelIndex &index : nodeIndices)
    {
        removeNode(index);
    }
}

void GloxiniaConfigurator::editSensor()
{
    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();
    GCSensorSHT35 *sensorSHT35;
    GCSensorAPDS9306 *sensorAPDS9306;
    int result;

    QList<GMessage> configMs;

    // sensor is selected -> run menu
    if (index.isValid() && index.parent().isValid())
    {
        QVariant data = model->data(index, Qt::EditRole);
        sensorSHT35 = data.value<GCSensorSHT35 *>();
        sensorAPDS9306 = data.value<GCSensorAPDS9306 *>();

        // selected sensor is SHT35 -> edit parameters of this sensor
        if (sensorSHT35 != nullptr)
        {
            sensorSHT35Dialog->setSensorSettings(sensorSHT35);
            sensorSHT35Dialog->setWindowModality(Qt::ApplicationModal);
            result = sensorSHT35Dialog->exec();
            if(result == QDialog::Rejected){
                return;
            }
            sensorSHT35Dialog->updateSensor(sensorSHT35);
            model->setData(index, QVariant::fromValue(sensorSHT35), Qt::EditRole);
            return;
        }

        // selected sensor is APDS9306 065 -> edit parameters of this sensor
        if (sensorAPDS9306 != nullptr)
        {
            sensorAPDS9306_065Dialog->setSensorSettings(sensorAPDS9306);
            sensorAPDS9306_065Dialog->setWindowModality(Qt::ApplicationModal);
            result = sensorAPDS9306_065Dialog->exec();
            if(result == QDialog::Rejected){
                return;
            }
            sensorAPDS9306_065Dialog->updateSensor(sensorAPDS9306);
            model->setData(index, QVariant::fromValue(sensorAPDS9306), Qt::EditRole);
            return;
        }

        // there is no sensor in the system (nullptr) -> we need to select a sensor
        QStringList items;
        items << "-- select sensor--";
        items << "SHT35";
        items << "APDS9306 065";

        bool ok;
        GCSensor *sensor = nullptr;
        GCNode *node = nullptr;
        QVariant nodeData = model->data(index.parent(), Qt::EditRole);
        GCNodeDicio *nodeD = nodeData.value<GCNodeDicio*>();
        GCNodeSylvatica *nodeS = nodeData.value<GCNodeSylvatica*>();
        GCNodePlanalta *nodeP = nodeData.value<GCNodePlanalta*>();

        if(nodeD != nullptr){
            node = nodeD;
        }
        if(nodeS != nullptr){
            node = nodeS;
        }
        if(nodeP != nullptr){
            node = nodeP;
        }

        if(node == nullptr)
        {
            // TODO: something went wrong, display error message to the user
        }

        // request sensor type from user
        QString item = QInputDialog::getItem(this, tr("Select sensor type"), tr("Sensor:"), items, 0, false, &ok);
        int sensorIndex = items.indexOf(item);

        // using the index, we pop-up the sensor configuration dialog and add it at the relevant location
        if (ok && !item.isEmpty())
        {
            switch (sensorIndex)
            {
            case 1:
                sensorSHT35Dialog->setWindowModality(Qt::ApplicationModal);
                result = sensorSHT35Dialog->exec();
                if (result == QDialog::Rejected)
                {
                    return;
                }
                sensorSHT35 = new GCSensorSHT35(node);
                sensorSHT35Dialog->updateSensor(sensorSHT35);
                sensorSHT35->setInterfaceID((quint8)index.row());
                model->setData(index, QVariant::fromValue(sensorSHT35), Qt::EditRole);

                // update hardware configuration
                configMs = sensorSHT35->getConfigurationMessages();

                for(const GMessage &m : configMs){
                    sendSerialMessage(m);
                    qInfo() << "Send SHT35 config" << m.toString();

                }
                break;
            case 2:
                sensorAPDS9306_065Dialog->setWindowModality(Qt::ApplicationModal);
                result = sensorAPDS9306_065Dialog->exec();
                if (result == QDialog::Rejected)
                {
                    return;
                }
                sensorAPDS9306 = new GCSensorAPDS9306(node);
                sensorAPDS9306_065Dialog->updateSensor(sensorAPDS9306);
                sensorAPDS9306->setInterfaceID((quint8)index.row());
                model->setData(index, QVariant::fromValue(sensorAPDS9306), Qt::EditRole);

                configMs = sensorAPDS9306->getConfigurationMessages();

                for(const GMessage &m : configMs){
                    sendSerialMessage(m);
                    qInfo() << "Send APDS9306 065 config" << m.toString();
                    QThread::msleep(10);
                }
            default:
                sensor = nullptr;
                break;
            }
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
    if (index.isValid() && !index.parent().isValid())
    {
        nodeCurrent = true;
    }

    // sensor is selected -> add sensor as new row
    if (index.isValid() && index.parent().isValid())
    {
        sensorCurrent = true;
    }

    QMenu m(tr("Context menu"), this);

    QAction mDelete("Delete", this);
    connect(&mDelete, &QAction::triggered, this, &GloxiniaConfigurator::removeItems);
    m.addAction(&mDelete);

    // QAction mAddNode("Add node", this);
    // connect(&mAddNode, &QAction::triggered, this, &GloxiniaConfigurator::addNode);
    // m.addAction(&mAddNode);

    QAction mAddSensor("Add sensor", this);
    connect(&mAddSensor, &QAction::triggered, this, &GloxiniaConfigurator::addSensor);
    m.addAction(&mAddSensor);

    QAction mEditNode("Edit node", this);
    connect(&mEditNode, &QAction::triggered, this, &GloxiniaConfigurator::editNode);
    m.addAction(&mEditNode);

    QAction mEditSensor("Edit sensor", this);
    connect(&mEditSensor, &QAction::triggered, this, &GloxiniaConfigurator::editSensor);
    m.addAction(&mEditSensor);

    if (sensorCurrent)
    {
        // mAddNode.setEnabled(false);
        mEditNode.setEnabled(false);
    }

    if (nodeCurrent)
    {
        mEditSensor.setEnabled(false);
    }

    m.exec(ui->systemOverview->mapToGlobal(pos));
}

GCSensor *GloxiniaConfigurator::selectSensor(void)
{
    QStringList items;
    // items << "-- select sensor--";
    items << "SHT35";
    items << "APDS9306";

    bool ok;
    GCSensor *sensor = nullptr;

    QString item = QInputDialog::getItem(this, tr("Select sensor type"), tr("Sensor:"), items, 0, false, &ok);
    int sensorIndex = items.indexOf(item);
    if (ok && !item.isEmpty())
    {
        switch (sensorIndex)
        {
        case 0:
            sensorSHT35Dialog->setWindowModality(Qt::ApplicationModal);
            sensorSHT35Dialog->exec();
            // sensor = configureSHT35Dialog->getSensor();
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
    // removeRowAction->setEnabled(hasSelection);
    // removeColumnAction->setEnabled(hasSelection);

    const bool hasCurrent = ui->systemOverview->selectionModel()->currentIndex().isValid();
    // addNode->setEnabled(hasCurrent);
    // insertColumnAction->setEnabled(hasCurrent);

    if (hasCurrent)
    {
        ui->systemOverview->closePersistentEditor(ui->systemOverview->selectionModel()->currentIndex());

        const int row = ui->systemOverview->selectionModel()->currentIndex().row();
        const int column = ui->systemOverview->selectionModel()->currentIndex().column();
        if (ui->systemOverview->selectionModel()->currentIndex().parent().isValid())
            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        else
            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
    }
}
