#include "gmessage.h"
#include <QDateTime>


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
    quint16 cmd = (quint16) code;

    data[0] = GMessage::GMessageStartByte;
    data[headerSize + this->data.size() - 1] = GMessage::GMessageStopByte;
    data[1] = (char) (cmd >> 8);
    data[2] = (char) cmd;
    data[3] = messageID;
    data[4] = sensorID >> 8;
    data[5] = sensorID & 0xff;
    data[6] = this->data.size();
    for(int i =0; i < this->data.size(); i++){
        data[headerSize-1+i] = this->data[i];
    }

    return headerSize + this->data.size();
}

QString GMessage::toString() const{
    QString formattedData;
    for(char i : data)
    {
        formattedData.append(QStringLiteral("%1").arg(i, 2, 16, QLatin1Char('0')) + " ");
    }
    return "[" + GMessage::codeToString(code) + "] (" + QString::number(messageID) + ", " + QString::number(sensorID) + ")" + formattedData;
}

std::ostream& operator << ( std::ostream& outs, const GMessage & m )
{
  return outs << m.toString().toStdString();
}


QString GMessage::codeToString(GMessage::Code code)
{
    switch(code){
        case CAN_REQUEST_ADDRESS_AVAILABLE:
            return "request address available";
            break;
        case CAN_ADDRESS_TAKEN:
            return "address taken";
            break;
        case CAN_UPDATE_ADDRESS:
            return "update address";
            break;
        case CAN_DISCOVERY:
            return "node discovery";
            break;
        case GMessage::Code::startMeasurement:
            return "start measurement";
            break;
        case GMessage::Code::stopMeasurement:
            return "stop measurement";
            break;
        case GMessage::Code::activate_sensor:
            return "activate senosr";
            break;
        case GMessage::Code::deactivate_sensor:
            return "deactivate sensor";
            break;
        case GMessage::Code::reset_node:
            return "reset node";
            break;
        case GMessage::Code::reset_system:
            return "reset system";
            break;
        case GMessage::Code::text_message:
            return "text message";
            break;
        case GMessage::Code::sensor_data:
            return "sensor data";
            break;
        case GMessage::Code::sensor_status:
            return "sensor status";
            break;
        case GMessage::Code::measurement_period:
            return "measurement period";
            break;
        case GMessage::Code::error_message:
            return "error message";
            break;
        case GMessage::Code::loopback_message:
            return "loopback message";
            break;
        case GMessage::Code::actuator_status:
            return "actuator status";
            break;
        case GMessage::Code:: hello_message:
            return "hello message";
            break;
            case GMessage::Code::init_sampling:
            return "init sampling";
            break;
        case GMessage::Code:: init_sensors:
            return "init sensors";
            break;
        case GMessage::Code:: sensor_error:
            return "sensor error";
            break;
        case GMessage::Code::lia_gain:
            return "lia gain";
            break;
        case GMessage::Code::unknown :
            return "unkown";
            break;
        case GMessage::Code::meas_trigger :
            return "measurement trigger";
            break;
        case GMessage::Code::sensor_config:
            return "sensor config";
            break;
        case GMessage::Code:: actuator_data :
            return "actuator data";
            break;
        case GMessage::Code:: actuator_error:
            return "actuator error";
            break;
        case GMessage::Code::actuator_trigger:
            return "actuator trigger";
            break;
        case GMessage::Code::actuator_gc_temp :
            return "actuator gc temperature";
            break;
        case GMessage::Code::actuator_gc_rh    :
            return "actuator gc relative humidity";
            break;
        case GMessage::Code:: sensor_start:
            return "sensor start";
            break;
        case GMessage::Code::actuator_relay:
            return "actuator relay";
            break;
        case GMessage::Code::sensor_actuator_enable:
            return "sensor actuator enable";
            break;
        case GMessage::Code::actuator_relay_now:
            return "actuator relay now";
            break;
        default:
            return "unknown";
            break;
    }
}

std::ostream& operator << ( std::ostream& outs, const GMessage::Code &code)
{
    return outs << GMessage::codeToString(code).toStdString();
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


QString GMessage::toLogString() const
{
    QString formattedData;
    QString cTime = QDateTime::currentDateTime().toString("hh:mm:ss");
    for(char i : data)
    {
        formattedData.append(QStringLiteral("%1").arg(i, 2, 16, QLatin1Char('0')) + " ");
    }
    return "[" + cTime + "] (" + QString::number(messageID) + ", " + QString::number(sensorID) + ") " + GMessage::codeToString(code) + " " + formattedData;
}
