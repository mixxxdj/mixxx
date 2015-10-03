#include <QtDebug>
#include <QMap>
#include <QMutexLocker>
#include <QPair>
#include <QRegExp>

#include "track/keyutils.h"
#include "util/math.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

// OpenKey notation, the numbers 1-12 followed by d (dur, major) or m (moll, minor).
static const char* s_openKeyPattern = "^\\s*(1[0-2]|[1-9])([dm])\\s*$";

// Lancelot notation, the numbers 1-12 followed by a (minor) or b (major).
static const char* s_lancelotKeyPattern = "^\\s*(1[0-2]|[1-9])([ab])\\s*$";

// a-g followed by any number of sharps or flats, optionally followed by
// a scale spec (m = minor, min, maj)
// anchor the pattern so we don't get accidental sub-string matches
// (?:or)? allows unabbreviated major|minor without capturing
static const char* s_keyPattern = "^\\s*([a-g])([#♯b♭]*)"
    "(min(?:or)?|maj(?:or)?|m)?\\s*$";

static const QString s_sharpSymbol = QString::fromUtf8("♯");
static const QString s_flatSymbol = QString::fromUtf8("♭");

static const char *s_traditionalKeyNames[] = {
    "INVALID",
    "C", "D♭", "D", "E♭", "E", "F", "F♯/G♭", "G", "A♭", "A", "B♭", "B",
    "Cm", "C♯m", "Dm", "D♯m/E♭m", "Em", "Fm", "F♯m", "Gm", "G♯m", "Am", "B♭m", "Bm"
};

// Maps an OpenKey number to its major and minor key.
const ChromaticKey s_openKeyToKeys[][2] = {
    // 0 is not a valid OpenKey number.
    { mixxx::track::io::key::INVALID,       mixxx::track::io::key::INVALID },
    { mixxx::track::io::key::C_MAJOR,       mixxx::track::io::key::A_MINOR }, // 1
    { mixxx::track::io::key::G_MAJOR,       mixxx::track::io::key::E_MINOR }, // 2
    { mixxx::track::io::key::D_MAJOR,       mixxx::track::io::key::B_MINOR }, // 3
    { mixxx::track::io::key::A_MAJOR,       mixxx::track::io::key::F_SHARP_MINOR }, // 4
    { mixxx::track::io::key::E_MAJOR,       mixxx::track::io::key::C_SHARP_MINOR }, // 5
    { mixxx::track::io::key::B_MAJOR,       mixxx::track::io::key::G_SHARP_MINOR }, // 6
    { mixxx::track::io::key::F_SHARP_MAJOR, mixxx::track::io::key::E_FLAT_MINOR }, // 7
    { mixxx::track::io::key::D_FLAT_MAJOR,  mixxx::track::io::key::B_FLAT_MINOR }, // 8
    { mixxx::track::io::key::A_FLAT_MAJOR,  mixxx::track::io::key::F_MINOR }, // 9
    { mixxx::track::io::key::E_FLAT_MAJOR,  mixxx::track::io::key::C_MINOR }, // 10
    { mixxx::track::io::key::B_FLAT_MAJOR,  mixxx::track::io::key::G_MINOR }, // 11
    { mixxx::track::io::key::F_MAJOR,       mixxx::track::io::key::D_MINOR } // 12
};

// This is a quick hack to convert an ASCII letter into a key. Lookup the key
// using (letter.toLower() - 'a') as the index. Make sure the letter matches
// [a-gA-G] first.
const ChromaticKey s_letterToMajorKey[] = {
    mixxx::track::io::key::A_MAJOR,
    mixxx::track::io::key::B_MAJOR,
    mixxx::track::io::key::C_MAJOR,
    mixxx::track::io::key::D_MAJOR,
    mixxx::track::io::key::E_MAJOR,
    mixxx::track::io::key::F_MAJOR,
    mixxx::track::io::key::G_MAJOR
};

QMutex KeyUtils::s_notationMutex;
QMap<ChromaticKey, QString> KeyUtils::s_notation;
QMap<QString, ChromaticKey> KeyUtils::s_reverseNotation;

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

