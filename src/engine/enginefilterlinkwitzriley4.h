#ifndef ENGINEFILTELINKWITZRILEY4_H
#define ENGINEFILTELINKWITZRILEY4_H

#include "engine/enginefilteriir.h"

class EngineFilterLinkwtzRiley4Low : public EngineFilterIIR<4, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterLinkwtzRiley4Low(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};


class EngineFilterLinkwtzRiley4High : public EngineFilterIIR<4, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterLinkwtzRiley4High(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};

#endif // ENGINEFILTERLINKWITZRILEY8_H
