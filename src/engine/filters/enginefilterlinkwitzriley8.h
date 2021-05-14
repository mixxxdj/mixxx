#pragma once

#include "engine/filters/enginefilteriir.h"

class EngineFilterLinkwitzRiley8Low : public EngineFilterIIR<8, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterLinkwitzRiley8Low(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};


class EngineFilterLinkwitzRiley8High : public EngineFilterIIR<8, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterLinkwitzRiley8High(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};
