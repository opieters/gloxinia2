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
#include <QThread>
#include <QSettings>
#include <QStandardPaths>
#include <cfloat>
#include <QtGlobal>
#include <QRandomGenerator>

GloxiniaConfigurator::GloxiniaConfigurator(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::GloxiniaConfigurator),
      serial(new QSerialPort(this)),
      status(new QLabel),
      systemSettings(new SettingsDialog),
      sensorSettings(new SensorDialog),
      globalMeasurementPolicyDialog(new GlobalMeasurementPolicyDialog),
      sensorAnalogueDialog(new SensorAnalogueDialog),
      sensorAPDS9306_065Dialog(new SensorAPDS9306_065Dialog),
      sensorSHT35Dialog(new SensorSHT35Dialog),
      nodeDicioDialog(new NodeDicioDialog),
      chart(new QChart),
      messageModel(new QStringListModel),
      measurementSettings(new MeasurementSettingsDialog),
      newProjectDialog(new NewProjectDialog)
{
    ui->setupUi(this);
    ui->statusbar->addWidget(status);

    this->treeModel = new TreeModel(this);

    // create recent files actions
    ui->menuFile->insertSeparator(ui->actionExit);
    QAction* pAction = ui->actionSave, *cAction;
    for(int i = 0; i < maxRecentFiles; i++)
    {
        cAction = new QAction(this);
        recentFileActions.append(cAction);
        ui->menuFile->insertAction(pAction, cAction);
    }
    recentFileSeparatorAction = ui->menuFile->insertSeparator(recentFileActions[0]);

    // serial data readout trigger
    connect(serial, &QSerialPort::readyRead, this, &GloxiniaConfigurator::readData);

    // connect File menu to functions
    connect(ui->actionSave, &QAction::triggered, this, &GloxiniaConfigurator::saveProject);
    connect(ui->actionOpen, &QAction::triggered, this, &GloxiniaConfigurator::openProject);
    connect(ui->actionNew, &QAction::triggered, this, &GloxiniaConfigurator::newProject);
    connect(ui->actionExit, &QAction::triggered, this, &GloxiniaConfigurator::exit);

    // connect Edit menu to functions
    connect(ui->actionEditNode, &QAction::triggered, this, &GloxiniaConfigurator::editNode);
    connect(ui->actionEditSensor, &QAction::triggered, this, &GloxiniaConfigurator::editSensor);
    connect(ui->actionDelete, &QAction::triggered, this, &GloxiniaConfigurator::removeItems);
    connect(ui->actionPreferences, &QAction::triggered, this, &GloxiniaConfigurator::preferencesMenu);
    connect(ui->actionRunDiscovery, &QAction::triggered, this, &GloxiniaConfigurator::runDiscovery);
    connect(ui->actionStartMeasuring, &QAction::triggered, this, &GloxiniaConfigurator::startMeasuring);
    connect(ui->actionStopMeasuring, &QAction::triggered, this, &GloxiniaConfigurator::stopMeasuring);
    connect(ui->actionMeasurementSettings, &QAction::triggered, this, &GloxiniaConfigurator::editMeasurementSettings);

    // connect System menu to functions
    connect(ui->actionConnect, &QAction::triggered, this, &GloxiniaConfigurator::connectToDevice);
    // connect(ui->actionUpdate, &QAction::triggered, this, &GloxiniaConfigurator::);
    connect(ui->actionDisconnect, &QAction::triggered, this, &GloxiniaConfigurator::closeSerialPort);
    connect(ui->actionPreferences, &QAction::triggered, this, &GloxiniaConfigurator::updatePreferences);
    connect(ui->actionReset, &QAction::triggered, this, &GloxiniaConfigurator::resetSystem);


    // set data models
    ui->messageView->setModel(this->messageModel);
    ui->systemOverview->setModel(this->treeModel);

    // set model properties
    ui->systemOverview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // ui->systemOverview->setModel(this->model);
    // ui->systemOverview->setColumnCount(1);
    ui->systemOverview->setHeaderHidden(true);
    ui->systemOverview->setContextMenuPolicy(Qt::CustomContextMenu);

    // right-click action in the system tree view
    connect(ui->systemOverview, &QTreeView::customContextMenuRequested, this, &GloxiniaConfigurator::showContextMenu);

    // connect sample period settings dialogs
    sensorSHT35Dialog->setPeriodDialog(globalMeasurementPolicyDialog);
    sensorAPDS9306_065Dialog->setPeriodDialog(globalMeasurementPolicyDialog);

    // add plot window to UI
    dummySeries = new QLineSeries();
    chart->addSeries(dummySeries);
    //chart->legend()->hide();
    //chart->setTitle("Analog readout [a.u.]");

    xAxis = new QDateTimeAxis;
    xAxis->setTickCount(10);
    xAxis->setFormat("hh:mm:ss");
    xAxis->setTitleText("time");
    chart->addAxis(xAxis, Qt::AlignBottom);
    dummySeries->attachAxis(xAxis);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // auto update when points are removed/added
    //connect(series, &QXYSeries::pointAdded, this, &GloxiniaConfigurator::autoScaleChart);
    //seriesConnections.insert(0, connect(dummySeries, &QXYSeries::pointAdded, this, [this]{autoScaleSeries(dummySeries); }));

    ui->vLayout->addWidget(chartView);

    // timer use to send discovery messages
    discoveryTimer = new QTimer();
    connect(discoveryTimer, &QTimer::timeout, this, &GloxiniaConfigurator::runDiscovery);

    // Load application configuration and set default values
    readSettings();
    settings.success = false;

    updateUI();

    // application settings
    applicationSettings.messageBufferSize = 1000;
    applicationSettings.plotBufferWindow = 250;

    // update status
    //messageModel->insertRow(0);
    //QModelIndex mIndex = messageModel->index(0, 0);
    //messageModel->setData(mIndex, "Application started.");
    showStatusMessage("Application started.");
}

