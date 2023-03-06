#include "nodediciodialog.h"
#include "ui_nodediciodialog.h"

NodeDicioDialog::NodeDicioDialog(QWidget *parent) : QDialog(parent),
                                                    ui(new Ui::NodeDicioDialog),
                                                    config(new GCNodeDicio(0))
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    // ui->idEdit->setValidator(new QIntValidator(0, 100, this));

    connect(ui->okBox, &QDialogButtonBox::accepted, this, &NodeDicioDialog::apply);
    // connect(ui->idBox, &QCheckBox::stateChanged, this, &NodeDicioDialog::updateAutoID);

    // ui->idBox->setCheckState(Qt::Checked);
    ui->idEdit->setDisabled(true);

    updateUISettings();
}

NodeDicioDialog::~NodeDicioDialog()
{
    delete ui;
    if (config != nullptr)
        delete config;
}

void NodeDicioDialog::updateUISettings()
{
    // ui->typeBox->setCurrentIndex(config->getType());
    // if(config->getID() == 0){
    //     ui->idBox->setCheckState(Qt::Checked);
    // }
    ui->idEdit->setValue(config->getID());
    ui->altEdit->setText(config->getLabel());
}
void NodeDicioDialog::setNodeSettings(GCNodeDicio *const s)
{
    config->setID(s->getID());
    config->setLabel(s->getLabel());
    updateUISettings();
}

void NodeDicioDialog::apply()
{
    config->setID(ui->idEdit->value());
    config->setLabel(ui->altEdit->displayText());
}

void NodeDicioDialog::updateNode(GCNodeDicio *node) const
{
    node->setLabel(config->getLabel());
    node->setID(config->getID());
}

void NodeDicioDialog::updateAutoID(void)
{
    // if(ui->idBox->isChecked()){
    //     ui->idEdit->setText(QString("0"));
    //     ui->idEdit->setDisabled(true);
    // } else {
    ui->idEdit->setDisabled(true);
    //}
}
