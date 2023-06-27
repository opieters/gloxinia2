#include "gliaengine.h"


GLIAEngine::GLIAEngine(void)
{    
    freqHigh = GLIAEngineFrequency::FREQ_50KHZ;
    freqLow = GLIAEngineFrequency::FREQ_10HZ;

    mode = GLIAEngineMode::LIA_5KHZ;
}

GLIAEngine::GLIAEngine(const GLIAEngine& e)
{
    freqHigh = e.freqHigh;
    freqLow = e.freqLow;
    mode = e.mode;
}

GLIAEngine::~GLIAEngine() {
}



void GLIAEngine::setFrequencyHigh(const GLIAEngine::GLIAEngineFrequency freq)
{
    freqHigh = freq;
}

void GLIAEngine::setFrequencyLow(const GLIAEngine::GLIAEngineFrequency freq)
{
    freqLow = freq;
}
void GLIAEngine::setMode(const GLIAEngine::GLIAEngineMode mode)
{
    this->mode = mode;
}

GLIAEngine::GLIAEngineFrequency GLIAEngine::getFrequencyHigh(void) const
{
    return freqHigh;
}
GLIAEngine::GLIAEngineFrequency GLIAEngine::getFrequencyLow(void) const
{
    return freqLow;
}
GLIAEngine::GLIAEngineMode GLIAEngine::getMode(void) const
{
    return mode;
}
