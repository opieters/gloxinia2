#include "gcsensor.h"
#include <algorithm>
#include <QList>

GCSensor::GCSensor(quint8 id): interfaceID(id)
{
}

GCSensor::~GCSensor(){}


/*QString GCSensor::sensorTypeToString()
{
    switch(t){
    case Disabeled:
        return QString("Disabeled sensor port");
    case SHT35:
        return QString("SHT35");
    case APDS9306:
        return QString("APDS9306");
    }
    return QString("Unknown sensor type");
}*/

/*GCSensor::SensorType GCSensor::getSensorType(void)
{
    return sensorType;
}*/

quint8 GCSensor::getInterfaceID(void)
{
    return interfaceID;
}

void GCSensor::setInterfaceID(quint8 id)
{
    interfaceID = id;
}

QString GCSensor::getLabel(void) const
{
    return label;
}

void GCSensor::setLabel(const QString label)
{
    this->label = label;
}


QDebug operator<<(QDebug dbg, const GCSensor&)
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

GCSensorI2C::GCSensorI2C(quint8 i2cAddress, quint8 id):
    GCSensor(id), i2cAddress(i2cAddress)
{

}

GCSensorI2C::~GCSensorI2C(){}

bool GCSensorI2C::setI2CAddress(const quint8 a)
{
    i2cAddress = a;
    return true;
}

int GCSensorI2C::i2cAddressToInt(quint8 a)
{
    return (int) a;
}

const quint8 GCSensorI2C::getI2CAddress(void)
{
    return i2cAddress;
}


GCSensorSHT35::GCSensorSHT35(quint8 i2cAddress, quint8 id) :
    GCSensorI2C(i2cAddress, id)
{
    repeatability = 0;
    clockStretching = 0;
    rate = 0;
    periodicity = 0;
}


GCSensorSHT35::~GCSensorSHT35(){}


bool GCSensorSHT35::setI2CAddress(const quint8 a)
{
    i2cAddress = a;
    if((i2cAddress != I2CAddressA) && (i2cAddress != I2CAddressB))
    {
        i2cAddress = I2CAddressA;
    }
    return a == i2cAddress;
}
bool GCSensorSHT35::setRepeatability(const quint8 r)
{
    repeatability = std::min(r, (quint8) 2);
    return r == repeatability;
}
bool GCSensorSHT35::setClockStretching(const quint8 c)
{
    clockStretching = std::min(c, (quint8) 1);
    return c == clockStretching;
}
bool GCSensorSHT35::setRate(const quint8 r)
{
    rate = std::min(r, (quint8) 4);
    return r == rate;
}
bool GCSensorSHT35::setPeriodicity(const quint8 p)
{
    periodicity = std::min(p, (quint8) 1);
    return p == periodicity;
}


const quint8 GCSensorSHT35::getRepeatability(void)
{
    return repeatability;
}
const quint8 GCSensorSHT35::getClockStretching(void)
{
    return clockStretching;
}
const quint8 GCSensorSHT35::getRate(void)
{
    return rate;
}
const quint8 GCSensorSHT35::getPeriodicity(void)
{
    return periodicity;
}

int GCSensorSHT35::i2cAddressToInt(const quint8 a)
{
    for(int i = 0; i < GCSensorSHT35::i2cAddresses.size(); i++)
    {
        if(i2cAddresses[i] == a)
        {
            return i;
        }
    }
    return -1;
}

QString GCSensorSHT35::toString(void) const
{
    QString dLabel;
    if(!label.isEmpty()){
        dLabel = label;
    } else {
        dLabel = "SHT35";
    }
    return "[" + QString::number(interfaceID) + "] " + dLabel + " (" + QString::number(i2cAddress) + ")";
}


QString GCSensorSHT35::toConfigString(void) const
{
    QString nLabel = label;
    nLabel.replace(' ', "\\ ");
    return "S SHT35 "
            + QString::number(interfaceID) + " "
            + nLabel + " "
            + QString::number(i2cAddress) + " "
            + QString::number(repeatability) + " "
            + QString::number(clockStretching) + " "
            + QString::number(rate) + " "
            + QString::number(periodicity) + " ;";

}

