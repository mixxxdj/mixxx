#include "colorpalette.h"

constexpr mixxx::RgbColor kColorMixxxRed = mixxx::RgbColor(0xC50A08);
constexpr mixxx::RgbColor kColorMixxxYellow = mixxx::RgbColor(0x32BE44);
constexpr mixxx::RgbColor kColorMixxxGreen = mixxx::RgbColor(0x0044FF);
constexpr mixxx::RgbColor kColorMixxxCeleste = mixxx::RgbColor(0xF8D200);
constexpr mixxx::RgbColor kColorMixxxBlue = mixxx::RgbColor(0x42D4F4);
constexpr mixxx::RgbColor kColorMixxxPurple = mixxx::RgbColor(0xAF00CC);
constexpr mixxx::RgbColor kColorMixxxPink = mixxx::RgbColor(0xFCA6D7);
constexpr mixxx::RgbColor kColorMixxxWhite = mixxx::RgbColor(0xF2F2FF);

const ColorPalette ColorPalette::mixxxPalette =
        ColorPalette(QList<mixxx::RgbColor>{
                kColorMixxxRed,
                kColorMixxxYellow,
                kColorMixxxGreen,
                kColorMixxxCeleste,
                kColorMixxxBlue,
                kColorMixxxPurple,
                kColorMixxxPink,
                kColorMixxxWhite,
        });
