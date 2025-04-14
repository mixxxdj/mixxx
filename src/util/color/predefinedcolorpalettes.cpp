#include "predefinedcolorpalettes.h"

namespace {

// Default Mixxx Hotcue Color Palette
constexpr mixxx::RgbColor kColorMixxxRed(0xC50A08);
constexpr mixxx::RgbColor kColorMixxxGreen(0x32BE44);
constexpr mixxx::RgbColor kColorMixxxCeleste(0x42D4F4);
constexpr mixxx::RgbColor kColorMixxxYellow(0xF8D200);
constexpr mixxx::RgbColor kColorMixxxBlue(0x0044FF);
constexpr mixxx::RgbColor kColorMixxxPurple(0xAF00CC);
constexpr mixxx::RgbColor kColorMixxxPink(0xFCA6D7);
// white already declared in header

// Default Mixxx Track Color Palette
constexpr mixxx::RgbColor kMixxxTrackColorDarkRed(0x880000);
constexpr mixxx::RgbColor kMixxxTrackColorRed(0xFF0000);
constexpr mixxx::RgbColor kMixxxTrackColorDarkOrange(0xFF8800);
constexpr mixxx::RgbColor kMixxxTrackColorLemonGlacier(0xFFFF00);
constexpr mixxx::RgbColor kMixxxTrackColorChartreuse(0x88FF00);
constexpr mixxx::RgbColor kMixxxTrackColorElectricGreen(0x00FF00);
constexpr mixxx::RgbColor kMixxxTrackColorIndiaGreen(0x008800);
constexpr mixxx::RgbColor kMixxxTrackColorDarkCyan(0x008888);
constexpr mixxx::RgbColor kMixxxTrackColorDodgerBlue(0x0088FF);
constexpr mixxx::RgbColor kMixxxTrackColorBlue(0x0000FF);
constexpr mixxx::RgbColor kMixxxTrackColorNavyBlue(0x000088);
constexpr mixxx::RgbColor kMixxxTrackColorMardiGras(0x8800088);
constexpr mixxx::RgbColor kMixxxTrackColorVividViolet(0xAA00FF);
constexpr mixxx::RgbColor kMixxxTrackColorFuchsia(0xFF00FF);
constexpr mixxx::RgbColor kMixxxTrackColorViolet(0xFF88FF);
constexpr mixxx::RgbColor kMixxxTrackColorWhite(0xFFFFFF);
constexpr mixxx::RgbColor kMixxxTrackColorAqua(0x00FFFF);
constexpr mixxx::RgbColor kMixxxTrackColorSpringGreen(0x00FF88);
constexpr mixxx::RgbColor kMixxxTrackColorBattleshipGrey(0x888888);

// Rekordbox Track Color Palette
constexpr mixxx::RgbColor kRekordboxTrackColorPink(0xF870F8);
constexpr mixxx::RgbColor kRekordboxTrackColorRed(0xF870900);
constexpr mixxx::RgbColor kRekordboxTrackColorOrange(0xF8A030);
constexpr mixxx::RgbColor kRekordboxTrackColorYellow(0xF8E331);
constexpr mixxx::RgbColor kRekordboxTrackColorGreen(0x1EE000);
constexpr mixxx::RgbColor kRekordboxTrackColorAqua(0x16C0F8);
constexpr mixxx::RgbColor kRekordboxTrackColorBlue(0x0150F8);
constexpr mixxx::RgbColor kRekordboxTrackColorPurple(0x9808F8);

// Rekordbox Hotcue Color Palette
constexpr mixxx::RgbColor kRekordboxHotcueColor1(0xDE44CF);
constexpr mixxx::RgbColor kRekordboxHotcueColor2(0xB432FF);
constexpr mixxx::RgbColor kRekordboxHotcueColor3(0xAA42FF);
constexpr mixxx::RgbColor kRekordboxHotcueColor4(0x6473FF);
constexpr mixxx::RgbColor kRekordboxHotcueColor5(0x305AFF);
constexpr mixxx::RgbColor kRekordboxHotcueColor6(0x50B4FF);
constexpr mixxx::RgbColor kRekordboxHotcueColor7(0x00E0FF);
constexpr mixxx::RgbColor kRekordboxHotcueColor8(0x1FA382);
constexpr mixxx::RgbColor kRekordboxHotcueColor9(0x10B176);
constexpr mixxx::RgbColor kRekordboxHotcueColor10(0x28E214);
constexpr mixxx::RgbColor kRekordboxHotcueColor11(0xA5E116);
constexpr mixxx::RgbColor kRekordboxHotcueColor12(0xB4BE04);
constexpr mixxx::RgbColor kRekordboxHotcueColor13(0xC3AF04);
constexpr mixxx::RgbColor kRekordboxHotcueColor14(0xE0641B);
constexpr mixxx::RgbColor kRekordboxHotcueColor15(0xE62828);
constexpr mixxx::RgbColor kRekordboxHotcueColor16(0xFF127B);

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
constexpr mixxx::RgbColor kSeratoDJProHotcueColorBlue2(0x0F88CA);
constexpr mixxx::RgbColor kSeratoDJProHotcueColorDarkBlue1(0x16308B);
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

// Default Mixxx Key Color Palette (obtained from Mixxx Keywheel)

constexpr mixxx::RgbColor kMixxxKeyColor1(0xFC4949);
constexpr mixxx::RgbColor kMixxxKeyColor2(0xFE642D);
constexpr mixxx::RgbColor kMixxxKeyColor3(0xF98C27);
constexpr mixxx::RgbColor kMixxxKeyColor4(0xFED600);
constexpr mixxx::RgbColor kMixxxKeyColor5(0x99FE00);
constexpr mixxx::RgbColor kMixxxKeyColor6(0x42FE3E);
constexpr mixxx::RgbColor kMixxxKeyColor7(0x0AD58F);
constexpr mixxx::RgbColor kMixxxKeyColor8(0x0AE7E7);
constexpr mixxx::RgbColor kMixxxKeyColor9(0x04C9FE);
constexpr mixxx::RgbColor kMixxxKeyColor10(0x3D8AFD);
constexpr mixxx::RgbColor kMixxxKeyColor11(0xAC64FE);
constexpr mixxx::RgbColor kMixxxKeyColor12(0xFD3FEA);

// Traktor Key Color Palette
// NOTE: C Major is 1d in Open Key Notation

constexpr mixxx::RgbColor kTraktorKeyColor1(0xB960A2);
constexpr mixxx::RgbColor kTraktorKeyColor2(0x8269AB);
constexpr mixxx::RgbColor kTraktorKeyColor3(0x527FC0);
constexpr mixxx::RgbColor kTraktorKeyColor4(0x3CC0EF);
constexpr mixxx::RgbColor kTraktorKeyColor5(0x5BC1CE);
constexpr mixxx::RgbColor kTraktorKeyColor6(0x4CB686);
constexpr mixxx::RgbColor kTraktorKeyColor7(0x73B629);
constexpr mixxx::RgbColor kTraktorKeyColor8(0x9FC516);
constexpr mixxx::RgbColor kTraktorKeyColor9(0xFDD615);
constexpr mixxx::RgbColor kTraktorKeyColor10(0xF28B2E);
constexpr mixxx::RgbColor kTraktorKeyColor11(0xEC6637);
constexpr mixxx::RgbColor kTraktorKeyColor12(0xE84C4D);

// Mixed In Key Key Color Palette
// NOTE: C Major is 8B in Camelot Notation

constexpr mixxx::RgbColor kMIKKeyColor1(0xF17EDB);
constexpr mixxx::RgbColor kMIKKeyColor2(0xD18BFD);
constexpr mixxx::RgbColor kMIKKeyColor3(0x9EB4FD);
constexpr mixxx::RgbColor kMIKKeyColor4(0x4DD3F8);
constexpr mixxx::RgbColor kMIKKeyColor5(0x01EAEC);
constexpr mixxx::RgbColor kMIKKeyColor6(0x00EECB);
constexpr mixxx::RgbColor kMIKKeyColor7(0x20EF7F);
constexpr mixxx::RgbColor kMIKKeyColor8(0x7FF448);
constexpr mixxx::RgbColor kMIKKeyColor9(0xE0CA6D);
constexpr mixxx::RgbColor kMIKKeyColor10(0xFDA078);
constexpr mixxx::RgbColor kMIKKeyColor11(0xFF8693);
constexpr mixxx::RgbColor kMIKKeyColor12(0xFD7EB3);

// Accessible Color Palettes
// When arranged in a circle, the hue varies vertically and lightness/saturation
// varies horizontally so that every color is unique, but adjacent colors are
// similar.

// Protanopia / Protanomaly

constexpr mixxx::RgbColor kProtKeyColor1(0x2626D9);
constexpr mixxx::RgbColor kProtKeyColor2(0x7582D7);
constexpr mixxx::RgbColor kProtKeyColor3(0xA7C2DD);
constexpr mixxx::RgbColor kProtKeyColor4(0xB8E0E0);
constexpr mixxx::RgbColor kProtKeyColor5(0xA7DDC2);
constexpr mixxx::RgbColor kProtKeyColor6(0x75D782);
constexpr mixxx::RgbColor kProtKeyColor7(0x26D926);
constexpr mixxx::RgbColor kProtKeyColor8(0x0DA522);
constexpr mixxx::RgbColor kProtKeyColor9(0x02783D);
constexpr mixxx::RgbColor kProtKeyColor10(0x006666);
constexpr mixxx::RgbColor kProtKeyColor11(0x023D78);
constexpr mixxx::RgbColor kProtKeyColor12(0x0D22A5);

// Deuteranopia / Deuteranomaly

constexpr mixxx::RgbColor kDeutKeyColor1(0xD92626);
constexpr mixxx::RgbColor kDeutKeyColor2(0xD77582);
constexpr mixxx::RgbColor kDeutKeyColor3(0xDDA7C2);
constexpr mixxx::RgbColor kDeutKeyColor4(0xE0B8E0);
constexpr mixxx::RgbColor kDeutKeyColor5(0xC2A7DD);
constexpr mixxx::RgbColor kDeutKeyColor6(0x8275D7);
constexpr mixxx::RgbColor kDeutKeyColor7(0x2626D9);
constexpr mixxx::RgbColor kDeutKeyColor8(0x220DA5);
constexpr mixxx::RgbColor kDeutKeyColor9(0x3D0278);
constexpr mixxx::RgbColor kDeutKeyColor10(0x660066);
constexpr mixxx::RgbColor kDeutKeyColor11(0x78023D);
constexpr mixxx::RgbColor kDeutKeyColor12(0xA50D22);

// Deuteranopia / Deuteranomaly

constexpr mixxx::RgbColor kTritKeyColor1(0x26D926);
constexpr mixxx::RgbColor kTritKeyColor2(0x82D775);
constexpr mixxx::RgbColor kTritKeyColor3(0xC2DDA7);
constexpr mixxx::RgbColor kTritKeyColor4(0xE0E0B8);
constexpr mixxx::RgbColor kTritKeyColor5(0xDDC2A7);
constexpr mixxx::RgbColor kTritKeyColor6(0xD78275);
constexpr mixxx::RgbColor kTritKeyColor7(0xD92626);
constexpr mixxx::RgbColor kTritKeyColor8(0xA5220D);
constexpr mixxx::RgbColor kTritKeyColor9(0x783D02);
constexpr mixxx::RgbColor kTritKeyColor10(0x666600);
constexpr mixxx::RgbColor kTritKeyColor11(0x3D7802);
constexpr mixxx::RgbColor kTritKeyColor12(0x22A50D);

} // anonymous namespace

