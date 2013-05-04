#include <QtDebug>
#include <QMap>
#include <QRegExp>
#include <QMutexLocker>

#include "track/keyutils.h"
#include "mathstuff.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

// OpenKey notation, the numbers 1-12 followed by d (dur, major) or m (moll, minor).
const char* s_openKeyPattern = "(1[0-2]|[1-9])([dm])";

// Lancelot notation, the numbers 1-12 followed by a (minor) or b (major).
const char* s_lancelotKeyPattern = "(1[0-2]|[1-9])([ab])";

// a-g followed by any number of sharps or flats.
const char* s_keyPattern = "([a-g])([#b]*)";

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

// static
const char* KeyUtils::keyDebugName(ChromaticKey key) {
    static const char *keyNames[] = {"INVALID", "C","C#","D","D#","E","F","F#","G","G#","A","A#","B","c","c#","d","d#","e","f","f#","g","g#","a","a#","b"};
    if (!ChromaticKey_IsValid(key)) {
        return keyNames[0];
    }
    return keyNames[static_cast<int>(key)];
}

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

QString KeyUtils::keyToString(ChromaticKey key) {
    QMap<ChromaticKey, QString>::const_iterator it = s_notation.find(key);
    if (it != s_notation.end()) {
        return it.value();
    }
    return keyDebugName(key);
}

// static
ChromaticKey KeyUtils::guessKeyFromText(const QString& text) {
    QString trimmed = text.trimmed();

    QRegExp openKeyMatcher(s_openKeyPattern, Qt::CaseInsensitive);
    if (openKeyMatcher.exactMatch(trimmed)) {
        bool ok = false;
        int tonic = openKeyMatcher.cap(1).toInt(&ok);

        // Regex should mean this never happens.
        if (!ok || tonic < 1 || tonic > 12) {
            return mixxx::track::io::key::INVALID;
        }

        bool major = openKeyMatcher.cap(2)
                .compare("d", Qt::CaseInsensitive) == 0;

        return s_openKeyToKeys[tonic][major ? 0 : 1];
    }

    QRegExp lancelotKeyMatcher(s_lancelotKeyPattern, Qt::CaseInsensitive);
    if (lancelotKeyMatcher.exactMatch(trimmed)) {
        bool ok = false;
        int tonic = lancelotKeyMatcher.cap(1).toInt(&ok);

        // Regex should mean this never happens.
        if (!ok || tonic < 1 || tonic > 12) {
            return mixxx::track::io::key::INVALID;
        }

        // Lancelot notation is OpenKey notation rotated counter-clockwise by 5.
        int openKeyTonic = tonic + 5;
        if (openKeyTonic > 12) {
            openKeyTonic -= 12;
        }

        bool major = lancelotKeyMatcher.cap(2)
                .compare("b", Qt::CaseInsensitive) == 0;

        return s_openKeyToKeys[openKeyTonic][major ? 0 : 1];
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

        ChromaticKey letterKey = static_cast<ChromaticKey>(
            s_letterToMajorKey[letterIndex] + (major ? 0 : 12));

        // Now apply sharps and flats to the letter key.
        QString adjustments = keyMatcher.cap(2);
        int steps = 0;
        for (QString::const_iterator it = adjustments.begin();
             it != adjustments.end(); ++it) {
            steps += *it == '#' ? 1 : -1;
        }
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
    // value is minor.
    bool minor = key >= mixxx::track::io::key::C_MINOR;

    // Tonic, 0-indexed.
    int tonic = static_cast<int>(key) - (minor ? 13 : 1);

    // Add the key_changes, mod 12.
    tonic = (tonic + key_changes) % 12;

    // If tonic + key_changes ended up negative, the modulus could have ended up
    // negative (it's implementation specific). Add 12 until we are positive.
    while (tonic < 0) {
        tonic += 12;
    }

    // Return the key, adding 12 to the tonic if it was minor and 1 to make it
    // 1-indexed again.
    return static_cast<ChromaticKey>(tonic + (minor ? 13 : 1));
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
