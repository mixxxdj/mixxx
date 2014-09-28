#ifndef ENGINEFILTERMOOGLADDER4_H
#define ENGINEFILTERMOOGLADDER4_H

#include "engine/enginefilteriir.h"

class EngineFilterMoogLadder4Low : public EngineFilterIIR<4, IIR_LPMO> {
    Q_OBJECT
  public:
    EngineFilterMoogLadder4Low(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};


class EngineFilterMoogLadder4High : public EngineFilterIIR<4, IIR_HPMO> {
    Q_OBJECT
  public:
    EngineFilterMoogLadder4High(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};

#endif // ENGINEFILTERMOOGLADDER4_H
