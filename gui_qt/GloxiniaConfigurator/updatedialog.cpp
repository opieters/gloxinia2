#include "updatedialog.h"
#include "ui_updatedialog.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include "gmessage.h"
#include "gloxiniaconfigurator.h"
#include <algorithm>

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
    //GMessage m(GMessage::Code::NODE_RESET, node->getID(), GMessage::NoSensorID, true);
    GMessage m(GMessage::Code::BOOT_RESET_DEVICE, GMessage::ComputerAddress, GMessage::NoSensorID, false);
    emit configurator->devCom->queueMessage(m);
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
    checksumStartAddress = 0x0;
    checksumEndAddress = 0x0;
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
    fileChecksum = header[pos] | (header[pos+1] << 8) | (header[pos+4] << 16) | (header[pos+5] << 24);
    pos = 8;
    checksumStartAddress = header[pos] | (header[pos+1] << 8) | (header[pos+4] << 16) | (header[pos+5] << 24);
    checksumStartAddress *= 2; // convert to bytes
    pos = 16;
    checksumEndAddress = header[pos] | (header[pos+1] << 8) | (header[pos+4] << 16) | (header[pos+5] << 24);
    checksumEndAddress *= 2; // convert to bytes

    if((fileSWVersion == 0x0)
            || (fileHWVersion == 0x0)
            || (checksumStartAddress == 0x0)
            || (checksumEndAddress == 0x0)){
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

    // update status field (can be overwritten below!)
    if(!hexFileValid){
        ui->status->setText("No (valid) .hex file loaded.");
    } else {
        if(hexFileValid  && !deviceBootLoaderActive)
            ui->status->setText("Device not in bootloader mode.");
        else
            if(flashCount == 0)
                ui->status->setText("Ready to flash.");
            else
                if(!flashOngoing)
                    ui->status->setText("Flash complete.");
    }


    // update flash button
    if(deviceBootLoaderActive && hexFileValid)
    {
        if(deviceType == node->getBootLloaderID())
        {
            ui->flashButton->setDisabled(false);
            if(!flashOngoing)
                ui->status->setText("Ready to flash.");
        } else {
            QMessageBox msgBox;
            msgBox.setText("The selected node and hex file do not match.");
            msgBox.exec();
            ui->status->setText("The selected node and hex file do not match.");

            ui->flashButton->setDisabled(true);
        }

        // check that we are not downgrading the software
        /*if((info.swVersion >= fileSWVersion) && (info.swVersion != 0xffffffff))
        {
            ui->status->setText("Downgrading the firmware is not allowed.");
            ui->flashButton->setDisabled(true);
        }

        // check that hardware version matches
        if((info.hwVersion != fileHWVersion) && (info.hwVersion != 0xffffffff))
        {
            ui->status->setText("The hardware of the device and the hex-file do not match.");
            ui->flashButton->setDisabled(true);
        }*/
    }
    else
    {
        ui->flashButton->setDisabled(true);
    }

    if(!deviceBootLoaderActive)
    {
        ui->resetButton->setEnabled(true);
    } else {
        ui->resetButton->setDisabled(true);
    }

    // update node info
    if(deviceBootLoaderActive)
    {
        if((info.swVersion == 0xffffffff) || (info.hwVersion == 0xffffffff))
        {
            ui->nodeInfo->setText("empty application image");
        } else {
            uint8_t vMajor = (uint8_t) (info.swVersion >> 16);
            uint8_t vMinor = (uint8_t) (info.swVersion >> 8);
            uint8_t vPatch = (uint8_t) info.swVersion;
            ui->nodeInfo->setText(QString("software v%1.%2.%3 - hardware id %4.%5").arg(
                                      QString::number(vMajor),
                                      QString::number(vMinor),
                                      QString::number(vPatch),
                                      QString::number(info.hwVersion >> 16),
                                      QString::number(info.hwVersion & 0xffff)));
        }
    } else {
        ui->nodeInfo->setText("no device found");
    }
}

void UpdateDialog::detectedError()
{
    errorReported = true;

    ui->status->setText("Device reported error.");
}

void UpdateDialog::setFlashProgress(uint32_t address)
{
    ui->progressBar->setValue(address);
    qInfo() << "Progress at" << address << "of" << ui->progressBar->maximum();

    if(address == ui->progressBar->maximum())
    {
            flashOngoing = false;
    }
}

