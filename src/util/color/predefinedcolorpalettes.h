#pragma once
#include "util/color/colorpalette.h"

namespace mixxx {

class PredefinedColorPalettes {
  public:
    static const ColorPalette kMixxxHotcueColorPalette;
    static const ColorPalette kSeratoTrackMetadataHotcueColorPalette;
    static const ColorPalette kSeratoDJProHotcueColorPalette;
    static const ColorPalette kRekordboxCOLD1HotcueColorPalette;
    static const ColorPalette kRekordboxCOLD2HotcueColorPalette;
    static const ColorPalette kRekordboxCOLORFULHotcueColorPalette;

    static const ColorPalette kMixxxTrackColorPalette;
    static const ColorPalette kRekordboxTrackColorPalette;
    static const ColorPalette kSeratoDJProTrackColorPalette;
    static const ColorPalette kTraktorProTrackColorPalette;
    static const ColorPalette kVirtualDJTrackColorPalette;

    static const ColorPalette kMixxxKeyColorPalette;
    static const ColorPalette kTraktorKeyColorPalette;
    static const ColorPalette kMIKKeyColorPalette;
    static const ColorPalette kProtKeyColorPalette;
    static const ColorPalette kDeutKeyColorPalette;
    static const ColorPalette kTritKeyColorPalette;

    static const ColorPalette kDefaultHotcueColorPalette;
    static const ColorPalette kDefaultTrackColorPalette;
    static const ColorPalette kDefaultKeyColorPalette;

    static const QList<ColorPalette> kPalettes;
    static const mixxx::RgbColor kDefaultCueColor;
    static const mixxx::RgbColor kDefaultLoopColor;
};

} // namespace mixxx
