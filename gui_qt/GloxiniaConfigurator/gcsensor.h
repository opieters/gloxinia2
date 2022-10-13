#ifndef GCSENSOR_H
#define GCSENSOR_H

#include <Qt>
#include <QString>
#include <QAbstractItemModel>

class GCSensor
{

public:
    enum SensorType {
        Disabeled = 0,
        SHT35 = 1,
        APDS9306 = 2
    };

    GCSensor(SensorType t = Disabeled, quint8 id = 0);
    ~GCSensor();

    static QString sensorTypeToString(SensorType t);

    SensorType getSensorType(void);
    quint8 getSensorID(void);

    void setSensorID(quint8 id);
protected:
    SensorType sensorType;
    quint8 sensorID;
};

Q_DECLARE_METATYPE(GCSensor)

class GCSensorI2C : public GCSensor
{
public:
    GCSensorI2C(quint8 i2cAddress, SensorType t = Disabeled, quint8 id = 0);
    ~GCSensorI2C();

    virtual bool setI2CAddress(const quint8 a);

    const quint8 getI2CAddress(void);

    virtual int i2cAddressToInt(quint8 a);
protected:
    quint8 i2cAddress;
};

Q_DECLARE_METATYPE(GCSensorI2C)

class GCSensorSHT35 : public GCSensorI2C
{
public:
    GCSensorSHT35(quint8 i2cAddress = I2CAddressA, quint8 id = 0);
    ~GCSensorSHT35();

    static constexpr quint8 I2CAddressA = 0x44;
    static constexpr quint8 I2CAddressB = 0x45;
    static constexpr std::array<int, 2> i2cAddresses = {I2CAddressA, I2CAddressB};

    bool setI2CAddress(const quint8 a) override;
    bool setRepeatability(const quint8 a);
    bool setClockStretching(const quint8 a);
    bool setRate(const quint8 a);
    bool setPeriodicity(const quint8 a);

    const quint8 getRepeatability(void);
    const quint8 getClockStretching(void);
    const quint8 getRate(void);
    const quint8 getPeriodicity(void);

    int i2cAddressToInt(quint8 a) override;

protected:
    quint8 repeatability;
    quint8 clockStretching;
    quint8 rate;
    quint8 periodicity;
};

Q_DECLARE_METATYPE(GCSensorSHT35)

class GCSensorAPDS9306 : public GCSensorI2C
{
public:
    GCSensorAPDS9306(quint8 i2cAddress, quint8 id = 0);
    ~GCSensorAPDS9306();

    bool setI2CAddress(const quint8 a) override ;

    const quint8 getI2CAddress(void);

    int i2cAddressToInt(quint8 a) override;

private:
    quint8 i2cAddress;
};

Q_DECLARE_METATYPE(GCSensorAPDS9306)

#endif // GCSENSOR_H
