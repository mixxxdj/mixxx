#include "engine/filters/enginefilterlinkwitzriley4.h"


EngineFilterLinkwitzRiley4Low::EngineFilterLinkwitzRiley4Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley4Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs2(sampleRate, 2,
            "LpBu2", freqCorner1, 0, 0,
            "LpBu2", freqCorner1, 0, 0);
}

EngineFilterLinkwitzRiley4High::EngineFilterLinkwitzRiley4High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley4High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs2(sampleRate, 2,
            "HpBu2", freqCorner1, 0, 0,
            "HpBu2", freqCorner1, 0, 0);
}
