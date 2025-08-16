#include "track/keyutils.h"

#include <QMap>
#include <QPair>
#include <QRegularExpression>
#include <QtDebug>

#include "preferences/keydetectionsettings.h"
#include "util/color/colorpalette.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/color/rgbcolor.h"
#include "util/compatibility/qmutex.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

namespace {

// All of these regular expressions need exact matching.
// QRegularExpression::anchoredPattern cannot be used because Mixxx supports Qt < 5.15, so
// instead do what QRegularExpression::anchoredPattern by wrapping each regex in \\A(?: ... )\\z

// OpenKey notation, the numbers 1-12 followed by d (dur, major) or m (moll, minor).
const QRegularExpression s_openKeyRegex(QStringLiteral(
        "\\A(?:^\\s*(1[0-2]|[1-9])([dm])\\s*$)\\z"));

// Lancelot notation, the numbers 1-12 followed by A (minor) or B(I) (major).
// or "I", "L", "M", "D", "P", "C" for the advanced modes
const QRegularExpression s_lancelotKeyRegex(
        QStringLiteral("\\A(?:^\\s*0*(1[0-2]|[1-9])([ABILMDPC])\\s*$)\\z"),
        QRegularExpression::CaseInsensitiveOption);
constexpr std::string_view s_lancelotMajorModes = "BILM";

// a-g followed by any number of sharps or flats, optionally followed by
// a scale mode spec (m = minor, min, maj ..)
// Note: ## = x exists: https://jadebultitude.com/double-sharp-sign-in-music
const QRegularExpression s_keyRegex(
        QString::fromUtf8(
                "\\A(?:^\\s*([a-g])([#♯b♭]*) *([a-z]{3}.*|m)?\\s*$)\\z"),
        QRegularExpression::CaseInsensitiveOption);

const QString s_sharpSymbol = QString::fromUtf8("♯");
//static const QString s_flatSymbol = QString::fromUtf8("♭");

const QString s_traditionalKeyNames[] = {
        QStringLiteral(u"INVALID"),
        QStringLiteral(u"C"),
        QStringLiteral(u"D♭"),
        QStringLiteral(u"D"),
        QStringLiteral(u"E♭"),
        QStringLiteral(u"E"),
        QStringLiteral(u"F"),
        QStringLiteral(u"F♯/G♭"),
        QStringLiteral(u"G"),
        QStringLiteral(u"A♭"),
        QStringLiteral(u"A"),
        QStringLiteral(u"B♭"),
        QStringLiteral(u"B"),
        QStringLiteral(u"Cm"),
        QStringLiteral(u"C♯m"),
        QStringLiteral(u"Dm"),
        QStringLiteral(u"D♯m/E♭m"),
        QStringLiteral(u"Em"),
        QStringLiteral(u"Fm"),
        QStringLiteral(u"F♯m"),
        QStringLiteral(u"Gm"),
        QStringLiteral(u"G♯m"),
        QStringLiteral(u"Am"),
        QStringLiteral(u"B♭m"),
        QStringLiteral(u"Bm")};

// ID3v2.3.0 specification (https://id3.org/id3v2.3.0):
// The 'Initial key' frame contains the musical key in which the sound starts.
// It is represented as a string with a maximum length of three characters.
// The ground keys are represented with "A","B","C","D","E", "F" and "G" and halfkeys
// represented with "b" and "#". Minor is represented as "m". Example "Cbm".
// Off key is represented with an "o" only.
const std::array s_IDv3KeyNames = {
        // these are QStringLiterals because they're used as QStrings below, even though they
        // only contain ASCII characters
        QStringLiteral("o"),
        QStringLiteral("C"),
        QStringLiteral("Db"),
        QStringLiteral("D"),
        QStringLiteral("Eb"),
        QStringLiteral("E"),
        QStringLiteral("F"),
        QStringLiteral("F#"),
        QStringLiteral("G"),
        QStringLiteral("Ab"),
        QStringLiteral("A"),
        QStringLiteral("Bb"),
        QStringLiteral("B"),
        QStringLiteral("Cm"),
        QStringLiteral("C#m"),
        QStringLiteral("Dm"),
        QStringLiteral("Ebm"),
        QStringLiteral("Em"),
        QStringLiteral("Fm"),
        QStringLiteral("F#m"),
        QStringLiteral("Gm"),
        QStringLiteral("G#m"),
        QStringLiteral("Am"),
        QStringLiteral("Bbm"),
        QStringLiteral("Bm")};

// Maps an OpenKey number to its major and minor key.
constexpr ChromaticKey s_openKeyToKeys[][2] = {
        // 0 is not a valid OpenKey number.
        {mixxx::track::io::key::INVALID, mixxx::track::io::key::INVALID},
        {mixxx::track::io::key::C_MAJOR, mixxx::track::io::key::A_MINOR},            // 1
        {mixxx::track::io::key::G_MAJOR, mixxx::track::io::key::E_MINOR},            // 2
        {mixxx::track::io::key::D_MAJOR, mixxx::track::io::key::B_MINOR},            // 3
        {mixxx::track::io::key::A_MAJOR, mixxx::track::io::key::F_SHARP_MINOR},      // 4
        {mixxx::track::io::key::E_MAJOR, mixxx::track::io::key::C_SHARP_MINOR},      // 5
        {mixxx::track::io::key::B_MAJOR, mixxx::track::io::key::G_SHARP_MINOR},      // 6
        {mixxx::track::io::key::F_SHARP_MAJOR, mixxx::track::io::key::E_FLAT_MINOR}, // 7
        {mixxx::track::io::key::D_FLAT_MAJOR, mixxx::track::io::key::B_FLAT_MINOR},  // 8
        {mixxx::track::io::key::A_FLAT_MAJOR, mixxx::track::io::key::F_MINOR},       // 9
        {mixxx::track::io::key::E_FLAT_MAJOR, mixxx::track::io::key::C_MINOR},       // 10
        {mixxx::track::io::key::B_FLAT_MAJOR, mixxx::track::io::key::G_MINOR},       // 11
        {mixxx::track::io::key::F_MAJOR, mixxx::track::io::key::D_MINOR}             // 12
};

// This is a quick hack to convert an ASCII letter into a key. Lookup the key
// using (letter.toLower() - 'a') as the index. Make sure the letter matches
// [a-gA-G] first.
constexpr ChromaticKey s_letterToMajorKey[] = {
        mixxx::track::io::key::A_MAJOR,
        mixxx::track::io::key::B_MAJOR,
        mixxx::track::io::key::C_MAJOR,
        mixxx::track::io::key::D_MAJOR,
        mixxx::track::io::key::E_MAJOR,
        mixxx::track::io::key::F_MAJOR,
        mixxx::track::io::key::G_MAJOR};

constexpr int s_sortKeysCircleOfFifths[] = {
        0,  // INVALID
        1,  // C_MAJOR
        15, // D_FLAT_MAJOR
        5,  // D_MAJOR
        19, // E_FLAT_MAJOR
        9,  // E_MAJOR
        23, // F_MAJOR
        13, // F_SHARP_MAJOR
        3,  // G_MAJOR
        17, // A_FLAT_MAJOR
        7,  // A_MAJOR
        21, // B_FLAT_MAJOR
        11, // B_MAJOR
        20, // C_MINOR
        10, // C_SHARP_MINOR
        24, // D_MINOR
        14, // E_FLAT_MINOR
        4,  // E_MINOR
        18, // F_MINOR
        8,  // F_SHARP_MINOR
        22, // G_MINOR
        12, // G_SHARP_MINOR
        2,  // A_MINOR
        16, // B_FLAT_MINOR
        6,  // B_MINOR
};

constexpr int s_sortKeysCircleOfFifthsLancelot[] = {
        0,  // INVALID
        16, // C_MAJOR
        6,  // D_FLAT_MAJOR
        20, // D_MAJOR
        10, // E_FLAT_MAJOR
        24, // E_MAJOR
        14, // F_MAJOR
        4,  // F_SHARP_MAJOR
        18, // G_MAJOR
        8,  // A_FLAT_MAJOR
        22, // A_MAJOR
        12, // B_FLAT_MAJOR
        2,  // B_MAJOR
        9,  // C_MINOR
        23, // C_SHARP_MINOR
        13, // D_MINOR
        3,  // E_FLAT_MINOR
        17, // E_MINOR
        7,  // F_MINOR
        21, // F_SHARP_MINOR
        11, // G_MINOR
        1,  // G_SHARP_MINOR
        15, // A_MINOR
        5,  // B_FLAT_MINOR
        19, // B_MINOR
};

struct ScaleModeInfo {
    KeyUtils::ScaleMode mode;
    const char* text;
    const char* textShort;
    bool isMajor;
    int transposeSteps;
};

// Strings used by Rapid Evolution when exporting detailed keys to file tags
constexpr ScaleModeInfo s_scaleModeInfo[] = {
        {KeyUtils::ScaleMode::Ionian, // standard major
                "ionian",
                "ion",
                true,
                0},
        {KeyUtils::ScaleMode::Aeolian, // natural minor
                "aeolian",
                "aeo",
                false,
                0},
        {KeyUtils::ScaleMode::Lydian, // major with raised 4th
                "lydian",
                "lyd",
                true,
                -5},
        {KeyUtils::ScaleMode::Mixolydian, // major with lowered 7th
                "mixolydian",
                "mix",
                true,
                +5},
        {KeyUtils::ScaleMode::Dorian, // minor with raised 6th
                "dorian",
                "dor",
                false,
                -5},
        {KeyUtils::ScaleMode::Phrygian, // minor with lowered 2nd
                "phrygian",
                "phr",
                false,
                +5},
        {KeyUtils::ScaleMode::Locrian, //  minor with lowered 2nd and 7th
                "locrian",
                "loc",
                false,
                -2},
};

// transposeSteps is used to express the detailed modes
// as a compatible natural minor or standard major scale
// The following table shows the relation for C major in the Circle of Fifths
// "C ionian"     8B    C – D – E – F – G – A – B
// "Db"
// "D dorian"     8D    D – E – F – G – A – B – C
// "Eb"
// "E phrygian"   8P    E – F – G – A – B – C – D
// "F lydian" 	  8L    F – G – A – B – C – D – E
// "Gb"
// "G mixolydian  8M    G – A – B – C – D – E – F
// "Ab"
// "A aeolian"    8A    A – B – C – D – E – F – G
// "Bb"
// "B locrian"    8C    B – C – D – E – F – G – A

// We transpose according to isMajor:
// lydian to ionian
// mixolydian to ionian
// dorian to aeolian
// phrygian to aeolian
// locrian to aeolian

// Lancelot notation is OpenKey notation rotated counter-clockwise by 5.
inline int openKeyNumberToLancelotNumber(const int okNumber)  {
    int lancelotNumber = okNumber - 5;
    if (lancelotNumber < 1) {
        lancelotNumber += 12;
    }
    return lancelotNumber;
}

// Lancelot notation is OpenKey notation rotated counter-clockwise by 5.
inline int lancelotNumberToOpenKeyNumber(const int lancelotNumber)  {
    int okNumber = lancelotNumber + 5;
    if (okNumber > 12) {
        okNumber -= 12;
    }
    return okNumber;
}

} // namespace

