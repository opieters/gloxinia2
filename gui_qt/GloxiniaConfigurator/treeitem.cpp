/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include "treeitem.h"

//! [0]
TreeItem::TreeItem(const QVariant &data, TreeItem *parent)
    : itemData(data), parentItem(parent)
{}
//! [0]

//! [1]
TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}
//! [1]

//! [2]
TreeItem *TreeItem::child(int number)
{
    if (number < 0 || number >= childItems.size())
        return nullptr;
    return childItems.at(number);
}
//! [2]

//! [3]
int TreeItem::childCount() const
{
    return childItems.count();
}
//! [3]

//! [4]
int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}
//! [4]

//! [5]
int TreeItem::columnCount() const
{
    return 1;
}
//! [5]

//! [6]
QVariant TreeItem::data(int column) const
{
    return itemData;
}
//! [6]

//! [7]
bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QList<QVariant> data(columns);
        TreeItem *item = new TreeItem(data, this);
        childItems.insert(position, item);
    }

    return true;
}
//! [7]


//! [9]
TreeItem *TreeItem::parent()
{
    return parentItem;
}
//! [9]

//! [10]
bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}
//! [10]

//! [11]
bool TreeItem::setData(const QVariant &value)
{
    itemData = value;
    return true;
}
//! [11]
