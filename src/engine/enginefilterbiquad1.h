#ifndef ENGINEFILTERBIQUAD1_H
#define ENGINEFILTERBIQUAD1_H

#include "engine/enginefilteriir.h"

class EngineFilterBiquad1LowShelving : public EngineFilterIIR<5, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBiquad1LowShelving(int sampleRate, double centerFreq, double Q);
    void setFrequencyCorners(int sampleRate, double centerFreq,
                             double Q, double dBgain);
};

class EngineFilterBiquad1Peaking : public EngineFilterIIR<5, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBiquad1Peaking(int sampleRate, double centerFreq, double Q);
    void setFrequencyCorners(int sampleRate, double centerFreq,
                             double Q, double dBgain);
};

class EngineFilterBiquad1HighShelving : public EngineFilterIIR<5, IIR_BP> {
    Q_OBJECT
  public:
    EngineFilterBiquad1HighShelving(int sampleRate, double centerFreq, double Q);
    void setFrequencyCorners(int sampleRate, double centerFreq,
                             double Q, double dBgain);
};

#endif // ENGINEFILTERBIQUAD1_H
