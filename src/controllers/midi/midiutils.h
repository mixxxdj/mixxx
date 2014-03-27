#ifndef MIDIUTILS_H
#define MIDIUTILS_H

#include "controllers/midi/midimessage.h"

class MidiUtils {
  public:
    static QString opCodeToTranslatedString(MidiOpCode code);
    static QString formatByte(unsigned char value);
    static QString midiOptionToTranslatedString(MidiOption option);
};


#endif /* MIDIUTILS_H */
