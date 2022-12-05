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
GMessage::GMessage(GMessage::Code code, uint8_t messageID, uint16_t sensorID, bool request, std::vector<quint8> data) : code(code),
                                                                                                                        messageID(messageID),
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
    data[headerSize + this->data.size() - 1] = GMessage::GMessageStopByte;
    data[1] = (quint8)(messageID >> 8);
    data[2] = (quint8)messageID;
    data[3] = cmd;
    data[4] = request ? 1 : 0;
    data[5] = (quint8)(sensorID >> 8);
    data[6] = (quint8)(sensorID & 0xff);
    data[7] = this->data.size();
    for (int i = 0; i < this->data.size(); i++)
    {
        data[headerSize - 1 + i] = this->data[i];
    }

    return headerSize + this->data.size();
}

QString GMessage::toString() const
{
    QString formattedData;
    for (quint8 i : data)
    {
        formattedData.append(QStringLiteral("%1").arg(i, 2, 16, QLatin1Char('0')) + " ");
    }
    return "[" + GMessage::codeToString(code) + "] (" + QString::number(messageID) + ", " + QString::number(sensorID) + ")" + formattedData;
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
    case ACTUATOR_STATUS:
        return "actuator config";
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

quint8 GMessage::getMessageID(void) const
{
    return messageID;
}
quint16 GMessage::getSensorID(void) const
{
    return sensorID;
}
std::vector<quint8> GMessage::getData(void) const
{
    return std::vector<quint8>(data);
}

QString GMessage::toLogString() const
{
    QString formattedData;
    QString cTime = QDateTime::currentDateTime().toString("hh:mm:ss");
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
    return "[" + cTime + "] (" + QString::number(messageID) + ", " + QString::number(sensorID) + ") " + GMessage::codeToString(code) + " " + formattedData;
}
