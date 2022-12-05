#ifndef GLOXINIACONFIGURATOR_H
#define GLOXINIACONFIGURATOR_H

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
#include <sensoranaloguedialog.h>
#include <sensorapds9306_065dialog.h>
#include <sensorsht35dialog.h>
#include <gcsystem.h>
#include <treemodel.h>
#include <nodedialog.h>
#include <gmessage.h>
#include <QTimer>

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

    void saveToFile(void);
    void loadFromFile(void);
    void clearAll(void);
    void selectDataFile(void);

    enum SerialReadoutState
    {
        FindStartByte,
        ReadIdH,
        ReadIdL,
        ReadCommand,
        ReadRequest,
        ReadSensorIdH,
        ReadSensorIdL,
        ReadLength,
        ReadData,
        DetectStopByte
    };

private slots:
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void setSerialPort();

    void runDiscovery(void);
    // void addNode(void);
    void editNode(void);
    bool removeNode(const QModelIndex &index);
    void startMeasuring(void);
    void stopMeasuring(void);

    void addSensor(void);
    void editSensor(void);
    bool removeSensor(const QModelIndex &index);

    void removeItems();

    void showContextMenu(const QPoint &pos);

    void preferencesMenu(void);
    // void setSensor();
    // void writeData(const QByteArray &data);
    // void handleError(QSerialPort::SerialPortError error);

private:
    /*
     * UI main object
     */
    Ui::GloxiniaConfigurator *ui;

    /*
     * status bar information
     */
    QLabel *status;

    /*
     * This widget is used to plot incoming sensor data
     */
    QChart *chart;

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
     * Menu selected serial port that will be used for communication
     */
    QString serialPortName;

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

    // message handling
    void processIncomingGMessage(const GMessage &m);
    void processCANDiscoveryMessage(const GMessage &m);
    void processTextMessage(const GMessage &m);
    void processNodeInfoMessage(const GMessage &m);
    void processSensorData(const GMessage& m);
    void processSensorStatus(const GMessage& m);

    void sendSerialMessage(const GMessage &m);

    /*
     * Dialog screen to edit the system settings
     */
    SettingsDialog *systemSettings = nullptr;

    /*
     * Node configuration screen
     */
    NodeDicioDialog *nodeDicioDialog = nullptr;

    /*
     * General sensor type selection screen
     */
    SensorDialog *sensorSettings = nullptr;
    /*
     * Configuration screens for sensors
     */
    SensorMeasurementDialog* sensorMeasurementDialog;
    GlobalMeasurementPolicyDialog* globalMeasurementPolicyDialog;
    SensorAnalogueDialog *sensorAnalogueDialog;
    SensorAPDS9306_065Dialog *sensorAPDS9306_065Dialog;
    SensorSHT35Dialog *sensorSHT35Dialog;

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
};

#endif // GLOXINIACONFIGURATOR_H
