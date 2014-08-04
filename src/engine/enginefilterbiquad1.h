#ifndef ENGINEFILTERBIQUAD1_H
#define ENGINEFILTERBIQUAD1_H

#include "engine/enginefilteriir.h"

class EngineFilterBiquad1Band : public EngineFilterIIR<2, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBiquad1Band(int sampleRate, double centerFreq, int Q);
    void setFrequencyCorners(int sampleRate, double centerFreq);
  private:
    QString specification;
};

#endif // ENGINEFILTERBIQUAD1_H
