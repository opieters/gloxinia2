#ifndef GCNODE_H
#define GCNODE_H

#include <QObject>
#include <QAbstractItemModel>
#include <gcsensor.h>

class GCSystem;

class GCNode
{

public:
    GCNode(GCSystem* parent);

    /*QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex getSystem(const QModelIndex &index) const;
    int sensorCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;*/

private:
    QList<GCSensor> sensors;

    GCSystem *parent;
};

#endif // GCNODE_H
