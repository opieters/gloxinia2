#include "updatedialog.h"
#include "ui_updatedialog.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include "gmessage.h"
#include "gloxiniaconfigurator.h"

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog),
    hexFile(nullptr)
{
    ui->setupUi(this);

    info.swVersion = 0;
    info.hwVersion = 0;

    // set default info
    info.maxPacketSize = 64;
    //info.deviceID = 0;
    info.eraseRowSize = 16;
    info.writeRowSize = 16;


    //connect(ui->actionSave, &QAction::triggered, this, &GloxiniaConfigurator::saveProject);
    connect(ui->fileEdit, &QPushButton::clicked, this, &UpdateDialog::selectHexFile);
    connect(ui->flashButton, &QPushButton::clicked, this, &UpdateDialog::flash);
    connect(ui->closeButton, &QPushButton::clicked, this, &UpdateDialog::close);
    connect(ui->resetButton, &QPushButton::clicked, this, &UpdateDialog::resetNode);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(10);
    ui->progressBar->setValue(0);

    ui->status->setText("No file nor device selected.");
}

void UpdateDialog::resetNode(void)
{
    GMessage m(GMessage::Code::NODE_RESET, node->getID(), GMessage::NoSensorID, true);
    configurator->sendSerialMessage(m);
}

void UpdateDialog::selectHexFile(void)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open HEX file"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    tr("hex files (*.hex)"));

    if(!fileName.isEmpty()){

        // clear memory
        if(hexFile != nullptr){
            if(hexFile->isOpen())
                hexFile->close();
            delete hexFile;
        }

        hexFile = new QFile(fileName);

        if(!hexFile->open(QFile::ReadOnly)){
            QMessageBox msgBox;
            msgBox.setText("Unable to open file at " + fileName + ".");
            msgBox.exec();
            return;
        }

        ui->file->setText(fileName);
        float fileSize = hexFile->size();
        if(fileSize > 1000){
            fileSize = fileSize / 1000.0;
            ui->fileSize->setText(QString::number(fileSize) + "kB");
        } else {
            ui->fileSize->setText(QString::number(fileSize) + "B");
        }


        ui->status->setText(".hex file selected.");
    }

    fileSWVersion = 0x0;
    fileHWVersion = 0x0;
    fileStartAddress = 0x0;
    fileEndAddress = 0x0;
    hexFileValid = true;
    readHexAppHeader();
}


