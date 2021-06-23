#include "track/serato/beatsimporter.h"

#include "track/serato/tags.h"

namespace mixxx {

SeratoBeatsImporter::SeratoBeatsImporter()
        : BeatsImporter(),
          m_pTerminalMarker(nullptr) {
}

SeratoBeatsImporter::SeratoBeatsImporter(
        const QList<SeratoBeatGridNonTerminalMarkerPointer>& nonTerminalMarkers,
        SeratoBeatGridTerminalMarkerPointer pTerminalMarker)
        : BeatsImporter(),
          m_nonTerminalMarkers(nonTerminalMarkers),
          m_pTerminalMarker(pTerminalMarker) {
    DEBUG_ASSERT(pTerminalMarker);
}

bool SeratoBeatsImporter::isEmpty() const {
    return !m_pTerminalMarker;
};

QVector<double> SeratoBeatsImporter::importBeatsAndApplyTimingOffset(
        const QString& filePath, const audio::StreamInfo& streamInfo) {
    const audio::SignalInfo& signalInfo = streamInfo.getSignalInfo();
    const double timingOffsetMillis = SeratoTags::guessTimingOffsetMillis(
            filePath, signalInfo);

    return importBeatsAndApplyTimingOffset(timingOffsetMillis, signalInfo);
}

QVector<double> SeratoBeatsImporter::importBeatsAndApplyTimingOffset(
        double timingOffsetMillis, const audio::SignalInfo& signalInfo) {
    VERIFY_OR_DEBUG_ASSERT(!isEmpty()) {
        return {};
    }

    QVector<double> beats;
    double beatPositionMillis = 0;
    // Calculate beat positions for non-terminal markers
    for (int i = 0; i < m_nonTerminalMarkers.size(); ++i) {
        SeratoBeatGridNonTerminalMarkerPointer pMarker = m_nonTerminalMarkers.at(i);
        beatPositionMillis = static_cast<double>(pMarker->positionSecs()) * 1000;
        VERIFY_OR_DEBUG_ASSERT(pMarker->positionSecs() >= 0 &&
                pMarker->beatsTillNextMarker() > 0) {
            return {};
        }
        const double nextBeatPositionMillis =
                static_cast<double>(i == (m_nonTerminalMarkers.size() - 1)
                                ? m_pTerminalMarker->positionSecs()
                                : m_nonTerminalMarkers.at(i + 1)
                                          ->positionSecs()) *
                1000;
        VERIFY_OR_DEBUG_ASSERT(nextBeatPositionMillis > beatPositionMillis) {
            return {};
        }
        const double beatLengthMillis =
                (nextBeatPositionMillis - beatPositionMillis) /
                pMarker->beatsTillNextMarker();

        beats.reserve(beats.size() + pMarker->beatsTillNextMarker());
        for (quint32 j = 0; j < pMarker->beatsTillNextMarker(); ++j) {
            beats.append(signalInfo.millis2frames(
                    beatPositionMillis + timingOffsetMillis));
            beatPositionMillis += beatLengthMillis;
        }
    }

    // Calculate remaining beat positions between the last non-terminal marker
    // beat (or the start of the track if none exist) and the terminal marker,
    // using the given BPM.
    //
    //                beatPositionMillis          Terminal Marker
    //                     v                              v
    //     | | | | | | | | |[###### Remaining range ######]
    const double beatLengthMillis = 60000.0 / static_cast<double>(m_pTerminalMarker->bpm());
    VERIFY_OR_DEBUG_ASSERT(m_pTerminalMarker->positionSecs() >= 0 && beatLengthMillis > 0) {
        return {};
    }

    const double rangeEndBeatPositionMillis =
            static_cast<double>(m_pTerminalMarker->positionSecs() * 1000);

    if (beats.isEmpty()) {
        DEBUG_ASSERT(beatPositionMillis == 0);
        // If there are no beats yet (because there were no non-terminal
        // markers), beatPositionMillis will be 0. In that case we have to find
        // the offset backwards from the terminal marker position.
        // Because we're working with doubles, we can't use modulo and need to
        // implement it ourselves.
        const double divisor = std::floor(rangeEndBeatPositionMillis / beatLengthMillis);
        beatPositionMillis = rangeEndBeatPositionMillis - (divisor * beatLengthMillis);
    } else if (beatPositionMillis > rangeEndBeatPositionMillis) {
        beatPositionMillis = rangeEndBeatPositionMillis;
    }

    // Now fill the range with beats until the end is reached. Add a half beat
    // length, to make sure that the last beat is actually included.
    while (beatPositionMillis <= (rangeEndBeatPositionMillis + beatLengthMillis / 2)) {
        beats.append(signalInfo.millis2frames(
                beatPositionMillis + timingOffsetMillis));
        beatPositionMillis += beatLengthMillis;
    }

    m_nonTerminalMarkers.clear();
    m_pTerminalMarker.reset();

    return beats;
}

} // namespace mixxx
