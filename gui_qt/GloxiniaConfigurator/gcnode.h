#ifndef GCNODE_H
#define GCNODE_H

#include <QMetaType>
#include <QStringList>

class GCNode
{
enum NodeType {
    GCSystem,
    GCModule,
    GCSensor,
};

public:
    GCNode() = default;
    ~GCNode() = default;
    GCNode(const GCNode&) = default;
    GCNode &operator=(const GCNode&) = default;

    GCNode(const int nodeID, const NodeType type);

    int getID() const;
    NodeType getType() const;

friend QDebug operator<<(QDebug dbg, const GCNode&);
private:
    int id;
    NodeType type;

};

Q_DECLARE_METATYPE(GCNode);


#endif
