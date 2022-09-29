#ifndef GCSENSOR_H
#define GCSENSOR_H

#include "qobjectdefs.h"
#include <Qt>
#include <QString>
#include <array>
#include <QAbstractItemModel>

class GCNode;

class GCSensor
{

public:
    enum SensorType {
        Disabeled = 0,
        SHT35 = 1,
        APDS9306 = 2
    };

    GCSensor(SensorType t = Disabeled, quint8 id = 0);

    static QString sensorTypeToString(SensorType t);

    SensorType getSensorType(void);
    quint8 getSensorID(void);

    void setSensorID(quint8 id);

    GCNode* getNode(void) { return node; };

private:
    SensorType sensorType;
    quint8 sensorID;

    GCNode* node;
};
Q_DECLARE_METATYPE(GCSensor*)

class GCSensorSHT35 : public GCSensor
{
public:
    GCSensorSHT35(quint8 id = 0);

    static constexpr quint8 I2CAddressA = 0x44;
    static constexpr quint8 I2CAddressB = 0x45;
    static constexpr std::array<int, 2> i2cAddresses = {I2CAddressA, I2CAddressB};

    bool setI2CAddress(const quint8 a);
    bool setRepeatability(const quint8 a);
    bool setClockStretching(const quint8 a);
    bool setRate(const quint8 a);
    bool setPeriodicity(const quint8 a);

    const quint8 getI2CAddress(void);
    const quint8 getRepeatability(void);
    const quint8 getClockStretching(void);
    const quint8 getRate(void);
    const quint8 getPeriodicity(void);

    static int i2cAddressToInt(quint8 a);

private:
    quint8 i2cAddress;
    quint8 repeatability;
    quint8 clockStretching;
    quint8 rate;
    quint8 periodicity;
};

#endif // GCSENSOR_H