QMutex KeyUtils::s_notationMutex;
QMap<ChromaticKey, QString> KeyUtils::s_notation;
QMap<QString, ChromaticKey> KeyUtils::s_reverseNotation;

// static
ChromaticKey KeyUtils::openKeyNumberToKey(int openKeyNumber, bool major) {
        return s_openKeyToKeys[openKeyNumber][major ? 0 : 1];
}

// static
QString KeyUtils::keyDebugName(ChromaticKey key) {
    if (!ChromaticKey_IsValid(key)) {
        key = mixxx::track::io::key::INVALID;
    }
    return s_traditionalKeyNames[static_cast<int>(key)];
}

// static
void KeyUtils::setNotation(const QMap<ChromaticKey, QString>& notation) {
    const auto locker = lockMutex(&s_notationMutex);
    s_notation = notation;
    s_reverseNotation.clear();

    for (auto it = s_notation.constBegin(); it != s_notation.constEnd(); ++it) {
        if (s_reverseNotation.contains(it.value())) {
            qWarning() << "Key notation is surjective (has duplicate values).";
        }
        s_reverseNotation.insert(it.value(), it.key());
    }
}

// static
int KeyUtils::keyToOpenKeyNumber(mixxx::track::io::key::ChromaticKey key) {
    switch (key) {
    case mixxx::track::io::key::C_MAJOR:
    case mixxx::track::io::key::A_MINOR:
        return 1;
    case mixxx::track::io::key::G_MAJOR:
    case mixxx::track::io::key::E_MINOR:
        return 2;
    case mixxx::track::io::key::D_MAJOR:
    case mixxx::track::io::key::B_MINOR:
        return 3;
    case mixxx::track::io::key::A_MAJOR:
    case mixxx::track::io::key::F_SHARP_MINOR:
        return 4;
    case mixxx::track::io::key::E_MAJOR:
    case mixxx::track::io::key::C_SHARP_MINOR:
        return 5;
    case mixxx::track::io::key::B_MAJOR:
    case mixxx::track::io::key::G_SHARP_MINOR:
        return 6;
    case mixxx::track::io::key::F_SHARP_MAJOR:
    case mixxx::track::io::key::E_FLAT_MINOR:
        return 7;
    case mixxx::track::io::key::D_FLAT_MAJOR:
    case mixxx::track::io::key::B_FLAT_MINOR:
        return 8;
    case mixxx::track::io::key::A_FLAT_MAJOR:
    case mixxx::track::io::key::F_MINOR:
        return 9;
    case mixxx::track::io::key::E_FLAT_MAJOR:
    case mixxx::track::io::key::C_MINOR:
        return 10;
    case mixxx::track::io::key::B_FLAT_MAJOR:
    case mixxx::track::io::key::G_MINOR:
        return 11;
    case mixxx::track::io::key::F_MAJOR:
    case mixxx::track::io::key::D_MINOR:
        return 12;
    default:
        return 0;
    }
}