GloxiniaConfigurator::~GloxiniaConfigurator()
{
    delete ui;
}

void GloxiniaConfigurator::autoScaleSeries(QXYSeries* series)
{
    // TODO: update scales!!
    auto axes = series->attachedAxes();
    for(QAbstractAxis* i : axes)
    {
        QValueAxis* yAx = dynamic_cast<QValueAxis*>(i);

        if(yAx == nullptr)
            continue;

        qreal ymax = yAx->max();
        qreal ymin = yAx->min();
        QPointF p = series->at(series->count()-1);

        yAx->setRange(std::min(ymin,p.y() ), std::max(ymax, p.y()));
    }

    // scale x-axis
    qint64 xmax = xAxis->max().toMSecsSinceEpoch();
    qint64 vlast = (qint64) series->at(series->count() - 1).x();
    xmax = std::max(xmax,vlast);

    xAxis->setRange(QDateTime::fromMSecsSinceEpoch(xmax - applicationSettings.plotBufferWindow * 1000), QDateTime::fromMSecsSinceEpoch(xmax));

}

void GloxiniaConfigurator::resetSystem(void)
{
    GMessage reset(GMessage::Code::NODE_RESET, GMessage::ComputerAddress, GMessage::NoSensorID, true);

    sendSerialMessage(reset);

    // disable start option, enable stop option
    ui->actionStartMeasuring->setEnabled(true);
    ui->actionStopMeasuring->setEnabled(false);

    // TODO: clear treemodel

}

void GloxiniaConfigurator::updatePreferences(void)
{
    systemSettings->setApplicationSettings(applicationSettings);
    systemSettings->setWindowModality(Qt::ApplicationModal);
    int result = systemSettings->exec();
    if(result == QDialog::Rejected){
        return;
    }

    applicationSettings = systemSettings->getApplicationSettings();

    // update buffer lengths of sensors
    // loop over all nodes and sensors
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

            sensor->setMaxPlotSize(((unsigned int) applicationSettings.plotBufferWindow) * 10 / (sensor->getMeasurementPeriod() + 1));
        }
    }
}

void GloxiniaConfigurator::autoScaleChart(void)
{
    auto series = chart->series();
    double xMin = DBL_MAX, xMax = 0, yMin, yMax;

    for(QAbstractSeries* i : series)
    {
        auto j = dynamic_cast<QXYSeries*>(i);
        xMin = std::min(xMin, j->at(0).x());
        xMax = std::max(xMax, j->at(j->count()-1).x());
    }
    //chart->ax
}

void GloxiniaConfigurator::readSettings(void)
{
    QSettings settings;

    settings.beginGroup("main");

    const auto geometry = settings.value("geometry", QByteArray()).toByteArray();

    if(!geometry.isEmpty())
        restoreGeometry(geometry);

    this->settings.projectDir = settings.value("projectDir", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    this->settings.runDiscovery = settings.value("runDiscovery", true).toBool();
    updateRecentProjects();


    settings.endGroup();
}

void GloxiniaConfigurator::updateRecentProjects(void)
{
    QSettings settings;

    settings.beginGroup("main");
    const auto recentProjects = settings.value("recentProjets", QStringList()).toStringList();

    for(int i = 0; i < recentProjects.size(); i++)
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(recentProjects[i]).fileName());
        recentFileActions[i]->setText(text);
        recentFileActions[i]->setData(recentProjects[i]);
        recentFileActions[i]->setVisible(true);
    }
    for(int i = recentProjects.size(); i < maxRecentFiles; i++)
    {
        recentFileActions[i]->setVisible(false);
    }

    recentFileSeparatorAction->setVisible(recentProjects.size() > 0);

    settings.endGroup();
}

