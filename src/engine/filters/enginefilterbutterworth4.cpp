#include "engine/filters/enginefilterbutterworth4.h"


EngineFilterButterworth4Low::EngineFilterButterworth4Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth4Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs("LpBu4", sampleRate, freqCorner1);
}


EngineFilterButterworth4Band::EngineFilterButterworth4Band(int sampleRate, double freqCorner1,
                                         double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterButterworth4Band::setFrequencyCorners(int sampleRate,
                                             double freqCorner1,
                                             double freqCorner2) {
    setCoefs("BpBu4", sampleRate, freqCorner1, freqCorner2);
}


EngineFilterButterworth4High::EngineFilterButterworth4High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth4High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs("HpBu4", sampleRate, freqCorner1);
}
