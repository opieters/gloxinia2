#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QFile>
#include "gcnode.h"

class GloxiniaConfigurator;


namespace Ui {
class UpdateDialog;
}

struct FlashNodeInfo {
    uint32_t swVersion;
    uint32_t hwVersion;
    uint16_t bootVersion;

    uint8_t maxPacketSize;
    uint8_t eraseRowSize;
    uint8_t writeRowSize;
};

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent = nullptr);
    ~UpdateDialog();

    enum BootLoaderCommand {
        ReadVerson = 0x0,
        ReadFlash = 0x1,
        WriteFlash = 0x2,
        EraseFlash = 0x3,
        ReadEEData = 0x4,
        WriteEEData = 0x5,
        ReadConfig = 0x6,
        WriteConfig = 0x7,
        CalculateChecksum = 0x8,
        ResetDevice = 0x9,
        ActivateBootLoader = 0x10,
    };

    void selectHexFile(void);
    void detectedError(void);
    void receivedChecksum(uint32_t crc);
    void setVerification(bool status);
    void setBootReady(bool status);
    void setFlashRange(uint32_t startAddressInBytes, uint32_t stopAddressInBytes);
    void setConfigurator(GloxiniaConfigurator* c);
    void setNode( GCNode* node );
    void setNodeInfo(FlashNodeInfo& info);
    void showEvent( QShowEvent* event ) override;
    void setFlashProgress(uint32_t address);
private slots:
    void resetNode(void);
private:
    Ui::UpdateDialog *ui;

    GloxiniaConfigurator* configurator = nullptr;

    GCNode* node = nullptr;

    QFile* hexFile = nullptr;
    bool hexFileValid = false;

    uint32_t fileHWVersion = 0x0;
    uint32_t fileSWVersion = 0x0;
    uint32_t fileChecksum = 0x0;
    uint32_t checksumStartAddress = 0x0;
    uint32_t checksumEndAddress = 0x0;

    uint32_t flashStartAddress = 0x0;
    uint32_t flashEndAddress = 0x0;

    uint32_t computedFileCRC = 0x0;

    unsigned int flashCount = 0;

    uint32_t deviceHWVersion = 0x0;
    uint32_t deviceSWVersion = 0x0;
    uint32_t deviceChecksum = 0x0;
    bool deviceVerification = false;
    bool deviceBootLoaderActive = false;
    bool flashOngoing = false;

    volatile bool errorReported = false;

    FlashNodeInfo info;

    void getVersion(void);

    void readHexAppHeader(void);

    void flash(void);

    void eraseFlash(uint32_t address, uint16_t n_rows);
    void writeFlash(uint32_t address, uint8_t* data, uint8_t len);
    void resetDevice();

    void updateUI(void);
};

#endif // UPDATEDIALOG_H