// static
QString KeyUtils::keyToString(ChromaticKey key,
                              KeyNotation notation) {
    if (!ChromaticKey_IsValid(key) ||
        key == mixxx::track::io::key::INVALID) {
        return {};
    }

    if (notation == KeyNotation::Custom) {
        // The default value for notation is KeyUtils::KeyNotation::Custom, so
        // this executes when the function is called without a notation specified
        // after KeyUtils::setNotation has set up s_notation.
        const auto locker = lockMutex(&s_notationMutex);
        auto it = s_notation.constFind(key);
        if (it != s_notation.constEnd()) {
            return it.value();
        }
    } else if (notation == KeyNotation::OpenKey) {
        bool major = keyIsMajor(key);
        int number = keyToOpenKeyNumber(key);
        return QString::number(number) + (major ? "d" : "m");
    } else if (notation == KeyNotation::OpenKeyAndTraditional) {
        bool major = keyIsMajor(key);
        int number = keyToOpenKeyNumber(key);
        QString trad = s_traditionalKeyNames[static_cast<int>(key)];
        return QString::number(number) + (major ? "d" : "m") + " (" + trad + ")";
    } else if (notation == KeyNotation::Lancelot) {
        bool major = keyIsMajor(key);
        int number = openKeyNumberToLancelotNumber(keyToOpenKeyNumber(key));
        return QString::number(number) + (major ? "B" : "A");
    } else if (notation == KeyNotation::LancelotAndTraditional) {
        bool major = keyIsMajor(key);
        int number = openKeyNumberToLancelotNumber(keyToOpenKeyNumber(key));
        QString trad = s_traditionalKeyNames[static_cast<int>(key)];
        return QString::number(number) + (major ? "B" : "A") + " (" + trad + ")";
    } else if (notation == KeyNotation::Traditional) {
        return s_traditionalKeyNames[static_cast<int>(key)];
    } else if (notation == KeyNotation::ID3v2) {
        return s_IDv3KeyNames[static_cast<int>(key)];
    }
    return keyDebugName(key);
}

