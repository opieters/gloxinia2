#ifndef GMESSAGE_H
#define GMESSAGE_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <Qt>
#include <QString>

class GMessage
{
public:
    // is two bytes wide in the hardware, see C files for documentation
    enum Code
    {
        // CAN code are normal codes
        NOP = 0x00,
        REQUEST_ADDRESS_AVAILABLE = 0x01,
        ADDRESS_TAKEN = 0x02,
        UPDATE_ADDRESS = 0x03,
        READY = 0x04,
        DISCOVERY = 0x05,
        HELLO = 0x06,
        MSG_TEXT = 0x07,
        NODE_INFO = 0x08,
        NODE_RESET = 0x09,
        SENSOR_STATUS = 0x10,
        SENSOR_ERROR = 0x11,
        SENSOR_DATA = 0x12,
        SENSOR_CONFIG = 0x13,
        ACTUATOR_STATUS = 0x20,
    };

    static constexpr uint8_t GMessageStartByte = 0x01;
    static constexpr uint8_t GMessageStopByte = 0x04;
    static constexpr uint8_t headerSize = 9;

    static constexpr uint8_t ComputerAddress = 0x00;
    static constexpr uint8_t SearchStartAddress = 0x01;
    static constexpr uint8_t UnsetAddress = 0xff;
    static constexpr uint16_t NoSensorID = 0x0000;

    GMessage(GMessage::Code code = NOP, quint8 messegaID = ComputerAddress, quint16 sensorID = NoSensorID, bool request = false, std::vector<quint8> data = std::vector<quint8>());

    int toBytes(quint8 *data, unsigned int maxLength) const;

    GMessage::Code getCode(void) const;
    quint8 getMessageID(void) const;
    quint16 getSensorID(void) const;
    std::vector<quint8> getData(void) const;

    friend std::ostream &operator<<(std::ostream &outs, const GMessage &m);

    QString toString() const;

    static QString codeToString(GMessage::Code c);

    QString toLogString() const;

private:
    GMessage::Code code;
    std::vector<quint8> data;

    bool request;
    quint8 messageID;
    quint16 sensorID;
};

std::ostream &operator<<(std::ostream &outs, const GMessage::Code &code);

#endif // GMESSAGE_H
