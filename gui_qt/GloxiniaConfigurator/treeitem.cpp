#include "treeitem.h"
#include "gcsensor.h"
#include "gcnode.h"

TreeItem::TreeItem(const QVariant &data, TreeItem::NodeType nType, TreeItem *parent)
    : itemData(data), nType(nType), parentItem(parent)
{}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

TreeItem *TreeItem::child(int number)
{
    if (number < 0 || number >= childItems.size())
        return nullptr;
    return childItems.at(number);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}


int TreeItem::columnCount() const
{
    return 1;
}



QVariant TreeItem::data() const
{
    return itemData;
/*
    GCSensor* s;
    GCNode* n;
    switch(nType)
    {
        case NodeType::Sensor:
            s = itemData.value<GCSensor*>();
            if(s != nullptr){
                return s->toString();
            }
            break;
        case NodeType::Root:
            return "Root Node";
            break;
        case NodeType::Node:
            n = itemData.value<GCNode*>();
            if(n != nullptr){
                return n->toString();
            }
            break;
    }
    return "[no data]";
    return itemData.toString();*/
}



bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size() || nType == TreeItem::Sensor)
        return false;

    for (int row = 0; row < count; ++row) {
        QList<QVariant> data(columns);
        TreeItem *item;
        switch(nType){
        case TreeItem::Interface:
            item = new TreeItem(data, TreeItem::Sensor, this);
            break;
        case TreeItem::Sensor:
            return false;
            break;
        case TreeItem::Root:
            item = new TreeItem(data, TreeItem::Node, this);
            break;
        case TreeItem::Node:
            item = new TreeItem(data, TreeItem::Interface, this);
            break;
        }
        childItems.insert(position, item);
    }

    return true;
}




TreeItem *TreeItem::parent()
{
    return parentItem;
}



bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}



bool TreeItem::setData(const QVariant &value)
{
    itemData = value;
    return true;
}


QIcon TreeItem::getIcon() const
{
    switch(nType){
    case TreeItem::Root:
        return QIcon(":/images/node.png");
    case TreeItem::Node:
        return QIcon(":/images/cube-solid.png");
    case TreeItem::Interface:
        return QIcon(":/images/unknowm-sensor.png");
    case TreeItem::Sensor:
        return QIcon(":/images/unknown-sensor.png");
    }

    return QIcon(":/images/node.png");
}

TreeItem::NodeType TreeItem::getNType()
{
    return nType;
}
