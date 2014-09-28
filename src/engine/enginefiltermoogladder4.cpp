#include "engine/enginefiltermoogladder4.h"
#include "util/math.h"


EngineFilterMoogLadder4Low::EngineFilterMoogLadder4Low(int sampleRate,
                                               double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterMoogLadder4Low::setFrequencyCorners(int sampleRate,
                                                 double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs("LpMo4", sampleRate, freqCorner1);
}

EngineFilterMoogLadder4High::EngineFilterMoogLadder4High(int sampleRate,
                                                 double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterMoogLadder4High::setFrequencyCorners(int sampleRate,
                                                  double freqCorner1) {
    setCoefs("HpMo4", sampleRate, freqCorner1);
}
