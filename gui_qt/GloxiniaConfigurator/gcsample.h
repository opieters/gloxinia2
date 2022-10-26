#ifndef GCSAMPLE_H
#define GCSAMPLE_H

#include <QMap>

class GCSample
{
public:
    GCSample();

private:
    QMap<QString, double> data;
};

#endif // GCSAMPLE_H
