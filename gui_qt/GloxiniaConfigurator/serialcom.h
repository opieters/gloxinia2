#ifndef SERIALCOM_H
#define SERIALCOM_H

#include <QSerialPort>
#include <string>


class SerialCom
{
public:
    SerialCom(std::string portName);
    ~SerialCom();

private:
    std::string portName;
};

#endif // SERIALCOM_H
