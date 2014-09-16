#ifndef MIDIUTILS_H
#define MIDIUTILS_H

#include "controllers/midi/midimessage.h"

class MidiUtils {
  public:
    static inline unsigned char channelFromStatus(unsigned char status) {
        return status & 0x0F;
    }

    static inline MidiOpCode opCodeFromStatus(unsigned char status) {
        unsigned char opCode = status & 0xF0;
        // MIDI_SYSEX and higher don't have a channel and occupy the entire byte.
        if (opCode == 0xF0) {
            opCode = status;
        }
        return static_cast<MidiOpCode>(opCode);
    }

    static inline bool isMessageTwoBytes(unsigned char opCode) {
        switch (opCode) {
            case MIDI_SONG:
            case MIDI_NOTE_OFF:
            case MIDI_NOTE_ON:
            case MIDI_AFTERTOUCH:
            case MIDI_CC:
                return true;
            default:
                return false;
        }
    }

    static inline bool isClockSignal(const MidiKey& mappingKey) {
        return (mappingKey.key & MIDI_TIMING_CLK) == MIDI_TIMING_CLK;
    }

    static QString opCodeToTranslatedString(MidiOpCode code);
    static QString formatByteAsHex(unsigned char value);
    static QString midiOptionToTranslatedString(MidiOption option);
};


#endif /* MIDIUTILS_H */
