#ifndef GCSENSOR_H
#define GCSENSOR_H

#include <Qt>
#include <QString>
#include <QAbstractItemModel>

class GCSensor
{

public:

    GCSensor(quint8 id = 0);
    ~GCSensor();

    enum sensor_class {
        NOT_SET = 0x00,
        SHT35 = 0x01,
        APDS9306_065 = 0x02,
        ANALOGUE = 0x03,
    };

    enum sensor_status {
        INACTIVE =0x00,
        IDLE = 0x01,
        ACTIVE = 0x02,
        ERROR = 0x03
    };

    //static QString sensorTypeToString(SensorType t);

    //SensorType getSensorType(void);
    quint8 getInterfaceID(void);

    void setInterfaceID(quint8 id);

    QString getLabel(void) const;
    void setLabel(const QString label);

    friend QDebug operator<<(QDebug dbg, const GCSensor&);

    virtual QString toString(void) const = 0;
    virtual QString toConfigString(void) const = 0;
    virtual bool fromConfigString(const QStringList& config) = 0;

    friend QDataStream &operator<<(QDataStream &out, const GCSensor &myObj);
    friend QDataStream &operator>>(QDataStream &in, GCSensor &myObj);
protected:
    //SensorType sensorType;
    quint8 interfaceID;

    QString label;
};

Q_DECLARE_METATYPE(GCSensor*)

class GCSensorI2C : public GCSensor
{
public:
    GCSensorI2C(quint8 i2cAddress, quint8 id = 0);
    ~GCSensorI2C();

    virtual bool setI2CAddress(const quint8 a);

    const quint8 getI2CAddress(void);

    virtual int i2cAddressToInt(quint8 a);

    virtual QString toString(void) const = 0;
    virtual QString toConfigString(void) const = 0;
    virtual bool fromConfigString(const QStringList& config) = 0;
protected:
    quint8 i2cAddress;
};

Q_DECLARE_METATYPE(GCSensorI2C*)

class GCSensorSHT35 : public GCSensorI2C
{
public:
    GCSensorSHT35(quint8 i2cAddress = I2CAddressA, quint8 id = 0);
    GCSensorSHT35(const GCSensorSHT35& s) = default;
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

    QString toString(void) const override;
    QString toConfigString(void) const override;
    bool fromConfigString(const QStringList& config) override;

protected:
    quint8 repeatability;
    quint8 clockStretching;
    quint8 rate;
    quint8 periodicity;
};

Q_DECLARE_METATYPE(GCSensorSHT35*)

class GCSensorAPDS9306 : public GCSensorI2C
{
public:
    GCSensorAPDS9306(quint8 i2cAddress = 0x52, quint8 id = 0);
    GCSensorAPDS9306(const GCSensorAPDS9306& s) = default;
    ~GCSensorAPDS9306();

    bool setI2CAddress(const quint8 a) override;
    bool setAlsMeasurementRate(quint8);
    bool setAlsResolution(quint8);
    bool setAlsGain(quint8);
    bool setAlsIVCount(quint8);
    bool setAlsTHHigh(quint32);
    bool setAlsTHLow(quint32);

    quint8 getAlsMeasurementRate(void);
    quint8 getAlsResolution(void);
    quint8 getAlsGain(void);
    quint8 getAlsIVCount(void);
    quint32 getAlsTHHigh(void);
    quint32 getAlsTHLow(void);

    int i2cAddressToInt(quint8 a) override;

    QString toString(void) const override;
    QString toConfigString(void) const override;
    bool fromConfigString(const QStringList& config) override;

private:
    quint8 alsMeasurementRate;
    quint8 alsResolution;
    quint8 alsGain;
    quint8 alsIVCount;

    quint32 alsTHHigh;
    quint32 alsTHLow;

};

Q_DECLARE_METATYPE(GCSensorAPDS9306*)

#endif // GCSENSOR_H
