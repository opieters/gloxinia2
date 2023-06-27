#ifndef LIAENGINEDIALOG_H
#define LIAENGINEDIALOG_H

#include <QDialog>
#include <gliaengine.h>

namespace Ui {
class LIAEngineDialog;
}

class LIAEngineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LIAEngineDialog(QWidget *parent = nullptr);
    ~LIAEngineDialog();

    void updateUISettings(const GLIAEngine* engine);
    void apply(GLIAEngine* engine);
private:
    Ui::LIAEngineDialog *ui;
};

#endif // LIAENGINEDIALOG_H
