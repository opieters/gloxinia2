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
        SENSOR_START = 0x14,
        SENSOR_STOP = 0x15,
        SENSOR_CONFIG_END = 0x16,
        ACTUATOR_STATUS = 0x20,
        TX_ERROR = 0x30,
        DATA_CLEAR = 0x40,
        DATA_READ = 0x41,
        DATA_WRITE = 0x42,
        DATA_BURST_START = 0x43,
        DATA_BURST = 0x44,
        DATA_BURST_STOP = 0x45,
        BOOT_READ_VERSION = 0x80,
        BOOT_READ_FLASH = 0x81,
        BOOT_WRITE_FLASH = 0x82,
        BOOT_ERASE_FLASH = 0x83,
        BOOT_WRITE_FLASH_INIT = 0x84,
        BOOT_WRITE_FLASH_DONE = 0x85,
        BOOT_RESET_DEVICE = 0x89,
        BOOT_SELF_VERIFY = 0x8A,
        BOOT_GET_MEMORY_ADDRESS_RANGE_COMMAND = 0x8B,
        BOOT_READY = 0x8F,
    };

    static constexpr uint8_t GMessageStartByte = 0x5B;
    static constexpr uint8_t GMessageStopByte = 0x5D;
    static constexpr uint8_t headerSize = 9;

    static constexpr uint8_t ComputerAddress = 0x00;
    static constexpr uint8_t SearchStartAddress = 0x01;
    static constexpr uint8_t UnsetAddress = 0xff;
    static constexpr uint16_t NoSensorID = 0x0000;
    static constexpr uint16_t UnlockSequence = 0x55AA;

    GMessage(GMessage::Code code = NOP, quint8 messegaID = ComputerAddress, quint16 sensorID = NoSensorID, bool request = false, std::vector<quint8> data = std::vector<quint8>());

    int toBytes(quint8 *data, unsigned int maxLength) const;

    GMessage::Code getCode(void) const;
    quint8 getMessageID(void) const;
    quint16 getSensorID(void) const;
    std::vector<quint8> getData(void) const;
    bool getRequest(void) const;

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
