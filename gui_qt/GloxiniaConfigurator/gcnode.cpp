#include "gcnode.h"

#include <QDebug>

GCNode::GCNode(const int nodeID, const NodeType type)
    : id(nodeID), type(type)
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

QString GCNode::toString(void) const{
    return label + " [" + QString::number(id) + "]";
}

int GCNode::getIDNew()
{
    static int idCounter = 0;
    return idCounter++;
}
