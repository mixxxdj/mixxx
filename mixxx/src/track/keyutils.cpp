#include <QtDebug>
#include <QMap>

#include "track/keyutils.h"

const char* keyDebugName(mixxx::track::io::key::ChromaticKey key) {
    static const char *keyNames[] = {"INVALID", "C","C#","D","D#","E","F","F#","G","G#","A","A#","B","c","c#","d","d#","e","f","f#","g","g#","a","a#","b"};
    return keyNames[static_cast<int>(key)];
}

mixxx::track::io::key::ChromaticKey guessKeyFromText(const QString& text) {
    return mixxx::track::io::key::INVALID;
}

mixxx::track::io::key::ChromaticKey calculateGlobalKey(
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
