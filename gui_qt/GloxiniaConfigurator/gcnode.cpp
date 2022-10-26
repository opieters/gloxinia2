#include "gcnode.h"

#include <QDebug>

GCNode::GCNode(const int nodeID, const NodeType type, const QString label)
    : id(nodeID), type(type), label(label)
{
    if(id == 0){
        id = getIDNew();
    }

    qInfo() << "Generating node with id " << id;

}


QDebug operator<<(QDebug dbg, const GCNode &n)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Node(" << n.id << n.type << ")";
    return dbg;
}

int GCNode::getID() const
{
    return id;
}

GCNode::NodeType GCNode::getType() const
{
    return type;
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

void GCNode::setType(const NodeType type)
{
    this->type = type;
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
    switch(type)
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
    }

    QString nodeLabel = label;
    nodeLabel.replace(' ', "\\ ");

    return "N "
            + typeLabel + " "
            + QString::number(id) + " "
            + nodeLabel + " ;";
}
bool GCNode::fromConfigString(const QStringList& config)
{
    int id = config[2].toInt();
    QString label = config[3];

    GCNode::NodeType t;
    if(config[1] == "D")
    {
        t = GCNode::NodeType::GCDicio;
    } else if(config[1] == "S")
    {
        t = GCNode::NodeType::GCSylvatica;
    } else if(config[1] == "P")
    {
         t = GCNode::NodeType::GCPlanalta;
    } else {
        return false;
    }

    this->label = label;
    this->id = id;
    this->type = t;

    return true;
}

