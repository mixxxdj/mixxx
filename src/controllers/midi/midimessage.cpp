#include "controllers/midi/midiutils.h"

QDebug operator<<(QDebug debug, MidiOpCode midiOpCode) {
    debug << static_cast<uint8_t>(midiOpCode);
    return debug;
}

uint qHash(MidiOpCode key, uint seed) {
    return qHash(static_cast<uint8_t>(key), seed);
}

MidiKey::MidiKey()
        : status(0),
          control(0) {
}

MidiKey::MidiKey(unsigned char status, unsigned char control)
        : status(status),
          // When it's part of the message, include it. If the message is
          // not two bytes, signify that the second byte is part of the
          // payload with 0xFF.
          control(MidiUtils::isMessageTwoBytes(
              MidiUtils::opCodeFromStatus(status)) ? control : 0xFF) {
}
