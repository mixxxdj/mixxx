#include "engine/filters/enginefilterbutterworth8.h"

EngineFilterButterworth8Low::EngineFilterButterworth8Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth8Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs("LpBu8", sampleRate, freqCorner1);
}


EngineFilterButterworth8Band::EngineFilterButterworth8Band(int sampleRate, double freqCorner1,
                                         double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterButterworth8Band::setFrequencyCorners(int sampleRate,
                                             double freqCorner1,
                                             double freqCorner2) {
    setCoefs("BpBu8", sampleRate, freqCorner1, freqCorner2);
}

EngineFilterButterworth8High::EngineFilterButterworth8High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth8High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs("HpBu8", sampleRate, freqCorner1);
}
