#include <QtDebug>
#include <QMap>
#include <QRegExp>
#include <QMutexLocker>

#include "track/keyutils.h"
#include "mathstuff.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

// OpenKey notation, the numbers 1-12 followed by d (dur, major) or m (moll, minor).
static const char* s_openKeyPattern = "(1[0-2]|[1-9])([dm])";

// Lancelot notation, the numbers 1-12 followed by a (minor) or b (major).
static const char* s_lancelotKeyPattern = "(1[0-2]|[1-9])([ab])";

// a-g followed by any number of sharps or flats.
static const char* s_keyPattern = "([a-g])([#b]*m?)";

static const char *s_traditionalKeyNames[] = {
    "INVALID",
    "C", "Db", "D", "Eb", "E", "F", "F#/Gb", "G", "Ab", "A", "Bb", "B",
    "Cm", "C#m", "Dm", "D#m/Ebm", "Em", "Fm", "F#m", "Gm", "G#m", "Am", "Bbm", "Bm"
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
const char* KeyUtils::keyDebugName(ChromaticKey key) {
    if (!ChromaticKey_IsValid(key)) {
        return s_traditionalKeyNames[0];
    }
    return s_traditionalKeyNames[static_cast<int>(key)];
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
        return s_traditionalKeyNames[static_cast<int>(key)];
    }
    return keyDebugName(key);
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
            // An m only comes at the end and
            if (it->toLower() == 'm') {
                major = false;
                break;
            }
            steps += *it == '#' ? 1 : -1;
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
ChromaticKey KeyUtils::scaleKeyOctaves(ChromaticKey key, double octave_change) {
    // Convert the octave_change from percentage of octave to the nearest
    // integer of key changes. We need the rounding to be in the same direction
    // so that a -1.0 and 1.0 scale of C makes it back to C.
    double key_changes_scaled = octave_change * 12;
    int key_changes = int(key_changes_scaled +
                          (key_changes_scaled > 0 ? 0.5 : -0.5));

    return scaleKeySteps(key, key_changes);
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

// //static
// ChromaticKey KeyUtils::keyToRelativeMajorOrMinor(ChromaticKey key) {
//     bool major = keyIsMajor(key);
//     int tonic = keyToOpenKeyNumber(key);
// 
//     // if the key was major, return relative minor and vice versa
//     return s_openKeyToKeys[tonic][major ? 1 : 0];
// }


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

    int upSteps = targetTonic - tonic + 12;
    if (abs(upSteps) < abs(steps)) {
        steps = upSteps;
    }

    int downSteps = targetTonic - tonic - 12;
    if (abs(downSteps) < abs(steps)) {
        steps = downSteps;
    }

    return steps;
}

// static
int KeyUtils::shortestStepsToCompatibleKey(
    mixxx::track::io::key::ChromaticKey key,
    mixxx::track::io::key::ChromaticKey target_key) {
    // For invalid keys just return zero steps.
    if (!ChromaticKey_IsValid(key) ||
        key == mixxx::track::io::key::INVALID ||
        !ChromaticKey_IsValid(target_key) ||
        target_key == mixxx::track::io::key::INVALID) {
        return 0;
    }

    // We know the key is in the set of valid values. Save whether or not the
    // value is minor.
    bool major = keyIsMajor(key);
    bool targetMajor = keyIsMajor(target_key);

    // If both keys are major/minor, then we just want to take the shortest
    // number of steps to match the key.
    if (major == targetMajor) {
        return shortestStepsToKey(key, target_key);
    }

    int targetOpenKeyNumber = KeyUtils::keyToOpenKeyNumber(target_key);

    // Get the key that matches target_key on the Circle of Fifths but is the
    // same major/minor as key.
    target_key = openKeyNumberToKey(targetOpenKeyNumber, major);

    return shortestStepsToKey(key, target_key);
}
