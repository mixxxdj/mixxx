#include "controllers/midi/midiutils.h"

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
