#include "engine/enginefilterlinkwitzriley8.h"


EngineFilterLinkwtzRiley8Low::EngineFilterLinkwtzRiley8Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwtzRiley8Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs2(sampleRate, 4,
            "LpBu4", freqCorner1, 0, 0,
            "LpBu4", freqCorner1, 0, 0);
}

EngineFilterLinkwtzRiley8High::EngineFilterLinkwtzRiley8High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwtzRiley8High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs2(sampleRate, 4,
            "HpBu4", freqCorner1, 0, 0,
            "HpBu4", freqCorner1, 0, 0);
}
