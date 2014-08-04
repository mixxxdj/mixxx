#include "engine/enginefilterbiquad1.h"

EngineFilterBiquad1Band::EngineFilterBiquad1Band(int sampleRate,
                                                 double centerFreq, int Q) {
    specification = QString("BpBq/%1").arg(Q);
    setFrequencyCorners(sampleRate, centerFreq);
}

void EngineFilterBiquad1Band::setFrequencyCorners(int sampleRate, double centerFreq) {
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}
