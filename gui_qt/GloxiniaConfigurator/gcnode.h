#ifndef GCNODE_H
#define GCNODE_H

#include <QMetaType>
#include <QStringList>

class GCNode
{
public:
    enum NodeType
    {
        GCUnknown = 0x00,
        GCDicio = 0x01,
        GCPlanalta = 0x02,
        GCSylvatica = 0x03,
    };

    GCNode() = default;
    ~GCNode() = default;
    GCNode(const GCNode &) = default;
    GCNode &operator=(const GCNode &) = default;

    GCNode(const int nodeID, const QString label);

    int getID() const;
    void setID(const int id);
    NodeType getType() const;
    void setType(const NodeType type);
    QString getLabel() const;
    void setLabel(QString label);
    void setSoftwareVersion(uint8_t major, uint8_t minor);
    void setHardwareVersion(uint8_t v);

    static GCNode* fromQVariant(const QVariant data);

    friend QDebug operator<<(QDebug dbg, const GCNode &);

    QString toString(void) const;

    QString toConfigString(void) const;
    bool fromConfigString(const QStringList &config);

    virtual const unsigned int getNInterfaces(void) const = 0;

protected:
    int id;
    QString label;

    uint8_t majorSoftwareVersion = 0;
    uint8_t minorSoftwareVersion = 0;
    uint8_t hardwareVersion = 0;

private:
    static int getIDNew();
};

Q_DECLARE_METATYPE(GCNode *);

class GCNodeDicio : public GCNode
{
public:
    GCNodeDicio(const int nodeID, const QString label = "dicio node");

    const unsigned int getNInterfaces(void) const override;

protected:
};

Q_DECLARE_METATYPE(GCNodeDicio *);

class GCNodePlanalta : public GCNode
{
public:
    GCNodePlanalta(const int nodeID, const QString label = "planalta node");

    const unsigned int getNInterfaces(void) const override;

protected:
};

Q_DECLARE_METATYPE(GCNodePlanalta *);

class GCNodeSylvatica : public GCNode
{
public:
    GCNodeSylvatica(const int nodeID, const QString label = "sylvatica node");

    const unsigned int getNInterfaces(void) const override;

protected:
};

Q_DECLARE_METATYPE(GCNodeSylvatica *);

#endif
