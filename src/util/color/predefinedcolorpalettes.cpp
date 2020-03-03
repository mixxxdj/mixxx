#include "predefinedcolorpalettes.h"

namespace {

constexpr mixxx::RgbColor kColorMixxxRed(0xC50A08);
constexpr mixxx::RgbColor kColorMixxxYellow(0x32BE44);
constexpr mixxx::RgbColor kColorMixxxGreen(0x42D4F4);
constexpr mixxx::RgbColor kColorMixxxCeleste(0xF8D200);
constexpr mixxx::RgbColor kColorMixxxBlue(0x0044FF);
constexpr mixxx::RgbColor kColorMixxxPurple(0xAF00CC);
constexpr mixxx::RgbColor kColorMixxxPink(0xFCA6D7);
constexpr mixxx::RgbColor kColorMixxxWhite(0xF2F2FF);

constexpr mixxx::RgbColor kRekordboxTrackColorPink(0xF870F8);
constexpr mixxx::RgbColor kRekordboxTrackColorRed(0xF870900);
constexpr mixxx::RgbColor kRekordboxTrackColorOrange(0xF8A030);
constexpr mixxx::RgbColor kRekordboxTrackColorYellow(0xF8E331);
constexpr mixxx::RgbColor kRekordboxTrackColorGreen(0x1EE000);
constexpr mixxx::RgbColor kRekordboxTrackColorAqua(0x16C0F8);
constexpr mixxx::RgbColor kRekordboxTrackColorBlue(0x0150F8);
constexpr mixxx::RgbColor kRekordboxTrackColorPurple(0x9808F8);

constexpr mixxx::RgbColor kTraktorProTrackColorRed(0xFA4B35);
constexpr mixxx::RgbColor kTraktorProTrackColorOrange(0xFF8402);
constexpr mixxx::RgbColor kTraktorProTrackColorYellow(0xFFF700);
constexpr mixxx::RgbColor kTraktorProTrackColorGreen(0x00F329);
constexpr mixxx::RgbColor kTraktorProTrackColorBlue(0x0187FF);
constexpr mixxx::RgbColor kTraktorProTrackColorViolet(0xA669FF);
constexpr mixxx::RgbColor kTraktorProTrackColorMagenta(0xFE55EA);

constexpr mixxx::RgbColor kSeratoDJProTrackColorGrey1(0x333333);
constexpr mixxx::RgbColor kSeratoDJProTrackColorGrey2(0x555555);
constexpr mixxx::RgbColor kSeratoDJProTrackColorPink1(0x993399);
constexpr mixxx::RgbColor kSeratoDJProTrackColorPink2(0x993377);
constexpr mixxx::RgbColor kSeratoDJProTrackColorRed1(0x993355);
constexpr mixxx::RgbColor kSeratoDJProTrackColorRed2(0x993333);
constexpr mixxx::RgbColor kSeratoDJProTrackColorOrange(0x995533);
constexpr mixxx::RgbColor kSeratoDJProTrackColorBrown1(0x997733);
constexpr mixxx::RgbColor kSeratoDJProTrackColorBrown2(0x999933);
constexpr mixxx::RgbColor kSeratoDJProTrackColorBrown3(0x779933);
constexpr mixxx::RgbColor kSeratoDJProTrackColorGreen1(0x559933);
constexpr mixxx::RgbColor kSeratoDJProTrackColorGreen2(0x339933);
constexpr mixxx::RgbColor kSeratoDJProTrackColorGreen3(0x339955);
constexpr mixxx::RgbColor kSeratoDJProTrackColorTurquoise1(0x339977);
constexpr mixxx::RgbColor kSeratoDJProTrackColorTurquoise2(0x339999);
constexpr mixxx::RgbColor kSeratoDJProTrackColorTurquoise3(0x337799);
constexpr mixxx::RgbColor kSeratoDJProTrackColorBlue(0x335599);
constexpr mixxx::RgbColor kSeratoDJProTrackColorPurple1(0x333399);
constexpr mixxx::RgbColor kSeratoDJProTrackColorPurple2(0x553399);
constexpr mixxx::RgbColor kSeratoDJProTrackColorPurple3(0x773399);

constexpr mixxx::RgbColor kVirtualDJTrackColorRed(0xFF0000);
constexpr mixxx::RgbColor kVirtualDJTrackColorYellow(0xFFFF00);
constexpr mixxx::RgbColor kVirtualDJTrackColorGreen(0x00FF00);
constexpr mixxx::RgbColor kVirtualDJTrackColorCyan(0x00FFFF);
constexpr mixxx::RgbColor kVirtualDJTrackColorBlue(0x0000FF);
constexpr mixxx::RgbColor kVirtualDJTrackColorFuchsia(0xFF00FF);
constexpr mixxx::RgbColor kVirtualDJTrackColorWhite(0xFFFFFF);

// Replaces "no color" values and is used for new cues if auto_hotcue_colors is
// disabled
constexpr mixxx::RgbColor kSchemaMigrationReplacementColor(0xFF8000);

} // anonymous namespace