void UpdateDialog::readHexAppHeader(void)
{
    constexpr unsigned int headerLength = 17*4;
    const quint64 current_location = hexFile->pos();
    hexFile->seek(0);

    bool headerRead = false;

    quint32 baseAddress = 0;
    bool ok, endCmdFound = false;
    qint64 address;
    quint8 len, checksum, dataType;
    int n_read;

    bool headerBytesCheck[headerLength] = {false};
    uint8_t header[headerLength] = {0};
    const quint32 startAddress = 0x2800*2;

    // search for application header
    while(!hexFile->atEnd() && !headerRead && !endCmdFound)
    {

        //QByteArray hexCmd = QByteArray::fromHex(hexFile->readLine());
        QString hexCmd = QString(hexFile->readLine());
        if(hexCmd.size() == 0)
            continue;

        hexCmd.replace("\r", "");
        hexCmd.replace("\n", "");

        std::vector<uint8_t> raw_data;

        for(int i = 1; i < hexCmd.size(); i+=2)
        {
            raw_data.push_back(hexCmd.sliced(i,2).toUShort(&ok,16));
        }

        // check start indicator
        if(hexCmd.at(0) != ':'){
            continue;
        }

        // read header fields
        len = raw_data[0];
        address = baseAddress + (raw_data[2] | (((quint32) raw_data[1]) << 8));
        dataType = raw_data[3];

        // read checksum
        checksum = raw_data[raw_data.size()-1];

        // validate checksum
        quint32 crc = 0;
        for(int i = 0; i < (raw_data.size() - 1); i++)
            crc = crc + raw_data[i];
        crc = crc & 0xff;
        ok = ((crc + checksum) & 0xff) == 0x0;

        if(!ok){
            qInfo() << "Checksum failed. Expected" << checksum << "calculated" << crc;
            hexFileValid = false;
        }


        switch(dataType)
        {
        case 4:
            // set new base address
            if(raw_data.size() < 6)
                continue;
            baseAddress = raw_data[5] | (((quint32) raw_data[4]) << 8);
            baseAddress = baseAddress << 16;
            break;
        case 0:
            for(int i = 0; i < len; i++)
            {
                if((((address - startAddress + i) >= 0) && ((address - startAddress + i) < headerLength)))
                {
                    header[address - startAddress + i] = raw_data[4+i];
                    headerBytesCheck[address - startAddress + i] = true;
                }
            }

            headerRead = true;
            for(int i = 0; (i < headerLength) && headerRead; i++)
            {
                headerRead = headerRead && headerBytesCheck[i];
            }
            break;
        case 1:
            // end of file, stop reading
            endCmdFound = true;
            break;
        default:
            break;
        }

    }

    // todo: calculate CRC of HEX file and store it

    // todo: check that address range and programming address ranges are valid

    // parse HEX info: loop over headers
    int pos = 0x24;
    int hFieldlen = 0x0;
    for(int i = 0; i < header[0x20]; i++){
        switch(header[pos])
        {
            case 0x02: // software version header
            pos += 4;
            hFieldlen = (header[pos+1] << 8) | header[pos];
            if(hFieldlen == 0x4){
                pos += 4;
                fileSWVersion = header[pos] | (header[pos+1] << 8) | (header[pos+4] << 16) | (header[pos+5] << 24);
            } else {
                qInfo() <<"Invalid software version in hex file.";
                hexFileValid = false;
            }
            break;
        case 0x03: // software version header
            pos += 4;
            hFieldlen = (header[pos+1] << 8) | header[pos];
            if(hFieldlen == 0x4){
                pos += 4;
                fileHWVersion = header[pos] | (header[pos+1] << 8) | (header[pos+4] << 16) | (header[pos+5] << 24);
            } else {
                qInfo() << "Invalid hardware version in hex file.";
                hexFileValid = false;
            }

            break;
        default: // unknown header, skip
            pos += 4;
            hFieldlen = (header[pos+1] << 8) | header[pos];
            break;
        }
        pos += hFieldlen*2;
    }

    pos = 0;
    fileCRC = header[pos] | (header[pos+1] << 8) | (header[pos+4] << 16) | (header[pos+5] << 24);
    pos = 4;
    fileStartAddress = header[pos] | (header[pos+1] << 8) | (header[pos+4] << 16) | (header[pos+5] << 24);
    pos = 4;
    fileEndAddress = header[pos] | (header[pos+1] << 8) | (header[pos+4] << 16) | (header[pos+5] << 24);

    if((fileSWVersion == 0x0)
            || (fileHWVersion == 0x0)
            || (fileStartAddress == 0x0)
            || (fileEndAddress == 0x0)){
        hexFileValid = false;
    }

    if(!hexFileValid){
        ui->status->setText("Invalid hex file.");
    }


    // go back to the start
    hexFile->seek(current_location);

    updateUI();
}

