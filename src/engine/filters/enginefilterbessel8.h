#pragma once

#include "engine/filters/enginefilteriir.h"

class EngineFilterBessel8Low : public EngineFilterIIR<8, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterBessel8Low(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
    // This function selects a corner frequency near the
    // desiredCorner1Ratio freqCorner / sampleRate
    // the produces an integer group delay at the passband
    // Optimized for freqCorner / 20
    int setFrequencyCornersForIntDelay(double desiredCorner1Ratio, int maxDelay);
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
