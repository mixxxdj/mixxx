#include "predefinedcolorpalettes.h"

namespace {

// Default Mixxx Hotcue Color Palette
constexpr mixxx::RgbColor kColorMixxxRed(0xC50A08);
constexpr mixxx::RgbColor kColorMixxxYellow(0x32BE44);
constexpr mixxx::RgbColor kColorMixxxGreen(0x42D4F4);
constexpr mixxx::RgbColor kColorMixxxCeleste(0xF8D200);
constexpr mixxx::RgbColor kColorMixxxBlue(0x0044FF);
constexpr mixxx::RgbColor kColorMixxxPurple(0xAF00CC);
constexpr mixxx::RgbColor kColorMixxxPink(0xFCA6D7);
constexpr mixxx::RgbColor kColorMixxxWhite(0xF2F2FF);

// Rekordbox Track Color Palette
constexpr mixxx::RgbColor kRekordboxTrackColorPink(0xF870F8);
constexpr mixxx::RgbColor kRekordboxTrackColorRed(0xF870900);
constexpr mixxx::RgbColor kRekordboxTrackColorOrange(0xF8A030);
constexpr mixxx::RgbColor kRekordboxTrackColorYellow(0xF8E331);
constexpr mixxx::RgbColor kRekordboxTrackColorGreen(0x1EE000);
constexpr mixxx::RgbColor kRekordboxTrackColorAqua(0x16C0F8);
constexpr mixxx::RgbColor kRekordboxTrackColorBlue(0x0150F8);
constexpr mixxx::RgbColor kRekordboxTrackColorPurple(0x9808F8);

// Traktor Track Color Palette
constexpr mixxx::RgbColor kTraktorProTrackColorRed(0xFA4B35);
constexpr mixxx::RgbColor kTraktorProTrackColorOrange(0xFF8402);
constexpr mixxx::RgbColor kTraktorProTrackColorYellow(0xFFF700);
constexpr mixxx::RgbColor kTraktorProTrackColorGreen(0x00F329);
constexpr mixxx::RgbColor kTraktorProTrackColorBlue(0x0187FF);
constexpr mixxx::RgbColor kTraktorProTrackColorViolet(0xA669FF);
constexpr mixxx::RgbColor kTraktorProTrackColorMagenta(0xFE55EA);

// Serato Track Metadata Hotcue Color Palette
// The Serato DJ Pro hotcue colors, shown in the GUI, are stored as these
// colors into the Serato's file metadata.
// Original these colors where shown in the obsolete Serato DJ Intro.
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorRed(0xCC0000);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorOrange(0xCC4400);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorBrown(0xCC8800);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorYellow(0xCCCC00);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorEmerald(0x88CC00);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorKelly(0x44CC00);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorGreen(0x00CC00);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorSea(0x00CC44);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorJade(0x00CC88);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorTurquoise(0x00CCCC);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorTeal(0x0088CC);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorBlue(0x0044CC);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorDarkBlue(0x0000CC);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorViolet(0x4400CC);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorPurple(0x8800CC);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorFuchsia(0xCC00CC);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorMagenta(0xCC0088);
constexpr mixxx::RgbColor kSeratoTrackMetadataHotcueColorCarmine(0xCC0044);

// Serato DJ Pro Hotcue Color Palette
constexpr mixxx::RgbColor kSeratoDJProHotcueColorRed1(0xC02626);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorOrange1(0xDB4E27);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorOrange2(0xF8821A);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorYellow(0xFAC313);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorGreen1(0x4EB648);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorGreen2(0x006838);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorGreen3(0x1FAD26);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorGreen4(0x8DC63F);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorBlue1(0x2B3673);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorCyan(0x1DBEBD);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorBlue2(0x173BA2);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorDarkBlue1(0x173BA2);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorDarkBlue2(0x173BA2);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorViolet1(0x5C3F97);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorViolet2(0x6823B6);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorMagenta(0xCE359E);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorPurple(0xDC1D49);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorRed2(0xC71136);

// Serato DJ Pro Track Color Palette (as shown in library column)
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

// VirtualDJ Track Color Palette
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
                        kSchemaMigrationReplacementColor,
                },
                // Exclude kSchemaMigrationReplacementColor from the colors assigned to hotcues.
                // If there were 9 colors assigned to hotcues, that would look weird on
                // controllers with >8 hotcue buttons, for example a Novation Launchpad.
                QList<int>{0, 1, 2, 3, 4, 5, 6, 7});

