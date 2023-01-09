#include "gcsensor.h"
#include <algorithm>
#include <QList>
#include <QDateTime>
#include <QDir>

QString GCSensor::sensorFileDir = "";

GCSensor::GCSensor(GCNode* node, quint8 id) : node(node), interfaceID(id)
{
}

GCSensor::~GCSensor() {
    if(file != nullptr){
        file->flush();
        delete file;
    }
}


quint8 GCSensor::getInterfaceID(void)
{
    return interfaceID;
}

QString GCSensor::getLabel(void) const
{
    return label;
}

void GCSensor::setLabel(const QString label)
{
    this->label = label;
}

quint16 GCSensor::getMeasurementPeriod() const
{
    return measurementPeriod;
}
void GCSensor::setMeasurementPeriod(quint16 period)
{
    measurementPeriod = period;
}

bool GCSensor::getUseGlobalPeriodFlag(void) const
{
    return useGlobalPeriod;
}

void GCSensor::setUseGlobalPeriodFlag(bool flag)
{
    useGlobalPeriod = flag;
}

bool GCSensor::startMeasurement(void)
{
    if(status != IDLE){
        return false;
    }

    QString fullPath = QDir::cleanPath(GCSensor::sensorFileDir + "/" + this->filePath);
    file = new QFile(fullPath);
    if(!file->open(QIODevice::WriteOnly)){
        // TODO: pop-up error that file could not be openened for saving
        delete file;
        file = nullptr;
        status = GCSensorStatus::ERROR;
        qDebug() << "Unable to open file" << fullPath;
        return false;
    }

    qDebug() << "File open success!";

    printHeader();
    status = GCSensorStatus::ACTIVE;
    return true;
}

GMessage GCSensor::getStartMessage(void)
{
    std::vector<quint8> data = {GCSensor::GCSensorStatus::ACTIVE};
    return GMessage(GMessage::Code::SENSOR_STATUS, this->node->getID(), this->getInterfaceID(), false, data);
}
GMessage GCSensor::getStopMessage(void)
{
    std::vector<quint8> data = {GCSensor::GCSensorStatus::STOPPED};
    return GMessage(GMessage::Code::SENSOR_STATUS, this->node->getID(), this->getInterfaceID(), false, data);
}

void GCSensor::stopMeasurement(void)
{
    if(file == nullptr)
        return;

    file->close();
    delete file;
    file = nullptr;
    status = GCSensorStatus::STOPPED;
}

void GCSensor::setStatus(GCSensorStatus s)
{
    status = s;
}

GCSensor* GCSensor::fromQVariant(const QVariant data)
{
    GCSensorSHT35 *sensorSHT35 = data.value<GCSensorSHT35 *>();
    GCSensorAPDS9306 *sensorAPDS9306 = data.value<GCSensorAPDS9306 *>();

    if(sensorSHT35 != nullptr)
        return sensorSHT35;
    if(sensorAPDS9306 != nullptr)
        return sensorAPDS9306;
    return nullptr;
}

QString GCSensor::statusToString(GCSensorStatus s)
{
    switch(s){
    case INACTIVE:
        return "inactive";
    case IDLE:
        return "idle";
    case ACTIVE:
        return "active";
    case RUNNING:
        return "running";
    case STOPPED:
        return "stopped";
    case ERROR:
        return "error";
    }

    return "";
}

QDebug operator<<(QDebug dbg, const GCSensor &)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Sensor";
    return dbg;
}

/*QString GCSensor::toString(void) const
{
    return "Empty sensor slot [" + QString::number(interfaceID) + "]";
}*/

QDataStream &operator<<(QDataStream &out, const GCSensor &myObj)
{
    return out << "SENSOR";
}

void GCSensor::setSensorFileDir(const QString dir)
{
    GCSensor::sensorFileDir = dir;
}

QString GCSensor::getSensorFileDir(void)
{
    return GCSensor::sensorFileDir;
}

