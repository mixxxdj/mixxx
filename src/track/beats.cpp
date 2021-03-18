#include "track/beats.h"

namespace mixxx {

int Beats::numBeatsInRange(double dStartSample, double dEndSample) const {
    double dLastCountedBeat = 0.0;
    int iBeatsCounter;
    for (iBeatsCounter = 1; dLastCountedBeat < dEndSample; iBeatsCounter++) {
        dLastCountedBeat = findNthBeat(dStartSample, iBeatsCounter);
        if (dLastCountedBeat == -1) {
            break;
        }
    }
    return iBeatsCounter - 2;
};

double Beats::findNBeatsFromSample(double fromSample, double beats) const {
    double nthBeat;
    double prevBeat;
    double nextBeat;

    if (!findPrevNextBeats(fromSample, &prevBeat, &nextBeat)) {
        return fromSample;
    }
    double fromFractionBeats = (fromSample - prevBeat) / (nextBeat - prevBeat);
    double beatsFromPrevBeat = fromFractionBeats + beats;

    int fullBeats = static_cast<int>(beatsFromPrevBeat);
    double fractionBeats = beatsFromPrevBeat - fullBeats;

    // Add the length between this beat and the fullbeats'th beat
    // to the end position
    if (fullBeats > 0) {
        nthBeat = findNthBeat(nextBeat, fullBeats);
    } else {
        nthBeat = findNthBeat(prevBeat, fullBeats - 1);
    }

    if (nthBeat == -1) {
        return fromSample;
    }

    // Add the fraction of the beat
    if (fractionBeats != 0) {
        nextBeat = findNthBeat(nthBeat, 2);
        if (nextBeat == -1) {
            return fromSample;
        }
        nthBeat += (nextBeat - nthBeat) * fractionBeats;
    }

    return nthBeat;
};

} // namespace mixxx