// static
QString KeyUtils::formatGlobalKey(const Keys& keys, KeyNotation notation) {
    const mixxx::track::io::key::ChromaticKey globalKey = keys.getGlobalKey();
    if (globalKey != mixxx::track::io::key::INVALID) {
        return keyToString(globalKey, notation);
    } else {
        // Fall back on text global name
        return keys.getGlobalKeyText();
    }
}

// static
ChromaticKey KeyUtils::guessKeyFromText(const QString& text) {
    // Remove Shift (Tuning) Information used by Rapid Evolution like: "A#m +50";
    int shiftStart = text.indexOf('+');
    if (shiftStart < 0) {
        shiftStart = text.indexOf('-');
    }
    const QString trimmed =
            shiftStart >= 0 ? text.left(shiftStart).trimmed() : text.trimmed();

    if (trimmed.isEmpty()) {
        return mixxx::track::io::key::INVALID;
    }

    // Try using the user's custom notation.
    {
        const auto locker = lockMutex(&s_notationMutex);
        auto it = s_reverseNotation.constFind(text);
        if (it != s_reverseNotation.constEnd()) {
            return it.value();
        }
    }

    QRegularExpressionMatch openKeyMatch = s_openKeyRegex.match(trimmed);
    if (openKeyMatch.hasMatch()) {
        bool ok = false;
        int openKeyNumber = openKeyMatch.captured(1).toInt(&ok);

        // Regex should mean this never happens.
        if (!ok || openKeyNumber < 1 || openKeyNumber > 12) {
            return mixxx::track::io::key::INVALID;
        }

        bool major = openKeyMatch.captured(2)
                             .compare("d") == 0;

        return openKeyNumberToKey(openKeyNumber, major);
    }

    QRegularExpressionMatch lancelotMatch = s_lancelotKeyRegex.match(trimmed);
    if (lancelotMatch.hasMatch()) {
        bool ok = false;
        int lancelotNumber = lancelotMatch.captured(1).toInt(&ok);

        // Regex should mean this never happens.
        if (!ok || lancelotNumber < 1 || lancelotNumber > 12) {
            return mixxx::track::io::key::INVALID;
        }

        int openKeyNumber = lancelotNumberToOpenKeyNumber(lancelotNumber);

        const QChar lancelotScaleMode = lancelotMatch.captured(2).at(0);
        bool major = (s_lancelotMajorModes.find(
                              lancelotScaleMode.toUpper().toLatin1()) !=
                std::string::npos);
        return openKeyNumberToKey(openKeyNumber, major);
    }

    QRegularExpressionMatch keyMatch = s_keyRegex.match(trimmed);
    if (keyMatch.hasMatch()) {
        // Take the first letter, lowercase it and subtract 'a' and we get a
        // number between 0-6. Look up the major key associated with that letter
        // from s_letterToMajorKey. Upper-case means major, lower-case means
        // minor. Then apply the sharps or flats to the key.
        QChar letter = keyMatch.captured(1).at(0);
        int letterIndex = letter.toLower().toLatin1() - 'a';
        bool major = letter.isUpper();

        // Now apply sharps and flats to the letter key.
        QString adjustments = keyMatch.captured(2);
        int steps = 0;
        for (const auto* it = adjustments.constBegin();
                it != adjustments.constEnd();
                ++it) {
            steps += (*it == '#' || *it == s_sharpSymbol[0]) ? 1 : -1;
        }

        QString scale = keyMatch.captured(3);
        // we override major if a scale definition exists
        if (!scale.isEmpty()) {
            if (scale.compare("m", Qt::CaseInsensitive) == 0) {
                // Aeolian
                major = false;
            } else if (scale.startsWith("min", Qt::CaseInsensitive)) {
                // Aeolian
                major = false;
            } else if (scale.startsWith("maj", Qt::CaseInsensitive)) {
                // Ionian
                major = true;
            } else {
                // Try to find detailed scale mode
                ScaleMode scaleMode = ScaleMode::Unknown;
                for (const ScaleModeInfo& modeInfo : s_scaleModeInfo) {
                    if (scale.startsWith(modeInfo.textShort, Qt::CaseInsensitive)) {
                        scaleMode = modeInfo.mode;
                        major = modeInfo.isMajor;
                        steps += modeInfo.transposeSteps;
                        break;
                    }
                }
                if (scaleMode == ScaleMode::Unknown) {
                    return mixxx::track::io::key::INVALID;
                }
            }
        }
        ChromaticKey letterKey = static_cast<ChromaticKey>(
            s_letterToMajorKey[letterIndex] + (major ? 0 : 12));
        return scaleKeySteps(letterKey, steps);
    }

    // We didn't figure out the key. Womp womp.
    return mixxx::track::io::key::INVALID;
}