void UpdateDialog::updateUI()
{
    uint16_t deviceType = fileHWVersion >> 16;
    // update node type
    if(hexFileValid){
        switch(deviceType){
            case GCNode::BootLoaderNodeID::Dicio:
            ui->nodeType->setText("dicio");
            break;
        case GCNode::BootLoaderNodeID::Sylvatica:
            ui->nodeType->setText("dicio");
            break;
        case GCNode::BootLoaderNodeID::Planalta:
            ui->nodeType->setText("planalta");
            break;
        default:
            ui->nodeType->setText("unknown");
            hexFileValid = false;
            break;
        }
    } else {
        ui->nodeType->setText("");
    }

    // update software field
    if(!hexFileValid){
        ui->swVersion->setText("");
    }
    else {
        ui->flashButton->setDisabled(false);
        uint8_t vMajor = (uint8_t) (fileSWVersion >> 16);
        uint8_t vMinor = (uint8_t) (fileSWVersion >> 8);
        uint8_t vPath = (uint8_t) fileSWVersion;

        ui->swVersion->setText(
                    QString::number(vMajor) + "." +
                    QString::number(vMinor) + "." +
                    QString::number(vPath));

    }

    // update flash button
    if(deviceBootLoaderActive && hexFileValid)
    {
        if(deviceType == node->getBootLloaderID())
        {
            ui->flashButton->setDisabled(false);
        } else {
            QMessageBox msgBox;
            msgBox.setText("The selected node and hex file do not match.");
            msgBox.exec();

            ui->flashButton->setDisabled(true);
        }
    }
    else
    {
        ui->flashButton->setDisabled(true);
    }

    if(!deviceBootLoaderActive)
    {
        ui->status->setText("Device not in bootloader mode.");
        ui->resetButton->setEnabled(true);
    } else {
        ui->resetButton->setDisabled(true);
    }
}

void UpdateDialog::detectedError()
{
    errorReported = true;

    ui->status->setText("Device reported error.");
}

// https://microchipdeveloper.com/ipe:sqtp-hex-file-format
// https://microchipdeveloper.com/ipe:sqtp-basic-example
void UpdateDialog::flash(void)
{
    quint8 uartBuffer[9+64];
    if(hexFile == nullptr)
    {
        ui->status->setText("No .hex file selected.");
        return;
    }
    if(!hexFile->isOpen())
    {
        ui->status->setText(".hex file open error.");
        return;
    }

    quint32 baseAddress = 0;
    bool ok, endCmdFound = false;
    quint32 address;
    quint8 len, checksum, dataType;
    int n_read;

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(hexFile->size());
    ui->progressBar->setValue(hexFile->pos());

    // erase device
    ui->status->setText("Erasing device.");
    // TODO

    ui->status->setText("Flashing device firmware.");

    errorReported = false;
    while(!hexFile->atEnd() && !endCmdFound && !errorReported)
    {


        //QByteArray hexCmd = QByteArray::fromHex(hexFile->readLine());
        QString hexCmd = QString(hexFile->readLine());
        ui->progressBar->setValue(hexFile->pos());

        hexCmd.replace("\r", "");
        hexCmd.replace("\n", "");

        std::vector<uint8_t> raw_data;

        for(int i = 1; i < hexCmd.size(); i+=2)
        {
            raw_data.push_back(hexCmd.sliced(i,2).toUShort(&ok,16));
        }

        // check start indicator
        if(hexCmd.at(0) != ':'){
            continue;
        }
        // read header fields
        len = raw_data[0];
        address = baseAddress + (raw_data[2] | (((quint32) raw_data[1]) << 8));
        dataType = raw_data[3];

        // read checksum
        checksum = raw_data[raw_data.size()-1];

        // validate checksum
        quint32 crc = 0;
        for(int i = 0; i < (raw_data.size() - 1); i++)
            crc = crc + raw_data[i];
        crc = crc & 0xff;
        ok = ((crc + checksum) & 0xff) == 0x0;

        if(!ok){
            qInfo() << "Checksum failed. Expected" << checksum << "calculated" << crc;
            hexFileValid = false;
        }


        switch(dataType)
        {
        case 4:
            // set new base address
            if(raw_data.size() < 6)
                continue;
            baseAddress = raw_data[5] | (((quint32) raw_data[4]) << 8);
            baseAddress = baseAddress << 16;
            break;
        case 0:
            for(int i = 0; i < len; i++){
                uartBuffer[i] = raw_data[4+i];
            }
            writeFlash(address, uartBuffer, len); // TODO: check return code
            break;
        case 1:
            // end of file, stop reading
            endCmdFound = true;
            break;
        default:
            break;
        }
    }

    if(!hexFileValid){
        ui->status->setText("Invalid hex file.");
    }
}

