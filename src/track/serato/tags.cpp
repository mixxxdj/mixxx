#include "track/serato/tags.h"

#include <mp3guessenc.h>

#include <optional>

#include "sources/soundsourceproxy.h"
#if defined(__COREAUDIO__)
#include "sources/soundsourcecoreaudio.h"
#endif
#if defined(__FFMPEG__)
#include "sources/soundsourceffmpeg.h"
#endif
#if defined(__MAD__)
#include "sources/soundsourcemp3.h"
#endif
#include "track/serato/beatsimporter.h"
#include "track/serato/cueinfoimporter.h"
#include "track/taglib/trackmetadata_file.h"
#include "util/color/predefinedcolorpalettes.h"

namespace {

QString getPrimaryDecoderNameForFilePath(const QString& filePath) {
    const QString fileExtension =
            mixxx::SoundSource::getTypeFromFile(filePath);
    const mixxx::SoundSourceProviderPointer pPrimaryProvider =
            SoundSourceProxy::getPrimaryProviderForFileExtension(fileExtension);
    if (pPrimaryProvider) {
        return pPrimaryProvider->getDisplayName();
    } else {
        return QString(); // unknown
    }
}

/// In Serato, loops and hotcues are kept separate, i. e. you can
/// have a loop and a hotcue with the same number. In Mixxx, loops
/// and hotcues share indices. Hence, we import them with an offset
/// of 8 (the maximum number of hotcues in Serato).
constexpr int kFirstLoopIndex = mixxx::kFirstHotCueIndex + 8;
constexpr int kNumCuesInMarkersTag = 5;

bool isCueInfoValid(const mixxx::CueInfo& cueInfo) {
    if (cueInfo.getType() == mixxx::CueType::Loop &&
            cueInfo.getEndPositionMillis().value_or(0) == 0) {
        // These entries are likely added via issue #11283
        qWarning() << "Discard loop cue" << cueInfo.getHotCueIndex()
                   << "with length of 0";
        return false;
    }
    if (cueInfo.getType() == mixxx::CueType::HotCue &&
            cueInfo.getStartPositionMillis().value_or(0) == 0 &&
            cueInfo.getColor().value_or(mixxx::RgbColor(0)) == mixxx::RgbColor(0)) {
        // These entries are likely added via issue #11283
        qWarning() << "Discard black hot cue" << cueInfo.getHotCueIndex()
                   << "at position 0";
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(cueInfo.getHotCueIndex()) {
        return false;
    }
    return true;
}

} // namespace

namespace mixxx {

double SeratoTags::guessTimingOffsetMillis(
        const QString& filePath,
        const audio::SignalInfo& signalInfo) {
    // The following code accounts for timing offsets required to
    // correctly align timing information (e.g. cue points) exported from
    // Serato. This is caused by different MP3 decoders treating MP3s encoded
    // in a variety of different cases differently. The mp3guessenc library is
    // used to determine which case the MP3 is classified in. See the following
    // PR for more detailed information:
    // https://github.com/mixxxdj/mixxx/pull/2119
    double timingOffset = 0;
    if (taglib::getFileTypeFromFileName(filePath) == taglib::FileType::MP3) {
        const QString primaryDecoderName =
                getPrimaryDecoderNameForFilePath(filePath);
        // There should always be an MP3 decoder available
        VERIFY_OR_DEBUG_ASSERT(!primaryDecoderName.isEmpty()) {
            return 0;
        }
        bool timingOffsetGuessed = false;
#if defined(__COREAUDIO__)
        if (primaryDecoderName == mixxx::SoundSourceProviderCoreAudio::kDisplayName) {
            DEBUG_ASSERT(!timingOffsetGuessed);
            DEBUG_ASSERT(timingOffset == 0);

            Q_UNUSED(signalInfo)
            int timingShiftCase = mp3guessenc_timing_shift_case(filePath.toStdString().c_str());

            // TODO: Find missing timing offsets
            switch (timingShiftCase) {
            case EXIT_CODE_CASE_A:
                timingOffset = -12;
                timingOffsetGuessed = true;
                break;
            case EXIT_CODE_CASE_B:
                timingOffset = -40;
                timingOffsetGuessed = true;
                break;
            case EXIT_CODE_CASE_C:
            case EXIT_CODE_CASE_D:
                timingOffset = -60;
                timingOffsetGuessed = true;
                break;
            }
        }
#endif
#if defined(__MAD__) || defined(__FFMPEG__)
        bool usingMadOrFFmpeg = false;
#if defined(__MAD__)
        if (primaryDecoderName == mixxx::SoundSourceProviderMp3::kDisplayName) {
            DEBUG_ASSERT(!usingMadOrFFmpeg);
            usingMadOrFFmpeg = true;
        }
#endif
#if defined(__FFMPEG__)
        if (primaryDecoderName == mixxx::SoundSourceProviderFFmpeg::kDisplayName) {
            DEBUG_ASSERT(!usingMadOrFFmpeg);
            usingMadOrFFmpeg = true;
        }
#endif
        if (usingMadOrFFmpeg) {
            DEBUG_ASSERT(!timingOffsetGuessed);
            DEBUG_ASSERT(timingOffset == 0);

            switch (signalInfo.getSampleRate()) {
            case 48000:
                timingOffset = -24;
                timingOffsetGuessed = true;
                break;
            case 44100:
                // This is an estimate and tracks will vary unpredictably within ~1 ms
                timingOffset = -26;
                timingOffsetGuessed = true;
                break;
            default:
                qWarning()
                        << "Unknown timing offset for Serato tags with sample rate"
                        << signalInfo.getSampleRate();
            }
        }
#endif
        if (timingOffsetGuessed) {
            qDebug()
                    << "Detected timing offset"
                    << timingOffset
                    << "using"
                    << primaryDecoderName
                    << "for MP3 file"
                    << filePath;
        } else {
            qDebug()
                    << "Unknown timing offset"
                    << timingOffset
                    << "using"
                    << primaryDecoderName
                    << "for MP3 file"
                    << filePath;
        }
    }

    return timingOffset;
}

BeatsImporterPointer SeratoTags::importBeats() const {
    if (m_seratoBeatGrid.isEmpty() || !m_seratoBeatGrid.terminalMarker()) {
        return nullptr;
    }
    return std::make_shared<SeratoBeatsImporter>(
            m_seratoBeatGrid.nonTerminalMarkers(),
            m_seratoBeatGrid.terminalMarker());
}

CueInfoImporterPointer SeratoTags::importCueInfos() const {
    auto cueInfos = getCueInfos();
    if (cueInfos.isEmpty()) {
        return nullptr;
    }
    return std::make_shared<SeratoCueInfoImporter>(std::move(cueInfos));
}

QList<CueInfo> SeratoTags::getCueInfos() const {
    // Import "Serato Markers2" first, then overwrite values with those
    // from "Serato Markers_". This is what Serato does too (i.e. if
    // "Serato Markers_" and "Serato Markers2" contradict each other,
    // Serato will use the values from "Serato Markers_").

    QMap<int, CueInfo> hotCueMap;
    QMap<int, CueInfo> loopCueMap;
    const QList<CueInfo> cuesMarkers2 = m_seratoMarkers2.getCues();
    for (const CueInfo& cueInfo : cuesMarkers2) {
        if (!isCueInfoValid(cueInfo)) {
            continue;
        }
        if (cueInfo.getType() == CueType::Loop) {
            loopCueMap.insert(*cueInfo.getHotCueIndex(), cueInfo);
        } else {
            hotCueMap.insert(*cueInfo.getHotCueIndex(), cueInfo);
        }
    };

    const QList<CueInfo> cuesMarkers = m_seratoMarkers.getCues();
    if (cuesMarkers.size()) {
        // The "Serato Markers_" tag always contains entries for the first five
        // cues. If a cue is not set, that entry is present but empty.
        // If a cue is set in "Serato Markers2" but not in "Serato Markers_",
        // Serato DJ Pro considers it as "not set" and ignores it.
        // To mirror the behaviour of Serato, we need to remove from the output of
        // this function.
        QSet<int> unsetCuesInMarkersTag;
        unsetCuesInMarkersTag.reserve(kNumCuesInMarkersTag);
        for (int i = 0; i < kNumCuesInMarkersTag; i++) {
            unsetCuesInMarkersTag.insert(i);
        }

        for (const CueInfo& cueInfo : cuesMarkers) {
            if (!isCueInfoValid(cueInfo)) {
                continue;
            }
            // Take a pre-existing CueInfo object that was read from
            // "SeratoMarkers2" from the CueMap (or a default constructed CueInfo
            // object if none exists) and use it as template for the new CueInfo
            // object. Then overwrite all object values that are present in the
            // "SeratoMarkers_"tag.
            CueInfo& newCueInfo = cueInfo.getType() == CueType::Loop
                    ? loopCueMap[*cueInfo.getHotCueIndex()]
                    : hotCueMap[*cueInfo.getHotCueIndex()];
            newCueInfo.setType(cueInfo.getType());
            newCueInfo.setStartPositionMillis(cueInfo.getStartPositionMillis());
            newCueInfo.setEndPositionMillis(cueInfo.getEndPositionMillis());
            newCueInfo.setHotCueIndex(*cueInfo.getHotCueIndex());
            newCueInfo.setFlags(cueInfo.flags());
            newCueInfo.setColor(cueInfo.getColor());

            // This cue is set in the "Serato Markers_" tag, so remove it from the
            // set of unset cues
            unsetCuesInMarkersTag.remove(*cueInfo.getHotCueIndex());
        };

        // Now that we know which cues should be present in the "Serato Markers_"
        // tag but aren't, remove them from the set.
        for (const int index : unsetCuesInMarkersTag) {
            hotCueMap.remove(index);
        }
    }

    // Serato has a separate indexing for loop and hot cues.
    // We sort the Loops Cues into the HotCue map if the given index is free,
    // add an offset of 8 otherwise or loop until we find a free index.
    // Cues above index 8 are not accessible in Mixxx, only visible in waveforms
    // During export the Mixxx indexes are kept to allow a perfect round-trip.
    for (const CueInfo& loopCueInfo : qAsConst(loopCueMap)) {
        if (hotCueMap.contains(*loopCueInfo.getHotCueIndex())) {
            CueInfo cueInfo = loopCueInfo;
            cueInfo.setHotCueIndex(*loopCueInfo.getHotCueIndex() + kFirstLoopIndex);
            while (hotCueMap.contains(*cueInfo.getHotCueIndex())) {
                cueInfo.setHotCueIndex(*cueInfo.getHotCueIndex() + 1);
            }
            hotCueMap.insert(*cueInfo.getHotCueIndex(), cueInfo);
        } else {
            hotCueMap.insert(*loopCueInfo.getHotCueIndex(), loopCueInfo);
        }
    }

    const QList<CueInfo> cueInfos = hotCueMap.values();
    qDebug() << "SeratoTags::getCueInfos()";
    for (const CueInfo& cueInfo : cueInfos) {
        qDebug() << cueInfo;
    }

    return cueInfos;
}

void SeratoTags::setCueInfos(const QList<CueInfo>& cueInfos, double timingOffsetMillis) {
    // Filter out all cues that cannot be mapped to Serato's tag data,
    // ensure that each hotcue number is unique (by using a map), apply the
    // timing offset and split up cues and loops.
    QMap<int, CueInfo> cueMap;
    QMap<int, CueInfo> loopMap;
    for (const CueInfo& cueInfo : qAsConst(cueInfos)) {
        if (!cueInfo.getHotCueIndex()) {
            continue;
        }

        int hotcueIndex = *cueInfo.getHotCueIndex();
        if (hotcueIndex < kFirstHotCueIndex) {
            continue;
        }

        CueInfo newCueInfo(cueInfo);
        if (!cueInfo.getStartPositionMillis().has_value()) {
            continue;
        }
        newCueInfo.setStartPositionMillis(
                *cueInfo.getStartPositionMillis() - timingOffsetMillis);

        if (cueInfo.getEndPositionMillis().has_value()) {
            newCueInfo.setEndPositionMillis(*cueInfo.getEndPositionMillis() - timingOffsetMillis);
        }
        newCueInfo.setFlags(cueInfo.flags());

        switch (cueInfo.getType()) {
        case CueType::HotCue:
            cueMap.insert(hotcueIndex, newCueInfo);
            break;
        case CueType::Loop:
            if (!newCueInfo.getEndPositionMillis().has_value()) {
                qWarning() << "Loop Cue" << hotcueIndex << "has no end position";
                DEBUG_ASSERT(false);
                continue;
            }
            loopMap.insert(hotcueIndex, newCueInfo);
            break;
        default:
            qWarning() << "Skipping incompatible cue type";
            continue;
        }
    };

    // Check if loops were imported or set using a constant offset
    int loopIndexOffset = 0;
    if (!loopMap.isEmpty()) {
        if (loopMap.firstKey() >= kFirstLoopIndex) {
            loopIndexOffset = kFirstLoopIndex;
        }
    }

    // Apply loop index offset and create list
    QList<CueInfo> cueInfoList = cueMap.values();
    auto it = loopMap.constBegin();
    while (it != loopMap.constEnd()) {
        CueInfo cueInfo(it.value());
        cueInfo.setHotCueIndex(*cueInfo.getHotCueIndex() - loopIndexOffset);
        cueInfoList.append(cueInfo);
        it++;
    }

    m_seratoMarkers.setCues(cueInfoList);
    m_seratoMarkers2.setCues(cueInfoList);
}

std::optional<RgbColor::optional_t> SeratoTags::getTrackColor() const {
    std::optional<mixxx::SeratoStoredTrackColor> pStoredColor = m_seratoMarkers.getTrackColor();

    if (!pStoredColor) {
        // Markers_ is empty, but we may have a color in Markers2
        pStoredColor = m_seratoMarkers2.getTrackColor();
    }

    if (!pStoredColor) {
        return std::nullopt;
    }

    return std::optional<RgbColor::optional_t>(
            pStoredColor->toDisplayedColor());
}

void SeratoTags::setTrackColor(RgbColor::optional_t color) {
    auto storedColor = SeratoStoredTrackColor::fromDisplayedColor(color);
    m_seratoMarkers.setTrackColor(storedColor);
    m_seratoMarkers2.setTrackColor(storedColor);
}

bool SeratoTags::isBpmLocked() const {
    return m_seratoMarkers2.isBpmLocked();
}

void SeratoTags::setBpmLocked(bool bpmLocked) {
    m_seratoMarkers2.setBpmLocked(bpmLocked);
}

} // namespace mixxx
