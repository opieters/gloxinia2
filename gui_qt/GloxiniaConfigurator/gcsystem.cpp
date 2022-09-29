#include "gcsystem.h"

GCSystem::GCSystem(QObject *parent) : QAbstractItemModel(parent)
{
    rootNode = new GCNode();
    rootNode->insertChildren(0, 1, 0);
}

GCSystem::~GCSystem()
{
    delete rootNode;
}

int GCSystem::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant GCSystem::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    //TreeItem *item = getItem(index);

    //return item->data(index.column());

    return "No Data Here";
}

Qt::ItemFlags GCSystem::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}


QModelIndex GCSystem::index(int row, int column, const QModelIndex &parent) const
{
    // column must be zero
    if (parent.isValid() && parent.column() != 0 && column != 0)
        return QModelIndex();

    GCNode* n = getNode(parent);
    if(n == nullptr){
        return QModelIndex();
    }

    n = n->getChild(row);
    if(n != nullptr){
        return createIndex(row, column, n);
    }

    return QModelIndex();
}

GCNode *GCSystem::getNode(const QModelIndex &index) const
{
    if (index.isValid()) {
        GCNode *item = static_cast<GCNode*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootNode;
}

bool GCSystem::insertRows(int position, int rows, const QModelIndex &parent)
{
    GCNode* n = static_cast<GCNode*>(parent.internalPointer());
    if(n == nullptr)
    {
        return false;
    }

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = n->insertChildren(position, rows, 0);
    endInsertRows();

    return success;
}


QModelIndex GCSystem::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    GCNode* n = static_cast<GCNode*>(index.internalPointer());
    n = (n != nullptr)? n->getParent() : nullptr;

    if(n != nullptr){
        return createIndex(n->childNumber(), 0, n);
    }

    return QModelIndex();
}

bool GCSystem::removeRows(int position, int rows, const QModelIndex &parent)
{
    /*TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();*/

    // return success;
    return true;
}

//! [8]
int GCSystem::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > 0)
        return 0;

    GCNode* n = static_cast<GCNode*>(parent.internalPointer());

    return n != nullptr ? n->childNumber() : 0;
}
//! [8]

bool GCSystem::setData(const QModelIndex &index, const QVariant &value, int role)
{
    /*if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return result;*/
    return false;
}


