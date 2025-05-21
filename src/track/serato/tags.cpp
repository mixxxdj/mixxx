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
    const QString fileType =
            mixxx::SoundSource::getTypeFromFile(QFileInfo(filePath));
    const mixxx::SoundSourceProviderPointer pPrimaryProvider =
            SoundSourceProxy::getPrimaryProviderForFileType(fileType);
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
constexpr int kFirstSeratoHotCueIndex = 0;
constexpr int kLastSeratoHotCueIndex = 7;
constexpr int kFirstSeratoLoopIndex = 0;
constexpr int kLastSeratoLoopIndex = 7;
constexpr int kLoopImportIndexOffset = 8;

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
        const QString& fileType,
        const audio::SignalInfo& signalInfo) {
    // The following code accounts for timing offsets required to
    // correctly align timing information (e.g. cue points) exported from
    // Serato. This is caused by different MP3 decoders treating MP3s encoded
    // in a variety of different cases differently. The mp3guessenc library is
    // used to determine which case the MP3 is classified in. See the following
    // PR for more detailed information:
    // https://github.com/mixxxdj/mixxx/pull/2119
    double timingOffset = 0;
    if (fileType == "mp3") {
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

std::unique_ptr<CueInfoImporter> SeratoTags::createCueInfoImporter() const {
    auto cueInfos = getCueInfos();
    if (cueInfos.isEmpty()) {
        return nullptr;
    }
    return std::make_unique<SeratoCueInfoImporter>(std::move(cueInfos));
}

QList<CueInfo> SeratoTags::getCueInfos() const {
    // Import "Serato Markers2" first, then overwrite values with those
    // from "Serato Markers_". This is what Serato does too (i.e. if
    // "Serato Markers_" and "Serato Markers2" contradict each other,
    // Serato will use the values from "Serato Markers_").

    QMap<int, CueInfo> cueMap;
    const QList<CueInfo> cuesMarkers2 = m_seratoMarkers2.getCues();
    for (const CueInfo& cueInfo : cuesMarkers2) {
        if (!isCueInfoValid(cueInfo)) {
            continue;
        }
        int hotcueIndex = *cueInfo.getHotCueIndex();
        if (cueInfo.getType() == CueType::Loop) {
            if (hotcueIndex < kFirstSeratoLoopIndex ||
                    hotcueIndex > kLastSeratoLoopIndex) {
                continue;
            }
            hotcueIndex += kLoopImportIndexOffset;
            CueInfo mixxxCue = cueInfo;
            mixxxCue.setHotCueIndex(hotcueIndex);
            cueMap.insert(hotcueIndex, mixxxCue);
        } else {
            if (hotcueIndex < kFirstSeratoHotCueIndex ||
                    hotcueIndex > kLastSeratoHotCueIndex) {
                continue;
            }
            cueMap.insert(*cueInfo.getHotCueIndex(), cueInfo);
        }
    };

    const QList<CueInfo> cuesMarkers = m_seratoMarkers.getCues();
    if (cuesMarkers.size() > 0) {
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
            int hotcueIndex = *cueInfo.getHotCueIndex();
            if (cueInfo.getType() == CueType::Loop) {
                if (hotcueIndex < kFirstSeratoLoopIndex ||
                        hotcueIndex > kLastSeratoLoopIndex) {
                    continue;
                }
                hotcueIndex += kLoopImportIndexOffset;
            } else {
                if (hotcueIndex < kFirstSeratoHotCueIndex ||
                        hotcueIndex > kLastSeratoHotCueIndex) {
                    continue;
                }
            }

            CueInfo& newCueInfo = cueMap[hotcueIndex];
            newCueInfo.setType(cueInfo.getType());
            newCueInfo.setStartPositionMillis(cueInfo.getStartPositionMillis());
            newCueInfo.setEndPositionMillis(cueInfo.getEndPositionMillis());
            newCueInfo.setHotCueIndex(hotcueIndex);
            newCueInfo.setFlags(cueInfo.flags());
            newCueInfo.setColor(cueInfo.getColor());

            // This cue is set in the "Serato Markers_" tag, so remove it from the
            // set of unset cues
            unsetCuesInMarkersTag.remove(hotcueIndex);
        };

        // Now that we know which cues should be present in the "Serato Markers_"
        // tag but aren't, remove them from the set.
        for (const int index : unsetCuesInMarkersTag) {
            cueMap.remove(index);
        }
    }

    const QList<CueInfo> cueInfos = cueMap.values();
    // qDebug() << "SeratoTags::getCueInfos()";
    for (const CueInfo& cueInfo : cueInfos) {
        qDebug() << cueInfo;
    }

    return cueInfos;
}

void SeratoTags::setCueInfos(const QList<CueInfo>& cueInfos, double timingOffsetMillis) {
    // Filter out all cues that cannot be mapped to Serato's tag data,
    // ensure that each hotcue number is unique (by using a map), apply the
    // timing offset and split up cues and loops.
    QList<CueInfo> cueList;
    for (const CueInfo& cueInfo : std::as_const(cueInfos)) {
        if (!cueInfo.getHotCueIndex()) {
            continue;
        }

        CueInfo cueInfoSeratoAdjusted = cueInfo;

        if (!cueInfo.getStartPositionMillis().has_value()) {
            continue;
        }
        cueInfoSeratoAdjusted.setStartPositionMillis(
                *cueInfo.getStartPositionMillis() - timingOffsetMillis);

        if (cueInfo.getEndPositionMillis().has_value()) {
            cueInfoSeratoAdjusted.setEndPositionMillis(
                    *cueInfo.getEndPositionMillis() - timingOffsetMillis);
        }

        int hotcueIndex = *cueInfo.getHotCueIndex();
        switch (cueInfo.getType()) {
        case CueType::HotCue:
            if (hotcueIndex < kFirstSeratoHotCueIndex ||
                    hotcueIndex > kLastSeratoHotCueIndex) {
                continue;
            }
            cueList.append(cueInfoSeratoAdjusted);
            break;
        case CueType::Loop:
            if (!cueInfoSeratoAdjusted.getEndPositionMillis().has_value()) {
                qWarning() << "Loop Cue" << hotcueIndex << "has no end position";
                DEBUG_ASSERT(false);
                continue;
            }
            hotcueIndex -= kLoopImportIndexOffset;
            if (hotcueIndex < kFirstSeratoLoopIndex ||
                    hotcueIndex > kLastSeratoLoopIndex) {
                continue;
            }
            cueInfoSeratoAdjusted.setHotCueIndex(hotcueIndex);
            cueList.append(cueInfoSeratoAdjusted);
            break;
        default:
            qWarning() << "Skipping incompatible cue type";
            continue;
        }
    };

    m_seratoMarkers.setCues(cueList);
    m_seratoMarkers2.setCues(cueList);
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

void SeratoTags::setTrackColor(const RgbColor::optional_t& color) {
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