void GloxiniaConfigurator::writeSettings(void)
{
    QSettings settings;

    settings.beginGroup("main");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("projectDir", this->settings.projectDir);
    settings.setValue("runDiscovery", this->settings.runDiscovery);

    QStringList files = settings.value("recentProjects").toStringList();
    files.prepend(QDir::cleanPath(this->settings.projectDir + "/" + this->settings.projectName + ".gc"));
    if(files.size() > GloxiniaConfigurator::maxRecentFiles){
        files.removeLast();
    }
    settings.setValue("recentProjects", files);

    settings.endGroup();
}

void GloxiniaConfigurator::closeEvent(QCloseEvent *event)
{
    // TODO: pop-up
    //if (userReallyWantsToQuit()) {
        writeSettings();
        event->accept();
    //} else {
    //    event->ignore();
    //}
}

void GloxiniaConfigurator::exit(void)
{
    // TODO: check if config saved??

    close();
}

void GloxiniaConfigurator::setProject(NewProjectDialog::ProjectSettings& settings){
    this->settings = settings;
}

void GloxiniaConfigurator::updateUI(){


    if(settings.success){
        ui->actionSave->setEnabled(true);

        if(serial->isOpen()){
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
            ui->actionLoadDeviceConfig->setEnabled(true);
            ui->actionConfigureSerial->setEnabled(false);
            ui->actionEditNode->setEnabled(true);
            ui->actionEditSensor->setEnabled(true);
            ui->actionDelete->setEnabled(true);
            ui->actionStartMeasuring->setEnabled(true);
            ui->actionReset->setEnabled(true);
            ui->actionStopMeasuring->setEnabled(true);
            ui->actionMeasurementSettings->setEnabled(true);
            ui->actionRunDiscovery->setEnabled(true);
        } else {
            ui->actionConnect->setEnabled(true);
            ui->actionDisconnect->setEnabled(false);
            ui->actionConfigureSerial->setEnabled(true);
            ui->actionLoadDeviceConfig->setEnabled(false);
            ui->actionEditNode->setEnabled(false);
            ui->actionEditSensor->setEnabled(false);
            ui->actionDelete->setEnabled(false);
            ui->actionStartMeasuring->setEnabled(false);
            ui->actionReset->setEnabled(false);
            ui->actionStopMeasuring->setEnabled(false);
            ui->actionMeasurementSettings->setEnabled(true);
            ui->actionRunDiscovery->setEnabled(false);
        }
    } else {
        ui->actionSave->setDisabled(true);
        ui->actionConnect->setDisabled(true);
        ui->actionDisconnect->setEnabled(false);
        ui->actionConfigureSerial->setEnabled(false);
        ui->actionLoadDeviceConfig->setEnabled(false);
        ui->actionEditNode->setEnabled(false);
        ui->actionEditSensor->setEnabled(false);
        ui->actionDelete->setEnabled(false);
        ui->actionStartMeasuring->setEnabled(false);
        ui->actionReset->setEnabled(false);
        ui->actionStopMeasuring->setEnabled(false);
        ui->actionMeasurementSettings->setEnabled(false);
        ui->actionRunDiscovery->setEnabled(false);
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
    // cancel discovery
    discoveryTimer->stop();

    // create directory for sensor data
    QDateTime now = QDateTime::currentDateTime();
    QString sensorDataName = settings.projectDir + "/" + now.toString("yyyy-MM-dd_hh-mm-ss");
    QDir sensorDataDir(sensorDataName);
    if(!sensorDataDir.mkpath(sensorDataName)){
        QMessageBox msgBox;
        msgBox.setText("Unable to create sensor data directory " + sensorDataName + ".");
        msgBox.exec();
        return;
    }

    // set path prefix for sensor data
    GCSensor::setSensorFileDir(sensorDataName);

    // disable start option, enable stop option
    ui->actionStartMeasuring->setEnabled(false);
    ui->actionStopMeasuring->setEnabled(true);

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
            }

        }
    }

    GMessage mStart = GMessage(GMessage::Code::SENSOR_START, GMessage::ComputerAddress, GMessage::NoSensorID, true);
    sendSerialMessage(mStart);
}
void GloxiniaConfigurator::stopMeasuring(void)
{
    GMessage mStop = GMessage(GMessage::Code::SENSOR_STOP, GMessage::ComputerAddress, GMessage::NoSensorID, true);
    sendSerialMessage(mStop);

    // enable start option, disable stop option
    ui->actionStartMeasuring->setEnabled(true);
    ui->actionStopMeasuring->setEnabled(false);

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

            sensor->stopMeasurement();
        }

    }
}

