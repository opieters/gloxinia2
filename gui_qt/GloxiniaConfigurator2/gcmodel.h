#ifndef GCMODEL_H
#define GCMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <gcsensor.h>
#include <gcnode.h>

class GCSystem {
public:
    explicit GCSystem();
    ~GCSystem();



    void appendNode(GCNode* n);
    bool insertRow(int position, GCNode* n);
    GCNode* removeNode(int position);
    GCNode* getNode(int row) const;
    int nodeCount() const;
private:
    QList<GCNode*> nodes;
};

class GCModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    GCModel(QObject* parent);

    ~GCModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    //Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

private:
    GCSystem* root;
};



#endif // GCNODE_H
