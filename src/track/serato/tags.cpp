#include "track/serato/tags.h"

#include <mp3guessenc.h>

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

} // namespace

namespace mixxx {

RgbColor::optional_t SeratoTags::storedToDisplayedTrackColor(RgbColor color) {
    // Serato stores Track colors differently from how they are displayed in
    // the library column. Instead of the color from the library view, the
    // value from the color picker is stored instead (which is different).
    // To make sure that the track looks the same in both Mixxx' and Serato's
    // libraries, we need to convert between the two values.
    //
    // See this for details:
    // https://github.com/Holzhaus/serato-tags/blob/master/docs/colors.md#track-colors

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

double SeratoTags::findTimingOffsetMillis(const QString& filePath) {
    // The following code accounts for timing offsets required to
    // correctly align timing information (e.g. cue points) exported from
    // Serato. This is caused by different MP3 decoders treating MP3s encoded
    // in a variety of different cases differently. The mp3guessenc library is
    // used to determine which case the MP3 is clasified in. See the following
    // PR for more detailed information:
    // https://github.com/mixxxdj/mixxx/pull/2119

    double timingOffset = 0;
    if (filePath.toLower().endsWith(".mp3")) {
        int timingShiftCase = mp3guessenc_timing_shift_case(filePath.toStdString().c_str());

        // TODO: Find missing timing offsets
        switch (timingShiftCase) {
#if defined(__COREAUDIO__)
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
#elif defined(__MAD__) || defined(__FFMPEG__)
        // Apparently all mp3guessenc cases have the same offset for MAD
        // and FFMPEG
        default:
            timingOffset = -19;
            break;
#endif
        }
        qDebug()
                << "Detected timing offset "
                << timingOffset
                << "("
                << kDecoderName
                << ", case"
                << timingShiftCase
                << ") for MP3 file:"
                << filePath;
    }

    return timingOffset;
}

QList<CueInfo> SeratoTags::getCues(const QString& filePath) const {
    // Import "Serato Markers2" first, then overwrite values with those
    // from "Serato Markers_". This is what Serato does too (i.e. if
    // "Serato Markers_" and "Serato Markers2" contradict each other,
    // Serato will use the values from "Serato Markers_").

    double timingOffsetMillis = SeratoTags::findTimingOffsetMillis(filePath);

    QMap<int, CueInfo> cueMap;
    for (const CueInfo& cueInfo : m_seratoMarkers2.getCues(timingOffsetMillis)) {
        VERIFY_OR_DEBUG_ASSERT(cueInfo.getHotCueNumber()) {
            qWarning() << "SeratoTags::getCues: Cue without number found!";
            continue;
        }

        int index = *cueInfo.getHotCueNumber();
        VERIFY_OR_DEBUG_ASSERT(index >= 0) {
            qWarning() << "SeratoTags::getCues: Cue with number < 0 found!";
        }

        if (cueInfo.getType() != CueType::HotCue) {
            qWarning() << "SeratoTags::getCues: Ignoring cue with non-hotcue type!";
            continue;
        }

        CueInfo newCueInfo(cueInfo);
        RgbColor::optional_t color = cueInfo.getColor();
        if (color) {
            // TODO: Make this conversion configurable
            newCueInfo.setColor(storedToDisplayedSeratoDJProCueColor(*color));
        }
        newCueInfo.setHotCueNumber(index);
        cueMap.insert(index, newCueInfo);
    };

    // TODO(jholthuis): If a hotcue is set in SeratoMarkers2, but not in
    // SeratoMarkers_, we could remove it from the output. We'll just leave it
    // in for now.
    for (const CueInfo& cueInfo : m_seratoMarkers.getCues(timingOffsetMillis)) {
        VERIFY_OR_DEBUG_ASSERT(cueInfo.getHotCueNumber()) {
            qWarning() << "SeratoTags::getCues: Cue without number found!";
            continue;
        }

        int index = *cueInfo.getHotCueNumber();
        VERIFY_OR_DEBUG_ASSERT(index >= 0) {
            qWarning() << "SeratoTags::getCues: Cue with number < 0 found!";
        }

        if (cueInfo.getType() != CueType::HotCue) {
            qWarning() << "SeratoTags::getCues: Ignoring cue with non-hotcue type!";
            continue;
        }

        // Take a pre-existing CueInfo object that was read from
        // "SeratoMarkers2" from the CueMap (or a default constructed CueInfo
        // object if none exists) and use it as template for the new CueInfo
        // object. Then overwrite all object values that are present in the
        // "SeratoMarkers_"tag.
        CueInfo newCueInfo = cueMap.value(index);
        newCueInfo.setType(cueInfo.getType());
        newCueInfo.setStartPositionMillis(cueInfo.getStartPositionMillis());
        newCueInfo.setEndPositionMillis(cueInfo.getEndPositionMillis());
        newCueInfo.setHotCueNumber(index);

        RgbColor::optional_t color = cueInfo.getColor();
        if (color) {
            // TODO: Make this conversion configurable
            newCueInfo.setColor(storedToDisplayedSeratoDJProCueColor(*color));
        }
        cueMap.insert(index, newCueInfo);
    };

    return cueMap.values();
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
