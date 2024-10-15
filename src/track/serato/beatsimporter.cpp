#include "track/serato/beatsimporter.h"

#include "track/serato/tags.h"

namespace mixxx {
constexpr audio::BeatsPerBar kDefaultBeatPerBar = audio::BeatsPerBar(4);

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
    auto pMarker = !m_nonTerminalMarkers.isEmpty() ? m_nonTerminalMarkers.first() : nullptr;
    for (int i = 1; i <= m_nonTerminalMarkers.size(); i++) {
        auto pNextMarker = i < m_nonTerminalMarkers.size() ? m_nonTerminalMarkers[i] : nullptr;
        VERIFY_OR_DEBUG_ASSERT(pMarker != nullptr) {
            return nullptr;
        }

        const auto position = audio::FramePos(
                signalInfo.secs2frames(pMarker->positionSecs()) +
                timingOffsetFrames)
                                      .toLowerFrameBoundary();
        const auto beatsTillNextMarker = static_cast<double>(pMarker->beatsTillNextMarker());
        if (static_cast<quint32>(beatsTillNextMarker) != pMarker->beatsTillNextMarker()) {
            // This will *never* happen with proper data, but better safe than sorry.
            qWarning() << "SeratoBeatsImporter: Import failed because number "
                          "of beats till next marker exceeds int range!";
            return nullptr;
        }

        const auto nextPosition = audio::FramePos(
                signalInfo.secs2frames(pNextMarker
                                ? pNextMarker->positionSecs()
                                : m_pTerminalMarker->positionSecs()) +
                timingOffsetFrames)
                                          .toLowerFrameBoundary();

        DEBUG_ASSERT(position < nextPosition);

        markers.push_back(BeatMarker{position,
                (nextPosition - position) / beatsTillNextMarker,
                kDefaultBeatPerBar});
        pMarker = pNextMarker;
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
            kDefaultBeatPerBar,
            lastMarkerBpm);
}

} // namespace mixxx