// static
ChromaticKey KeyUtils::keyFromNumericValue(double value) {
    int value_floored = static_cast<int>(value);
    return keyFromNumericValue(value_floored);
}

// static
ChromaticKey KeyUtils::keyFromNumericValue(int value) {
    if (!ChromaticKey_IsValid(value)) {
        return mixxx::track::io::key::INVALID;
    }

    return static_cast<ChromaticKey>(value);
}

KeyUtils::KeyNotation KeyUtils::keyNotationFromNumericValue(double value) {
    int value_floored = static_cast<int>(value);
    if (value_floored < static_cast<int>(KeyNotation::Invalid)
        || value_floored >= static_cast<int>(KeyNotation::NumKeyNotations)) {
        return KeyNotation::Invalid;
    }
    return static_cast<KeyNotation>(value_floored);
}

KeyUtils::KeyNotation KeyUtils::keyNotationFromString(const QString& notationName) {
    if (notationName == KEY_NOTATION_CUSTOM) {
        return KeyUtils::KeyNotation::Custom;
    } else if (notationName == KEY_NOTATION_OPEN_KEY) {
        return KeyUtils::KeyNotation::OpenKey;
    } else if (notationName == KEY_NOTATION_LANCELOT) {
        return KeyUtils::KeyNotation::Lancelot;
    } else if (notationName == KEY_NOTATION_TRADITIONAL) {
        return KeyUtils::KeyNotation::Traditional;
    } else if (notationName == KEY_NOTATION_OPEN_KEY_AND_TRADITIONAL) {
        return KeyUtils::KeyNotation::OpenKeyAndTraditional;
    } else if (notationName == KEY_NOTATION_LANCELOT_AND_TRADITIONAL) {
        return KeyUtils::KeyNotation::LancelotAndTraditional;
    } else {
        return KeyUtils::KeyNotation::Invalid;
    }
}

