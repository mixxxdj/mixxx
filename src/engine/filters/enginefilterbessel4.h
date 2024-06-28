#pragma once

#include "audio/types.h"
#include "engine/filters/enginefilteriir.h"

class EngineFilterBessel4Low : public EngineFilterIIR<4, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterBessel4Low(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    // This function selects a corner frequency near the
    // desiredCorner1Ratio freqCorner / sampleRate
    // the produces an integer group delay at the passband
    // Optimized for freqCorner / 20
    int setFrequencyCornersForIntDelay(double desiredCorner1Ratio, int maxDelay);
};

class EngineFilterBessel4Band : public EngineFilterIIR<8, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBessel4Band(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
};

class EngineFilterBessel4High : public EngineFilterIIR<4, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterBessel4High(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};