// https://microchipdeveloper.com/ipe:sqtp-hex-file-format
// https://microchipdeveloper.com/ipe:sqtp-basic-example
void UpdateDialog::flash(void)
{
    flashOngoing = false;

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
    if((checksumStartAddress % info.writeRowSize) != 0){
        ui->status->setText("Bootloader readout error: start address not a multiple of write block size. Cannot flash.");
        return;
    }
    if(checksumStartAddress < flashStartAddress){
        ui->status->setText(".hex flash content address error.");
        return;
    }
    if(checksumEndAddress > flashEndAddress){
        ui->status->setText(".hex flash content address error.");
        return;
    }

    // erase device
    ui->status->setText("Erasing device.");
    // TODO

    // will contain entire hex file contents
    std::vector<uint8_t> flashMemory(flashEndAddress - flashStartAddress + 2); // TODO: check +2
    std::vector<bool> flashMemoryLoaded(flashEndAddress - flashStartAddress + 2);

    // set all zeros
    for(int i = 0; i < (flashEndAddress - flashStartAddress); i++)
    {
        flashMemory[i] = 0;
        flashMemoryLoaded[i] = false;
    }

    // load file contents
    quint32 baseAddress = 0;
    bool ok, endCmdFound = false;
    quint32 address;
    quint8 len, checksum, dataType;
    int n_read;

    ui->status->setText("Flashing device firmware.");
    errorReported = false;
    hexFile->seek(0);
    while(!hexFile->atEnd() && !endCmdFound && !errorReported)
    {
        QString hexCmd = QString(hexFile->readLine());

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
                if(((address+i) >= flashStartAddress) && ((address+i) <= checksumEndAddress)) {
                    flashMemory[address - flashStartAddress + i] = raw_data[4+i];
                    flashMemoryLoaded[address - flashStartAddress + i] = true;
                }
                else {
                    //hexFileValid = false;
                }
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

    if(!hexFileValid){
        ui->status->setText("Invalid hex file.");
    }

    // compute the checksum and write in datastream
    for(int i = 0; i < 8; i++)
    {
        flashMemory[i] = 0U;
    }
    uint64_t flash_checksum = 0U;
    for(int i = checksumStartAddress - flashStartAddress; i < (flashMemory.size()-3); i+=4)
    {
        uint32_t instruction = 0;
        for(int j = 0; j < 4; j++){
            instruction = (instruction << 8) + flashMemory[i+3-j];
        }
        flash_checksum += instruction;

    }
    flash_checksum &= 0xffffffffU;

    qInfo() << "Inserting checksum" << flash_checksum;

    flashMemory[5] = (uint8_t) (flash_checksum >> 24);
    flashMemory[4] = (uint8_t) (flash_checksum >> 16);
    flashMemory[1] = (uint8_t) (flash_checksum >> 8);
    flashMemory[0] = (uint8_t) (flash_checksum);

    // todo: find last write byte for progress bar
    int lastDataLoc = 0;
    for(lastDataLoc = flashEndAddress - flashStartAddress - 1; (flashMemoryLoaded[lastDataLoc] == false) && (lastDataLoc >= 0); lastDataLoc--);
    ui->progressBar->setMinimum(flashStartAddress);
    ui->progressBar->setMaximum(flashStartAddress + lastDataLoc - (lastDataLoc % 8));
    ui->progressBar->setValue(flashStartAddress);

    // set checksum end address
    uint32_t checksumEndAddress = (flashStartAddress + lastDataLoc) / 2;
    // the address should be a multiple of 2 to be correctly aligned
    checksumEndAddress = checksumEndAddress - (checksumEndAddress % 2);
    flashMemory[0x10] = (uint8_t) (checksumEndAddress);
    flashMemory[0x11] = (uint8_t) (checksumEndAddress >> 8);
    flashMemory[0x14] = (uint8_t) (checksumEndAddress >> 16);
    flashMemory[0x15] = (uint8_t) (checksumEndAddress >> 24);

    // write actual data to the device
    uint8_t flash_data[info.writeRowSize];

    // start flash sequence
    GMessage mStartFlash(GMessage::Code::BOOT_WRITE_FLASH_INIT, GMessage::ComputerAddress ,GMessage::NoSensorID, false);
    emit configurator->devCom->queueMessage(mStartFlash);

    flashOngoing = true;
    ui->status->setText("Flashing...");

    for(int i = 0; i < flashMemory.size() - info.writeRowSize + 1; i += info.writeRowSize)
    {
        // check if this memory bock should be written
        bool dataFound = false;
        for(size_t j = 0; j < info.writeRowSize; j++){
            if(flashMemoryLoaded[i+j])
                dataFound = true;
        }
        if(!dataFound)
            continue;
        for(int j = 0; j < info.writeRowSize; j++)
        {
            flash_data[j] = flashMemory[i+j];
        }
        writeFlash(i+flashStartAddress, flash_data, info.writeRowSize);
    }

    GMessage mStopFlash(GMessage::Code::BOOT_WRITE_FLASH_DONE, GMessage::ComputerAddress ,GMessage::NoSensorID, false);
    emit configurator->devCom->queueMessage(mStopFlash);

    // send device verification
    GMessage mVerify(GMessage::Code::BOOT_SELF_VERIFY, GMessage::ComputerAddress ,GMessage::NoSensorID, false);
    emit configurator->devCom->queueMessage(mVerify);

    // send reset to device
    /*GMessage mReset(GMessage::Code::BOOT_RESET_DEVICE, GMessage::ComputerAddress,GMessage::NoSensorID, false);
    configurator->sendSerialMessage(mReset);

    deviceBootLoaderActive = false;*/

    flashCount += 1;

    updateUI();
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
               GMessage::NoInterfaceID,
             GMessage::NoSensorID,
             false,
             std::vector<quint8>()
            );

    emit configurator->devCom->queueMessage(m);

    ui->status->setText("Device parameters read.");
}

