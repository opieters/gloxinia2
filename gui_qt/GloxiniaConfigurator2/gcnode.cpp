#include "gcnode.h"

GCNode::GCNode(GCSystem* parent) : parent(parent)
{

}
/*
QModelIndex GCNode::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    GCSystem *parentItem;

    if (!parent.isValid())
        parentItem = parent;
    else
        parentItem = static_cast<GCSystem*>(parent.internalPointer());

    GCNode *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}
QModelIndex GCNode::parent(const QModelIndex &index) const
{

}
int GCNode::rowCount(const QModelIndex &parent) const
{

}
int GCNode::columnCount(const QModelIndex &parent) const
{

}
QVariant GCNode::data(const QModelIndex &index, int role) const
{

}
*/
