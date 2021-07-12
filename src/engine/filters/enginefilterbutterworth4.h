#pragma once

#include "engine/filters/enginefilteriir.h"

class EngineFilterButterworth4Low : public EngineFilterIIR<4, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterButterworth4Low(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};

class EngineFilterButterworth4Band : public EngineFilterIIR<8, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterButterworth4Band(int sampleRate, double freqCorner1,
            double freqCorner2);
    void setFrequencyCorners(int sampleRate, double freqCorner1,
            double freqCorner2);
};

class EngineFilterButterworth4High : public EngineFilterIIR<4, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterButterworth4High(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};