void GloxiniaConfigurator::editMeasurementSettings(void)
{
    nodeDicioDialog->setWindowModality(Qt::ApplicationModal);
    measurementSettings->exec();
    measurementSettings->getSettings();
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

        QVariant data = model->data(index, Qt::EditRole);
        GCSensorSHT35 *sensorSHT35;
        GCSensorAPDS9306 *sensorAPDS9306;

        sensorSHT35 = data.value<GCSensorSHT35 *>();
        sensorAPDS9306 = data.value<GCSensorAPDS9306 *>();

        if(sensorSHT35 != nullptr)
            delete sensorSHT35;
        if(sensorAPDS9306 != nullptr)
            delete sensorAPDS9306;

        return model->setData(index, QVariant());
        //return model->removeRow(index.row(), index.parent());
    }

    return false;
}

void GloxiniaConfigurator::removeItems()
{
    QItemSelectionModel *selectionModel = ui->systemOverview->selectionModel();
    //QAbstractItemModel *model = ui->systemOverview->model();

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

            configMs = sensorSHT35->getConfigurationMessages();

            for(const GMessage &m : configMs){
                sendSerialMessage(m);
                qInfo() << "Send SHT35 config" << m.toString();

            }

            sensorSHT35->setMaxPlotSize(((unsigned int) applicationSettings.plotBufferWindow) * 10 / (sensorSHT35->getMeasurementPeriod() + 1));


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

            configMs = sensorAPDS9306->getConfigurationMessages();

            for(const GMessage &m : configMs){
                sendSerialMessage(m);
                qInfo() << "Send APDS9306 065 config" << m.toString();

            }

            sensorAPDS9306->setMaxPlotSize(((unsigned int) applicationSettings.plotBufferWindow) * 10 / (sensorAPDS9306->getMeasurementPeriod() + 1));

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
                sensorSHT35 = new GCSensorSHT35(node, (quint8)index.row());
                sensorSHT35Dialog->updateSensor(sensorSHT35);
                model->setData(index, QVariant::fromValue(sensorSHT35), Qt::EditRole);

                // update hardware configuration
                configMs = sensorSHT35->getConfigurationMessages();

                for(const GMessage &m : configMs){
                    sendSerialMessage(m);
                    qInfo() << "Send SHT35 config" << m.toString();
                }
                sensorSHT35->setMaxPlotSize(((unsigned int) applicationSettings.plotBufferWindow) * 10 / (sensorSHT35->getMeasurementPeriod() + 1));

                break;
            case 2:
                sensorAPDS9306_065Dialog->setWindowModality(Qt::ApplicationModal);
                result = sensorAPDS9306_065Dialog->exec();
                if (result == QDialog::Rejected)
                {
                    return;
                }
                sensorAPDS9306 = new GCSensorAPDS9306(node, (quint8)index.row());
                sensorAPDS9306_065Dialog->updateSensor(sensorAPDS9306);
                model->setData(index, QVariant::fromValue(sensorAPDS9306), Qt::EditRole);

                configMs = sensorAPDS9306->getConfigurationMessages();

                for(const GMessage &m : configMs){
                    sendSerialMessage(m);
                    qInfo() << "Send APDS9306 065 config" << m.toString();

                }
                sensorAPDS9306->setMaxPlotSize(((unsigned int) applicationSettings.plotBufferWindow) * 10 / (sensorAPDS9306->getMeasurementPeriod() + 1));

                break;
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

void GloxiniaConfigurator::addToPlot(void)
{
    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();
    GCSensor *sensor;
    int result;
    QXYSeries* series = nullptr;
    GCSensor::VariableType seriesType;

    // sensor is selected -> run menu
    if (index.isValid() && index.parent().isValid())
    {
        QVariant data = model->data(index, Qt::EditRole);
        sensor = GCSensor::fromQVariant(data);
        if(sensor == nullptr)
            return;
        auto plotSeries = sensor->getPlotSeries();
        if(plotSeries.size() > 1){
            QStringList labels;
            for(int i = 0; i < plotSeries.size(); i++)
            {
                labels.append(plotSeries[i]->name());
            }
            // todo: load menu to select plots
            bool ok;
            QString text = QInputDialog::getItem(this, tr("Select variable to plot from ") + sensor->getLabel(),
                                                 tr("Variables:"), labels, 0, false, &ok);
            if (ok && !text.isEmpty()){
                int index = labels.indexOf(text);
                if((index < 0) || (index > plotSeries.size()))
                    return;
                series = plotSeries[index];
                seriesType = sensor->getVariableTypes().at(index);
            }
        } else {
            series = plotSeries[0];
            seriesType = sensor->getVariableTypes().at(0);
        }
    }

    // add item if not yet in list
    if(series != nullptr){
        QList<QAbstractSeries*> cSeries = chart->series();

        if(cSeries.indexOf(series) < 0){
            // check if axes exists already

            seriesConnections.append(connect(series, &QXYSeries::pointAdded, this, [this,series]{autoScaleSeries(series); }));

            if(yAxisTypes.contains(seriesType))
            {
                int index = yAxisTypes.indexOf(seriesType);

                chart->addSeries(series);
                series->attachAxis(yAxes.at(index));
                series->attachAxis(xAxis);

            } else {
                QValueAxis *axisY = new QValueAxis;

                axisY->setLabelFormat("%.1f");
                axisY->setTitleText(GCSensor::VariableTypeToString(seriesType));
                chart->addAxis(axisY, Qt::AlignLeft);

                yAxes.append(axisY);
                yAxisTypes.append(seriesType);

                chart->addSeries(series);
                series->attachAxis(axisY);
                series->attachAxis(xAxis);

            }
            chartView->update();
        }
    }

    // if there is at least one plot, remove dummy plot
    if((chart->series().count() > 1) && (chart->series().contains(dummySeries)))
    {
        chart->removeSeries(dummySeries);
    }

}
void GloxiniaConfigurator::removeFromPlot(void)
{
    // request plot to remove
    QStringList labels;
    for(auto i : chart->series())
    {
        labels.append(i->name());
    }

    if(labels.isEmpty())
        return;

    // todo: load menu to select plots
    bool ok;
    QString text = QInputDialog::getItem(this, tr("Select plot to remove"),
                                         tr("Plots:"), labels, 0, false, &ok);
    if(!ok)
        return;

    int index = labels.indexOf(text);
    // TODO: add dummy plot if it is the last one left, keep the last y axis alive also
    QObject::disconnect(seriesConnections.at(index));
    seriesConnections.removeAt(index);

    chart->removeSeries(chart->series().at(index));

    // there are not more series, so we have to re-add the dummySeries
    if(seriesConnections.isEmpty()){
        chart->addSeries(dummySeries);
    }
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

    //QAction mAddSensor("Add sensor", this);
    //connect(&mAddSensor, &QAction::triggered, this, &GloxiniaConfigurator::addSensor);
    //m.addAction(&mAddSensor);

    QAction mEditNode("Edit node", this);
    connect(&mEditNode, &QAction::triggered, this, &GloxiniaConfigurator::editNode);
    m.addAction(&mEditNode);

    QAction mEditSensor("Edit sensor", this);
    connect(&mEditSensor, &QAction::triggered, this, &GloxiniaConfigurator::editSensor);
    m.addAction(&mEditSensor);

    QAction mAddToPlot("Add to plot", this);
    connect(&mAddToPlot, &QAction::triggered, this, &GloxiniaConfigurator::addToPlot);
    m.addAction(&mAddToPlot);

    QAction mRemoveFromPlot("Remove from plot", this);
    connect(&mRemoveFromPlot, &QAction::triggered, this, &GloxiniaConfigurator::removeFromPlot);
    m.addAction(&mRemoveFromPlot);

    mDelete.setEnabled(false);
    mEditNode.setEnabled(false);
    mEditSensor.setEnabled(false);
    mAddToPlot.setEnabled(false);
    mRemoveFromPlot.setEnabled(false);

    if (sensorCurrent)
    {
        mDelete.setEnabled(true);
        mEditSensor.setEnabled(true);
        // todo: check if data displayed in plot
        mAddToPlot.setEnabled(true);
        if(!chart->series().contains(dummySeries))
            mRemoveFromPlot.setEnabled(true);
    }

    if (nodeCurrent)
    {
        mDelete.setEnabled(true);
        mEditNode.setEnabled(true);
        if(!chart->series().contains(dummySeries))
            mRemoveFromPlot.setEnabled(true);
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
