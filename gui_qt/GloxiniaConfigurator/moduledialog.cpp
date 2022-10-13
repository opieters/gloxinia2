#include "moduledialog.h"
#include "ui_moduledialog.h"


ModuleDialog::ModuleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModuleDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowMaximizeButtonHint);

    ui->idEdit->setValidator(new QIntValidator(0, 100, this));

    connect(ui->okBox, &QDialogButtonBox::accepted, this, &ModuleDialog::apply);
    connect(ui->idBox, &QCheckBox::stateChanged, this, &ModuleDialog::updateAutoID);

    ui->idBox->setCheckState(Qt::Checked);

    updateUISettings();
}

ModuleDialog::~ModuleDialog()
{
    delete ui;
}


void ModuleDialog::updateUISettings()
{
    ui->typeBox->setCurrentIndex(nodeConfig.getType());
    if(nodeConfig.getID() == 0){
        ui->idBox->setCheckState(Qt::Checked);
    }
    ui->idEdit->setText(QString::number(nodeConfig.getID()));
    ui->altEdit->setText(nodeConfig.getLabel());
}
void ModuleDialog::setNodeSettings(GCNode& s)
{
    nodeConfig = GCNode(s);
}

void ModuleDialog::apply()
{
    nodeConfig.setID(ui->idEdit->displayText().toInt(nullptr, 0));
    nodeConfig.setLabel(ui->altEdit->displayText());
    nodeConfig.setType((GCNode::NodeType) ui->typeBox->currentIndex());
}

GCNode ModuleDialog::getNode()
{
    return GCNode(nodeConfig);
}

void ModuleDialog::updateAutoID(void)
{
    if(ui->idBox->isChecked()){
        ui->idEdit->setText(QString("0"));
        ui->idEdit->setDisabled(true);
    } else {
        ui->idEdit->setDisabled(false);
    }
}
