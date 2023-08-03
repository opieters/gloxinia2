#ifndef GCREADOUTDIALOG_H
#define GCREADOUTDIALOG_H

#include <QDialog>

namespace Ui {
class GCReadoutDialog;
}

class GCReadoutDialog : public QDialog
{
    Q_OBJECT

public:

    struct Settings {
        int startAddress;
        int stopAddress;
        bool autoDetect;
    };

    explicit GCReadoutDialog(QWidget *parent = nullptr);
    ~GCReadoutDialog();


    static constexpr int ADDRESS_START = 0x20;
    static constexpr int ADDRESS_STOP  = 0x8000;
    static constexpr int ADDRESS_AUTO_DETECT = 0;

    Settings settings() const;

private slots:
    void updateBoxes(void);
    void readData(void);

private:
    Ui::GCReadoutDialog *ui;
};

#endif // GCREADOUTDIALOG_H
