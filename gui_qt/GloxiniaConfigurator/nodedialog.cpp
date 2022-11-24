#include "nodedialog.h"
#include "ui_NodeDicioDialog.h"

NodeDicioDialog::NodeDicioDialog(QWidget *parent) : QDialog(parent),
                                                    ui(new Ui::NodeDicioDialog),
                                                    nodeConfig(nullptr)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    ui->idEdit->setValidator(new QIntValidator(0, 100, this));

    connect(ui->okBox, &QDialogButtonBox::accepted, this, &NodeDicioDialog::apply);
    connect(ui->idBox, &QCheckBox::stateChanged, this, &NodeDicioDialog::updateAutoID);

    ui->idBox->setCheckState(Qt::Checked);

    updateUISettings();
}

NodeDicioDialog::~NodeDicioDialog()
{
    delete ui;
    if (nodeConfig != nullptr)
        delete nodeConfig;
}

void NodeDicioDialog::updateUISettings()
{
    ui->typeBox->setCurrentIndex(nodeConfig->getType());
    if (nodeConfig->getID() == 0)
    {
        ui->idBox->setCheckState(Qt::Checked);
    }
    ui->idEdit->setText(QString::number(nodeConfig->getID()));
    ui->altEdit->setText(nodeConfig->getLabel());
}
void NodeDicioDialog::setNodeSettings(GCNode *s)
{
    if (nodeConfig != nullptr)
    {
        delete nodeConfig;
    }
    nodeConfig = s;
    updateUISettings();
}

void NodeDicioDialog::apply()
{
    nodeConfig->setID(ui->idEdit->displayText().toInt(nullptr, 0));
    nodeConfig->setLabel(ui->altEdit->displayText());
    nodeConfig->setType((GCNode::NodeType)ui->typeBox->currentIndex());
}

GCNode *NodeDicioDialog::getNode()
{
    return new GCNode(*nodeConfig);
}

void NodeDicioDialog::updateAutoID(void)
{
    if (ui->idBox->isChecked())
    {
        ui->idEdit->setText(QString("0"));
        ui->idEdit->setDisabled(true);
    }
    else
    {
        ui->idEdit->setDisabled(false);
    }
}
