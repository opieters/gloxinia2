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
    enum Code {
        // CAN code are normal codes
        CAN_REQUEST_ADDRESS_AVAILABLE   = 0x0001,
        CAN_ADDRESS_TAKEN               = 0x0002,
        CAN_UPDATE_ADDRESS              = 0x0003,
        CAN_DISCOVERY                   = 0x0005,

        // UART-only codes start with MSB set
        startMeasurement       = 0x8000,
        stopMeasurement        = 0x8001,
        activate_sensor        = 0x8002,
        deactivate_sensor      = 0x8003,
        reset_node             = 0x8004,
        reset_system           = 0x8005,
        text_message           = 0x8006,
        sensor_data            = 0x8007,
        sensor_status          = 0x8008,
        measurement_period     = 0x8009,
        error_message          = 0x800A,
        loopback_message       = 0x800B,
        actuator_status        = 0x800C,
        hello_message          = 0x800D,
        init_sampling          = 0x800E,
        init_sensors           = 0x800F,
        sensor_error           = 0x8010,
        lia_gain               = 0x8011,
        unknown                = 0x8012,
        meas_trigger           = 0x8013,
        sensor_config          = 0x8014,
        actuator_data          = 0x8015,
        actuator_error         = 0x8016,
        actuator_trigger       = 0x8017,
        actuator_gc_temp       = 0x8018,
        actuator_gc_rh         = 0x8019,
        sensor_start           = 0x801A,
        actuator_relay         = 0x801B,
        sensor_actuator_enable = 0x801C,
        actuator_relay_now     = 0x801D,
    };

    static constexpr uint8_t GMessageStartByte  = 0x01;
    static constexpr uint8_t GMessageStopByte  = 0x04;
    static constexpr uint8_t headerSize = 8;

    GMessage(GMessage::Code code, quint8 messegaID, quint16 sensorID, quint8* data = nullptr, uint64_t size = 0);
    GMessage(GMessage::Code code, quint8 messegaID, quint16 sensorID, std::vector<quint8> data);

    int toBytes(quint8* data, unsigned int maxLength) const;

    GMessage::Code getCode(void) const;
    quint8 getMessageID(void) const;
    quint16 getSensorID(void) const;
    std::vector<quint8> getData(void) const;

    friend std::ostream& operator << ( std::ostream& outs, const GMessage & m );

    QString toString() const;

    static QString codeToString(GMessage::Code c);

    QString toLogString() const;

private:
    GMessage::Code code;
    std::vector<quint8> data;

    quint8 messageID;
    quint16 sensorID;


};

std::ostream& operator << ( std::ostream& outs, const GMessage::Code &code);


#endif // GMESSAGE_H
