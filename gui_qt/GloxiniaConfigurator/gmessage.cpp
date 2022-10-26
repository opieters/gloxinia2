#include "gmessage.h"


GMessage::GMessage(GMessage::Code code, uint8_t messageID, uint16_t sensorID, char* data, uint64_t size):
    code(code),
    messageID(messageID),
    sensorID(sensorID)
{
    this->data = std::vector<char>(data, data+size);
}
GMessage::GMessage(GMessage::Code code, uint8_t messageID, uint16_t sensorID, std::vector<char> data):
    code(code),
    messageID(messageID),
    sensorID(sensorID),
    data(data)
{

}

int GMessage::toBytes(char* data, unsigned int maxLength) const
{
    if(maxLength < (headerSize +this->data.size())){
        return -1;
    }

    data[0] = GMessage::GMessageStartByte;
    data[headerSize + this->data.size() - 1] = GMessage::GMessageStopByte;
    data[1] = messageID;
    data[2] = sensorID >> 8;
    data[3] = sensorID & 0xff;
    data[4] = this->data.size();
    for(int i =0; i < this->data.size(); i++){
        data[5+i] = this->data[i];
    }

    return headerSize + this->data.size();
}


std::ostream& operator << ( std::ostream& outs, const GMessage & m )
{
  return outs << "[" << (int) m.code << "," << m.messageID << ")";
}


std::ostream& operator << ( std::ostream& outs, const GMessage::Code &code)
{
    switch(code){
        case GMessage::Code::startMeasurement:
            outs << "start measurement";
            break;
        case GMessage::Code::stopMeasurement:
            outs << "stop measurement";
            break;
        case GMessage::Code::activate_sensor:
            outs << "activate senosr";
            break;
        case GMessage::Code::deactivate_sensor:
            outs << "deactivate sensor";
            break;
        case GMessage::Code::reset_node:
            outs << "reset node";
            break;
        case GMessage::Code::reset_system:
            outs << "reset system";
            break;
        case GMessage::Code::text_message:
            outs << "text message";
            break;
        case GMessage::Code::sensor_data:
            outs << "sensor data";
            break;
        case GMessage::Code::sensor_status:
            outs << "sensor status";
            break;
        case GMessage::Code::measurement_period:
            outs << "measurement period";
            break;
        case GMessage::Code::error_message:
            outs << "error message";
            break;
        case GMessage::Code::loopback_message:
            outs << "loopback message";
            break;
        case GMessage::Code::actuator_status:
            outs << "actuator status";
            break;
        case GMessage::Code:: hello_message:
            outs << "hello message";
            break;
            case GMessage::Code::init_sampling:
            outs << "init sampling";
            break;
        case GMessage::Code:: init_sensors:
            outs << "init sensors";
            break;
        case GMessage::Code:: sensor_error:
            outs << "sensor error";
            break;
        case GMessage::Code::lia_gain:
            outs << "lia gain";
            break;
        case GMessage::Code::unknown :
            outs << "unkown";
            break;
        case GMessage::Code::meas_trigger :
            outs << "measurement trigger";
            break;
        case GMessage::Code::sensor_config:
            outs << "sensor config";
            break;
        case GMessage::Code:: actuator_data :
            outs << "actuator data";
            break;
        case GMessage::Code:: actuator_error:
            outs << "actuator error";
            break;
        case GMessage::Code::actuator_trigger:
            outs << "actuator trigger";
            break;
        case GMessage::Code::actuator_gc_temp :
            outs << "actuator gc temperature";
            break;
        case GMessage::Code::actuator_gc_rh    :
            outs << "actuator gc relative humidity";
            break;
        case GMessage::Code:: sensor_start:
            outs << "sensor start";
            break;
        case GMessage::Code::actuator_relay:
            outs << "actuator relay";
            break;
        case GMessage::Code::sensor_actuator_enable:
            outs << "sensor actuator enable";
            break;
        case GMessage::Code::actuator_relay_now:
            outs << "actuator relay now";
            break;
        default:
            outs << "unknown";
            break;
    }

    return outs;
}

GMessage::Code GMessage::getCode(void) const
{
    return code;
}

quint8 GMessage::getMessageID(void) const
{
    return messageID;
}
quint16 GMessage::getSensorID(void) const
{
    return sensorID;
}
std::vector<char> GMessage::getData(void) const
{
    return std::vector<char>(data);
}

