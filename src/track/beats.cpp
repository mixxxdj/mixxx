#include "track/beats.h"

#include "track/beatgrid.h"
#include "track/beatmap.h"

namespace mixxx {

// static
mixxx::BeatsPointer Beats::fromByteArray(
        mixxx::audio::SampleRate sampleRate,
        const QString& beatsVersion,
        const QString& beatsSubVersion,
        const QByteArray& beatsSerialized) {
    mixxx::BeatsPointer pBeats = nullptr;
    if (beatsVersion == BEAT_GRID_1_VERSION || beatsVersion == BEAT_GRID_2_VERSION) {
        pBeats = mixxx::BeatGrid::fromByteArray(sampleRate, beatsSubVersion, beatsSerialized);
    } else if (beatsVersion == BEAT_MAP_VERSION) {
        pBeats = mixxx::BeatMap::fromByteArray(sampleRate, beatsSubVersion, beatsSerialized);
    } else {
        qWarning().nospace() << "Failed to deserialize Beats (" << beatsVersion
                             << "): Invalid beats version";
        return nullptr;
    }

    if (!pBeats) {
        qWarning().nospace() << "Failed to deserialize Beats (" << beatsVersion
                             << "): Parsing failed";
        return nullptr;
    }

    qDebug().nospace() << "Successfully deserialized Beats (" << beatsVersion << ")";
    return pBeats;
}

// static
BeatsPointer Beats::fromConstTempo(
        audio::SampleRate sampleRate,
        audio::FramePos position,
        Bpm bpm,
        const QString& subVersion) {
    return BeatGrid::makeBeatGrid(sampleRate, bpm, position, subVersion);
}

// static
BeatsPointer Beats::fromBeatPositions(
        audio::SampleRate sampleRate,
        const QVector<mixxx::audio::FramePos>& beatPositions,
        const QString& subVersion) {
    return BeatMap::makeBeatMap(sampleRate, subVersion, beatPositions);
}

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

audio::FramePos Beats::findNextBeat(audio::FramePos position) const {
    return findNthBeat(position, 1);
}

audio::FramePos Beats::findPrevBeat(audio::FramePos position) const {
    return findNthBeat(position, -1);
}

audio::FramePos Beats::findClosestBeat(audio::FramePos position) const {
    if (!isValid()) {
        return audio::kInvalidFramePos;
    }
    audio::FramePos prevBeatPosition;
    audio::FramePos nextBeatPosition;
    findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, true);
    if (!prevBeatPosition.isValid()) {
        // If both positions are invalid, we correctly return an invalid position.
        return nextBeatPosition;
    }

    if (!nextBeatPosition.isValid()) {
        return prevBeatPosition;
    }

    // Both position are valid, return the closest position.
    return (nextBeatPosition - position > position - prevBeatPosition)
            ? prevBeatPosition
            : nextBeatPosition;
}

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
