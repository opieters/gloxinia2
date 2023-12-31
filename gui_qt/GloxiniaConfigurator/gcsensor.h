#ifndef GCSENSOR_H
#define GCSENSOR_H

#include <Qt>
#include <QString>
#include <QList>
#include <QAbstractItemModel>
#include <QFile>
#include "gmessage.h"
#include "gcnode.h"
#include <QLineSeries>

class GCSensor
{

public:
    GCSensor(GCNode* const node = nullptr, quint8 ifid = 0x0, quint8 id = 0x0);
    virtual ~GCSensor();

    enum sensor_class
    {
        NOT_SET = 0x00,
        SHT35 = 0x01,
        APDS9306_065 = 0x02,
        ADC12 = 0x03,
        ADC16 = 0x04,
        LIA = 0x05,
        ADS1219 = 0x06,
    };

    enum GCSensorStatus
    {
        INACTIVE = 0x00,
        IDLE = 0x01,
        ACTIVE = 0x02,
        RUNNING = 0x03,
        STOPPED = 0x04,
        ERROR = 0x05
    };

    enum VariableType {
        Temperature,
        RelativeHumidity,
        Light,
        Analogue,
    };

    enum PGAGain
    {
        PGA_GAIN_1 = 0b0000,   ///< Gain = 1
        PGA_GAIN_2 = 0b0001,   ///< Gain = 2
        PGA_GAIN_5 = 0b0010,   ///< Gain = 5
        PGA_GAIN_10 = 0b0011,  ///< Gain = 10
        PGA_GAIN_20 = 0b0100,  ///< Gain = 20
        PGA_GAIN_50 = 0b0101,  ///< Gain = 50
        PGA_GAIN_100 = 0b0110, ///< Gain = 100
        PGA_GAIN_200 = 0b0111, ///< Gain = 200
    };

    // static QString sensorTypeToString(SensorType t);

    quint8 getSensorID(void);
    quint8 getInterfaceID(void);

    GCNode* getNode(void) const;

    QString getLabel(void) const;
    void setLabel(const QString label);
    quint16 getMeasurementPeriod(void) const;
    void setMeasurementPeriod(quint16 period);
    bool getUseGlobalPeriodFlag(void) const;
    void setUseGlobalPeriodFlag(bool flag);
    void setStatus(GCSensorStatus s);
    GCSensorStatus getStatus() const;
    QList<VariableType> getVariableTypes() const;
    QList<QLineSeries*> getPlotSeries() const;

    static GCSensor* fromQVariant(const QVariant data);

    static QString statusToString(GCSensorStatus s);

    virtual bool startMeasurement(void);
    GMessage getStartMessage(void);
    GMessage getStopMessage(void);
    virtual void stopMeasurement(void);
    virtual void saveData(std::vector<quint8>& data) = 0;
    virtual void printHeader(void) = 0;

    friend QDebug operator<<(QDebug dbg, const GCSensor &);

    virtual QString toString(void) const = 0;
    virtual QString toConfigString(void) const = 0;
    virtual bool fromConfigString(const QStringList &config) = 0;
    virtual QList<GMessage> getConfigurationMessages() = 0;

    friend QDataStream &operator<<(QDataStream &out, const GCSensor &myObj);
    friend QDataStream &operator>>(QDataStream &in, GCSensor &myObj);

    static void setSensorFileDir(const QString dir);
    static QString getSensorFileDir(void);

    static QString VariableTypeToString(VariableType t);

    unsigned int getMaxPlotSize(void);
    void setMaxPlotSize(unsigned int value);

    static int getFullID(short interface, short sensor);

protected:
    GCNode* const node;
    const quint8 sensorID;
    const quint8 interfaceID;

    GCSensorStatus status = INACTIVE;
    quint16 measurementPeriod = 9;
    bool useGlobalPeriod = false;
    QString label;
    QFile* file = nullptr;
    QString filePath;

    static QString sensorFileDir;
    unsigned int maxPlotSize = 100;

    // data storage for plotting
    QList<QLineSeries*> plotSeries;
    QList<VariableType> measurementVariableTypes;
};

Q_DECLARE_METATYPE(GCSensor *)

