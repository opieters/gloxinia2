#include "gcsensor.h"
#include <algorithm>
#include <QList>

GCSensor::GCSensor(GCSensor::SensorType t, quint8 id): sensorType(t), sensorID(id)
{
}

GCSensor::~GCSensor(){}


QString GCSensor::sensorTypeToString(SensorType t)
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
}

GCSensor::SensorType GCSensor::getSensorType(void)
{
    return sensorType;
}

quint8 GCSensor::getSensorID(void)
{
    return sensorID;
}

void GCSensor::setSensorID(quint8 id)
{
    sensorID = id;
}


GCSensorI2C::GCSensorI2C(quint8 i2cAddress, SensorType t, quint8 id):
    GCSensor(t, id), i2cAddress(i2cAddress)
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
    GCSensorI2C(i2cAddress, SensorType::SHT35, id)
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

GCSensorAPDS9306::GCSensorAPDS9306(quint8 i2cAddress, quint8 id):
    GCSensorI2C(i2cAddress, SensorType::APDS9306, id){

}

GCSensorAPDS9306::~GCSensorAPDS9306(){}

bool GCSensorAPDS9306::setI2CAddress(const quint8 a){
    i2cAddress = a;
    return true;
}

int GCSensorAPDS9306::i2cAddressToInt(quint8 a){
    return a;
}
