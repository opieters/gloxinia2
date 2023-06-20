#ifndef GLOXINIACONFIGURATOR_H
#define GLOXINIACONFIGURATOR_H

#include "updatedialog.h"
#include <QMainWindow>
#include <QSerialPort>
#include <QStringListModel>
#include <QLabel>
#include <QChart>
#include <array>
#include <gcnode.h>
#include <settingsdialog.h>
#include <sensordialog.h>
#include <sensormeasurementdialog.h>
#include <globalmeasurementpolicydialog.h>
#include <sensorapds9306_065dialog.h>
#include <sensorsht35dialog.h>
#include <sensoradc12dialog.h>
#include <sensoradc16dialog.h>
#include <gcsystem.h>
#include <treemodel.h>
#include <nodedialog.h>
#include <gmessage.h>
#include <QTimer>
#include <measurementsettingsdialog.h>
#include <newprojectdialog.h>
#include <QChartView>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QHBoxLayout>
#include <gdevicecommunication.h>
#include <sensorliadialog.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class GloxiniaConfigurator;
}
QT_END_NAMESPACE

class GloxiniaConfigurator : public QMainWindow
{
    Q_OBJECT

public:
    GloxiniaConfigurator(QWidget *parent = nullptr);
    ~GloxiniaConfigurator();

    static constexpr unsigned int maxRecentFiles = 5;


    void setProject(GCProjectSettings& settings);

    void saveProject(void);
    void readSettings(void);
    void updateRecentProjects(void);
    void writeSettings(void);
    void newProject(void);
    void openProject(void);
    void clearAll(void);
    void selectDataFile(void);
    void exit(void);
    bool handleBootMessageStatus(const GMessage &m);

    enum SerialReadoutState
    {
        FindStartByte,
        ReadAddress,
        ReadCommand,
        ReadRequest,
        ReadInterfaceID,
        ReadSensorId,
        ReadLength,
        ReadData,
        DetectStopByte
    };

    friend class UpdateDialog;

private slots:
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void setSerialPort();

    void updatePreferences(void);
    void resetSystem(void);
    void updateDevice(void);

    void runDiscovery(void);
    // void addNode(void);
    void editNode(void);
    bool removeNode(const QModelIndex &index);
    void startMeasuring(void);
    void stopMeasuring(void);
    void editMeasurementSettings(void);

    void addSensor(void);
    void editSensor(void);
    bool removeSensor(const QModelIndex &index);
    void addToPlot(void);
    void removeFromPlot(void);

    void autoScaleChart(void);

    void removeItems();

    void showContextMenu(const QPoint &pos);

    void connectToDevice(void);
    // void setSensor();
    // void writeData(const QByteArray &data);
    // void handleError(QSerialPort::SerialPortError error);

private:
    /*
     * UI main object
     */
    Ui::GloxiniaConfigurator *ui;

    /*
     * Main UI elements
     */

    QHBoxLayout *mainLayout;
    // horizontal layout
    QSplitter* splitter, *splitter2;
    QListView* messageView;
    QTreeView* systemOverview;

    /*
     * status bar information
     */
    QLabel *status;

    QList<QAction*> recentFileActions;
    QAction* recentFileSeparatorAction;

    /*
     * This widget is used to plot incoming sensor data
     */
    QChart* chart;
    QChartView* chartView;
    QList<GCSensor::VariableType> yAxisTypes;
    QList<QValueAxis*> yAxes;
    QDateTimeAxis* xAxis;
    void autoScaleSeries(QXYSeries* series);
    QList<QMetaObject::Connection> seriesConnections;
    QLineSeries *dummySeries;

    /*
     * This model stores the system configuration (i.e. which nodes and sensors are connected and their respective numbers).
     */
    TreeModel *treeModel;

    /*
     * This model stores the message log (i.e. messages received over the serial connection and UI logging messages)
     */
    QStringListModel *messageModel;

    /*
     * Serial port used for communication with the system
     */
    QSerialPort *serial;

    /*
     * Stores a list of actions for the UI, each representing an option in the serial port selection list
     */
    QList<QAction *> serialPortActionList = QList<QAction *>();

    std::array<char, 0xff> rawDataBuffer;
    qint32 read_index = 0;
    qint32 write_index = 0;

    /*
     * Internal functions
     */
    void showStatusMessage(const QString &message);
    void updateSerialPortList(void);
    void updateActions();
    GCSensor *selectSensor(void);

    void updateUI(void);

    // message handling
    void processIncomingGMessage(const GMessage &m);
    void processCANDiscoveryMessage(const GMessage &m);
    void processTextMessage(const GMessage &m);
    void processNodeInfoMessage(const GMessage &m);
    void processSensorData(const GMessage& m);
    void processSensorStatus(const GMessage& m);
    void processSensorConfig(const GMessage& m);
    void processBootReadVersion(const GMessage&m);
    void processBootReadFlash(const GMessage&m);
    void processBootWriteFlash(const GMessage&m);
    void processBootEraseFlash(const GMessage&m);
    void processBootCalcChecksum(const GMessage&m);
    void processBootResetDevice(const GMessage&m);
    void processBootSelfVerify(const GMessage&m);
    void processBootGetMemoryAddressRangeCommand(const GMessage&m);
    void processBootReady(const GMessage&m);
    void processConfigDoneStartReadout(const GMessage&m);
    void processConfigDoneFinishedReadout(const GMessage&m);

    /*
     * Dialog screen to edit the system settings
     */
    SettingsDialog *systemSettings = nullptr;
    MeasurementSettingsDialog *measurementSettings = nullptr;
    GCProjectSettings settings;

    /*
     * Node configuration screen
     */
    NodeDicioDialog *nodeDicioDialog = nullptr;

    /*
     * General sensor type selection screen
     */
    SensorDialog *sensorSettings = nullptr;


    UpdateDialog* updateDialog = nullptr;


    NewProjectDialog* newProjectDialog;


    /*
     * Configuration screens for sensors
     */
    GlobalMeasurementPolicyDialog* globalMeasurementPolicyDialog;
    SensorAPDS9306_065Dialog *sensorAPDS9306_065Dialog;
    SensorSHT35Dialog *sensorSHT35Dialog;
    SensorADC12Dialog *sensorADC12Dialog;
    SensorADC16Dialog *sensorADC16Dialog;
    SensorLIADialog *sensorLIADialog;

    /*
     * The file stores the sensor data
     */
    QFile *dataFile = nullptr;
    QDataStream *dataStream = nullptr;

    /*
     * Timer for auto discovery
     */
    QTimer *discoveryTimer;

    void addSHT35Sensor(void);



protected:
    void closeEvent(QCloseEvent *event) override;

    GDeviceCommunication* devCom = nullptr;
};

#endif // GLOXINIACONFIGURATOR_H
