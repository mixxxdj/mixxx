#ifndef ENGINEFILTELINKWITZRILEY4_H
#define ENGINEFILTELINKWITZRILEY4_H

#include "engine/filters/enginefilteriir.h"

class EngineFilterLinkwitzRiley4Low : public EngineFilterIIR<4, IIR_LP> {
    Q_OBJECT
  public:
    EngineFilterLinkwitzRiley4Low(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};


class EngineFilterLinkwitzRiley4High : public EngineFilterIIR<4, IIR_HP> {
    Q_OBJECT
  public:
    EngineFilterLinkwitzRiley4High(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};

#endif // ENGINEFILTERLINKWITZRILEY4_H
