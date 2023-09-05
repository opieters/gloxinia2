#include "gloxiniaconfigurator.h"
#include <QThread>
#include <QDateTime>

void GloxiniaConfigurator::processIncomingGMessage(const GMessage &m)
{

    if(!m.getRequest())
    {
        switch (m.getCode())
        {
        case GMessage::Code::REQUEST_ADDRESS_AVAILABLE:
            break;
        case GMessage::Code::ADDRESS_TAKEN:
            break;
        case GMessage::Code::UPDATE_ADDRESS:
            break;
        case GMessage::Code::DISCOVERY:
            // check if node exists, if not create one and add to model
            processCANDiscoveryMessage(m);
            break;
        case GMessage::Code::MSG_TEXT:
            // processTextMessage(m);
            break;
        case GMessage::Code::NODE_INFO:
            processNodeInfoMessage(m);
            break;
        case GMessage::Code::SENSOR_DATA:
            processSensorData(m);
            break;
        case GMessage::Code::SENSOR_STATUS:
            processSensorStatus(m);
            break;
        case GMessage::Code::SENSOR_CONFIG:
            processSensorConfig(m);
            break;
        case GMessage::Code::BOOT_READ_VERSION:
            processBootReadVersion(m);
            break;
        case GMessage::Code::BOOT_READ_FLASH:
            processBootReadFlash(m);
            break;
        case GMessage::Code::BOOT_WRITE_FLASH:
            processBootWriteFlash(m);
            break;
        case GMessage::Code::BOOT_ERASE_FLASH:
            processBootEraseFlash(m);
            break;
        case GMessage::Code::BOOT_RESET_DEVICE:
            processBootResetDevice(m);
            break;
        case GMessage::Code::BOOT_SELF_VERIFY:
            processBootSelfVerify(m);
            break;
        case GMessage::Code::BOOT_GET_MEMORY_ADDRESS_RANGE_COMMAND:
            processBootGetMemoryAddressRangeCommand(m);
            break;
        case GMessage::Code::BOOT_READY:
            processBootReady(m);
            break;
        case GMessage::Code::SENSOR_START:
            processSensorStart(m);
            break;
        case GMessage::Code::CONFIG_SAVED:
            processConfigDoneFinishedReadout(m);
            break;
        case GMessage::Code::DICIO_LOAD_CONFIGURATION_FROM_SDCARD:
            processLoadConfigurationFromSDCard(m);
            break;
        case GMessage::Code::DICIO_CLEAR_CONFIGURATION_ON_SDCARD:
            processClearConfigurationOnSDCard(m);
            break;
        case GMessage::Code::DICIO_TIME:
            processDicioTime(m);
            break;
        default:
            break;
        }
    } else {
        switch(m.getCode()){
        case GMessage::Code::DICIO_TIME:
            processDicioTime(m);
            break;
        default:
            break;
        }
    }

    // add line for now
    messageModel->insertRow(0);
    QModelIndex mIndex = messageModel->index(0, 0);
    messageModel->setData(mIndex, m.toLogString());
    if(messageModel->rowCount() > settings.messageBufferSize){
        messageModel->removeRows(settings.messageBufferSize, messageModel->rowCount() -  settings.messageBufferSize);
    }
}

void GloxiniaConfigurator::processCANDiscoveryMessage(const GMessage &m)
{
    qInfo() << "Received discovery message";

    // if ID is not yet in the system, request info to create node
    if (treeModel->checkUniqueNodeID(m.getMessageAddress()))
    {
        // request node info
        GMessage reply(GMessage::Code::NODE_INFO, m.getMessageAddress(), GMessage::NoInterfaceID, GMessage::NoSensorID, true, std::vector<quint8>());
        emit queueMessage(reply);
    }
}

void GloxiniaConfigurator::processNodeInfoMessage(const GMessage &m)
{
    auto data = m.getData();
    QVariant modelData;
    GCNode *node;

    if (treeModel->checkUniqueNodeID(m.getMessageAddress()))
    {
        // create type of node
        switch ((GCNode::NodeType)data[0])
        {
        case GCNode::NodeType::GCDicio:
            modelData = QVariant::fromValue(new GCNodeDicio(m.getMessageAddress()));
            node = modelData.value<GCNodeDicio *>();
            break;
        case GCNode::NodeType::GCPlanalta:
            modelData = QVariant::fromValue(new GCNodePlanalta(m.getMessageAddress()));
            node = modelData.value<GCNodePlanalta *>();
            break;
        case GCNode::NodeType::GCSylvatica:
            modelData = QVariant::fromValue(new GCNodeSylvatica(m.getMessageAddress()));
            node = modelData.value<GCNodeSylvatica *>();
            break;
        default:
            return;
        }

        // set node info and display notification if there is an issue
        node->setSoftwareVersion(data[2], data[3]);
        node->setHardwareVersion(data[1]);

        QModelIndex index = QModelIndex();
        bool success = this->treeModel->insertRow(treeModel->rowCount(), index);
        if (!success)
        {
            delete node;
            return;
        }
        QModelIndex nodeIndex = this->treeModel->index(treeModel->rowCount() - 1, 0, index);

        this->treeModel->setData(nodeIndex, modelData, Qt::EditRole);

        // add sensors to interfaces
        for (int i = 0; i < node->getNInterfaces(); i++)
        {
            bool success = this->treeModel->insertRow(i, nodeIndex);
            if (!success)
            {
                return;
            }
            QModelIndex interfaceIndex = this->treeModel->index(i, 0, nodeIndex);
            for (int j = 0; j < GCNode::nSensorsPerInterface; j++)
            {
                bool success = this->treeModel->insertRow(j, interfaceIndex);
                if (!success)
                {
                    return;
                }

                GCSensor *sensorData = nullptr;

                // send a message to detect existing sensors
                GMessage sensor_request(GMessage::Code::SENSOR_CONFIG, m.getMessageAddress(), i, j, true, std::vector<quint8>());
                emit queueMessage(sensor_request);

                this->treeModel->setData(interfaceIndex, QVariant::fromValue(sensorData), Qt::EditRole);
            }
        }
    }
}

