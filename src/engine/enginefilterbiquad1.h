#ifndef ENGINEFILTERBIQUAD1_H
#define ENGINEFILTERBIQUAD1_H

#include "engine/enginefilteriir.h"

class EngineFilterBiquad1Low : public EngineFilterIIR<5, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBiquad1Low(int sampleRate, double centerFreq, double Q);
    void setFrequencyCorners(int sampleRate, double centerFreq,
                             double Q, double dBgain);
};

class EngineFilterBiquad1Band : public EngineFilterIIR<5, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBiquad1Band(int sampleRate, double centerFreq, double Q);
    void setFrequencyCorners(int sampleRate, double centerFreq,
                             double Q, double dBgain);
};

class EngineFilterBiquad1High : public EngineFilterIIR<5, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBiquad1High(int sampleRate, double centerFreq, double Q);
    void setFrequencyCorners(int sampleRate, double centerFreq,
                             double Q, double dBgain);
};

#endif // ENGINEFILTERBIQUAD1_H
