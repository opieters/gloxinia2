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

    quint16 getPeriod(void) const;
    void setPeriod(quint16 v);

private:
    void apply(void);

private slots:
    void update(void);

private:
    Ui::GlobalMeasurementPolicyDialog *ui;

    quint16 period;
};

#endif // GLOBALMEASUREMENTPOLICYDIALOG_H
