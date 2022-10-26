#include "nodedialog.h"
#include "ui_nodedialog.h"


NodeDialog::NodeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeDialog),
    nodeConfig(new GCNode())
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowMaximizeButtonHint);

    ui->idEdit->setValidator(new QIntValidator(0, 100, this));

    connect(ui->okBox, &QDialogButtonBox::accepted, this, &NodeDialog::apply);
    connect(ui->idBox, &QCheckBox::stateChanged, this, &NodeDialog::updateAutoID);

    ui->idBox->setCheckState(Qt::Checked);

    updateUISettings();
}

NodeDialog::~NodeDialog()
{
    delete ui;
    delete nodeConfig;
}


void NodeDialog::updateUISettings()
{
    ui->typeBox->setCurrentIndex(nodeConfig->getType());
    if(nodeConfig->getID() == 0){
        ui->idBox->setCheckState(Qt::Checked);
    }
    ui->idEdit->setText(QString::number(nodeConfig->getID()));
    ui->altEdit->setText(nodeConfig->getLabel());
}
void NodeDialog::setNodeSettings(GCNode* s)
{
    delete nodeConfig;
    nodeConfig = new GCNode(*s);
    updateUISettings();
}

void NodeDialog::apply()
{
    nodeConfig->setID(ui->idEdit->displayText().toInt(nullptr, 0));
    nodeConfig->setLabel(ui->altEdit->displayText());
    nodeConfig->setType((GCNode::NodeType) ui->typeBox->currentIndex());
}

GCNode* NodeDialog::getNode()
{
    return new GCNode(*nodeConfig);
}

void NodeDialog::updateAutoID(void)
{
    if(ui->idBox->isChecked()){
        ui->idEdit->setText(QString("0"));
        ui->idEdit->setDisabled(true);
    } else {
        ui->idEdit->setDisabled(false);
    }
}
