#include "gmessage.h"


GMessage::GMessage(GMessageCode code, uint8_t messageID, uint16_t sensorID, char* data, uint64_t size):code(code), messageID(messageID), sensorID(sensorID){
    this->data = std::vector<char>(data, data+size);
}
GMessage::GMessage(GMessageCode code, uint8_t messageID, uint16_t sensorID, std::vector<char> data):code(code), messageID(messageID), sensorID(sensorID), data(data){

}

int GMessage::toBytes(char* data, unsigned int maxLength){
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


std::ostream& operator << ( std::ostream& outs, const GMessageCode &code)
{
    switch(code){
        case GMessageCode::startMeasurement:
            outs << "start measurement";
            break;
        case GMessageCode::stopMeasurement:
            outs << "stop measurement";
            break;
        case GMessageCode::activate_sensor:
            outs << "activate senosr";
            break;
        case GMessageCode::deactivate_sensor:
            outs << "deactivate sensor";
            break;
        case GMessageCode::reset_node:
            outs << "reset node";
            break;
        case GMessageCode::reset_system:
            outs << "reset system";
            break;
        case GMessageCode::text_message:
            outs << "text message";
            break;
        case GMessageCode::sensor_data:
            outs << "sensor data";
            break;
        case GMessageCode::sensor_status:
            outs << "sensor status";
            break;
        case GMessageCode::measurement_period:
            outs << "measurement period";
            break;
        case GMessageCode::error_message:
            outs << "error message";
            break;
        case GMessageCode::loopback_message:
            outs << "loopback message";
            break;
        case GMessageCode::actuator_status:
            outs << "actuator status";
            break;
        case GMessageCode:: hello_message:
            outs << "hello message";
            break;
            case GMessageCode::init_sampling:
            outs << "init sampling";
            break;
        case GMessageCode:: init_sensors:
            outs << "init sensors";
            break;
        case GMessageCode:: sensor_error:
            outs << "sensor error";
            break;
        case GMessageCode::lia_gain:
            outs << "lia gain";
            break;
        case GMessageCode::unknown :
            outs << "unkown";
            break;
        case GMessageCode::meas_trigger :
            outs << "measurement trigger";
            break;
        case GMessageCode::sensor_config:
            outs << "sensor config";
            break;
        case GMessageCode:: actuator_data :
            outs << "actuator data";
            break;
        case GMessageCode:: actuator_error:
            outs << "actuator error";
            break;
        case GMessageCode::actuator_trigger:
            outs << "actuator trigger";
            break;
        case GMessageCode::actuator_gc_temp :
            outs << "actuator gc temperature";
            break;
        case GMessageCode::actuator_gc_rh    :
            outs << "actuator gc relative humidity";
            break;
        case GMessageCode:: sensor_start:
            outs << "sensor start";
            break;
        case GMessageCode::actuator_relay:
            outs << "actuator relay";
            break;
        case GMessageCode::sensor_actuator_enable:
            outs << "sensor actuator enable";
            break;
        case GMessageCode::actuator_relay_now:
            outs << "actuator relay now";
            break;
        default:
            outs << "unknown";
            break;
    }

    return outs;
}
