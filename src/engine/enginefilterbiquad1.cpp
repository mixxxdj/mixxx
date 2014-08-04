#include "engine/enginefilterbiquad1.h"

EngineFilterBiquad1Band::EngineFilterBiquad1Band(int sampleRate, double centerFreq) {
    setFrequencyCorners(sampleRate, centerFreq);
}

void EngineFilterBiquad1Band::setFrequencyCorners(int sampleRate, double centerFreq) {
    setCoefs("BpBq/2", sampleRate, centerFreq);
}