void GloxiniaConfigurator::processTextMessage(const GMessage &m)
{
}

void GloxiniaConfigurator::processSensorData(const GMessage& m)
{
    GCSensor* sensor = treeModel->getSensor(m.getMessageAddress(), m.getInterfaceID(), m.getSensorID());
    if(sensor == nullptr)
        return;
    auto data = m.getData();
    sensor->saveData(data);
    qDebug() << m.toLogString();
}

void GloxiniaConfigurator::processSensorStatus(const GMessage& m)
{
    // check if there is any data
    if(m.getData().size() < 1)
    {
        qDebug() << "Sensor status message with insufficient data";
        return;
    }

    // find sensor in the model
    GCSensor* sensor = treeModel->getSensor(m.getMessageAddress(), m.getInterfaceID(), m.getSensorID());

    if(sensor == nullptr)
    {
        qDebug() << "Unable to find sensor at (" << m.getMessageAddress() << ", " << m.getInterfaceID() << ", " << m.getSensorID() << ")";
        return; // TODO: report error here
    }

    // store update
    GCSensor::GCSensorStatus status = (GCSensor::GCSensorStatus) m.getData().at(0);
    sensor->setStatus(status);

    qDebug() << "Updated sensor status at (" << m.getMessageAddress() << ", " << m.getInterfaceID() << ", " << m.getSensorID() << ")";
}

void GloxiniaConfigurator::processSensorConfig(const GMessage &m)
{
    GCSensor* sensor = treeModel->getSensor(m.getMessageAddress(), m.getInterfaceID(), m.getSensorID());
    GCNode* node = treeModel->getNode(m.getMessageAddress());
    bool status = false;

    if(node == nullptr)
        return;

    if(sensor == nullptr)
    {
        QModelIndex index = treeModel->getIndex(m.getMessageAddress(), m.getInterfaceID(), m.getSensorID());
        // create sensor
        switch(m.getData().at(0)){
        case GCSensor::sensor_class::ADC12:
        {
            GCSensorADC12* sensor_adc12 = new GCSensorADC12(node, m.getInterfaceID(), m.getSensorID());
            status = treeModel->setData(index, QVariant::fromValue(sensor_adc12));
            break;
        }
        case GCSensor::sensor_class::ADC16:
        {
            GCSensorADC16* sensor_adc16 = new GCSensorADC16(node, m.getInterfaceID(), m.getSensorID());
            status = treeModel->setData(index, QVariant::fromValue(sensor_adc16));
            break;
        }
        case GCSensor::sensor_class::APDS9306_065:
        {
            GCSensorAPDS9306* sensor_apds9306_065 = new GCSensorAPDS9306(node,m.getInterfaceID(), m.getSensorID());
            status = treeModel->setData(index, QVariant::fromValue(sensor_apds9306_065));
            break;
        }
        case GCSensor::sensor_class::LIA:
        {
            GCSensorLIA* sensor_lia = new GCSensorLIA(node, m.getInterfaceID(), m.getSensorID());
            status = treeModel->setData(index, QVariant::fromValue(sensor_lia));
            break;
        }
        case GCSensor::sensor_class::SHT35:
        {
            GCSensorSHT35* sensor_sht35 = new GCSensorSHT35(node, m.getInterfaceID(), m.getSensorID());
            status = treeModel->setData(index, QVariant::fromValue(sensor_sht35));

            break;
        }
        case GCSensor::sensor_class::NOT_SET:
        default:
            return;
        }

        // new sensor, request status too
        GMessage status_request(GMessage::Code::SENSOR_STATUS, m.getMessageAddress(), m.getInterfaceID(), m.getSensorID(), true, std::vector<quint8>());
        emit queueMessage(status_request);
    } else {
        // update todos
    }

    //

}


