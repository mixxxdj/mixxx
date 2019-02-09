#include "engine/filters/enginefilterlinkwitzriley2.h"


EngineFilterLinkwitzRiley2Low::EngineFilterLinkwitzRiley2Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley2Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs2(sampleRate, 1,
            "LpBu1", freqCorner1, 0, 0,
            "LpBu1", freqCorner1, 0, 0);
}

EngineFilterLinkwitzRiley2High::EngineFilterLinkwitzRiley2High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley2High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs2(sampleRate, 1,
            "HpBu1", freqCorner1, 0, 0,
            "HpBu1", freqCorner1, 0, 0);
}
