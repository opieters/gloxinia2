#ifndef GMESSAGE_H
#define GMESSAGE_H

#include <stdint.h>
#include <vector>
#include <iostream>

enum class GMessageCode {
    startMeasurement       = 0x00,
    stopMeasurement        = 0x01,
    activate_sensor        = 0x02,
    deactivate_sensor      = 0x03,
    reset_node             = 0x04,
    reset_system           = 0x05,
    text_message           = 0x06,
    sensor_data            = 0x07,
    sensor_status          = 0x08,
    measurement_period     = 0x09,
    error_message          = 0x0A,
    loopback_message       = 0x0B,
    actuator_status        = 0x0C,
    hello_message          = 0x0D,
    init_sampling          = 0x0E,
    init_sensors           = 0x0F,
    sensor_error           = 0x10,
    lia_gain               = 0x11,
    unknown                = 0x12,
    meas_trigger           = 0x13,
    sensor_config          = 0x14,
    actuator_data          = 0x15,
    actuator_error         = 0x16,
    actuator_trigger       = 0x17,
    actuator_gc_temp       = 0x18,
    actuator_gc_rh         = 0x19,
    sensor_start           = 0x1A,
    actuator_relay         = 0x1B,
    sensor_actuator_enable = 0x1C,
    actuator_relay_now     = 0x1D,
    n_commands             = 0x1F,
};

class GMessage
{
public:
    static constexpr uint8_t GMessageStartByte  = 0x01;
    static constexpr uint8_t GMessageStopByte  = 0x04;
    static constexpr uint8_t headerSize = 7;

    GMessage(GMessageCode code, uint8_t messegaID, uint16_t sensorID, char* data, uint64_t size);
    GMessage(GMessageCode code, uint8_t messegaID, uint16_t sensorID, std::vector<char> data);

    int toBytes(char* data, unsigned int maxLength);

private:
    GMessageCode code;
    std::vector<char> data;

    uint8_t messageID;
    uint16_t sensorID;

    friend std::ostream& operator << ( std::ostream& outs, const GMessage & m );
};

#endif // GMESSAGE_H
