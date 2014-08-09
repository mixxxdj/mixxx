#include "engine/enginefilterbessel8.h"


EngineFilterBessel8Low::EngineFilterBessel8Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterBessel8Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs("LpBe8", sampleRate, freqCorner1);
}


EngineFilterBessel8Band::EngineFilterBessel8Band(int sampleRate, double freqCorner1,
                                         double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterBessel8Band::setFrequencyCorners(int sampleRate,
                                             double freqCorner1,
                                             double freqCorner2) {
    setCoefs("BpBe8", sampleRate, freqCorner1, freqCorner2);
}


EngineFilterBessel8High::EngineFilterBessel8High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterBessel8High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs("HpBe8", sampleRate, freqCorner1);
}
