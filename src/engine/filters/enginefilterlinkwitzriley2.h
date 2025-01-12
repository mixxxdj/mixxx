#pragma once

#include "audio/types.h"
#include "engine/filters/enginefilteriir.h"

class EngineFilterLinkwitzRiley2Low : public EngineFilterIIR<2, IIR_LP2> {
    Q_OBJECT
  public:
    EngineFilterLinkwitzRiley2Low(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};


class EngineFilterLinkwitzRiley2High : public EngineFilterIIR<2, IIR_HP2> {
    Q_OBJECT
  public:
    EngineFilterLinkwitzRiley2High(mixxx::audio::SampleRate sampleRate, double freqCorner1);
    void setFrequencyCorners(mixxx::audio::SampleRate sampleRate, double freqCorner1);
};
