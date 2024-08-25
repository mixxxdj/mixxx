#pragma once
#include "util/color/colorpalette.h"

namespace {

constexpr mixxx::RgbColor kColorMixxxWhite(0xF2F2FF);

// Replaces "no color" values and is used for new cues if auto_hotcue_colors is
// disabled
constexpr mixxx::RgbColor kSchemaMigrationReplacementColor(0xFF8000);
} // namespace
namespace mixxx {
namespace predefinedcolorpalettes {

constexpr static mixxx::RgbColor kDefaultCueColor = kSchemaMigrationReplacementColor;
constexpr static mixxx::RgbColor kDefaultLoopColor = kColorMixxxWhite;

struct PredefinedColorPalettes {
    ColorPalette mixxxHotcueColorPalette;
    ColorPalette seratoTrackMetadataHotcueColorPalette;
    ColorPalette seratoDJProHotcueColorPalette;
    ColorPalette rekordboxCOLD1HotcueColorPalette;
    ColorPalette rekordboxCOLD2HotcueColorPalette;
    ColorPalette rekordboxCOLORFULHotcueColorPalette;

    ColorPalette mixxxTrackColorPalette;
    ColorPalette rekordboxTrackColorPalette;
    ColorPalette seratoDJProTrackColorPalette;
    ColorPalette traktorProTrackColorPalette;
    ColorPalette virtualDJTrackColorPalette;

    ColorPalette mixxxKeyColorPalette;
    ColorPalette traktorKeyColorPalette;
    ColorPalette MIKKeyColorPalette;
    ColorPalette protKeyColorPalette;
    ColorPalette deutKeyColorPalette;
    ColorPalette tritKeyColorPalette;

    ColorPalette defaultHotcueColorPalette;
    ColorPalette defaultTrackColorPalette;
    ColorPalette defaultKeyColorPalette;

    QList<ColorPalette> palettes;
};

// since the palettes used here are supposed to be used in other translation
// units we can't make these global statics without getting involved in the
// "global initialization order fiasco". So instead we employ this getter which
// internally implements "initialization on first use". when accessing more than
// one member of the returned struct, its recommended to store the reference
// instead of repeatedly calling `get()`.
const PredefinedColorPalettes& get();

} // namespace predefinedcolorpalettes
} // namespace mixxx