class GCSensorI2C : public GCSensor
{
public:
    GCSensorI2C(GCNode* const node = nullptr, quint8 interface_id=0, quint8 id = 0, quint8 i2cAddress = 0x0);
    virtual ~GCSensorI2C() = 0;

    virtual bool setI2CAddress(const quint8 a);

    const quint8 getI2CAddress(void);

    virtual int i2cAddressToInt(quint8 a);

    virtual QString toString(void) const = 0;
    virtual QString toConfigString(void) const = 0;
    virtual bool fromConfigString(const QStringList &config) = 0;

protected:
    quint8 i2cAddress;
};

Q_DECLARE_METATYPE(GCSensorI2C *)

typedef struct {
    quint8 repeatability;
    quint8 clockStretching;
    quint8 rate;
    quint8 periodicity;
} sensor_config_sht35_t;

class GCSensorSHT35 : public GCSensorI2C
{
public:
    enum Register
    {
        MEASUREMENT = 0x00,
        CONFIG = 0x01,
    };

    static constexpr uint8_t crcPolynomial = 0x31U;

    GCSensorSHT35(GCNode* const node = nullptr, quint8 interface_id=0, quint8 id = 0, quint8 i2cAddress = I2CAddressA);
    GCSensorSHT35(const GCSensorSHT35 &s) = default;
    ~GCSensorSHT35() override;

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
    bool fromConfigString(const QStringList &config) override;
    QList<GMessage> getConfigurationMessages() override;
    void saveData(std::vector<quint8>& data) override;
    void printHeader(void) override;

private:
    uint8_t calculateCrc(uint8_t b, uint8_t crc, uint8_t poly);

protected:
    sensor_config_sht35_t config;


};

Q_DECLARE_METATYPE(GCSensorSHT35 *)

class GCSensorAPDS9306 : public GCSensorI2C
{
public:
    enum Register {
        MEASUREMENT = 0x0,
        CONFIG = 0x1,
        TH_HIGH = 0x2,
        TH_LOW = 0x3,
    };

    GCSensorAPDS9306(GCNode* const node = nullptr, quint8 interface_id=0, quint8 id = 0, quint8 i2cAddress = 0x52);
    GCSensorAPDS9306(const GCSensorAPDS9306 &s) = default;
    ~GCSensorAPDS9306() override;

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
    bool fromConfigString(const QStringList &config) override;
    QList<GMessage> getConfigurationMessages() override;
    void saveData(std::vector<quint8>& data) override;
    void printHeader(void) override;

private:
    quint8 alsMeasurementRate;
    quint8 alsResolution;
    quint8 alsGain;
    quint8 alsIVCount;

    quint32 alsTHHigh;
    quint32 alsTHLow;
};

Q_DECLARE_METATYPE(GCSensorAPDS9306 *)

class GCSensorADC12 : public GCSensor
{
public:
    enum Register
    {
        MEASUREMENT = 0x00,
        CONFIG = 0x01,
    };

    GCSensorADC12(GCNode* const node = nullptr, quint8 interface_id =0, quint8 id = 0);
    GCSensorADC12(const GCSensorADC12 &s) = default;
    ~GCSensorADC12() override;

    void setAverage(bool average);
    bool getAverage(void);

    QString toString(void) const override;
    QString toConfigString(void) const override;
    bool fromConfigString(const QStringList &config) override;
    QList<GMessage> getConfigurationMessages() override;
    void saveData(std::vector<quint8>& data) override;
    void printHeader(void) override;

protected:
    bool average;
};

Q_DECLARE_METATYPE(GCSensorADC12 *)


class GCSensorADC16 : public GCSensor
{
public:
    enum Register
    {
        MEASUREMENT = 0x00,
        CONFIG = 0x01,
        PGA = 0x02,
    };

    GCSensorADC16(GCNode* const node = nullptr, quint8 interface_id =0, quint8 id = 0);
    GCSensorADC16(const GCSensorADC16 &s) = default;
    ~GCSensorADC16() override;

    void setAverage(bool average);
    bool getAverage(void);

    void setGain(PGAGain gain);
    PGAGain getGain(void);

    void setAutoGain(bool autoGain);
    bool getAutoGain(void);

