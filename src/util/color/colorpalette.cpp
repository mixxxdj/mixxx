#include "colorpalette.h"

namespace {
constexpr mixxx::RgbColor kColorMixxxRed = mixxx::RgbColor(0xC50A08);
constexpr mixxx::RgbColor kColorMixxxYellow = mixxx::RgbColor(0x32BE44);
constexpr mixxx::RgbColor kColorMixxxGreen = mixxx::RgbColor(0x0044FF);
constexpr mixxx::RgbColor kColorMixxxCeleste = mixxx::RgbColor(0xF8D200);
constexpr mixxx::RgbColor kColorMixxxBlue = mixxx::RgbColor(0x42D4F4);
constexpr mixxx::RgbColor kColorMixxxPurple = mixxx::RgbColor(0xAF00CC);
constexpr mixxx::RgbColor kColorMixxxPink = mixxx::RgbColor(0xFCA6D7);
constexpr mixxx::RgbColor kColorMixxxWhite = mixxx::RgbColor(0xF2F2FF);
} // anonymous namespace

const ColorPalette ColorPalette::mixxxHotcuePalette =
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

mixxx::RgbColor ColorPalette::nextColor(mixxx::RgbColor color) const {
    //  Return first color if color not in palette ((-1 + 1) % size)
    return at((indexOf(color) + 1) % size());
}

mixxx::RgbColor ColorPalette::previousColor(mixxx::RgbColor color) const {
    int iIndex = indexOf(color);
    if (iIndex < 0) {
        // Return first color if color not in palette
        iIndex = 0;
    } else {
        iIndex = (iIndex + size() - 1) % size();
    }
    return at(iIndex);
}
