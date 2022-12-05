#include "gloxiniaconfigurator.h"

#include <QMessageBox>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <gmessage.h>
#include <QSerialPortInfo>
#include <QInputDialog>
#include <sensorsht35dialog.h>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QChartView>
#include <QValueAxis>
#include <QFileDialog>

void GloxiniaConfigurator::saveToFile(void)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Configuration"), "", tr("Gloxinia Config File (*.gcf);;All Files (*)"));

    // save only if filename is non-empty
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    QTextStream out(&file); // QDataStream is also possible for serialised data
    // out.setVersion(QDataStream::Qt_6_3);

    QStringList textConfig = treeModel->toTextConfig();
    for (QString i : textConfig)
    {
        out << i << "\n";
    }

    // TODO: update status bar with message indicating the file was saved
}
void GloxiniaConfigurator::loadFromFile(void)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Gloxinia Configuration"), "", tr("Gloxinia Config File (*.gcf);;All Files (*)"));

    // load only if filename is non-empty
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    QTextStream in(&file); // QDataStream is also possible for serialised data
    // out.setVersion(QDataStream::Qt_6_3);

    QString textConfigFull = file.readAll();
    QStringList textConfig = textConfigFull.split("\n");

    clearAll();

    // TODO: read file version
    // TODO: read filenames where data is created

    bool ok = treeModel->fromTextConfig(textConfig);

    // if(ok)
    //  TODO: update status bar with message indicating the file was saved
}

void GloxiniaConfigurator::clearAll(void)
{
    // remove all items from the tree
    qInfo() << "Number of rows:" << treeModel->rowCount();
    treeModel->removeRows(0, treeModel->rowCount());
}

void GloxiniaConfigurator::selectDataFile(void)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Open Data File"), "", tr("Comma Seperated Values (*.csv);;All Files (*)"));

    // save only if filename is non-empty
    if (fileName.isEmpty())
        return;

    if (dataFile != nullptr)
    {
        dataFile->close();
        delete dataFile;
    }

    if (dataStream != nullptr)
    {
        delete dataStream;
    }

    dataFile = new QFile(fileName);
    if (!dataFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), dataFile->errorString());
        return;
    }

    // create a new datastream
    dataStream = new QDataStream(dataFile);
    dataStream->setVersion(QDataStream::Qt_6_3);
}
