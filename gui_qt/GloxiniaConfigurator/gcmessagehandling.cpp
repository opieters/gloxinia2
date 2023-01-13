#include "gloxiniaconfigurator.h"

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
        default:
            break;
        }
    }

    // add line for now
    messageModel->insertRow(0);
    QModelIndex mIndex = messageModel->index(0, 0);
    messageModel->setData(mIndex, m.toLogString());
    if(messageModel->rowCount() > applicationSettings.messageBufferSize){
        messageModel->removeRows(applicationSettings.messageBufferSize, messageModel->rowCount() -  applicationSettings.messageBufferSize);
    }
}

void GloxiniaConfigurator::processCANDiscoveryMessage(const GMessage &m)
{
    qInfo() << "Received discovery message";

    // if ID is not yet in the system, request info to create node
    if (treeModel->checkUniqueNodeID(m.getMessageID()))
    {

        // request node info
        quint8 rawData[32];
        unsigned int length;

        GMessage reply(GMessage::Code::NODE_INFO, m.getMessageID(), GMessage::NoSensorID, true, std::vector<quint8>());

        length = reply.toBytes(rawData, 32);
        serial->write((char *)rawData, length);
    }
}

void GloxiniaConfigurator::processNodeInfoMessage(const GMessage &m)
{
    auto data = m.getData();
    QVariant modelData;
    GCNode *node;

    if (treeModel->checkUniqueNodeID(m.getMessageID()))
    {
        // create type of node
        switch ((GCNode::NodeType)data[0])
        {
        case GCNode::NodeType::GCDicio:
            modelData = QVariant::fromValue(new GCNodeDicio(m.getMessageID()));
            node = modelData.value<GCNodeDicio *>();
            break;
        case GCNode::NodeType::GCPlanalta:
            modelData = QVariant::fromValue(new GCNodePlanalta(m.getMessageID()));
            node = modelData.value<GCNodePlanalta *>();
            break;
        case GCNode::NodeType::GCSylvatica:
            modelData = QVariant::fromValue(new GCNodeSylvatica(m.getMessageID()));
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
        QModelIndex child = this->treeModel->index(treeModel->rowCount() - 1, 0, index);

        this->treeModel->setData(child, modelData, Qt::EditRole);

        // add sensors
        for (int i = 0; i < node->getNInterfaces(); i++)
        {
            bool success = this->treeModel->insertRow(i, child);
            if (!success)
            {
                return;
            }

            GCSensor *interfaceData = nullptr;

            QModelIndex interface = this->treeModel->index(treeModel->rowCount(child) - 1, 0, child);
            this->treeModel->setData(interface, QVariant::fromValue(interfaceData), Qt::EditRole);
        }
    }
}

void GloxiniaConfigurator::processTextMessage(const GMessage &m)
{
}

void GloxiniaConfigurator::processSensorData(const GMessage& m)
{
    GCSensor* sensor = treeModel->getSensor(m.getMessageID(), m.getSensorID());
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
    GCSensor* sensor = treeModel->getSensor(m.getMessageID(), m.getSensorID());

    if(sensor == nullptr)
    {
        qDebug() << "Unable to find sensor at (" << m.getMessageID() << "," << m.getSensorID() << ")";
        return; // TODO: report error here
    }

    // store update
    GCSensor::GCSensorStatus status = (GCSensor::GCSensorStatus) m.getData().at(0);
    sensor->setStatus(status);

    qDebug() << "Updated sensor status at (" << m.getMessageID() << "," << m.getSensorID() << ")";
}


void GloxiniaConfigurator::sendSerialMessage(const GMessage &m)
{
    unsigned int length;
    quint8 rawData[32];

    length = m.toBytes(rawData, 32);
    serial->write((char *)rawData, length);
    serial->flush();
}