// static
double KeyUtils::keyToNumericValue(ChromaticKey key) {
    return key;
}

// static
QColor KeyUtils::keyToColor(ChromaticKey key, const ColorPalette& palette) {
    int openKeyNumber = keyToOpenKeyNumber(key);

    if (openKeyNumber != 0) {
        DEBUG_ASSERT(openKeyNumber <= palette.size() && openKeyNumber >= 1);
        const auto rgbColor = palette.at(openKeyNumber - 1); // Open Key numbers start from 1
        return mixxx::RgbColor::toQColor(rgbColor);
    } else {
        return {}; // return invalid color
    }
}

// static
QPair<ChromaticKey, double> KeyUtils::scaleKeyOctaves(ChromaticKey key, double octave_change) {
    // Convert the octave_change from percentage of octave to the nearest
    // integer of key changes. We need the rounding to be in the same direction
    // so that a -1.0 and 1.0 scale of C makes it back to C.
    double key_changes_scaled = octave_change * 12;
    int key_changes = static_cast<int>(key_changes_scaled +
                          (key_changes_scaled > 0 ? 0.5 : -0.5));

    double diff_to_nearest_full_key = key_changes_scaled - key_changes;
    return QPair<ChromaticKey, double>(scaleKeySteps(key, key_changes), diff_to_nearest_full_key);
}

// static
ChromaticKey KeyUtils::scaleKeySteps(ChromaticKey key, int key_changes) {
    // Invalid scales to invalid.
    if (!ChromaticKey_IsValid(key) ||
        key == mixxx::track::io::key::INVALID) {
        return mixxx::track::io::key::INVALID;
    }

    if (key_changes == 0) {
        return key;
    }

    // We know the key is in the set of valid values. Save whether or not the
    // value is major.
    bool major = keyIsMajor(key);

    // Tonic, 0-indexed.
    int tonic = keyToTonic(key);

    // Add the key_changes, mod 12.
    tonic = (tonic + key_changes) % 12;

    // If tonic + key_changes ended up negative, the modulus could have ended up
    // negative (it's implementation specific). Add 12 until we are positive.
    while (tonic < 0) {
        tonic += 12;
    }

    return tonicToKey(tonic, major);
}

