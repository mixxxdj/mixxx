#include "controllers/midi/midiopcode.h"

QDebug operator<<(QDebug debug, MidiOpCode midiOpCode) {
    debug << static_cast<uint8_t>(midiOpCode);
    return debug;
}

qhash_seed_t qHash(MidiOpCode key, qhash_seed_t seed) {
    return qHash(static_cast<uint8_t>(key), seed);
}
