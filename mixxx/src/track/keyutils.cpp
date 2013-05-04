#include <QtDebug>
#include <QMap>

#include "track/keyutils.h"
#include "mathstuff.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

// static
const char* KeyUtils::keyDebugName(ChromaticKey key) {
    static const char *keyNames[] = {"INVALID", "C","C#","D","D#","E","F","F#","G","G#","A","A#","B","c","c#","d","d#","e","f","f#","g","g#","a","a#","b"};
    if (!ChromaticKey_IsValid(key)) {
        return keyNames[0];
    }
    return keyNames[static_cast<int>(key)];
}

// static
ChromaticKey KeyUtils::guessKeyFromText(const QString& text) {
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
    // Invalid scales to invalid.
    if (!ChromaticKey_IsValid(key) ||
        key == mixxx::track::io::key::INVALID) {
        return mixxx::track::io::key::INVALID;
    }

    // We know the key is in the set of valid values. Save whether or not the
    // value is minor.
    bool minor = key >= mixxx::track::io::key::C_MINOR;

    // Tonic, 0-indexed.
    int tonic = static_cast<int>(key) - (minor ? 13 : 1);

    // Convert the octave_change from percentage of octave to the nearest
    // integer of key changes. We need the rounding to be in the same direction
    // so that a -1.0 and 1.0 scale of C makes it back to C.
    double key_changes_scaled = octave_change * 12;
    int key_changes = int(key_changes_scaled +
                          (key_changes_scaled > 0 ? 0.5 : -0.5));

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