// static
mixxx::track::io::key::ChromaticKey KeyUtils::calculateGlobalKey(
        const KeyChangeList& key_changes, SINT totalFrames, mixxx::audio::SampleRate sampleRate) {
    if (key_changes.size() == 1) {
        qDebug() << keyDebugName(key_changes[0].first);
        return key_changes[0].first;
    }
    QMap<mixxx::track::io::key::ChromaticKey, double> key_histogram;

    for (int i = 0; i < key_changes.size(); ++i) {
        mixxx::track::io::key::ChromaticKey key = key_changes[i].first;
        const double start_frame = key_changes[i].second;
        const double next_frame = (i == key_changes.size() - 1)
                ? totalFrames
                : key_changes[i + 1].second;
        key_histogram[key] += (next_frame - start_frame);
    }


    double max_delta = 0;
    mixxx::track::io::key::ChromaticKey max_key = mixxx::track::io::key::INVALID;
    qDebug() << "Key Histogram";
    for (auto it = key_histogram.constBegin();
         it != key_histogram.constEnd(); ++it) {
        qDebug() << it.key() << ":" << keyDebugName(it.key()) << it.value() / sampleRate;
        if (it.value() > max_delta) {
            max_key = it.key();
            max_delta = it.value();
        }
    }
    return max_key;
}

// static
int KeyUtils::shortestStepsToKey(
    mixxx::track::io::key::ChromaticKey key,
    mixxx::track::io::key::ChromaticKey target_key) {
    // For invalid keys just return zero steps.
    if (!ChromaticKey_IsValid(key) ||
            key == mixxx::track::io::key::INVALID ||
            !ChromaticKey_IsValid(target_key) ||
            target_key == mixxx::track::io::key::INVALID) {
        return 0;
    }

    // NOTE(rryan): Even though it's kind of meaningless to call this with keys
    // that are not both major/minor, we are tolerant of that.

    // Tonic, 0-indexed.
    int tonic = keyToTonic(key);
    int targetTonic = keyToTonic(target_key);
    int steps = targetTonic - tonic;

    if (steps > 6) {
        steps -= 12;
    } else if (steps < -6) {
        steps += 12;
    }

    return steps;
}

// static
int KeyUtils::shortestStepsToCompatibleKey(
        mixxx::track::io::key::ChromaticKey key,
        mixxx::track::io::key::ChromaticKey target_key) {

    if (!ChromaticKey_IsValid(key) ||
            key == mixxx::track::io::key::INVALID ||
            !ChromaticKey_IsValid(target_key) ||
            target_key == mixxx::track::io::key::INVALID ||
            key == target_key) {
        // For invalid keys just return zero steps.
        return 0;
    }


    if (key == target_key) {
        // Already matching
        return 0;
    }

    // We know the key is in the set of valid values. Save whether or not the
    // value is major.
    bool major = keyIsMajor(key);
    bool targetMajor = keyIsMajor(target_key);

    // If we have a mode mismatch, matching to a the Relative mode
    // will produce a pitch up to +-6 semitones, which may sounds
    // too much chipmunked than expected.
    // Since the the relative major/minor key shares the same notes
    // we can convert the target mode
    if (major != targetMajor) {
        int openKeyNumber = KeyUtils::keyToOpenKeyNumber(target_key);
        target_key = openKeyNumberToKey(openKeyNumber, !targetMajor);
    }

    // Now both keys at the same mode
    // The Compatible Key is +-5 or 0 semitone steps away
    // 0(+-12) Tonic match
    // +5 Perfect 4th (Sub-Dominant)
    // -5 Perfect 5th (Dominant)

    // first we need the current step distance to decide
    // which case we choose
    int shortestDistance = shortestStepsToKey(key, target_key);
    // shortestDistance is in the range of -6 ..  +6
    if (shortestDistance < -2) {
        // Perfect 4th (Sub-Dominant)
        return 5 + shortestDistance;
    } if (shortestDistance > 2) {
        // Perfect 5th (Dominant)
        return -5 + shortestDistance;
    }
    // tonic match
    return shortestDistance; // in the range of -2 .. +2
}

