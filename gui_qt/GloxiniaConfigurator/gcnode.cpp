#include "gcnode.h"

#include <QDebug>

GCNode::GCNode(const int nodeID, const NodeType type)
    : id(nodeID), type(type)
{
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