void UpdateDialog::writeFlash(uint32_t address, uint8_t* data, uint8_t len)
{
    quint8 uartBuffer[9+64];
    int n_read;

    if(address < flashStartAddress)
        return;
    if(address > flashEndAddress)
        return;
    if(len == 0)
        return;

    // address in intel files is at the byte level
    // but addresses in the MCU are at word level
    address = address / 2;

    std::vector<quint8> mdata = std::vector<quint8>(len+4);
    mdata[0] = (quint8) ((address >> 24) & 0xff);
    mdata[1] = (quint8) ((address >> 16) & 0xff);
    mdata[2] = (quint8) ((address >> 8) & 0xff);
    mdata[3] = (quint8) (address & 0xff);

    for(int i = 0; i < len; i++){
        mdata[4+i] = data[i];
    }

    int padLen = len % info.writeRowSize;
    if(padLen != 0)
        padLen = info.writeRowSize - padLen;

    //add padding to make sure we write the minimum amount required
    for(int i = 0; i < padLen; i++){
        mdata.push_back(0x00);
    }

    GMessage m(GMessage::Code::BOOT_WRITE_FLASH,
             GMessage::ComputerAddress,
             GMessage::UnlockSequenceH,
               GMessage::UnlockSequenceL,
             false,
             mdata
            );

    emit configurator->devCom->queueMessage(m);
}


void UpdateDialog::resetDevice()
{
    GMessage m(GMessage::Code::BOOT_RESET_DEVICE,
             GMessage::ComputerAddress,
             GMessage::NoSensorID,
             false
            );

    emit configurator->devCom->queueMessage(m);

}

void UpdateDialog::receivedChecksum(uint32_t c)
{
    deviceChecksum = c;
}

void UpdateDialog::setVerification(bool status)
{
    if(status){
        ui->status->setText("Flash complete. Verification OK.");
        qInfo() << "Verification succes";
    }
    else {
         ui->status->setText("Flash complete. Verification error.");
         qInfo() << "Verification failed";
    }
    deviceVerification = status;
}

void UpdateDialog::setBootReady(bool status)
{
    deviceBootLoaderActive = status;
    if(status)
    {
        // send message to keep the device in bootloader mode
        GMessage m(GMessage::Code::BOOT_READ_VERSION,             GMessage::ComputerAddress,
                   GMessage::NoSensorID,false);
        emit configurator->devCom->queueMessage(m);
    }

    updateUI();
}

void UpdateDialog::setFlashRange(uint32_t start, uint32_t stop)
{
    flashStartAddress = start;
    flashEndAddress = stop;
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

void UpdateDialog::setNodeInfo(FlashNodeInfo& info)
{
    this->info = info;

    if(deviceBootLoaderActive)
    {
        // send message to keep the device in bootloader mode
        GMessage m(GMessage::Code::BOOT_GET_MEMORY_ADDRESS_RANGE_COMMAND,             GMessage::ComputerAddress,
                   GMessage::NoSensorID, false);
        emit configurator->devCom->queueMessage(m);
    }

    updateUI();
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
            emit configurator->devCom->queueMessage(m);
        }
    }
}