namespace mixxx {

const ColorPalette PredefinedColorPalettes::kMixxxHotcueColorPalette =
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

const ColorPalette PredefinedColorPalettes::kRekordboxTrackColorPalette =
        ColorPalette(
                QStringLiteral("Rekordbox Track Colors"),
                QList<mixxx::RgbColor>{
                        kRekordboxTrackColorPink,
                        kRekordboxTrackColorRed,
                        kRekordboxTrackColorOrange,
                        kRekordboxTrackColorYellow,
                        kRekordboxTrackColorGreen,
                        kRekordboxTrackColorAqua,
                        kRekordboxTrackColorBlue,
                        kRekordboxTrackColorPurple,
                });

const ColorPalette PredefinedColorPalettes::kSeratoDJProTrackColorPalette =
        ColorPalette(
                QStringLiteral("Serato DJ Pro Track Colors"),
                QList<mixxx::RgbColor>{
                        kSeratoDJProTrackColorGrey1,
                        kSeratoDJProTrackColorGrey2,
                        kSeratoDJProTrackColorPink1,
                        kSeratoDJProTrackColorPink2,
                        kSeratoDJProTrackColorRed1,
                        kSeratoDJProTrackColorRed2,
                        kSeratoDJProTrackColorOrange,
                        kSeratoDJProTrackColorBrown1,
                        kSeratoDJProTrackColorBrown2,
                        kSeratoDJProTrackColorBrown3,
                        kSeratoDJProTrackColorGreen1,
                        kSeratoDJProTrackColorGreen2,
                        kSeratoDJProTrackColorGreen3,
                        kSeratoDJProTrackColorTurquoise1,
                        kSeratoDJProTrackColorTurquoise2,
                        kSeratoDJProTrackColorTurquoise3,
                        kSeratoDJProTrackColorBlue,
                        kSeratoDJProTrackColorPurple1,
                        kSeratoDJProTrackColorPurple2,
                        kSeratoDJProTrackColorPurple3,
                });

const ColorPalette PredefinedColorPalettes::kTraktorProTrackColorPalette =
        ColorPalette(
                QStringLiteral("Traktor Pro Track Colors"),
                QList<mixxx::RgbColor>{
                        kTraktorProTrackColorRed,
                        kTraktorProTrackColorOrange,
                        kTraktorProTrackColorYellow,
                        kTraktorProTrackColorGreen,
                        kTraktorProTrackColorBlue,
                        kTraktorProTrackColorViolet,
                        kTraktorProTrackColorMagenta,
                });

const ColorPalette PredefinedColorPalettes::kVirtualDJTrackColorPalette =
        ColorPalette(
                QStringLiteral("VirtualDJ Track Colors"),
                QList<mixxx::RgbColor>{
                        kVirtualDJTrackColorRed,
                        kVirtualDJTrackColorYellow,
                        kVirtualDJTrackColorGreen,
                        kVirtualDJTrackColorCyan,
                        kVirtualDJTrackColorBlue,
                        kVirtualDJTrackColorFuchsia,
                        kVirtualDJTrackColorWhite,
                });

const ColorPalette PredefinedColorPalettes::kDefaultHotcueColorPalette =
        mixxx::PredefinedColorPalettes::kMixxxHotcueColorPalette;

const ColorPalette PredefinedColorPalettes::kDefaultTrackColorPalette =
        mixxx::PredefinedColorPalettes::kMixxxHotcueColorPalette;

const QList<ColorPalette> PredefinedColorPalettes::kPalettes{
        mixxx::PredefinedColorPalettes::kMixxxHotcueColorPalette,
        mixxx::PredefinedColorPalettes::kRekordboxTrackColorPalette,
        mixxx::PredefinedColorPalettes::kSeratoDJProTrackColorPalette,
        mixxx::PredefinedColorPalettes::kTraktorProTrackColorPalette,
        mixxx::PredefinedColorPalettes::kVirtualDJTrackColorPalette,
};

const mixxx::RgbColor PredefinedColorPalettes::kDefaultCueColor =
        kSchemaMigrationReplacementColor;

} // namespace mixxx
