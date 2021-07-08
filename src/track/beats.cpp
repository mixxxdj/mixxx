#include "track/beats.h"

namespace mixxx {

int Beats::numBeatsInRange(audio::FramePos startPosition, audio::FramePos endPosition) const {
    audio::FramePos lastPosition = audio::kStartFramePos;
    int i = 1;
    while (lastPosition < endPosition) {
        lastPosition = findNthBeat(startPosition, i);
        if (!lastPosition.isValid()) {
            break;
        }
        i++;
    }
    return i - 2;
};

audio::FramePos Beats::findNBeatsFromPosition(audio::FramePos position, double beats) const {
    audio::FramePos prevBeatPosition;
    audio::FramePos nextBeatPosition;

    if (!findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, true)) {
        return position;
    }
    const audio::FrameDiff_t fromFractionBeats = (position - prevBeatPosition) /
            (nextBeatPosition - prevBeatPosition);
    const audio::FrameDiff_t beatsFromPrevBeat = fromFractionBeats + beats;

    const int fullBeats = static_cast<int>(beatsFromPrevBeat);
    const audio::FrameDiff_t fractionBeats = beatsFromPrevBeat - fullBeats;

    // Add the length between this beat and the fullbeats'th beat
    // to the end position
    audio::FramePos nthBeatPosition;
    if (fullBeats > 0) {
        nthBeatPosition = findNthBeat(nextBeatPosition, fullBeats);
    } else {
        nthBeatPosition = findNthBeat(prevBeatPosition, fullBeats - 1);
    }

    if (!nthBeatPosition.isValid()) {
        return position;
    }

    // Add the fraction of the beat
    if (fractionBeats != 0) {
        nextBeatPosition = findNthBeat(nthBeatPosition, 2);
        if (!nextBeatPosition.isValid()) {
            return position;
        }
        nthBeatPosition += (nextBeatPosition - nthBeatPosition) * fractionBeats;
    }

    return nthBeatPosition;
};

} // namespace mixxx
