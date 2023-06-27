#include "gcnode.h"
#include "qvariant.h"

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

GCNode* GCNode::fromQVariant(const QVariant data)
{
    GCNodeDicio *nDicio = data.value<GCNodeDicio *>();
    GCNodePlanalta *nPlanalta = data.value<GCNodePlanalta *>();
    GCNodeSylvatica *nSylvatica = data.value<GCNodeSylvatica *>();

    if(nDicio != nullptr)
        return nDicio;
    if(nPlanalta != nullptr)
        return nPlanalta;
    if(nSylvatica != nullptr)
        return nSylvatica;

    return nullptr;
}

GCNodeDicio::GCNodeDicio(const int nodeID, const QString label) : GCNode(nodeID, label)
{
}
GCNodeDicio::GCNodeDicio(GCNodeDicio& c) : GCNode(c.id, c.label)
{

}

const unsigned int GCNodeDicio::getNInterfaces(void) const
{
    return 4;
}

const  GCNode::BootLoaderNodeID GCNodeDicio::getBootLloaderID(void) const{
    return GCNode::BootLoaderNodeID::Dicio;
}

GCNodePlanalta::GCNodePlanalta(const int nodeID, const QString label) : GCNode(nodeID, label)
{
}
GCNodePlanalta::GCNodePlanalta(GCNodePlanalta& c) : GCNode(c.id, c.label)
{
    liaEngine = c.liaEngine;
}

GCNodePlanalta::~GCNodePlanalta()
{

}

const unsigned int GCNodePlanalta::getNInterfaces(void) const
{
    return 4;
}

GLIAEngine GCNodePlanalta::getLIAEngine(void) const
{
    return GLIAEngine(liaEngine);
}

void GCNodePlanalta::setLIAEngine(const GLIAEngine& engine)
{
    liaEngine = engine;
}

const  GCNode::BootLoaderNodeID GCNodePlanalta::getBootLloaderID(void) const{
    return GCNode::BootLoaderNodeID::Planalta;
}

GCNodeSylvatica::GCNodeSylvatica(const int nodeID, const QString label) : GCNode(nodeID, label)
{
}

GCNodeSylvatica::GCNodeSylvatica(GCNodeSylvatica& c) : GCNode(c.id, c.label)
{

}

const unsigned int GCNodeSylvatica::getNInterfaces(void) const
{
    return 8;
}

const  GCNode::BootLoaderNodeID GCNodeSylvatica::getBootLloaderID(void) const{
    return GCNode::BootLoaderNodeID::Sylvatica;
}
