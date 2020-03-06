#include "colorpalette.h"

namespace {

constexpr mixxx::RgbColor kColorMixxxRed(0xC50A08);
constexpr mixxx::RgbColor kColorMixxxYellow(0x32BE44);
constexpr mixxx::RgbColor kColorMixxxGreen(0x0044FF);
constexpr mixxx::RgbColor kColorMixxxCeleste(0xF8D200);
constexpr mixxx::RgbColor kColorMixxxBlue(0x42D4F4);
constexpr mixxx::RgbColor kColorMixxxPurple(0xAF00CC);
constexpr mixxx::RgbColor kColorMixxxPink(0xFCA6D7);
constexpr mixxx::RgbColor kColorMixxxWhite(0xF2F2FF);

// Replaces "no color" values and is used for new cues if auto_hotcue_colors is
// disabled
constexpr mixxx::RgbColor kSchemaMigrationReplacementColor(0xFF8000);

} // anonymous namespace

const ColorPalette ColorPalette::mixxxHotcuePalette =
        ColorPalette(
                QStringLiteral("Mixxx Hotcue Colors"),
                QList<mixxx::RgbColor>{
                        kColorMixxxRed,
                        kColorMixxxYellow,
                        kColorMixxxGreen,
                        kColorMixxxCeleste,
                        kColorMixxxBlue,
                        kColorMixxxPurple,
                        kColorMixxxPink,
                        kColorMixxxWhite,
                });

const mixxx::RgbColor ColorPalette::kDefaultCueColor = kSchemaMigrationReplacementColor;

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

mixxx::RgbColor ColorPalette::colorForHotcueIndex(unsigned int index) const {
    // For hotcue n, get nth color from palette
    return at(index % size());
}