UpdateDialog::~UpdateDialog()
{
    if(hexFile != nullptr){
        if(hexFile->isOpen())
            hexFile->close();
        delete hexFile;
    }
    delete ui;
}

void UpdateDialog::getVersion(void)
{

    ui->status->setText("Reading device parameters.");

    GMessage m(GMessage::Code::BOOT_READ_VERSION,
             GMessage::ComputerAddress,
             GMessage::NoSensorID,
             false,
             std::vector<quint8>()
            );

    configurator->sendSerialMessage(m);

    ui->status->setText("Device parameters read.");
}

void UpdateDialog::writeFlash(uint32_t address, uint8_t* data, uint8_t len)
{
    quint8 uartBuffer[9+64];
    int n_read;

    std::vector<quint8> mdata = std::vector<quint8>(len+4);
    mdata[0] = (quint8) (address & 0xff);
    mdata[1] = (quint8) ((address >> 8) & 0xff);
    mdata[2] = (quint8) ((address >> 16) & 0xff);
    mdata[3] = (quint8) ((address >> 24) & 0xff);

    for(int i = 0; i < len; i++){
        mdata[4+i] = data[i];
    }


    GMessage m(GMessage::Code::BOOT_WRITE_FLASH,
             GMessage::ComputerAddress,
             GMessage::UnlockSequence,
             false,
             mdata
            );

    configurator->sendSerialMessage(m);
}

void UpdateDialog::calculateChecksum(uint32_t address, uint16_t n_rows, uint16_t* checksum)
{
    std::vector<quint8> mdata = std::vector<quint8>(2+4);

    mdata[0] = (uint8_t) (address & 0xff);
    mdata[1] = (uint8_t) ((address >> 8) & 0xff);
    mdata[2] = (uint8_t) ((address >> 16) & 0xff);
    mdata[3] = (uint8_t) ((address >> 24) & 0xff);
    mdata[4] = (uint8_t) n_rows & 0xff;
    mdata[5] = (uint8_t) (n_rows >> 8);

    GMessage m(GMessage::Code::BOOT_CALC_CHECKSUM,
             GMessage::ComputerAddress,
             GMessage::NoSensorID,
             false,
             mdata
            );

    configurator->sendSerialMessage(m);
}


void UpdateDialog::resetDevice()
{
    GMessage m(GMessage::Code::BOOT_RESET_DEVICE,
             GMessage::ComputerAddress,
             GMessage::NoSensorID,
             false
            );

    configurator->sendSerialMessage(m);

}

void UpdateDialog::receivedCRC(uint32_t crc)
{
    deviceCRC = crc;
}

void UpdateDialog::setVerification(bool status)
{
    deviceVerification = status;
}

void UpdateDialog::setBootReady(bool status)
{
    bool previousStatus = deviceBootLoaderActive;
    deviceBootLoaderActive = status;
    if(status && !previousStatus)
    {
        // send message to keep the device in bootloader mode
        GMessage m(GMessage::Code::BOOT_READ_VERSION,             GMessage::ComputerAddress,
                   GMessage::NoSensorID,false);
        configurator->sendSerialMessage(m);
    }
}

void UpdateDialog::setFlashRange(uint32_t start, uint32_t stop)
{
    fileStartAddress = start;
    fileEndAddress = stop;
}

void UpdateDialog::setConfigurator(GloxiniaConfigurator* c)
{
    configurator = c;
}

void UpdateDialog::setNode( GCNode* node )
{
    this->node = node;
    deviceBootLoaderActive = false;
}

void UpdateDialog::showEvent( QShowEvent* event ) {
    QWidget::showEvent( event );

    if(node == nullptr)
    {
        QMessageBox msgBox;
        msgBox.setText("No node selected. Click on a node in the device overview and try again.");
        msgBox.exec();
        reject();
    } else {
        updateUI();

        // send reset message to activate bootloader
        if(!deviceBootLoaderActive)
        {
            GMessage m(GMessage::Code::NODE_RESET, node->getID(), GMessage::NoSensorID, true);
            configurator->sendSerialMessage(m);
        }
    }
}
