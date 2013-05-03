#ifndef KEYUTILS_H
#define KEYUTILS_H

#include <QString>

#include "proto/keys.pb.h"
#include "track/keys.h"

const char* keyDebugName(mixxx::track::io::key::ChromaticKey key);

mixxx::track::io::key::ChromaticKey guessKeyFromText(const QString& text);

mixxx::track::io::key::ChromaticKey calculateGlobalKey(
    const KeyChangeList& key_changes, int iTotalSamples);

#endif /* KEYUTILS_H */
