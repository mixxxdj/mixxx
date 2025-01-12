#pragma once

#include "audio/types.h"
#include "engine/filters/enginefilteriir.h"

class EngineFilterBessel8Low : public EngineFilterIIR<8, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterBessel8Low(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    // This function selects a corner frequency near the
    // desiredCorner1Ratio freqCorner / sampleRate
    // the produces an integer group delay at the passband
    // Optimized for freqCorner / 20
    int setFrequencyCornersForIntDelay(double desiredCorner1Ratio, int maxDelay);
};

class EngineFilterBessel8Band : public EngineFilterIIR<16, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBessel8Band(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
};

class EngineFilterBessel8High : public EngineFilterIIR<8, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterBessel8High(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};
