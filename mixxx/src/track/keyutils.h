#ifndef KEYUTILS_H
#define KEYUTILS_H

#include <QString>

#include "proto/keys.pb.h"
#include "track/keys.h"

class KeyUtils {
  public:
    static const char* keyDebugName(mixxx::track::io::key::ChromaticKey key);

    static mixxx::track::io::key::ChromaticKey keyFromNumericValue(double value);

    static double keyToNumericValue(mixxx::track::io::key::ChromaticKey key);

    static mixxx::track::io::key::ChromaticKey scaleKeyOctaves(
        mixxx::track::io::key::ChromaticKey key, double scale);

    static mixxx::track::io::key::ChromaticKey scaleKeySteps(
        mixxx::track::io::key::ChromaticKey key, int steps);

    static mixxx::track::io::key::ChromaticKey guessKeyFromText(const QString& text);

    static mixxx::track::io::key::ChromaticKey calculateGlobalKey(
        const KeyChangeList& key_changes, int iTotalSamples);
};

#endif /* KEYUTILS_H */
