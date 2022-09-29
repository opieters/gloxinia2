#ifndef GSYSTEM_H
#define GSYSTEM_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <gcnode.h>

class GCSystem : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit GCSystem(QObject *parent = nullptr);
    ~GCSystem();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    //QVariant headerData(int section, Qt::Orientation orientation,
    //                    int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool removeRows(int position, int rows, const QModelIndex &parent) override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int position, int rows, const QModelIndex &parent) override;
    GCNode *getNode(const QModelIndex &index) const;

private:
    //void setupModelData(const QStringList &lines, TreeItem *parent);

    GCNode* rootNode;
};

#endif // GSYSTEM_H
