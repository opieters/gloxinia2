#ifndef GCNODE_H
#define GCNODE_H

#include <QMetaType>
#include <QStringList>

class GCElement {
public:

};

class GCNode
{
public:
    enum NodeType {
        GCDicio,
        GCPlanalta,
        GCSylvatica,
    };

    GCNode() = default;
    ~GCNode() = default;
    GCNode(const GCNode&) = default;
    GCNode &operator=(const GCNode&) = default;

    GCNode(const int nodeID, const NodeType type);

    int getID() const;
    void setID(const int id);
    NodeType getType() const;
    void setType(const NodeType type);
    QString getLabel() const;
    void setLabel(QString label);

    friend QDebug operator<<(QDebug dbg, const GCNode&);

    QString toString(void) const;

protected:
    int id;
    NodeType type;
    QString label;

private:
    static int getIDNew();

};

Q_DECLARE_METATYPE(GCNode);


#endif
