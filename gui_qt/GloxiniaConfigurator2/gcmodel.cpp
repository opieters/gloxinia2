#include "gcmodel.h"

GCSystem::GCSystem()
{

}

GCSystem::~GCSystem(){

}

void GCSystem::appendNode(GCNode* n)
{
    nodes.append(n);
}


bool GCSystem::insertRow(int position, GCNode* n){
    nodes.insert(position, n);
    return true;
}

GCNode* GCSystem::removeNode(int position){
    GCNode* removedNode = nodes.at(position);
    nodes.remove(position);
    return removedNode;
}



GCNode* GCSystem::getNode(int row) const
{
    if(row < 0 || row >= nodes.size()){
        return nullptr;
    }

    return nodes.at(row);
}
int GCSystem::nodeCount() const
{
    return nodes.count();
}

/**
 * GCModel code
 */

GCModel::GCModel(QObject* parent):QAbstractItemModel(parent){

}

GCModel::~GCModel(){

}


/*Qt::ItemFlags GCModel::flags(const QModelIndex &index) const
{

}*/

bool GCModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return true;
}




QModelIndex GCModel::index(int row, int column, const QModelIndex &parent) const
{
    return QModelIndex();
}

QModelIndex GCModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int GCModel::rowCount(const QModelIndex &parent) const
{
    GCSystem* rootEl;

    if(parent.column() > 0){
        return 0;
    }

    if(!parent.isValid()){
        rootEl = root;
    } else {
        rootEl = static_cast<GCSystem*>(parent.internalPointer());
    }

    return rootEl->nodeCount();
}

int GCModel::columnCount(const QModelIndex &parent) const
{
    /*if (parent.isValid())
        return static_cast<GCSystem*>(parent.internalPointer())->nodeCount();
    return root->nodeCount();*/
    return 1;
}

QVariant GCModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()){
        return QVariant();
    }

    if(role != Qt::DisplayRole && role != Qt::EditRole){
        return QVariant();
    }

    qInfo() << "Data reques called";

    return QString("Default");
}

bool GCModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for(int row = 0; row < rows; row++){
        qInfo() << "Inserting row " << row;
        root->insertRow(row, new GCNode(root));
    }
    endInsertRows();

    return true;
}

bool GCModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        GCNode* node = root->removeNode(row);
        if(node != nullptr){
            free(node);
        }
    }

    endRemoveRows();
    return true;
}
