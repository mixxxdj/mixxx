#pragma once
#include "util/color/colorpalette.h"

namespace mixxx {

class PredefinedColorPalettes {
  public:
    static const ColorPalette kMixxxHotcueColorPalette;
    static const ColorPalette kRekordboxTrackColorPalette;
    static const ColorPalette kSeratoDJProTrackColorPalette;
    static const ColorPalette kTraktorProTrackColorPalette;
    static const ColorPalette kVirtualDJTrackColorPalette;

    static const ColorPalette kDefaultHotcueColorPalette;
    static const ColorPalette kDefaultTrackColorPalette;

    static const QList<ColorPalette> kPalettes;
    static const mixxx::RgbColor kDefaultCueColor;
};

} // namespace mixxx
