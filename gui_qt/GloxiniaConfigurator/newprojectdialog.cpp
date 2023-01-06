#include "newprojectdialog.h"
#include "ui_newprojectdialog.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QSerialPortInfo>

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);

    connect(ui->editDirButton, &QPushButton::clicked, this, &NewProjectDialog::selectRootDir);
    connect(ui->comRefreshButton, &QPushButton::clicked, this, &NewProjectDialog::updateCOMList);
    connect(ui->okButton, &QPushButton::clicked, this, &NewProjectDialog::apply);
    connect(ui->cancelButton, &QPushButton::clicked, this, &NewProjectDialog::cancel);
    connect(ui->offlineBox, &QCheckBox::stateChanged, this, &NewProjectDialog::offlineTrigger);
    connect(ui->comBox, &QComboBox::currentIndexChanged, this,  &NewProjectDialog::updateCOM);
    connect(ui->baudrateBox, &QSpinBox::valueChanged, this, &NewProjectDialog::updateBaudrate);

    ui->baudrateBox->setMinimum(1);
    ui->baudrateBox->setMaximum(1000000);

    settings.success = false;
    settings.projectName = "MyProject";
    settings.projectDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + settings.projectName;
    settings.projectDir = QDir::cleanPath(settings.projectDir);
    settings.baudrate = 500000;
    settings.workOffline = false;

    updateCOMList();
    update();
}



NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

NewProjectDialog::ProjectSettings NewProjectDialog::getProject(void) const
{
    return settings;
}

void NewProjectDialog::setProject(NewProjectDialog::ProjectSettings& settings)
{
    this->settings = settings;
}

void NewProjectDialog::offlineTrigger(void)
{
    settings.workOffline = ui->offlineBox->isChecked();

    update();
}

void NewProjectDialog::updateBaudrate(void)
{
    settings.baudrate = ui->baudrateBox->value();

    update();
}

void NewProjectDialog::selectRootDir()
{
    settings.projectDir = QFileDialog::getExistingDirectory(this, tr("Select directory"),
                                                settings.projectDir,
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);

    if(!settings.projectDir.endsWith(settings.projectName)){
        settings.projectDir = settings.projectDir + "/" + settings.projectName;
        settings.projectDir = QDir::cleanPath(settings.projectDir);
    }

    update();
}

void NewProjectDialog::updateCOM()
{
    settings.comPort = ui->comBox->currentText();
}

void NewProjectDialog::updateCOMList()
{
    auto list = QSerialPortInfo::availablePorts();

    // store currently selected item
    QString selectedCOM = ui->comBox->currentText();

    // clear and add new items
    comList.clear();
    for (const QSerialPortInfo &i : list)
    {
        if(i.hasVendorIdentifier() && i.hasProductIdentifier()){
            if((i.vendorIdentifier() == 0x0403) && (i.productIdentifier() == 0x6001))
                comList.append(i.portName());
        }
    }

    // find index of selected COM port in list
    int index = comList.indexOf(selectedCOM);

    comList.sort();
    ui->comBox->clear();
    ui->comBox->addItems(comList);
    ui->comBox->setCurrentIndex(index);
}

void NewProjectDialog::update()
{
    ui->projectEdit->setText(settings.projectName);
    ui->rootValue->setText(settings.projectDir);

    if(settings.workOffline)
        ui->offlineBox->setCheckState(Qt::Checked);
    else
        ui->offlineBox->setCheckState(Qt::Unchecked);

    ui->baudrateBox->setValue(settings.baudrate);

    if(settings.workOffline){
        ui->comBox->setDisabled(true);
        ui->baudrateBox->setDisabled(true);
        ui->comRefreshButton->setDisabled(true);
    } else {
        ui->comBox->setDisabled(false);
        ui->baudrateBox->setDisabled(false);
        ui->comRefreshButton->setDisabled(false);
    }
}
void NewProjectDialog::apply()
{
    settings.success = true;

    settings.projectName = ui->projectEdit->text();
    settings.projectDir = ui->rootValue->text();
    settings.workOffline = ui->offlineBox->isChecked();
    settings.comPort = ui->comBox->currentText();
    settings.baudrate = ui->baudrateBox->value();

    if(settings.projectDir.isEmpty())
        settings.success = false;
    if(settings.projectName.isEmpty())
        settings.success = false;
    if((!settings.workOffline) && (ui->comBox->currentIndex() ))
        settings.success = false;

    if(settings.success)
        accept();
    else
        reject();
}
void NewProjectDialog::cancel()
{
    settings.success = false;
    reject();
}
