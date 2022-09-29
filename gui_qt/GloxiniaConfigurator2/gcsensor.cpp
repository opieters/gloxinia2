#include "gcsensor.h"

GCSensor::GCSensor(QString label, GCNode *parent): sensorLabel(label), parent(parent)
{

}

/*
QModelIndex GCSensor::index(int row, int column, const QModelIndex &parent) const
{

}
QModelIndex GCSensor::parent(const QModelIndex &index) const
{

}
int GCSensor::rowCount(const QModelIndex &parent) const
{
    return 0;
}
int GCSensor::columnCount(const QModelIndex &parent) const
{

}
QVariant GCSensor::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= 0)
        return QVariant();

    if (role == Qt::DisplayRole)
        return "TODO";
    else
        return QVariant();
}
*/
