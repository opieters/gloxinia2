#ifndef GCSAMPLEPOINT_H
#define GCSAMPLEPOINT_H

#include <QMap>

class GCSamplePoint
{
public:
    GCSamplePoint();

    bool addData(const QString& label, double value);

private:
    QMap<QString, double> data;
};

#endif // GCSAMPLEPOINT_H
