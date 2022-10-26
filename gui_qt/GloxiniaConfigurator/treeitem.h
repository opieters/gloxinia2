#ifndef TREEITEM_H
#define TREEITEM_H

#include <QVariant>
#include <QList>
#include <QIcon>


class TreeItem
{
public:
    enum NodeType {
        Root,
        Node,
        Sensor,
    };

    explicit TreeItem(const QVariant &data, NodeType nType, TreeItem *parent = nullptr);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data() const;
    bool insertChildren(int position, int count, int columns);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    int childNumber() const;
    bool setData(const QVariant &value);
    QIcon getIcon() const;
    NodeType getNType();

private:
    QList<TreeItem *> childItems;
    QVariant itemData;
    TreeItem *parentItem;
    NodeType nType;
};


#endif // TREEITEM_H
