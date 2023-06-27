#ifndef GLIAENGINE_H
#define GLIAENGINE_H

#include <Qt>
#include <QString>
#include <QAbstractItemModel>


class GLIAEngine
{

public:
    GLIAEngine(void);
    GLIAEngine(const GLIAEngine&);
    virtual ~GLIAEngine();

    enum GLIAEngineFrequency
    {
        FREQ_10HZ = 0,
        FREQ_20HZ = 1,
        FREQ_50HZ = 2,
        FREQ_100HZ = 3,
        FREQ_200HZ = 4,
        FREQ_500HZ = 5,
        FREQ_1KHZ = 6,
        FREQ_2KHZ = 7,
        FREQ_5KHZ = 8,
        FREQ_10KHZ = 9,
        FREQ_20KHZ = 10,
        FREQ_50KHZ = 11,
    };

    enum GLIAEngineMode {
        LIA_50KHZ = 0,
        LIA_25KHZ = 1,
        LIA_10KHZ = 2,
        LIA_5KHZ = 3,
        LIA_FS = 4
    };

    void setFrequencyHigh(const GLIAEngineFrequency freq);
    void setFrequencyLow(const GLIAEngineFrequency freq);
    void setMode(const GLIAEngineMode mode);

    GLIAEngineFrequency getFrequencyHigh(void) const;
    GLIAEngineFrequency getFrequencyLow(void) const;
    GLIAEngineMode getMode(void) const;

protected:
    GLIAEngineFrequency freqHigh = FREQ_50KHZ;
    GLIAEngineFrequency freqLow = FREQ_10HZ;

    GLIAEngineMode mode = LIA_5KHZ;
};

Q_DECLARE_METATYPE(GLIAEngine *)


#endif
