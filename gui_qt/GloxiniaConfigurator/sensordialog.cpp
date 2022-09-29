#include "sensordialog.h"
#include "ui_sensordialog.h"

#include <QStandardItemModel>
#include <Qt>

SensorDialog::SensorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorDialog)
{
    ui->setupUi(this);

    // add unsellectable item it sensorSelect
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->sensorSelect->model());
    QModelIndex firstIndex = model->index(0, ui->sensorSelect->modelColumn(), ui->sensorSelect->rootModelIndex());
    QStandardItem* firstItem = model->itemFromIndex(firstIndex);
    firstItem->setSelectable(false);
    firstItem->setEnabled(false);
    firstItem->setData(QBrush(Qt::gray), Qt::ForegroundRole);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SensorDialog::add);
}

SensorDialog::~SensorDialog()
{
    delete ui;
}

void SensorDialog::updateSensorSelect(void){
    int idx = ui->sensorSelect->currentIndex();

    switch(idx){
    case 1:
        break;
    default:
        break;
    }
}

GCSensor* SensorDialog::add(void)
{
    hide();

    int idx = ui->sensorSelect->currentIndex();
    qInfo() << idx;
    GCSensor *sensor = new GCSensor();

    switch(idx)
    {
        case 0:

            break;
        case 1:
            // SHT35
        default:
            // unknown sensor
            break;
    }

    return sensor;
}
