#include "gloxiniaconfigurator.h"
#include "./ui_gloxiniaconfigurator.h"

GloxiniaConfigurator::GloxiniaConfigurator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GloxiniaConfigurator)
    , systemModel(GCModel(nullptr))

{
    ui->setupUi(this);

    //systemModel.insertRow(0, QModelIndex());
    //systemModel.insertRow(1, QModelIndex());
    //systemModel.insertRow(2, QModelIndex());
    ui->systemOverview->setModel(&systemModel);
    ui->systemOverview->setHeaderHidden(true);

    connect(ui->actionAddNode, &QAction::triggered, this, &GloxiniaConfigurator::addNode);

}

GloxiniaConfigurator::~GloxiniaConfigurator()
{
    delete ui;
}

void GloxiniaConfigurator::addNode(){
    const QModelIndex index = ui->systemOverview->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->systemOverview->model();

    if(!model->insertRow(index.row()+1, index.parent())){
        return;
    }

    qInfo() << "Added a new node.";

    // update actions??

    // update data of node
}
