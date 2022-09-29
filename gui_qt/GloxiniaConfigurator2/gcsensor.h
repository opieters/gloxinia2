#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include <QAbstractItemModel>
#include <QString>

class GCNode;

class GCSensor
{
    enum SensorType {
        Disabeled = 0,
        SHT35 = 1,
        APDS9306 = 2
    };

public:
    GCSensor(QString label, GCNode *parent = nullptr);

private:
    QString sensorLabel;
    GCNode* parent;

};

/*
class SensorModel : public QAbstractItemModel
{
    enum SensorType {
        Disabeled = 0,
        SHT35 = 1,
        APDS9306 = 2
    };

public:
    SensorModel(QString label, QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QString sensorLabel;


};*/

#endif // SENSORMODEL_H
