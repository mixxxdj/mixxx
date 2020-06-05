#include "track/serato/tags.h"

#include <mp3guessenc.h>

#include "track/serato/cueinfoimporter.h"
#include "track/taglib/trackmetadata_file.h"
#include "util/color/predefinedcolorpalettes.h"

namespace {

#ifdef __COREAUDIO__
const QString kDecoderName(QStringLiteral("CoreAudio"));
#elif defined(__MAD__)
const QString kDecoderName(QStringLiteral("MAD"));
#elif defined(__FFMPEG__)
const QString kDecoderName(QStringLiteral("FFMPEG"));
#else
const QString kDecoderName(QStringLiteral("Unknown"));
#endif

/// In Serato, loops and hotcues are kept separate, i. e. you can
/// have a loop and a hotcue with the same number. In Mixxx, loops
/// and hotcues share indices. Hence, we import them with an offset
/// of 8 (the maximum number of hotcues in Serato).
const int kLoopIndexOffset = 8;

mixxx::RgbColor getColorFromOtherPalette(
        const ColorPalette& source,
        const ColorPalette& dest,
        mixxx::RgbColor color) {
    DEBUG_ASSERT(source.size() == dest.size());
    int sourceIndex = source.indexOf(color);
    if (sourceIndex >= 0 && sourceIndex < dest.size()) {
        return dest.at(sourceIndex);
    }
    return color;
}

std::optional<int> findIndexForCueInfo(const mixxx::CueInfo& cueInfo) {
    VERIFY_OR_DEBUG_ASSERT(cueInfo.getHotCueNumber()) {
        qWarning() << "SeratoTags::getCues: Cue without number found!";
        return std::nullopt;
    }

    int index = *cueInfo.getHotCueNumber();
    VERIFY_OR_DEBUG_ASSERT(index >= 0) {
        qWarning() << "SeratoTags::getCues: Cue with number < 0 found!";
        return std::nullopt;
    }

    switch (cueInfo.getType()) {
    case mixxx::CueType::HotCue:
        if (index >= kLoopIndexOffset) {
            qWarning()
                    << "SeratoTags::getCues: Non-loop Cue with number >="
                    << kLoopIndexOffset << "found!";
            return std::nullopt;
        }
        break;
    case mixxx::CueType::Loop:
        index += kLoopIndexOffset;
        break;
    default:
        return std::nullopt;
    }

    return index;
}

} // namespace

namespace mixxx {

/// Serato stores Track colors differently from how they are displayed in
/// the library column. Instead of the color from the library view, the
/// value from the color picker is stored instead (which is different).
/// To make sure that the track looks the same in both Mixxx' and Serato's
/// libraries, we need to convert between the two values.
///
/// See this for details:
/// https://github.com/Holzhaus/serato-tags/blob/master/docs/colors.md#track-colors
RgbColor::optional_t SeratoTags::storedToDisplayedTrackColor(RgbColor color) {
    if (color == 0xFFFFFF) {
        return RgbColor::nullopt();
    }

    if (color == 0x999999) {
        return RgbColor::optional(0x090909);
    }

    if (color == 0x000000) {
        return RgbColor::optional(0x333333);
    }

    RgbColor::code_t colorCode = color;
    colorCode = (colorCode < 0x666666) ? colorCode + 0x99999A : colorCode - 0x666666;
    return RgbColor::optional(colorCode);
}

RgbColor SeratoTags::displayedToStoredTrackColor(RgbColor::optional_t color) {
    if (!color) {
        return RgbColor(0xFFFFFF);
    }

    RgbColor::code_t colorCode = *color;

    if (colorCode == 0x090909) {
        return RgbColor(0x999999);
    }

    if (colorCode == 0x333333) {
        return RgbColor(0x000000);
    }

    // Special case: 0x999999 and 0x99999a are not representable as Serato
    // track color We'll just modify them a little, so that the look the
    // same in Serato.
    if (colorCode == 0x999999) {
        return RgbColor(0x999998);
    }

    if (colorCode == 0x99999a) {
        return RgbColor(0x99999b);
    }

    colorCode = (colorCode < 0x99999A) ? colorCode + 0x666666 : colorCode - 0x99999A;
    return RgbColor(colorCode);
}

RgbColor SeratoTags::storedToDisplayedSeratoDJProCueColor(RgbColor color) {
    return getColorFromOtherPalette(
            PredefinedColorPalettes::kSeratoTrackMetadataHotcueColorPalette,
            PredefinedColorPalettes::kSeratoDJProHotcueColorPalette,
            color);
}

RgbColor SeratoTags::displayedToStoredSeratoDJProCueColor(RgbColor color) {
    return getColorFromOtherPalette(
            PredefinedColorPalettes::kSeratoDJProHotcueColorPalette,
            PredefinedColorPalettes::kSeratoTrackMetadataHotcueColorPalette,
            color);
}

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
#if defined(__COREAUDIO__)
        Q_UNUSED(signalInfo);