void GloxiniaConfigurator::processBootReadVersion(const GMessage&m)
{
    FlashNodeInfo info;

    auto data = m.getData();

    info.bootVersion = (((uint16_t) data[0]) << 8) | (data[1]);
    info.swVersion = (((uint32_t) data[3]) << 24) | (((uint32_t) data[4]) << 16) | (((uint32_t) data[5]) << 8) | (data[6]);
    info.hwVersion = (((uint32_t) data[7]) << 24) | (((uint32_t) data[8]) << 16) | (((uint32_t) data[9]) << 8) | (data[10]);

    info.maxPacketSize = data[2];
    info.eraseRowSize = data[11];
    info.writeRowSize = data[12];

    qInfo() << "Received bootloader info";
    qInfo() << "bootloader version" << info.bootVersion;
    qInfo() << "sw version" << info.swVersion;
    qInfo() << "hw version" << info.hwVersion;
    qInfo() << "other info:" << info.maxPacketSize << info.eraseRowSize << info.writeRowSize;

    updateDialog->setNodeInfo(info);
}
void GloxiniaConfigurator::processBootReadFlash(const GMessage&m)
{

}
void GloxiniaConfigurator::processBootWriteFlash(const GMessage&m)
{
    auto data = m.getData();
    if(data.size() == 5){
        handleBootMessageStatus(m);

        uint32_t address = 0;
        for(int i = 0; i < 4; i++)
            address = (address << 8) | (data[i]);
        address *= 2;

        updateDialog->setFlashProgress(address);
    }
}

void GloxiniaConfigurator::processBootEraseFlash(const GMessage&m)
{
    handleBootMessageStatus(m);
}

void GloxiniaConfigurator::processBootResetDevice(const GMessage&m)
{
    handleBootMessageStatus(m);
}

void GloxiniaConfigurator::processBootSelfVerify(const GMessage&m)
{
    if(handleBootMessageStatus(m))
    {
        updateDialog->setVerification(true);
    } else {
        updateDialog->setVerification(false);
    }
}

void GloxiniaConfigurator::processBootGetMemoryAddressRangeCommand(const GMessage&m)
{
    auto data = m.getData();
    if(handleBootMessageStatus(m) && (data.size() == (5+8)))
    {
        uint32_t address1 = 0, address2 = 0;
        for(int i = 0; i < 4; i++){
            address1 = (address1 << 8) | data[5+i];
        }

        address2 = 0;
        for(int i = 0; i < 4; i++){
            address2 = (address2 << 8) | data[5+4+i];
        }

        // we need to multiply by 2 here since the device address space is in words, but we work in bytes here
        updateDialog->setFlashRange(address1*2, address2*2);
    }
}

void GloxiniaConfigurator::processBootReady(const GMessage&m)
{
    updateDialog->setBootReady(true);
}

bool GloxiniaConfigurator::handleBootMessageStatus(const GMessage &m){
    // get reply and process it
    auto data = m.getData();
    if(data.size() < 5){
        return false;
    }

    switch(data[4])
    {
        case 1:
            qInfo() << "success";
            return true;
            break;
        case 0xff:
            updateDialog->detectedError();
            qInfo() << "Command not supported";
            break;
        case 0xfe:
            updateDialog->detectedError();
            qInfo() << "Bootloader reported address error";
            break;
    case 0xfc:
        qInfo() << "Verification failed";
        default:
            updateDialog->detectedError();
            qInfo() << "Unknown command return received";
            break;
    }
    return false;
}

void GloxiniaConfigurator::processSensorStart(const GMessage&m)
{
    // we don't need to do anything for this message
}

void GloxiniaConfigurator::processConfigDoneFinishedReadout(const GMessage&m)
{
    //GMessage start_measurements(GMessage::Code::SENSOR_START, GMessage::ComputerAddress, GMessage::NoInterfaceID, GMessage::NoSensorID, true);
    //emit queueMessage(start_measurements);
}

void GloxiniaConfigurator::processLoadConfigurationFromSDCard(const GMessage&m)
{
    
}
void GloxiniaConfigurator::processClearConfigurationOnSDCard(const GMessage&m)
{

}

void GloxiniaConfigurator::processDicioTime(const GMessage&m)
{
    if(m.getRequest() && (m.getMessageAddress() == GMessage::ComputerAddress))
    {
        // send current time to device
        QDateTime utc_time(QDateTime::currentDateTimeUtc());
        std::vector<uint8_t> time_data(7, 0);
        time_data[0] = utc_time.date().year()-2000;
        time_data[1] = utc_time.date().month();
        time_data[2] = utc_time.date().day();
        time_data[3] = utc_time.date().dayOfWeek();
        time_data[4] = utc_time.time().hour();
        time_data[5] = utc_time.time().minute();
        time_data[6] = utc_time.time().second();
        GMessage m_time(GMessage::Code::DICIO_TIME, GMessage::ComputerAddress, GMessage::NoInterfaceID, GMessage::NoSensorID, false, time_data);

        emit queueMessage(m_time);
    }
}
