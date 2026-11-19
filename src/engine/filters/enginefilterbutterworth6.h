#pragma once

#include "audio/types.h"
#include "engine/filters/enginefilteriir.h"

class EngineFilterButterworth6Low : public EngineFilterIIR<6, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterButterworth6Low(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};

class EngineFilterButterworth6Band : public EngineFilterIIR<12, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterButterworth6Band(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
            double freqCorner1,
            double freqCorner2);
};

class EngineFilterButterworth6High : public EngineFilterIIR<6, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterButterworth6High(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};
