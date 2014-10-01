#include "engine/enginefiltermoogladder4.h"
#include "util/math.h"

// Moog 24 dB/oct resonant lowpass VCF
// References: CSound source code, Stilson/Smith CCRMA paper.
// Modified by paul.kellett@maxim.abel.co.uk July 2000
// Found at http://www.musicdsp.org/archive.php?classid=3#180

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