bool GCSensorSHT35::fromConfigString(const QStringList& config)
{
    GCSensorSHT35 s;

    if(config.count() != 9)
        return false;

    if((config[0] != "S") || (config[1] != "SHT35"))
        return false;

    bool success = true;
    s.setInterfaceID(config[2].toInt());
    s.setLabel(config[3]);
    success = success && s.setI2CAddress(config[4].toInt());
    success = success && s.setRepeatability(config[5].toInt());
    success = success && s.setClockStretching(config[6].toInt());
    success = success && s.setRate(config[7].toInt());
    success = success && s.setPeriodicity(config[8].toInt());

    if(success){
        this->setInterfaceID(s.getInterfaceID());
        this->setLabel(s.getLabel());
        this->setI2CAddress(s.getI2CAddress());
        this->setRepeatability(s.getRepeatability());
        this->setClockStretching(s.getClockStretching());
        this->setRate(s.getRate());
        this->setPeriodicity(s.getPeriodicity());
    }

    return success;
}


GCSensorAPDS9306::GCSensorAPDS9306(quint8 i2cAddress, quint8 id):
    GCSensorI2C(i2cAddress, id){

}

GCSensorAPDS9306::~GCSensorAPDS9306(){}

bool GCSensorAPDS9306::setI2CAddress(const quint8 a){
    i2cAddress = a;
    return true;
}

bool GCSensorAPDS9306::setAlsMeasurementRate(quint8 v)
{
    alsMeasurementRate = std::min(v, (quint8) 0b110);
    return v == alsMeasurementRate;
}
bool GCSensorAPDS9306::setAlsResolution(quint8 v)
{
    alsResolution = std::min(v, (quint8) 0b101);
    return v == alsResolution;
}
bool GCSensorAPDS9306::setAlsGain(quint8 v)
{
    alsGain = std::min(v, (quint8) 0b100);
    return v == alsGain;
}
bool GCSensorAPDS9306::setAlsIVCount(quint8 v)
{
    alsIVCount = std::min(v, (quint8) 0b111);
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

int GCSensorAPDS9306::i2cAddressToInt(quint8 a){
    return a;
}

QString GCSensorAPDS9306::toString(void) const
{
    QString dLabel;
    if(!label.isEmpty()){
        dLabel = label;
    } else {
        dLabel = "APDS9306";
    }
    return "[" + QString::number(interfaceID) + "] " + dLabel + " (" + QString::number(i2cAddress) + ")";
}

QString GCSensorAPDS9306::toConfigString(void) const
{
    QString nLabel = label;
    nLabel.replace(' ', "\\ ");
    return "S APDS9306 "
            + QString::number(interfaceID) + " "
            + nLabel + " "
            + QString::number(i2cAddress) + " "
            + QString::number(alsMeasurementRate) + " "
            + QString::number(alsResolution) + " "
            + QString::number(alsGain) + " "
            + QString::number(alsIVCount) + " "
            + QString::number(alsTHHigh) + " "
            + QString::number(alsTHLow) + " "
            + ";";

}

bool GCSensorAPDS9306::fromConfigString(const QStringList& config)
{
    GCSensorAPDS9306 s;

    if(config.count() != 11)
        return false;

    if((config[0] != "S") || (config[1] != "SHT35"))
        return false;

    bool success = true;
    s.setInterfaceID(config[2].toInt());
    s.setLabel(config[3]);
    success = success && s.setI2CAddress(config[4].toInt());
    success = success && s.setAlsMeasurementRate(config[5].toInt());
    success = success && s.setAlsResolution(config[6].toInt());
    success = success && s.setAlsGain(config[7].toInt());
    success = success && s.setAlsIVCount(config[8].toInt());
    success = success && s.setAlsTHHigh(config[9].toInt());
    success = success && s.setAlsTHLow(config[10].toInt());

    if(success){
        this->setInterfaceID(s.getInterfaceID());
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
