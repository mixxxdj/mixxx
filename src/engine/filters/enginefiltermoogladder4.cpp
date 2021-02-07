#include "engine/filters/enginefiltermoogladder4.h"

#include "moc_enginefiltermoogladder4.cpp"

EngineFilterMoogLadder4Low::EngineFilterMoogLadder4Low(int sampleRate,
                                               double freqCorner1,
                                               double resonance)
        : EngineFilterMoogLadderBase(sampleRate, (float)freqCorner1, (float)resonance) {
}

EngineFilterMoogLadder4High::EngineFilterMoogLadder4High(int sampleRate,
                                                 double freqCorner1,
                                                 double resonance)
        : EngineFilterMoogLadderBase(sampleRate, (float)freqCorner1, (float)resonance) {
}
