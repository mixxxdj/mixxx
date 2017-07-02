#include "engine/enginefilterlinkwitzriley2.h"


EngineFilterLinkwtzRiley2Low::EngineFilterLinkwtzRiley2Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwtzRiley2Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs2(sampleRate, 1,
            "LpBu1", freqCorner1, 0, 0,
            "LpBu1", freqCorner1, 0, 0);
}

EngineFilterLinkwtzRiley2High::EngineFilterLinkwtzRiley2High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwtzRiley2High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs2(sampleRate, 1,
            "HpBu1", freqCorner1, 0, 0,
            "HpBu1", freqCorner1, 0, 0);
}