GCSensorI2C::GCSensorI2C(GCNode* node, quint8 id, quint8 i2cAddress) : GCSensor(node, id), i2cAddress(i2cAddress)
{
}

GCSensorI2C::~GCSensorI2C() {}

bool GCSensorI2C::setI2CAddress(const quint8 a)
{
    i2cAddress = a;
    return true;
}

int GCSensorI2C::i2cAddressToInt(quint8 a)
{
    return (int)a;
}

const quint8 GCSensorI2C::getI2CAddress(void)
{
    return i2cAddress;
}

GCSensorSHT35::GCSensorSHT35(GCNode* node, quint8 id, quint8 i2cAddress) : GCSensorI2C(node, id, i2cAddress)
{
    config.repeatability = 0;
    config.clockStretching = 0;
    config.rate = 0;
    config.periodicity = 0;

    if(node == nullptr){
        filePath = "node-0-" + QString::number(id) + "-SHT35.csv";
    } else {
        filePath = "node-" + QString::number(node->getID()) + "-" + QString::number(id) + "-SHT35.csv";
    }
    filePath = QDir::cleanPath(filePath);
}

GCSensorSHT35::~GCSensorSHT35() {}

bool GCSensorSHT35::setI2CAddress(const quint8 a)
{
    i2cAddress = a;
    if ((i2cAddress != I2CAddressA) && (i2cAddress != I2CAddressB))
    {
        i2cAddress = I2CAddressA;
    }
    return a == i2cAddress;
}
bool GCSensorSHT35::setRepeatability(const quint8 r)
{
    config.repeatability = std::min(r, (quint8)2);
    return r == config.repeatability;
}
bool GCSensorSHT35::setClockStretching(const quint8 c)
{
    config.clockStretching = std::min(c, (quint8)1);
    return c == config.clockStretching;
}
bool GCSensorSHT35::setRate(const quint8 r)
{
    config.rate = std::min(r, (quint8)4);
    return r == config.rate;
}
bool GCSensorSHT35::setPeriodicity(const quint8 p)
{
    config.periodicity = std::min(p, (quint8)1);
    return p == config.periodicity;
}

const quint8 GCSensorSHT35::getRepeatability(void)
{
    return config.repeatability;
}
const quint8 GCSensorSHT35::getClockStretching(void)
{
    return config.clockStretching;
}
const quint8 GCSensorSHT35::getRate(void)
{
    return config.rate;
}
const quint8 GCSensorSHT35::getPeriodicity(void)
{
    return config.periodicity;
}

int GCSensorSHT35::i2cAddressToInt(const quint8 a)
{
    for (int i = 0; i < GCSensorSHT35::i2cAddresses.size(); i++)
    {
        if (i2cAddresses[i] == a)
        {
            return i;
        }
    }
    return -1;
}

QString GCSensorSHT35::toString(void) const
{
    QString dLabel;
    if (!label.isEmpty())
    {
        dLabel = label;
    }
    else
    {
        dLabel = "SHT35";
    }
    return "[" + QString::number(interfaceID) + "] " + dLabel + " (" + QString::number(i2cAddress) + ") - " + statusToString(status);
}

QString GCSensorSHT35::toConfigString(void) const
{
    QString nLabel = label;
    nLabel.replace(' ', "\\ ");
    return "S SHT35 " + QString::number(interfaceID) + " " + nLabel + " " + QString::number(i2cAddress) + " " + QString::number(config.repeatability) + " " + QString::number(config.clockStretching) + " " + QString::number(config.rate) + " " + QString::number(config.periodicity) + " ;";
}

bool GCSensorSHT35::fromConfigString(const QStringList &config)
{
    if (config.count() != 9)
        return false;

    if ((config[0] != "S") || (config[1] != "SHT35"))
        return false;

    bool success = true;
    GCSensorSHT35 s(nullptr, config[2].toInt());
    s.setLabel(config[3]);
    success = success && s.setI2CAddress(config[4].toInt());
    success = success && s.setRepeatability(config[5].toInt());
    success = success && s.setClockStretching(config[6].toInt());
    success = success && s.setRate(config[7].toInt());
    success = success && s.setPeriodicity(config[8].toInt());

    if (success)
    {
        //this->setInterfaceID(s.getInterfaceID());
        this->setLabel(s.getLabel());
        this->setI2CAddress(s.getI2CAddress());
        this->setRepeatability(s.getRepeatability());
        this->setClockStretching(s.getClockStretching());
        this->setRate(s.getRate());
        this->setPeriodicity(s.getPeriodicity());
    }

    return success;
}