namespace mixxx {
namespace predefinedcolorpalettes {

const PredefinedColorPalettes& get() {
    // All Rekordbox Palette types, share the same color, but their default colors
    // are not in the selection offered to the user. This usecase is not supported
    // by mixxx's ColorPalette. The compromise is slightly altering the default
    // colors used so they're part of the selection. The difference between these
    // colors should be imperceptible for the unknowing user.
    const QList<mixxx::RgbColor> kRekordboxColorsSelection = {
            kRekordboxHotcueColor1,
            kRekordboxHotcueColor2,
            kRekordboxHotcueColor3,
            kRekordboxHotcueColor4,
            kRekordboxHotcueColor5,
            kRekordboxHotcueColor6,
            kRekordboxHotcueColor7,
            kRekordboxHotcueColor8,
            kRekordboxHotcueColor9,
            kRekordboxHotcueColor10,
            kRekordboxHotcueColor11,
            kRekordboxHotcueColor12,
            kRekordboxHotcueColor13,
            kRekordboxHotcueColor14,
            kRekordboxHotcueColor15,
            kRekordboxHotcueColor16,
    };

    const static PredefinedColorPalettes kPalettes{
            .mixxxHotcueColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP(
                            "PredefinedColorPaletes", "Mixxx Hotcue Colors")),
                    {
                            kColorMixxxRed,
                            kColorMixxxGreen,
                            kColorMixxxCeleste,
                            kColorMixxxYellow,
                            kColorMixxxBlue,
                            kColorMixxxPurple,
                            kColorMixxxPink,
                            kColorMixxxWhite,
                            kSchemaMigrationReplacementColor,
                    },
                    // Exclude kSchemaMigrationReplacementColor from the colors
                    // assigned to hotcues. If there were 9 colors assigned to
                    // hotcues, that would look weird on controllers with >8
                    // hotcue buttons, for example a Novation Launchpad.
                    {0, 1, 2, 3, 4, 5, 6, 7}},
            .seratoTrackMetadataHotcueColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Serato DJ Track Metadata Hotcue Colors")),
                    {
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
                    {0, 2, 12, 3, 6, 15, 9, 14}},
            .seratoDJProHotcueColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Serato DJ Pro Hotcue Colors")),
                    {
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
                    {0, 2, 12, 3, 6, 15, 9, 14}},
            .rekordboxCOLD1HotcueColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Rekordbox COLD1 Hotcue Colors")),
                    kRekordboxColorsSelection,
                    {5, 8, 1, 6, 7, 2, 7, 5}},
            .rekordboxCOLD2HotcueColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Rekordbox COLD2 Hotcue Colors")),
                    kRekordboxColorsSelection,
                    {7, 5, 5, 5, 3, 4, 3, 2}},
            .rekordboxCOLORFULHotcueColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Rekordbox COLORFUL Hotcue Colors")),
                    kRekordboxColorsSelection,
                    {15, 5, 10, 2, 8, 13, 4, 12}},
            .mixxxTrackColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP(
                            "PredefinedColorPalettes", "Mixxx Track Colors")),
                    {
                            kMixxxTrackColorDarkRed,
                            kMixxxTrackColorRed,
                            kMixxxTrackColorDarkOrange,
                            kMixxxTrackColorLemonGlacier,
                            kMixxxTrackColorChartreuse,
                            kMixxxTrackColorElectricGreen,
                            kMixxxTrackColorIndiaGreen,
                            kMixxxTrackColorDarkCyan,
                            kMixxxTrackColorDodgerBlue,
                            kMixxxTrackColorBlue,
                            kMixxxTrackColorNavyBlue,
                            kMixxxTrackColorMardiGras,
                            kMixxxTrackColorVividViolet,
                            kMixxxTrackColorFuchsia,
                            kMixxxTrackColorViolet,
                            kMixxxTrackColorWhite,
                            kMixxxTrackColorAqua,
                            kMixxxTrackColorSpringGreen,
                            kMixxxTrackColorBattleshipGrey,
                    }},
            .rekordboxTrackColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Rekordbox Track Colors")),
                    {
                            kRekordboxTrackColorPink,
                            kRekordboxTrackColorRed,
                            kRekordboxTrackColorOrange,
                            kRekordboxTrackColorYellow,
                            kRekordboxTrackColorGreen,
                            kRekordboxTrackColorAqua,
                            kRekordboxTrackColorBlue,
                            kRekordboxTrackColorPurple,
                    }},
            .seratoDJProTrackColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Serato DJ Pro Track Colors")),
                    {
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
                    }},
            .traktorProTrackColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Traktor Pro Track Colors")),
                    {
                            kTraktorProTrackColorRed,
                            kTraktorProTrackColorOrange,
                            kTraktorProTrackColorYellow,
                            kTraktorProTrackColorGreen,
                            kTraktorProTrackColorBlue,
                            kTraktorProTrackColorViolet,
                            kTraktorProTrackColorMagenta,
                    }},
            .virtualDJTrackColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "VirtualDJ Track Colors")),
                    {
                            kVirtualDJTrackColorRed,
                            kVirtualDJTrackColorYellow,
                            kVirtualDJTrackColorGreen,
                            kVirtualDJTrackColorCyan,
                            kVirtualDJTrackColorBlue,
                            kVirtualDJTrackColorFuchsia,
                            kVirtualDJTrackColorWhite,
                    }},
            .mixxxKeyColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP(
                            "PredefinedColorPalettes", "Mixxx Key Colors")),
                    {
                            kMixxxKeyColor1,
                            kMixxxKeyColor2,
                            kMixxxKeyColor3,
                            kMixxxKeyColor4,
                            kMixxxKeyColor5,
                            kMixxxKeyColor6,
                            kMixxxKeyColor7,
                            kMixxxKeyColor8,
                            kMixxxKeyColor9,
                            kMixxxKeyColor10,
                            kMixxxKeyColor11,
                            kMixxxKeyColor12,
                    }},
            .traktorKeyColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP(
                            "PredefinedColorPalettes", "Traktor Key Colors")),
                    {
                            kTraktorKeyColor1,
                            kTraktorKeyColor2,
                            kTraktorKeyColor3,
                            kTraktorKeyColor4,
                            kTraktorKeyColor5,
                            kTraktorKeyColor6,
                            kTraktorKeyColor7,
                            kTraktorKeyColor8,
                            kTraktorKeyColor9,
                            kTraktorKeyColor10,
                            kTraktorKeyColor11,
                            kTraktorKeyColor12,
                    }},
            .MIKKeyColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Mixed In Key - Key Colors")),
                    {
                            kMIKKeyColor1,
                            kMIKKeyColor2,
                            kMIKKeyColor3,
                            kMIKKeyColor4,
                            kMIKKeyColor5,
                            kMIKKeyColor6,
                            kMIKKeyColor7,
                            kMIKKeyColor8,
                            kMIKKeyColor9,
                            kMIKKeyColor10,
                            kMIKKeyColor11,
                            kMIKKeyColor12,
                    }},
            .protKeyColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Protanopia / Protanomaly Key Colors")),
                    {
                            kProtKeyColor1,
                            kProtKeyColor2,
                            kProtKeyColor3,
                            kProtKeyColor4,
                            kProtKeyColor5,
                            kProtKeyColor6,
                            kProtKeyColor7,
                            kProtKeyColor8,
                            kProtKeyColor9,
                            kProtKeyColor10,
                            kProtKeyColor11,
                            kProtKeyColor12,
                    }},
            .deutKeyColorPalette{

                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Deuteranopia / Deuteranomaly Key Colors")),
                    {
                            kDeutKeyColor1,
                            kDeutKeyColor2,
                            kDeutKeyColor3,
                            kDeutKeyColor4,
                            kDeutKeyColor5,
                            kDeutKeyColor6,
                            kDeutKeyColor7,
                            kDeutKeyColor8,
                            kDeutKeyColor9,
                            kDeutKeyColor10,
                            kDeutKeyColor11,
                            kDeutKeyColor12,
                    }},
            .tritKeyColorPalette{
                    QStringLiteral(QT_TRANSLATE_NOOP("PredefinedColorPalettes",
                            "Tritanopia / Tritanomaly Key Colors")),
                    {
                            kTritKeyColor1,
                            kTritKeyColor2,
                            kTritKeyColor3,
                            kTritKeyColor4,
                            kTritKeyColor5,
                            kTritKeyColor6,
                            kTritKeyColor7,
                            kTritKeyColor8,
                            kTritKeyColor9,
                            kTritKeyColor10,
                            kTritKeyColor11,
                            kTritKeyColor12,
                    }},
            .defaultHotcueColorPalette = kPalettes.mixxxHotcueColorPalette,
            .defaultTrackColorPalette = kPalettes.mixxxTrackColorPalette,
            .defaultKeyColorPalette = kPalettes.mixxxKeyColorPalette,
            .palettes{
                    // Hotcue Color Palettes
                    &kPalettes.mixxxHotcueColorPalette,
                    &kPalettes.seratoDJProHotcueColorPalette,
                    &kPalettes.rekordboxCOLD1HotcueColorPalette,
                    &kPalettes.rekordboxCOLD2HotcueColorPalette,
                    &kPalettes.rekordboxCOLORFULHotcueColorPalette,
                    // Track Color Palettes
                    &kPalettes.mixxxTrackColorPalette,
                    &kPalettes.rekordboxTrackColorPalette,
                    &kPalettes.seratoDJProTrackColorPalette,
                    &kPalettes.traktorProTrackColorPalette,
                    &kPalettes.virtualDJTrackColorPalette,
                    // Key Color Palettes
                    &kPalettes.mixxxKeyColorPalette,
                    &kPalettes.traktorKeyColorPalette,
                    &kPalettes.MIKKeyColorPalette,
                    &kPalettes.protKeyColorPalette,
                    &kPalettes.deutKeyColorPalette,
                    &kPalettes.tritKeyColorPalette,
            }};
    return kPalettes;
}
} // namespace predefinedcolorpalettes
} // namespace mixxx
