#pragma once

#include "audio/types.h"
#include "engine/filters/enginefilteriir.h"

class EngineFilterButterworth2Low : public EngineFilterIIR<2, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterButterworth2Low(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};

class EngineFilterButterworth2Band : public EngineFilterIIR<4, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterButterworth2Band(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
};

class EngineFilterButterworth2High : public EngineFilterIIR<2, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterButterworth2High(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};
