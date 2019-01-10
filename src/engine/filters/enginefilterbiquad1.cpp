#include <stdio.h>
#include "engine/filters/enginefilterbiquad1.h"

EngineFilterBiquad1LowShelving::EngineFilterBiquad1LowShelving(int sampleRate,
                                                               double centerFreq,
                                                               double Q) {
    m_startFromDry = true;
    setFrequencyCorners(sampleRate, centerFreq, Q, 0);
}

void EngineFilterBiquad1LowShelving::setFrequencyCorners(int sampleRate,
                                                         double centerFreq,
                                                         double Q,
                                                         double dBgain) {
    format_fidspec(m_spec, sizeof(m_spec), "LsBq/%.10f/%.10f", Q, dBgain);
    setCoefs(m_spec, sampleRate, centerFreq);
}

EngineFilterBiquad1Peaking::EngineFilterBiquad1Peaking(int sampleRate,
                                                       double centerFreq, double Q) {
    m_startFromDry = true;
    setFrequencyCorners(sampleRate, centerFreq, Q, 0);
}

void EngineFilterBiquad1Peaking::setFrequencyCorners(int sampleRate,
                                                     double centerFreq,
                                                     double Q,
                                                     double dBgain) {
    format_fidspec(m_spec, sizeof(m_spec), "PkBq/%.10f/%.10f", Q, dBgain);
    setCoefs(m_spec, sampleRate, centerFreq);
}

EngineFilterBiquad1HighShelving::EngineFilterBiquad1HighShelving(int sampleRate,
                                                                 double centerFreq,
                                                                 double Q) {
    m_startFromDry = true;
    setFrequencyCorners(sampleRate, centerFreq, Q, 0);
}

void EngineFilterBiquad1HighShelving::setFrequencyCorners(int sampleRate,
                                                          double centerFreq,
                                                          double Q,
                                                          double dBgain) {
    format_fidspec(m_spec, sizeof(m_spec), "HsBq/%.10f/%.10f", Q, dBgain);
    setCoefs(m_spec, sampleRate, centerFreq);
}

EngineFilterBiquad1Low::EngineFilterBiquad1Low(int sampleRate,
                                               double centerFreq,
                                               double Q,
                                               bool startFromDry) {
    m_startFromDry = startFromDry;
    setFrequencyCorners(sampleRate, centerFreq, Q);
}

void EngineFilterBiquad1Low::setFrequencyCorners(int sampleRate,
                                                 double centerFreq,
                                                 double Q) {
    format_fidspec(m_spec, sizeof(m_spec), "LpBq/%.10f", Q);
    setCoefs(m_spec, sampleRate, centerFreq);
}

EngineFilterBiquad1Band::EngineFilterBiquad1Band(int sampleRate,
                                                 double centerFreq,
                                                 double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q);
}

void EngineFilterBiquad1Band::setFrequencyCorners(int sampleRate,
                                                  double centerFreq,
                                                  double Q) {
    format_fidspec(m_spec, sizeof(m_spec), "BpBq/%.10f", Q);
    setCoefs(m_spec, sampleRate, centerFreq);
}

EngineFilterBiquad1High::EngineFilterBiquad1High(int sampleRate,
                                                 double centerFreq,
                                                 double Q,
                                                 bool startFromDry) {
    m_startFromDry = startFromDry;
    setFrequencyCorners(sampleRate, centerFreq, Q);
}

void EngineFilterBiquad1High::setFrequencyCorners(int sampleRate,
                                                  double centerFreq,
                                                  double Q) {
    format_fidspec(m_spec, sizeof(m_spec), "HpBq/%.10f", Q);
    setCoefs(m_spec, sampleRate, centerFreq);
}