    QString toString(void) const override;
    QString toConfigString(void) const override;
    bool fromConfigString(const QStringList &config) override;
    QList<GMessage> getConfigurationMessages() override;
    void saveData(std::vector<quint8>& data) override;
    void printHeader(void) override;

protected:
    bool average;
    bool autoGainConfig;
    PGAGain gain;
};

Q_DECLARE_METATYPE(GCSensorADC16 *)

class GCSensorLIA : public GCSensor
{
public:
    enum Register
    {
        MEASUREMENT = 0x00,
        CONFIG = 0x01,
        PGA = 0x02,
    };

    enum Mode
    {
        F_50KHZ = 0x00,
        F_25KHZ = 0x01,
        F_10KHZ = 0x02,
        F_5KHZ  = 0x03,
        FS      = 0x04,
    };

    enum FrequencySetting
    {
        FREQ_10HZ  = 0,
        FREQ_20HZ  = 1,
        FREQ_50HZ  = 2,
        FREQ_100HZ = 3,
        FREQ_200HZ = 4,
        FREQ_500HZ = 5,
        FREQ_1KHZ  = 6,
        FREQ_2KHZ  = 7,
        FREQ_5KHZ  = 8,
        FREQ_10KHZ = 9,
        FREQ_20KHZ = 10,
        FREQ_50KHZ = 11,
    };


    GCSensorLIA(GCNode* const node = nullptr, quint8 interface_id =0, quint8 id = 0);
    GCSensorLIA(const GCSensorLIA &s) = default;
    ~GCSensorLIA() override;

    void setAutoGain(bool autoGain);
    bool getAutoGain(void);

    void setEnableOutput(bool output);
    bool getEnableOutput(void);

    void setGain(PGAGain gain);
    PGAGain getGain(void);

    QString toString(void) const override;
    QString toConfigString(void) const override;
    bool fromConfigString(const QStringList &config) override;
    QList<GMessage> getConfigurationMessages() override;
    void saveData(std::vector<quint8>& data) override;
    void printHeader(void) override;

protected:
    bool autoGainConfig;
    bool output;

    PGAGain gain;
    Mode mode;
    FrequencySetting fsHigh;
    FrequencySetting fsLow;

};

Q_DECLARE_METATYPE(GCSensorLIA *)

class GCSensorADS1219 : public GCSensorI2C
{
public:
    enum Register {
        MEASUREMENT = 0x0,
        CONFIG = 0x1,
        CHANNELS = 0x2,
    };

    enum ConversionMode {
        SingleShot = 0x00,
        Continuous = 0x01
    };

    enum ConversionRate {
        SPS_20 = 0x00,
        SPS_90 = 0x01,
        SPS_330 = 0x02,
        SPS_1000 = 0x03,
    };

    const std::vector<quint8> addressesOptions = {
        0b1000000,
        0b1000001,
        0b1000010,
        0b1000011,
        0b1000100,
        0b1000101,
        0b1000110,
        0b1000111,
        0b1001000,
        0b1001001,
        0b1001010,
        0b1001011,
        0b1001100,
        0b1001101,
        0b1001110,
        0b1001111};

    GCSensorADS1219(GCNode* const node = nullptr, quint8 interface_id=0, quint8 id = 0, quint8 i2cAddress = 0b1000000);
    GCSensorADS1219(const GCSensorADS1219 &s) = default;
    ~GCSensorADS1219() override;

    bool setI2CAddress(const quint8 a) override;
    bool setEnabledChannels(quint8);
    bool setGain(quint8);
    bool setConversionRate(quint8);
    bool setConversionMode(quint8);
    bool setReferenceVoltage(quint8);

    quint8 getEnabledChannels(void);
    quint8 getGain(void);
    quint8 getConversionRate(void);
    quint8 getConversionMode(void);
    quint8 getReferenceVoltage(void);

    int i2cAddressToInt(quint8 a) override;

    QString toString(void) const override;
    QString toConfigString(void) const override;
    bool fromConfigString(const QStringList &config) override;
    QList<GMessage> getConfigurationMessages() override;
    void saveData(std::vector<quint8>& data) override;
    void printHeader(void) override;

private:
    quint8 enabledChannels;
    quint8 gain;
    quint8 conversionRate;
    quint8 conversionMode;
    quint8 referenceVoltage;
};

Q_DECLARE_METATYPE(GCSensorADS1219 *)

#endif // GCSENSOR_H