QList<GMessage> GCSensorSHT35::getConfigurationMessages()
{
    QList<GMessage> mList;

    auto mData = std::vector<quint8>(4);
    mData[0] = (quint8)GCSensor::sensor_class::SHT35;
    mData[1] = GCSensorSHT35::Register::MEASUREMENT;
    mData[2] = (quint8) measurementPeriod >> 8;
    mData[3] = (quint8) measurementPeriod & 0xff;
    mList.append(GMessage(GMessage::Code::SENSOR_CONFIG, node->getID(), interfaceID, false, mData));

    mData = std::vector<quint8>(7);
    mData[0] = (quint8)GCSensor::sensor_class::SHT35;
    mData[1] = GCSensorSHT35::Register::CONFIG;
    mData[2] = i2cAddress;
    mData[3] = config.repeatability;
    mData[4] = config.clockStretching;
    mData[5] = config.rate;
    mData[6] = config.periodicity;
    mList.append(GMessage(GMessage::Code::SENSOR_CONFIG, node->getID(), interfaceID, false, mData));

    return mList;
}

void GCSensorSHT35::printHeader(void)
{
    file->write("# Intermediate storage file for SHT35\n");
    file->write("# SHT35 is a temperature and humidity sensor. The raw data is processed and stored in degree Celsius and percent. The printed format is sufficient for the accuracy reported in the datasheet. The data is stored in the `temp` (C) and `rh` (%) columns. Each value is timestamped.\n");
    file->write("# The sensor also produces a checksum to check for data integrity. A boolean value (0=error and 1=OK) is stored in the `checksum` column to report the status. Error correction is not possible using the checksum.");
    file->write("time; temp; rh; checksum\n");
    file->flush();
}
void GCSensorSHT35::saveData(std::vector<quint8>& data)
{
    QString formattedData;
    double temp, rh;
    uint8_t crc;
    bool checksum = true;


    QDateTime date = QDateTime::currentDateTime();
    formattedData.append(date.toString("dd.MM.yyyy hh:mm:ss"));
    formattedData.append("; ");
    if(data.size() == 6){
        temp = data[0] | (data[1] << 8);
        crc = 0xff;
        crc = calculateCrc(data[0], crc,crcPolynomial );
        crc = calculateCrc(data[1], crc,crcPolynomial );
        checksum = checksum & (crc == data[2]);
        formattedData.append(QString::number(temp, 'g', 4));
        formattedData.append("; ");

        rh = data[3] | (data[4] << 8);
        crc = calculateCrc(data[3], crc,crcPolynomial );
        crc = calculateCrc(data[4], crc,crcPolynomial );
        checksum = checksum & (crc == data[5]);
        checksum = true;
        formattedData.append(QString::number(temp, 'g', 4)); // print in scientific format with precision of 4
        formattedData.append("; ");
        if(checksum)
            formattedData.append(QString::number(1));
        else
            formattedData.append(QString::number(0));
    } else {
        formattedData.append("NaN; NaN; 0");
    }
    formattedData.append("\n");

    file->write(formattedData.toUtf8());
}
uint8_t GCSensorSHT35::calculateCrc(uint8_t b, uint8_t crc, uint8_t poly)
{
uint16_t i;
crc = crc ^ b;
for (i = 0; i < 8; i++)
{
    if ((crc & 0x80) == 0)
    {
        crc = crc << 1;
    }
    else
    {
        crc = (crc << 1) ^ poly;
    }
}
return crc;
}

