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
    findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false);
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

} // namespace mixxx
