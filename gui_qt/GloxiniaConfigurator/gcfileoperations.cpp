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
#include <QJsonDocument>
#include <QJsonObject>

void GloxiniaConfigurator::saveProject(void)
{
    if(settings.success)
    {
        // try to open and write file data
        QDir projectDir(settings.projectDir);
        if(!projectDir.mkpath(settings.projectDir)){
            QMessageBox msgBox;
            msgBox.setText("Unable to create project directory " + settings.projectDir + ".");
            msgBox.exec();
            return;
        }

        QString filePath = QDir::cleanPath(settings.projectDir + "/" + settings.projectName + ".gc");

        // check if file exists
        if(QFile::exists(filePath)){
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "File already exists.", "There already exists a file on the disk at " + filePath + ". Overwrite?", QMessageBox::Yes | QMessageBox::No);
            if(reply == QMessageBox::No){
                return;
            }
        }

        QFile file(filePath);

        if(file.open(QIODevice::WriteOnly)){
            QJsonDocument fileData;
            QJsonObject data;

            data.insert("project_name", settings.projectName);
            data.insert("com_port", settings.comPort);
            data.insert("baudrate", settings.baudrate);
            data.insert("work_offline", settings.workOffline);
            data.insert("message_buffer_size", settings.messageBufferSize);
            data.insert("plot_buffer_window", settings.plotBufferWindow);
            data.insert("global_measurement_period", settings.globalMeasurementPeriod);

            fileData.setObject(data);
            file.write(fileData.toJson());

            file.close();
        } else {
            settings.success = false;

            QMessageBox msgBox;
            msgBox.setText("Unable to open file at " + filePath + ".");
            msgBox.exec();
            return;
        }
    }
}
void GloxiniaConfigurator::newProject(void)
{
    closeSerialPort();
    settings.success = false;

    newProjectDialog->setWindowModality(Qt::ApplicationModal);
    if(newProjectDialog->exec() == QDialog::Rejected){
        showStatusMessage(tr("No file open."));
        return;
    }
    settings = newProjectDialog->getProject();

    if(settings.success)
    {
        // try to open and write file data
        QDir projectDir(settings.projectDir);
        if(!projectDir.mkpath(settings.projectDir)){
            QMessageBox msgBox;
            msgBox.setText("Unable to create project directory " + settings.projectDir + ".");
            msgBox.exec();
            return;
        }

        QString filePath = QDir::cleanPath(settings.projectDir + "/" + settings.projectName + ".gc");

        // check if file exists
        // check if file exists
        if(QFile::exists(filePath)){
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "File already exists.", "There already exists a file on the disk at " + filePath + ". Overwrite?", QMessageBox::Yes | QMessageBox::No);
            if(reply == QMessageBox::No){
                return;
            }
        }

        QFile file(filePath);

        if(file.open(QIODevice::WriteOnly)){
            QJsonDocument fileData;
            QJsonObject data;

            data.insert("project_name", settings.projectName);
            data.insert("com_port", settings.comPort);
            data.insert("baudrate", settings.baudrate);
            data.insert("work_offline", settings.workOffline);
            data.insert("message_buffer_size", settings.messageBufferSize);
            data.insert("plot_buffer_window", settings.plotBufferWindow);
            data.insert("global_measurement_period", settings.globalMeasurementPeriod);
            fileData.setObject(data);
            file.write(fileData.toJson());

            file.close();
        } else {
            settings.success = false;

            QMessageBox msgBox;
            msgBox.setText("Unable to open file at " + filePath + ".");
            msgBox.exec();
            return;
        }
    } else {
        QMessageBox msgBox;
        msgBox.setText("Selected file not valid.");
        msgBox.exec();
        return;
    }

    // file saved, now connect to the device if not working offline
    if(!settings.workOffline){
        openSerialPort();
    }

    // update UI
    updateUI();
    showStatusMessage(tr("Created project ") + settings.projectName);
}

void GloxiniaConfigurator::openProject(void)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open project"),
                                                    settings.projectDir,
                                                   tr("GC files (*.gc)"));

    if(!fileName.isEmpty()){
        QFile file(fileName);

        if(!file.open(QFile::ReadOnly)){
            QMessageBox msgBox;
            msgBox.setText("Unable to open file at " + fileName + ".");
            msgBox.exec();
            return;
        }

        closeSerialPort();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
        QJsonObject jsonData = jsonDoc.object();
        QStringList keys = jsonData.keys();
        if((!keys.contains("project_name")) || (!keys.contains("com_port")) ||
                (!keys.contains("baudrate")) || (!keys.contains("work_offline"))){
            QMessageBox msgBox;
            msgBox.setText("Invalid project file selected.");
            msgBox.exec();
            return;
        }

        settings.projectName = jsonData.value("project_name").toString();
        settings.comPort = jsonData["com_port"].toString();
        settings.baudrate = jsonData["baudrate"].toInt();
        settings.workOffline = jsonData["work_offline"].toBool();
        settings.messageBufferSize = jsonData["message_buffer_size"].toInt();
        settings.plotBufferWindow = jsonData["plot_buffer_window"].toInt();
        settings.globalMeasurementPeriod = (quint16) jsonData["global_measurement_settings"].toInt();
        settings.projectDir = QDir::cleanPath(fileName.replace(settings.projectName + ".gc", ""));
        settings.success = true;
    }

    // file saved, now connect to the device if not working offline
    if(!settings.workOffline){
        openSerialPort();
    }

    // update UI
    updateUI();
    showStatusMessage(tr("Openened project ") + settings.projectName);
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