QList<mixxx::track::io::key::ChromaticKey> KeyUtils::getCompatibleKeys(
        mixxx::track::io::key::ChromaticKey key) {
    QList<mixxx::track::io::key::ChromaticKey> compatible;
    if (!ChromaticKey_IsValid(key) || key == mixxx::track::io::key::INVALID) {
        return compatible;
    }

    int openKeyNumber = KeyUtils::keyToOpenKeyNumber(key);
    // We know the key is in the set of valid values. Save whether or not the
    // value is minor.
    bool major = keyIsMajor(key);

    // The compatible keys of particular key are:
    // * The relative major/minor key.
    // * The perfect 4th (sub-dominant) major/minor key.
    // * The perfect 5th (dominant) major/minor key.
    //
    // The Circle of Fifths is a handy tool that encodes this compatibility.
    // Keys on the same radial of the circle are compatible and adjacent keys on
    // the circle are compatible. OpenKey notation encodes the Circle of
    // Fifths. The OpenKey number represents the radial on the circle and
    // major/minor determines whether you are on the inner or outer ring of the
    // radial.

    // The key is compatible with tracks in the same key.
    compatible << key;

    auto relativeKey = openKeyNumberToKey(openKeyNumber, !major);
    int relativeOpenKeyNumber = KeyUtils::keyToOpenKeyNumber(relativeKey);

    // The relative major/minor key is compatible.
    compatible << relativeKey;

    // The perfect 4th and perfect 5th of BOTH major and minor key are compatible
    // (as explained by Phil Morse: https://youtu.be/9eECvYYAwbg?t=2370)
    compatible << openKeyNumberToKey(
            openKeyNumber == 12 ? 1 : openKeyNumber + 1, major);
    compatible << openKeyNumberToKey(
            relativeOpenKeyNumber == 12 ? 1 : relativeOpenKeyNumber + 1, !major);
    compatible << openKeyNumberToKey(
            openKeyNumber == 1 ? 12 : openKeyNumber - 1, major);
    compatible << openKeyNumberToKey(
            relativeOpenKeyNumber == 1 ? 12 : relativeOpenKeyNumber - 1, !major);
    return compatible;
}

int KeyUtils::keyToCircleOfFifthsOrder(mixxx::track::io::key::ChromaticKey key,
                                       KeyNotation notation) {
    if (!ChromaticKey_IsValid(key)) {
        key = mixxx::track::io::key::INVALID;
    }

    if (notation != KeyNotation::Lancelot && notation != KeyNotation::LancelotAndTraditional) {
        return s_sortKeysCircleOfFifths[static_cast<int>(key)];
    } else {
        return s_sortKeysCircleOfFifthsLancelot[static_cast<int>(key)];
    }
}

// static
QVariant KeyUtils::keyFromKeyTextAndIdFields(
        const QVariant& keyTextField, const QVariant& keyIdField) {
    // Helper function used by basetrackcache.cpp and basetracktablemodel.cpp
    // to determine the Key string from either the LIBRARYTABLE_KEY or the
    // LIBRARYTABLE_KEY_ID field.
    //
    // If we know the semantic key via the LIBRARYTABLE_KEY_ID
    // column (as opposed to the string representation of the key
    // currently stored in the DB) then lookup the key and render it
    // using the user's selected notation.
    if (keyIdField.isNull()) {
        // Otherwise, just use the KEY column value as is
        return keyTextField;
    }
    // Convert or clear invalid values
    VERIFY_OR_DEBUG_ASSERT(keyIdField.canConvert<int>()) {
        return keyTextField;
    }
    bool ok;
    const auto keyId = keyIdField.toInt(&ok);
    VERIFY_OR_DEBUG_ASSERT(ok) {
        return keyTextField;
    }
    const auto key = KeyUtils::keyFromNumericValue(keyId);
    if (key == mixxx::track::io::key::INVALID) {
        return keyTextField;
    }
    // Render the key with the user-provided notation
    return QVariant{KeyUtils::keyToString(key)};
}

// static
QString KeyUtils::keyFromKeyTextAndIdValues(const QString& keyText, const ChromaticKey& keyId) {
    return keyId == mixxx::track::io::key::INVALID ? keyText : KeyUtils::keyToString(keyId);
}