const ColorPalette PredefinedColorPalettes::kSeratoTrackMetadataHotcueColorPalette =
        ColorPalette(
                QStringLiteral("Serato DJ Track Metadata Hotcue Colors"),
                QList<mixxx::RgbColor>{
                        kSeratoTrackMetadataHotcueColorRed,
                        kSeratoTrackMetadataHotcueColorOrange,
                        kSeratoTrackMetadataHotcueColorBrown,
                        kSeratoTrackMetadataHotcueColorYellow,
                        kSeratoTrackMetadataHotcueColorEmerald,
                        kSeratoTrackMetadataHotcueColorKelly,
                        kSeratoTrackMetadataHotcueColorGreen,
                        kSeratoTrackMetadataHotcueColorSea,
                        kSeratoTrackMetadataHotcueColorJade,
                        kSeratoTrackMetadataHotcueColorTurquoise,
                        kSeratoTrackMetadataHotcueColorTeal,
                        kSeratoTrackMetadataHotcueColorBlue,
                        kSeratoTrackMetadataHotcueColorDarkBlue,
                        kSeratoTrackMetadataHotcueColorViolet,
                        kSeratoTrackMetadataHotcueColorPurple,
                        kSeratoTrackMetadataHotcueColorFuchsia,
                        kSeratoTrackMetadataHotcueColorMagenta,
                        kSeratoTrackMetadataHotcueColorCarmine,
                },
                QList<int>{0, 2, 12, 3, 6, 15, 9, 14});

const ColorPalette PredefinedColorPalettes::kSeratoDJProHotcueColorPalette =
        ColorPalette(
                QStringLiteral("Serato DJ Pro Hotcue Colors"),
                QList<mixxx::RgbColor>{
                        kSeratoDJProHotcueColorRed1,
                        kSeratoDJProHotcueColorOrange1,
                        kSeratoDJProHotcueColorOrange2,
                        kSeratoDJProHotcueColorYellow,
                        kSeratoDJProHotcueColorGreen1,
                        kSeratoDJProHotcueColorGreen2,
                        kSeratoDJProHotcueColorGreen3,
                        kSeratoDJProHotcueColorGreen4,
                        kSeratoDJProHotcueColorBlue1,
                        kSeratoDJProHotcueColorCyan,
                        kSeratoDJProHotcueColorBlue2,
                        kSeratoDJProHotcueColorDarkBlue1,
                        kSeratoDJProHotcueColorDarkBlue2,
                        kSeratoDJProHotcueColorViolet1,
                        kSeratoDJProHotcueColorViolet2,
                        kSeratoDJProHotcueColorMagenta,
                        kSeratoDJProHotcueColorPurple,
                        kSeratoDJProHotcueColorRed2,
                },
                QList<int>{0, 2, 12, 3, 6, 15, 9, 14});

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
        // Hotcue Color Palettes
        mixxx::PredefinedColorPalettes::kMixxxHotcueColorPalette,
        mixxx::PredefinedColorPalettes::kSeratoDJProHotcueColorPalette,
        // Track Color Palettes
        mixxx::PredefinedColorPalettes::kRekordboxTrackColorPalette,
        mixxx::PredefinedColorPalettes::kSeratoDJProTrackColorPalette,
        mixxx::PredefinedColorPalettes::kTraktorProTrackColorPalette,
        mixxx::PredefinedColorPalettes::kVirtualDJTrackColorPalette,
};

const mixxx::RgbColor PredefinedColorPalettes::kDefaultCueColor =
        kSchemaMigrationReplacementColor;

} // namespace mixxx