// static
ChromaticKey KeyUtils::openKeyNumberToKey(int openKeyNumber, bool major) {
        return s_openKeyToKeys[openKeyNumber][major ? 0 : 1];
}

// static
QString KeyUtils::keyDebugName(ChromaticKey key) {
    if (!ChromaticKey_IsValid(key)) {
        key = mixxx::track::io::key::INVALID;
    }
    return QString::fromUtf8(s_traditionalKeyNames[static_cast<int>(key)]);
}

// static
void KeyUtils::setNotation(const QMap<ChromaticKey, QString>& notation) {
    QMutexLocker locker(&s_notationMutex);
    s_notation = notation;
    s_reverseNotation.clear();

    for (QMap<ChromaticKey, QString>::const_iterator it = s_notation.begin();
         it != s_notation.end(); ++it) {
        if (s_reverseNotation.contains(it.value())) {
            qWarning() << "Key notation is surjective (has duplicate values).";
        }
        s_reverseNotation.insert(it.value(), it.key());
    }
}

// static
QString KeyUtils::keyToString(ChromaticKey key,
                              KeyNotation notation) {
    if (!ChromaticKey_IsValid(key) ||
        key == mixxx::track::io::key::INVALID) {
        // TODO(rryan): Maybe just the empty string?
        return "INVALID";
    }

    if (notation == DEFAULT) {
        QMutexLocker locker(&s_notationMutex);
        QMap<ChromaticKey, QString>::const_iterator it = s_notation.find(key);
        if (it != s_notation.end()) {
            return it.value();
        }
    } else if (notation == OPEN_KEY) {
        bool major = keyIsMajor(key);
        int number = keyToOpenKeyNumber(key);
        return QString::number(number) + (major ? "d" : "m");
    } else if (notation == LANCELOT) {
        bool major = keyIsMajor(key);
        int number = openKeyNumberToLancelotNumber(keyToOpenKeyNumber(key));
        return QString::number(number) + (major ? "B" : "A");
    } else if (notation == TRADITIONAL) {
        return QString::fromUtf8(s_traditionalKeyNames[static_cast<int>(key)]);
    }
    return keyDebugName(key);
}

// static
QString KeyUtils::getGlobalKeyText(const Keys& keys, KeyNotation notation) {
    const mixxx::track::io::key::ChromaticKey globalKey(keys.getGlobalKey());
    if (globalKey != mixxx::track::io::key::INVALID) {
        return keyToString(globalKey, notation);
    } else {
        // Fall back on text global name
        return keys.getGlobalKeyText();
    }
}

