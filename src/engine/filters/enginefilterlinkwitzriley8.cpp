#include "engine/filters/enginefilterlinkwitzriley8.h"


EngineFilterLinkwitzRiley8Low::EngineFilterLinkwitzRiley8Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley8Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs2(sampleRate, 4,
            "LpBu4", freqCorner1, 0, 0,
            "LpBu4", freqCorner1, 0, 0);
}

EngineFilterLinkwitzRiley8High::EngineFilterLinkwitzRiley8High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley8High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs2(sampleRate, 4,
            "HpBu4", freqCorner1, 0, 0,
            "HpBu4", freqCorner1, 0, 0);
}
