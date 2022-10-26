#ifndef GCNODE_H
#define GCNODE_H

#include <QMetaType>
#include <QStringList>

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

    GCNode(const int nodeID, const NodeType type, const QString label="");

    int getID() const;
    void setID(const int id);
    NodeType getType() const;
    void setType(const NodeType type);
    QString getLabel() const;
    void setLabel(QString label);

    friend QDebug operator<<(QDebug dbg, const GCNode&);

    QString toString(void) const;

    QString toConfigString(void) const;
    bool fromConfigString(const QStringList& config);

protected:
    int id;
    NodeType type;
    QString label;

private:
    static int getIDNew();

};

Q_DECLARE_METATYPE(GCNode*);


#endif
