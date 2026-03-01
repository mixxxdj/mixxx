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

BeatsPointer SeratoBeatsImporter::importBeatsAndApplyTimingOffset(
        const QString& filePath,
        const QString& fileType,
        const audio::StreamInfo& streamInfo) {
    const audio::SignalInfo& signalInfo = streamInfo.getSignalInfo();
    const double timingOffsetMillis = SeratoTags::guessTimingOffsetMillis(
            filePath, fileType, signalInfo);

    return importBeatsAndApplyTimingOffset(timingOffsetMillis, signalInfo);
}

BeatsPointer SeratoBeatsImporter::importBeatsAndApplyTimingOffset(
        double timingOffsetMillis, const audio::SignalInfo& signalInfo) {
    VERIFY_OR_DEBUG_ASSERT(!isEmpty()) {
        return nullptr;
    }

    const audio::FrameDiff_t timingOffsetFrames = signalInfo.millis2frames(timingOffsetMillis);

    std::vector<BeatMarker> markers;
    markers.reserve(m_nonTerminalMarkers.size());
    for (const auto& pMarker : std::as_const(m_nonTerminalMarkers)) {
        VERIFY_OR_DEBUG_ASSERT(pMarker != nullptr) {
            return nullptr;
        }

        const auto position = audio::FramePos(
                signalInfo.secs2frames(pMarker->positionSecs()) +
                timingOffsetFrames);
        const auto beatsTillNextMarker = static_cast<int>(pMarker->beatsTillNextMarker());
        if (static_cast<quint32>(beatsTillNextMarker) != pMarker->beatsTillNextMarker()) {
            // This will *never* happen with proper data, but better safe than sorry.
            qWarning() << "SeratoBeatsImporter: Import failed because number "
                          "of beats till next marker exceeds int range!";
            return nullptr;
        }
        markers.push_back(BeatMarker{position.toLowerFrameBoundary(), beatsTillNextMarker});
    }

    const auto lastMarkerPosition = audio::FramePos(
            signalInfo.secs2frames(m_pTerminalMarker->positionSecs()) +
            timingOffsetFrames);
    const auto lastMarkerBpm = Bpm(m_pTerminalMarker->bpm());

    m_nonTerminalMarkers.clear();
    m_pTerminalMarker.reset();

    return Beats::fromBeatMarkers(
            signalInfo.getSampleRate(),
            std::move(markers),
            lastMarkerPosition.toLowerFrameBoundary(),
            lastMarkerBpm);
}

} // namespace mixxx
