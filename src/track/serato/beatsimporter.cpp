#include "track/serato/beatsimporter.h"

#include "track/serato/tags.h"

namespace mixxx {

SeratoBeatsImporter::SeratoBeatsImporter()
        : BeatsImporter(),
          m_pTerminalMarker(nullptr) {
}

SeratoBeatsImporter::SeratoBeatsImporter(
        QList<SeratoBeatGridNonTerminalMarkerPointer> nonTerminalMarkers,
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
        SeratoBeatGridNonTerminalMarkerPointer pMarker = m_nonTerminalMarkers.at(i);
        beatPositionMillis =
                static_cast<double>(pMarker->positionSecs()) * 1000 +
                timingOffsetMillis;
        VERIFY_OR_DEBUG_ASSERT(pMarker->positionSecs() >= 0 &&
                pMarker->beatsTillNextMarker() > 0) {
            return {};
        }
        double nextBeatPositionMillis =
                static_cast<double>(i == (m_nonTerminalMarkers.size() - 1)
                                ? m_pTerminalMarker->positionSecs()
                                : m_nonTerminalMarkers.at(i + 1)
                                          ->positionSecs()) *
                        1000 +
                timingOffsetMillis;
        VERIFY_OR_DEBUG_ASSERT(nextBeatPositionMillis > beatPositionMillis) {
            return {};
        }
        beatLengthMillis = (nextBeatPositionMillis - beatPositionMillis) /
                pMarker->beatsTillNextMarker();

        beats.resize(beats.size() + pMarker->beatsTillNextMarker());
        for (quint32 j = 0; j < pMarker->beatsTillNextMarker(); ++j) {
            beats.append(streamInfo.getSignalInfo().millis2frames(
                    beatPositionMillis + (j * beatLengthMillis)));
        }
    }

    // Calculate remaining beat positions until track end
    beatPositionMillis =
            static_cast<double>(m_pTerminalMarker->positionSecs()) * 1000 +
            timingOffsetMillis;
    beatLengthMillis = 60'000.0 / static_cast<double>(m_pTerminalMarker->bpm());
    VERIFY_OR_DEBUG_ASSERT(m_pTerminalMarker->positionSecs() >= 0 && beatLengthMillis > 0) {
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
