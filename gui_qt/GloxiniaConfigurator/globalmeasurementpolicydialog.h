#ifndef GLOBALMEASUREMENTPOLICYDIALOG_H
#define GLOBALMEASUREMENTPOLICYDIALOG_H

#include <QDialog>

namespace Ui {
class GlobalMeasurementPolicyDialog;
}

class GlobalMeasurementPolicyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalMeasurementPolicyDialog(QWidget *parent = nullptr);
    ~GlobalMeasurementPolicyDialog();

    double getPeriod(void) const;
    void updatePeriod(quint16& v);


private:
    void apply(quint16 v);

private slots:
    void updateUI();

private:
    Ui::GlobalMeasurementPolicyDialog *ui;

    double period = 1.0;
};

#endif // GLOBALMEASUREMENTPOLICYDIALOG_H
