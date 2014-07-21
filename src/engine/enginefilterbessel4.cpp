#include "engine/enginefilterbessel4.h"


EngineFilterBessel4Low::EngineFilterBessel4Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterBessel4Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs("LpBe4", sampleRate, freqCorner1);
}


EngineFilterBessel4Band::EngineFilterBessel4Band(int sampleRate, double freqCorner1,
                                         double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterBessel4Band::setFrequencyCorners(int sampleRate,
                                             double freqCorner1,
                                             double freqCorner2) {
    setCoefs("BpBe4", sampleRate, freqCorner1, freqCorner2);
}


EngineFilterBessel4High::EngineFilterBessel4High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterBessel4High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs("HpBe4", sampleRate, freqCorner1);
}
