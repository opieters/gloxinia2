#include "gmessage.h"
#include <QDateTime>
#include "gcsensor.h"

/*
GMessage::GMessage(GMessage::Code code, uint8_t messageID, uint16_t sensorID, bool request, quint8* data, uint64_t size):
    code(code),
    messageID(messageID),
    sensorID(sensorID),
    request(request)
{
    this->data = std::vector<quint8>(data, data+size);
}*/
GMessage::GMessage(GMessage::Code code, uint8_t messageID, uint8_t interfaceID, uint8_t sensorID, bool request, std::vector<quint8> data) : code(code),
    messageAddress(messageID),
    interfaceID(interfaceID),
    sensorID(sensorID),
    request(request),
    data(data)
{
}

int GMessage::toBytes(quint8 *data, unsigned int maxLength) const
{
    if (maxLength < (headerSize + this->data.size()))
    {
        return -1;
    }
    quint8 cmd = (quint8)code;

    data[0] = GMessage::GMessageStartByte;
    data[headerSize - 1 + 8] = GMessage::GMessageStopByte;
    data[1] = (quint8)messageAddress;
    data[2] = cmd;
    data[3] = request ? 1 : 0;
    data[4] = interfaceID;
    data[5] = sensorID;
    data[6] = this->data.size();
    for (int i = 0; i < 8; i++)
    {
        if(i < this->data.size())
            data[headerSize - 1 + i] = this->data[i];
        else
            data[headerSize - 1 + i] = 0;
    }

    return headerSize + 8;
}

QString GMessage::toString() const
{
    QString formattedData;
    for (quint8 i : data)
    {
        formattedData.append(QStringLiteral("%1").arg(i, 2, 16, QLatin1Char('0')) + " ");
    }
    return "[" + GMessage::codeToString(code) + "] (" + QString::number(messageAddress) + ", " + QString::number(interfaceID) + ", " + QString::number(sensorID) + ", " + (request ? "RQS" : "STD") + ", " + QString::number(data.size()) + ")" + formattedData;
}

std::ostream &operator<<(std::ostream &outs, const GMessage &m)
{
    return outs << m.toString().toStdString();
}

QString GMessage::codeToString(GMessage::Code code)
{
    switch (code)
    {
    case REQUEST_ADDRESS_AVAILABLE:
        return "request address available";
        break;
    case ADDRESS_TAKEN:
        return "address taken";
        break;
    case UPDATE_ADDRESS:
        return "update address";
        break;
    case READY:
        return "node ready";
    case DISCOVERY:
        return "node discovery";
        break;
    case HELLO:
        return "hello";
    case MSG_TEXT:
        return "text";
    case NODE_INFO:
        return "node info";
    case NODE_RESET:
        return "node reset";
    case SENSOR_STATUS:
        return "sensor status";
    case SENSOR_ERROR:
        return "sensor error";
    case SENSOR_DATA:
        return "sensor data";
    case SENSOR_CONFIG:
        return "sensor config";
    case SENSOR_START:
        return "sensor start";
    case SENSOR_STOP:
        return "sensor stop";
    case SENSOR_CONFIG_END:
        return "sensor config end";
    case CONFIG_DONE_START_READOUT:
        return "config done start readout";	
    case CONFIG_SAVED:
        return "config saved";
    case ACTUATOR_STATUS:
        return "actuator config";
    case TX_ERROR:
        return "transmit error";
    case DATA_CLEAR:
        return "clear data";
    case DATA_READ :
        return "read data";
    case DATA_WRITE:
        return "data write";
    case DATA_BURST_START:
        return "start data burst";
    case DATA_BURST:
        return "data burst";
    case DATA_BURST_STOP:
        return "stop data burst";
    case BOOT_READ_VERSION:
        return "read bootloader version";
    case BOOT_READ_FLASH:
        return "read flash";
    case BOOT_WRITE_FLASH:
        return "write flash";
    case  BOOT_ERASE_FLASH :
        return "erase flash";
    case BOOT_RESET_DEVICE:
        return "reset device";
    case BOOT_SELF_VERIFY:
        return "self verification";
    case BOOT_GET_MEMORY_ADDRESS_RANGE_COMMAND:
        return "flash memory range";
    case BOOT_READY:
        return "bootloader ready";
    default:
        return "unknown (" + QString::number((int)code, 16) + ")";
        break;
    }
}

std::ostream &operator<<(std::ostream &outs, const GMessage::Code &code)
{
    return outs << GMessage::codeToString(code).toStdString();
}

GMessage::Code GMessage::getCode(void) const
{
    return code;
}

quint8 GMessage::getMessageAddress(void) const
{
    return messageAddress;
}
quint8 GMessage::getSensorID(void) const
{
    return sensorID;
}
quint8 GMessage::getInterfaceID(void) const
{
    return interfaceID;
}
std::vector<quint8> GMessage::getData(void) const
{
    return std::vector<quint8>(data);
}

bool GMessage::getRequest(void) const
{
    return request;
}

QString GMessage::toLogString() const
{
    QString formattedData;
    QString cTime = QDateTime::currentDateTime().toString("hh:mm:ss");
    if(!data.empty()){
        switch (code)
        {
        case MSG_TEXT:
        {
            std::string s(data.begin(), data.end());
            formattedData = QString::fromStdString(s);
            break;
        }
        case SENSOR_STATUS:
            switch ((GCSensor::GCSensorStatus)data[0])
            {
            case GCSensor::GCSensorStatus::INACTIVE:
                formattedData.append("inactive");
                break;
            case GCSensor::GCSensorStatus::IDLE:
                formattedData.append("idle");
                break;
            case GCSensor::GCSensorStatus::ACTIVE:
                formattedData.append("active");
                break;
            case GCSensor::GCSensorStatus::RUNNING:
                formattedData.append("running");
                break;
            case GCSensor::GCSensorStatus::STOPPED:
                formattedData.append("stopped");
                break;
            case GCSensor::GCSensorStatus::ERROR:
                formattedData.append("error");
                break;
            }

            break;
        default:
        {
            for (quint8 i : data)
            {
                formattedData.append(QStringLiteral("%1").arg(i, 2, 16, QLatin1Char('0')) + " ");
            }
        }
        }
    }

    return "[" + cTime + "] (" + QString::number(messageAddress) + ", " + QString::number(interfaceID) + ", " + QString::number(sensorID) + (request ? ", RQST" : ", STD") + ") " + GMessage::codeToString(code) + " " + formattedData;
}