GCSensorAPDS9306::GCSensorAPDS9306(GCNode* node, quint8 id, quint8 i2cAddress) : GCSensorI2C(node, id, i2cAddress)
{
    if(node == nullptr){
        filePath = "node-0-" + QString::number(id) + "-APDS9306_065.csv";
    } else {
        filePath = "node-" + QString::number(node->getID()) + "-" + QString::number(id) + "-APDS9306_065.csv";
    }
    filePath = QDir::cleanPath(filePath);
}

GCSensorAPDS9306::~GCSensorAPDS9306() {}

bool GCSensorAPDS9306::setI2CAddress(const quint8 a)
{
    i2cAddress = a;
    return true;
}

bool GCSensorAPDS9306::setAlsMeasurementRate(quint8 v)
{
    alsMeasurementRate = std::min(v, (quint8)0b110);
    return v == alsMeasurementRate;
}
bool GCSensorAPDS9306::setAlsResolution(quint8 v)
{
    alsResolution = std::min(v, (quint8)0b101);
    return v == alsResolution;
}
bool GCSensorAPDS9306::setAlsGain(quint8 v)
{
    alsGain = std::min(v, (quint8)0b100);
    return v == alsGain;
}
bool GCSensorAPDS9306::setAlsIVCount(quint8 v)
{
    alsIVCount = std::min(v, (quint8)0b111);
    return v == alsIVCount;
}
bool GCSensorAPDS9306::setAlsTHHigh(quint32 v)
{
    alsTHHigh = v;
    return true;
}
bool GCSensorAPDS9306::setAlsTHLow(quint32 v)
{
    alsTHLow = v;
    return true;
}

quint8 GCSensorAPDS9306::getAlsMeasurementRate(void)
{
    return alsMeasurementRate;
}
quint8 GCSensorAPDS9306::getAlsResolution(void)
{
    return alsResolution;
}
quint8 GCSensorAPDS9306::getAlsGain(void)
{
    return alsGain;
}
quint8 GCSensorAPDS9306::getAlsIVCount(void)
{
    return alsIVCount;
}
quint32 GCSensorAPDS9306::getAlsTHHigh(void)
{
    return alsTHHigh;
}
quint32 GCSensorAPDS9306::getAlsTHLow(void)
{
    return alsTHLow;
}

int GCSensorAPDS9306::i2cAddressToInt(quint8 a)
{
    return a;
}

QString GCSensorAPDS9306::toString(void) const
{
    QString dLabel;
    if (!label.isEmpty())
    {
        dLabel = label;
    }
    else
    {
        dLabel = "APDS9306 065";
    }
    return "[" + QString::number(interfaceID) + "] " + dLabel + " (" + QString::number(i2cAddress) + ") - " + statusToString(status);
}

QString GCSensorAPDS9306::toConfigString(void) const
{
    QString nLabel = label;
    nLabel.replace(' ', "\\ ");
    return "S APDS9306 " + QString::number(interfaceID) + " " + nLabel + " " + QString::number(i2cAddress) + " " + QString::number(alsMeasurementRate) + " " + QString::number(alsResolution) + " " + QString::number(alsGain) + " " + QString::number(alsIVCount) + " " + QString::number(alsTHHigh) + " " + QString::number(alsTHLow) + " " + ";";
}

bool GCSensorAPDS9306::fromConfigString(const QStringList &config)
{
    GCSensorAPDS9306 s(nullptr, 0);

    if (config.count() != 11)
        return false;

    if ((config[0] != "S") || (config[1] != "SHT35"))
        return false;

    bool success = true;
    //s.setInterfaceID(config[2].toInt());
    s.setLabel(config[3]);
    success = success && s.setI2CAddress(config[4].toInt());
    success = success && s.setAlsMeasurementRate(config[5].toInt());
    success = success && s.setAlsResolution(config[6].toInt());
    success = success && s.setAlsGain(config[7].toInt());
    success = success && s.setAlsIVCount(config[8].toInt());
    success = success && s.setAlsTHHigh(config[9].toInt());
    success = success && s.setAlsTHLow(config[10].toInt());

    if (success)
    {
        //this->setInterfaceID(s.getInterfaceID());
        this->setLabel(s.getLabel());
        this->setI2CAddress(s.getI2CAddress());
        this->setAlsMeasurementRate(s.getAlsMeasurementRate());
        this->setAlsResolution(s.getAlsResolution());
        this->setAlsGain(s.getAlsGain());
        this->setAlsIVCount(s.getAlsIVCount());
        this->setAlsTHHigh(s.getAlsTHHigh());
        this->setAlsTHLow(s.getAlsTHLow());
    }

    return success;
}

