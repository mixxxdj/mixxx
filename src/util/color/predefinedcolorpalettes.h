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

    static const ColorPalette kDefaultHotcueColorPalette;
    static const ColorPalette kDefaultTrackColorPalette;

    static const QList<ColorPalette> kPalettes;
    static const mixxx::RgbColor kDefaultCueColor;
    static const mixxx::RgbColor kDefaultLoopColor;
};

} // namespace mixxx