        int timingShiftCase = mp3guessenc_timing_shift_case(filePath.toStdString().c_str());

        // TODO: Find missing timing offsets
        switch (timingShiftCase) {
        case EXIT_CODE_CASE_A:
            timingOffset = -12;
            break;
        case EXIT_CODE_CASE_B:
            timingOffset = -40;
            break;
        case EXIT_CODE_CASE_C:
        case EXIT_CODE_CASE_D:
            timingOffset = -60;
            break;
        }
#elif defined(__MAD__) || defined(__FFMPEG__)
        switch (signalInfo.getSampleRate()) {
        case 48000:
            timingOffset = -24;
            break;
        case 44100:
            // This is an estimate and tracks will vary unpredictably within ~1 ms
            timingOffset = -26;
            break;
        default:
            qWarning()
                    << "Unknown timing offset for Serato tags with sample rate"
                    << signalInfo.getSampleRate();
        }
#endif
        qDebug()
                << "Detected timing offset "
                << timingOffset
                << "("
                << kDecoderName
                << ") for MP3 file:"
                << filePath;
    }

    return timingOffset;
}

CueInfoImporterPointer SeratoTags::importCueInfos() const {
    // Import "Serato Markers2" first, then overwrite values with those
    // from "Serato Markers_". This is what Serato does too (i.e. if
    // "Serato Markers_" and "Serato Markers2" contradict each other,
    // Serato will use the values from "Serato Markers_").

    QMap<int, CueInfo> cueMap;
    for (const CueInfo& cueInfo : m_seratoMarkers2.getCues()) {
        std::optional<int> index = findIndexForCueInfo(cueInfo);
        if (!index) {
            continue;
        }

        CueInfo newCueInfo(cueInfo);
        newCueInfo.setHotCueNumber(index);

        RgbColor::optional_t color = cueInfo.getColor();
        if (color) {
            // TODO: Make this conversion configurable
            newCueInfo.setColor(storedToDisplayedSeratoDJProCueColor(*color));
        }
        cueMap.insert(*index, newCueInfo);
    };

    // If the "Serato Markers_" tag does not exist at all, Serato DJ Pro just
    // takes data from the "Serato Markers2" tag, so we can exit early
    // here. If the "Serato Markers_" exists, its data will take precedence.
    if (m_seratoMarkers.isEmpty()) {
        return std::make_shared<SeratoCueInfoImporter>(cueMap.values());
    }

    // The "Serato Markers_" tag always contains entries for the first five
    // cues. If a cue is not set, that entry is present but empty.
    // If a cue is set in "Serato Markers2" but not in "Serato Markers_",
    // Serato DJ Pro considers it as "not set" and ignores it.
    // To mirror the behaviour of Serato, we need to remove from the output of
    // this function.
    QSet<int> unsetCuesInMarkersTag = {0, 1, 2, 3, 4};

    for (const CueInfo& cueInfo : m_seratoMarkers.getCues()) {
        std::optional<int> index = findIndexForCueInfo(cueInfo);
        if (!index) {
            continue;
        }

        // Take a pre-existing CueInfo object that was read from
        // "SeratoMarkers2" from the CueMap (or a default constructed CueInfo
        // object if none exists) and use it as template for the new CueInfo
        // object. Then overwrite all object values that are present in the
        // "SeratoMarkers_"tag.
        CueInfo newCueInfo = cueMap.value(*index);
        newCueInfo.setType(cueInfo.getType());
        newCueInfo.setStartPositionMillis(cueInfo.getStartPositionMillis());
        newCueInfo.setEndPositionMillis(cueInfo.getEndPositionMillis());
        newCueInfo.setHotCueNumber(index);

        RgbColor::optional_t color = cueInfo.getColor();
        if (color) {
            // TODO: Make this conversion configurable
            newCueInfo.setColor(storedToDisplayedSeratoDJProCueColor(*color));
        }
        cueMap.insert(*index, newCueInfo);

        // This cue is set in the "Serato Markers_" tag, so remove it from the
        // set of unset cues
        unsetCuesInMarkersTag.remove(*index);
    };

    // Now that we know which cues should be present in the "Serato Markers_"
    // tag but aren't, remove them from the set.
    for (const int index : unsetCuesInMarkersTag) {
        cueMap.remove(index);
    }

    return std::make_shared<SeratoCueInfoImporter>(cueMap.values());
}

RgbColor::optional_t SeratoTags::getTrackColor() const {
    RgbColor::optional_t color = m_seratoMarkers.getTrackColor();

    if (!color) {
        // Markers_ is empty, but we may have a color in Markers2
        color = m_seratoMarkers2.getTrackColor();
    }

    if (color) {
        color = SeratoTags::storedToDisplayedTrackColor(*color);
    }

    return color;
}

bool SeratoTags::isBpmLocked() const {
    return m_seratoMarkers2.isBpmLocked();
}

} // namespace mixxx
