#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <gcsensor.h>

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(QObject *parent = nullptr);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;

    QStringList toTextConfig(void);
    bool fromTextConfig(const QStringList& config);

    bool checkUniqueNodeID(int id);

    GCSensor* getSensor(int nodeID, int sensorID);

private:
    void setupModelData(const QStringList &lines, TreeItem *parent);
    void loadFile(const QStringList &lines, TreeItem *parent);
    bool parseNode(const QStringList &args, TreeItem* parent);
    bool parseSensor(const QStringList &args, TreeItem* parent);

    TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *rootItem;
};


#endif // TREEMODEL_H