// static
ChromaticKey KeyUtils::guessKeyFromText(const QString& text) {
    QString trimmed = text.trimmed();

    // Try using the user's custom notation.
    {
        QMutexLocker locker(&s_notationMutex);
        QMap<QString, ChromaticKey>::const_iterator it = s_reverseNotation.find(text);
        if (it != s_reverseNotation.end()) {
            return it.value();
        }
    }

    QRegExp openKeyMatcher(s_openKeyPattern, Qt::CaseInsensitive);
    if (openKeyMatcher.exactMatch(trimmed)) {
        bool ok = false;
        int openKeyNumber = openKeyMatcher.cap(1).toInt(&ok);

        // Regex should mean this never happens.
        if (!ok || openKeyNumber < 1 || openKeyNumber > 12) {
            return mixxx::track::io::key::INVALID;
        }

        bool major = openKeyMatcher.cap(2)
                .compare("d", Qt::CaseInsensitive) == 0;

        return openKeyNumberToKey(openKeyNumber, major);
    }

    QRegExp lancelotKeyMatcher(s_lancelotKeyPattern, Qt::CaseInsensitive);
    if (lancelotKeyMatcher.exactMatch(trimmed)) {
        bool ok = false;
        int lancelotNumber = lancelotKeyMatcher.cap(1).toInt(&ok);

        // Regex should mean this never happens.
        if (!ok || lancelotNumber < 1 || lancelotNumber > 12) {
            return mixxx::track::io::key::INVALID;
        }

        int openKeyNumber = lancelotNumberToOpenKeyNumber(lancelotNumber);

        bool major = lancelotKeyMatcher.cap(2)
                .compare("b", Qt::CaseInsensitive) == 0;

        return openKeyNumberToKey(openKeyNumber, major);
    }

    QRegExp keyMatcher(s_keyPattern, Qt::CaseInsensitive);
    if (keyMatcher.exactMatch(trimmed)) {
        // Take the first letter, lowercase it and subtract 'a' and we get a
        // number between 0-6. Look up the major key associated with that letter
        // from s_letterToMajorKey. Upper-case means major, lower-case means
        // minor. Then apply the sharps or flats to the key.
        QChar letter = keyMatcher.cap(1).at(0);
        int letterIndex = letter.toLower().toAscii() - 'a';
        bool major = letter.isUpper();

        // Now apply sharps and flats to the letter key.
        QString adjustments = keyMatcher.cap(2);
        int steps = 0;
        for (QString::const_iterator it = adjustments.begin();
             it != adjustments.end(); ++it) {
            steps += (*it == '#' || *it == s_sharpSymbol[0]) ? 1 : -1;
        }

        QString scale = keyMatcher.cap(3);
        // we override major if a scale definition exists
        if (! scale.isEmpty()) {
            if (scale.compare("m", Qt::CaseInsensitive) == 0) {
                major = false;
            } else if (scale.startsWith("min", Qt::CaseInsensitive)) {
                major = false;
            } else if (scale.startsWith("maj", Qt::CaseInsensitive)) {
                major = true;
            } else {
                qDebug() << "WARNING: scale from regexp has unexpected value."
                  " should never happen";
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
    int value_floored = int(value);

    if (!ChromaticKey_IsValid(value_floored)) {
        return mixxx::track::io::key::INVALID;
    }

    return static_cast<ChromaticKey>(value_floored);
}

// static
double KeyUtils::keyToNumericValue(ChromaticKey key) {
    return key;
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
    const KeyChangeList& key_changes, const int iTotalSamples) {
    const int iTotalFrames = iTotalSamples / 2;
    QMap<mixxx::track::io::key::ChromaticKey, double> key_histogram;

    for (int i = 0; i < key_changes.size(); ++i) {
        mixxx::track::io::key::ChromaticKey key = key_changes[i].first;
        const double start_frame = key_changes[i].second;
        const double next_frame = (i == key_changes.size() - 1) ?
                iTotalFrames : key_changes[i+1].second;
        key_histogram[key] += (next_frame - start_frame);
    }


    double max_delta = 0;
    mixxx::track::io::key::ChromaticKey max_key = mixxx::track::io::key::INVALID;
    qDebug() << "Key Histogram";
    for (QMap<mixxx::track::io::key::ChromaticKey, double>::const_iterator it = key_histogram.begin();
         it != key_histogram.end(); ++it) {
        qDebug() << it.key() << ":" << keyDebugName(it.key()) << it.value();
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

    // If we have a mode missmatch, matching to a the Relative mode
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

    // We know the key is in the set of valid values. Save whether or not the
    // value is minor.
    bool major = keyIsMajor(key);
    int openKeyNumber = KeyUtils::keyToOpenKeyNumber(key);

    // The compatible keys of particular key are:
    // * The relative major/minor key.
    // * The perfect 4th (sub-dominant) key.
    // * The perfect 5th (dominant) key.
    //
    // The Circle of Fifths is a handy tool that encodes this compatibility.
    // Keys on the same radial of the circle are compatible and adjacent keys on
    // the circle are compatible. OpenKey notation encodes the Circle of
    // Fifths. The OpenKey number represents the radial on the circle and
    // major/minor determines whether you are on the inner or outer ring of the
    // radial.

    // The key is compatible with tracks in the same key.
    compatible << key;

    // The relative major/minor key is compatible.
    compatible << openKeyNumberToKey(openKeyNumber, !major);

    // The perfect 4th and perfect 5th are compatible.
    compatible << openKeyNumberToKey(
            openKeyNumber == 12 ? 1 : openKeyNumber + 1, major);
    compatible << openKeyNumberToKey(
            openKeyNumber == 1 ? 12 : openKeyNumber - 1, major);
    return compatible;
}
