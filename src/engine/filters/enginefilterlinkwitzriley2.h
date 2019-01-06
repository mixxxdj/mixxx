#ifndef ENGINEFILTELINKWITZRILEY2_H
#define ENGINEFILTELINKWITZRILEY2_H

#include "engine/filters/enginefilteriir.h"

class EngineFilterLinkwitzRiley2Low : public EngineFilterIIR<2, IIR_LP2> {
    Q_OBJECT
  public:
    EngineFilterLinkwitzRiley2Low(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};


class EngineFilterLinkwitzRiley2High : public EngineFilterIIR<2, IIR_HP2> {
    Q_OBJECT
  public:
    EngineFilterLinkwitzRiley2High(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
};

#endif // ENGINEFILTERLINKWITZRILEY2_H
