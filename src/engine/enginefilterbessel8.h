#ifndef ENGINEFILTERBESSEL8_H
#define ENGINEFILTERBESSEL8_H

#include "engine/enginefilteriir.h"

class EngineFilterBessel8Low : public EngineFilterIIR<8, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterBessel8Low(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
    inline double delayFactor() {
        // group delay at 1 Hz freqCorner1 and 1 Hz Samplerate
        return 0.5067709751;
    }
};

class EngineFilterBessel8Band : public EngineFilterIIR<16, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBessel8Band(int sampleRate, double freqCorner1,
            double freqCorner2);
    void setFrequencyCorners(int sampleRate, double freqCorner1,
            double freqCorner2);
};

class EngineFilterBessel8High : public EngineFilterIIR<8, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterBessel8High(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};

#endif // ENGINEFILTERBESSEL8_H
