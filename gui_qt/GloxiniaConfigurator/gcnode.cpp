#include "gcnode.h"

#include <QDebug>

GCNode::GCNode(const int nodeID, const QString label)
    : id(nodeID), label(label)
{
    if (id == 0)
    {
        id = getIDNew();
    }

    qInfo() << "Generating node with id" << id << "and label" << label;
}

QDebug operator<<(QDebug dbg, const GCNode &n)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Node(" << n.id << ")";
    return dbg;
}

int GCNode::getID() const
{
    return id;
}

QString GCNode::getLabel() const
{
    return QString(label);
}

void GCNode::setLabel(QString label)
{
    this->label.clear();
    this->label.append(label);
}

void GCNode::setID(const int id)
{
    this->id = id;
}

void GCNode::setSoftwareVersion(uint8_t major, uint8_t minor)
{
    majorSoftwareVersion = major;
    minorSoftwareVersion = minor;
}
void GCNode::setHardwareVersion(uint8_t v)
{
    hardwareVersion = v;
}

QString GCNode::toString(void) const
{
    return label + " [" + QString::number(id) + "]";
}

int GCNode::getIDNew()
{
    static int idCounter = 0;
    return idCounter++;
}

QString GCNode::toConfigString(void) const
{
    QString typeLabel;
    /*switch(type)
    {
    case GCDicio:
        typeLabel = "D";
        break;
    case GCSylvatica:
        typeLabel = "S";
        break;
    case GCPlanalta:
        typeLabel = "P";
        break;
    }*/

    QString nodeLabel = label;
    nodeLabel.replace(' ', "\\ ");

    return "N " + typeLabel + " " + QString::number(id) + " " + nodeLabel + " ;";
}
bool GCNode::fromConfigString(const QStringList &config)
{
    int id = config[2].toInt();
    QString label = config[3];

    GCNode::NodeType t;
    if (config[1] == "D")
    {
        t = GCNode::NodeType::GCDicio;
    }
    else if (config[1] == "S")
    {
        t = GCNode::NodeType::GCSylvatica;
    }
    else if (config[1] == "P")
    {
        t = GCNode::NodeType::GCPlanalta;
    }
    else
    {
        return false;
    }

    this->label = label;
    this->id = id;

    return true;
}

GCNodeDicio::GCNodeDicio(const int nodeID, const QString label) : GCNode(nodeID, label)
{
}

const unsigned int GCNodeDicio::getNInterfaces(void) const
{
    return 4;
}

GCNodePlanalta::GCNodePlanalta(const int nodeID, const QString label) : GCNode(nodeID, label)
{
}

const unsigned int GCNodePlanalta::getNInterfaces(void) const
{
    return 4;
}

GCNodeSylvatica::GCNodeSylvatica(const int nodeID, const QString label) : GCNode(nodeID, label)
{
}

const unsigned int GCNodeSylvatica::getNInterfaces(void) const
{
    return 8;
}
