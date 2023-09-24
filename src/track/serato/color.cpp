#include "track/serato/color.h"

#include "util/color/predefinedcolorpalettes.h"

namespace {

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

/// Serato stores Track colors differently from how they are displayed in
/// the library column. Instead of the color from the library view, the
/// value from the color picker is stored instead (which is different).
/// To make sure that the track looks the same in both Mixxx' and Serato's
/// libraries, we need to convert between the two values.
///
/// See this for details:
/// https://github.com/Holzhaus/serato-tags/blob/main/docs/colors.md#track-colors
RgbColor::optional_t SeratoStoredTrackColor::toDisplayedColor() const {
    RgbColor::code_t colorCode = m_color;
    if (colorCode == SeratoStoredColor::kNoColor) {
        return RgbColor::nullopt();
    }

    if (colorCode == 0x999999) {
        return RgbColor::optional(0x090909);
    }

    if (colorCode == 0x000000) {
        return RgbColor::optional(0x333333);
    }
    colorCode = (colorCode < 0x666666) ? colorCode + 0x99999A : colorCode - 0x666666;
    return RgbColor::optional(colorCode);
}

// static
SeratoStoredTrackColor SeratoStoredTrackColor::fromDisplayedColor(RgbColor::optional_t color) {
    if (!color) {
        return SeratoStoredTrackColor(SeratoStoredColor::kNoColor);
    }

    RgbColor::code_t colorCode = *color;

    if (colorCode == 0x090909) {
        return SeratoStoredTrackColor(0x999999);
    }

    if (colorCode == 0x333333) {
        return SeratoStoredTrackColor(0x000000);
    }

    // Special case: 0x999999 and 0x99999a are not representable as Serato
    // track color We'll just modify them a little, so that the look the
    // same in Serato.
    if (colorCode == 0x999999) {
        return SeratoStoredTrackColor(0x999998);
    }

    if (colorCode == 0x99999a) {
        return SeratoStoredTrackColor(0x99999b);
    }

    colorCode = (colorCode < 0x99999A) ? colorCode + 0x666666 : colorCode - 0x99999A;
    return SeratoStoredTrackColor(colorCode);
}

RgbColor::optional_t SeratoStoredHotcueColor::toDisplayedColor() const {
    if (m_color == SeratoStoredColor::kNoColor) {
        return RgbColor::nullopt();
    }

    return RgbColor::optional(getColorFromOtherPalette(
            PredefinedColorPalettes::kSeratoTrackMetadataHotcueColorPalette,
            PredefinedColorPalettes::kSeratoDJProHotcueColorPalette,
            m_color));
}

// static
SeratoStoredHotcueColor SeratoStoredHotcueColor::fromDisplayedColor(RgbColor::optional_t color) {
    if (!color) {
        return SeratoStoredHotcueColor(SeratoStoredColor::kNoColor);
    }
    return SeratoStoredHotcueColor(getColorFromOtherPalette(
            PredefinedColorPalettes::kSeratoDJProHotcueColorPalette,
            PredefinedColorPalettes::kSeratoTrackMetadataHotcueColorPalette,
            *color));
}

} // namespace mixxx