QList<GMessage> GCSensorAPDS9306::getConfigurationMessages()
{
    QList<GMessage> mList;

    // check if the sensor is linked to a node
    if(node == nullptr){
        return mList;
    }

    auto mData = std::vector<quint8>(4);
    mData[0] = (quint8)GCSensor::sensor_class::APDS9306_065;
    mData[1] = GCSensorAPDS9306::Register::MEASUREMENT;
    mData[2] = (quint8) measurementPeriod >> 8;
    mData[3] = (quint8) measurementPeriod & 0xff;
    mList.append(GMessage(GMessage::Code::SENSOR_CONFIG, node->getID(), interfaceID, false, mData));

    mData = std::vector<quint8>(7);
    mData[0] = (quint8)GCSensor::sensor_class::APDS9306_065;
    mData[1] = GCSensorAPDS9306::Register::CONFIG;
    mData[2] = i2cAddress;
    mData[3] = 1; // TODO: really needed?
    mData[4] = alsMeasurementRate;
    mData[5] = alsResolution;
    mData[6] = alsGain;
    mList.append(GMessage(GMessage::Code::SENSOR_CONFIG, node->getID(), interfaceID, false, mData));

    mData = std::vector<quint8>(6);
    mData[0] = (quint8)GCSensor::sensor_class::APDS9306_065;
    mData[1] = GCSensorAPDS9306::Register::TH_HIGH;
    mData[2] = (quint8) (alsTHHigh >> 24) & 0xff;
    mData[3] = (quint8) (alsTHHigh >> 16) & 0xff;
    mData[4] = (quint8) (alsTHHigh >> 8) & 0xff;
    mData[5] = (quint8) (alsTHHigh) & 0xff;
    mList.append(GMessage(GMessage::Code::SENSOR_CONFIG, node->getID(), interfaceID, false, mData));

    mData = std::vector<quint8>(6);
    mData[0] = (quint8)GCSensor::sensor_class::APDS9306_065;
    mData[1] = GCSensorAPDS9306::Register::TH_LOW;
    mData[2] = (quint8) (alsTHLow >> 24) & 0xff;
    mData[3] = (quint8) (alsTHLow >> 16) & 0xff;
    mData[4] = (quint8) (alsTHLow >> 8) & 0xff;
    mData[5] = (quint8) (alsTHLow) & 0xff;
    mList.append(GMessage(GMessage::Code::SENSOR_CONFIG, node->getID(), interfaceID, false, mData));

    return mList;
}

void GCSensorAPDS9306::printHeader(void)
{
    file->write("# Intermediate storage file for APDS9306 065\n");
    file->write("# APDS9306 065 is a visible light sensor that produces data in lux. The data is stored in the `pd` column. Each value is timestamped.\n");
    file->write("time; pd\n");
}
void GCSensorAPDS9306::saveData(std::vector<quint8>& data)
{
    QString formattedData;
    uint32_t value;


    QDateTime date = QDateTime::currentDateTime();
    formattedData.append(date.toString("dd.MM.yyyy hh:mm:ss"));
    formattedData.append("; ");
    if(data.size() == 3){
        value = data[0] | (data[1] << 8) | (data[2] << 16);
        formattedData.append(QString::number(value)); // print in scientific format with precision of 6
    } else {
        formattedData.append("NaN");
    }
    formattedData.append("\n");

    file->write(formattedData.toUtf8());
}
