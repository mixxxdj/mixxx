#include "track/serato/beatsimporter.h"

#include "track/serato/tags.h"

namespace mixxx {

SeratoBeatsImporter::SeratoBeatsImporter(
        QList<SeratoBeatGridNonTerminalMarkerPointer> nonTerminalMarkers,
        SeratoBeatGridTerminalMarkerPointer pTerminalMarker)
        : BeatsImporter(),
          m_nonTerminalMarkers(nonTerminalMarkers),
          m_pTerminalMarker(pTerminalMarker) {
    DEBUG_ASSERT(pTerminalMarker);
}

bool SeratoBeatsImporter::isEmpty() const {
    return m_nonTerminalMarkers.isEmpty() && !m_pTerminalMarker;
};

QVector<double> SeratoBeatsImporter::importBeatsWithCorrectTiming(
        const QString& filePath, const audio::StreamInfo& streamInfo) {
    VERIFY_OR_DEBUG_ASSERT(!isEmpty()) {
        return {};
    }

    double timingOffsetMillis = SeratoTags::guessTimingOffsetMillis(
            filePath, streamInfo.getSignalInfo());

    QVector<double> beats;
    double beatPositionMillis;
    double beatLengthMillis;
    // Calculate beat positions for non-terminal markers
    for (int i = 0; i < m_nonTerminalMarkers.size(); ++i) {
        SeratoBeatGridNonTerminalMarkerPointer pMarker =
                m_nonTerminalMarkers.at(i);
        beatPositionMillis = static_cast<double>(pMarker->positionMillis()) +
                timingOffsetMillis;
        VERIFY_OR_DEBUG_ASSERT(
                pMarker->positionMillis() >= 0 && pMarker->beatsTillNextMarker() > 0) {
            return {};
        }
        double nextBeatPositionMillis =
                static_cast<double>((i == m_nonTerminalMarkers.size() - 1)
                                ? m_pTerminalMarker->positionMillis()
                                : m_nonTerminalMarkers.at(i)
                                          ->positionMillis()) +
                timingOffsetMillis;
        VERIFY_OR_DEBUG_ASSERT(nextBeatPositionMillis > beatPositionMillis) {
            return {};
        }
        beatLengthMillis = (nextBeatPositionMillis - beatPositionMillis) /
                pMarker->beatsTillNextMarker();

        beats.resize(beats.size() + pMarker->beatsTillNextMarker());
        for (int j = 0; j < pMarker->beatsTillNextMarker(); ++j) {
            beats.append(streamInfo.getSignalInfo().millis2frames(
                    beatPositionMillis + (j * beatLengthMillis)));
        }
    }

    // Calculate remaining beat positions until track end
    beatPositionMillis =
            static_cast<double>(m_pTerminalMarker->positionMillis()) +
            timingOffsetMillis;
    beatLengthMillis = 60'000.0 / static_cast<double>(m_pTerminalMarker->bpm());
    VERIFY_OR_DEBUG_ASSERT(m_pTerminalMarker->positionMillis() >= 0 && beatLengthMillis > 0) {
        return {};
    }
    while (beatPositionMillis < streamInfo.getDuration().toDoubleMillis()) {
        beats.append(streamInfo.getSignalInfo().millis2frames(beatPositionMillis));
        beatPositionMillis += beatLengthMillis;
    }

    m_nonTerminalMarkers.clear();
    m_pTerminalMarker.reset();

    return beats;
}

} // namespace mixxx
