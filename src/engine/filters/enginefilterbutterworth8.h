#pragma once

#include "audio/types.h"
#include "engine/filters/enginefilteriir.h"

class EngineFilterButterworth8Low : public EngineFilterIIR<8, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterButterworth8Low(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};

class EngineFilterButterworth8Band : public EngineFilterIIR<16, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterButterworth8Band(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
};

class EngineFilterButterworth8High : public EngineFilterIIR<8, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterButterworth8High(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};
