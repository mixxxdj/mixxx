#pragma once

#include <QList>
#include <QPair>
#include <QMetaType>

#include "preferences/usersettings.h"

// The second value of each OpCode will be the channel number the message
// corresponds to.  So 0xB0 is a CC on the first channel, and 0xB1 is a CC
// on the second channel.  When working with incoming midi data, first call
// MidiUtils::opCodeFromStatus to translate from raw status values to opcodes,
// then compare to these enums.
typedef enum {
    MIDI_NOTE_OFF       = 0x80,
    MIDI_NOTE_ON        = 0x90,
    MIDI_AFTERTOUCH     = 0xA0,
    MIDI_CC             = 0xB0,
    MIDI_PROGRAM_CH     = 0xC0,
    MIDI_CH_AFTERTOUCH  = 0xD0,
    MIDI_PITCH_BEND     = 0xE0,
    MIDI_SYSEX          = 0xF0,
    MIDI_TIME_CODE      = 0xF1,
    MIDI_SONG_POS       = 0xF2,
    MIDI_SONG           = 0xF3,
    MIDI_UNDEFINED1     = 0xF4,
    MIDI_UNDEFINED2     = 0xF5,
    MIDI_TUNE_REQ       = 0xF6,
    MIDI_EOX            = 0xF7,
    MIDI_TIMING_CLK     = 0xF8,
    MIDI_UNDEFINED3     = 0xF9,
    MIDI_START          = 0xFA,
    MIDI_CONTINUE       = 0xFB,
    MIDI_STOP           = 0xFC,
    MIDI_UNDEFINED4     = 0xFD,
    MIDI_ACTIVE_SENSE   = 0xFE,
    MIDI_SYSTEM_RESET   = 0xFF,
} MidiOpCode;

typedef unsigned int    uint32_t;
typedef unsigned short  uint16_t;

typedef enum {
    MIDI_OPTION_NONE          = 0x0000,
    MIDI_OPTION_INVERT        = 0x0001,
    MIDI_OPTION_ROT64         = 0x0002,
    MIDI_OPTION_ROT64_INV     = 0x0004,
    MIDI_OPTION_ROT64_FAST    = 0x0008,
    MIDI_OPTION_DIFF          = 0x0010,
    MIDI_OPTION_BUTTON        = 0x0020,
    MIDI_OPTION_SWITCH        = 0x0040,
    MIDI_OPTION_SPREAD64      = 0x0080,
    MIDI_OPTION_HERC_JOG      = 0x0100,
    MIDI_OPTION_SELECTKNOB    = 0x0200,
    MIDI_OPTION_SOFT_TAKEOVER = 0x0400,
    MIDI_OPTION_SCRIPT        = 0x0800,
    MIDI_OPTION_14BIT_MSB     = 0x1000,
    MIDI_OPTION_14BIT_LSB     = 0x2000,
    // Should mask all bits used.
    MIDI_OPTION_MASK          = 0xFFFF,
} MidiOption;

struct MidiOptions {
    MidiOptions()
            : all(0) {
    }

    bool operator==(const MidiOptions& other) const {
        return all == other.all;
    }

    union
    {
        uint32_t    all;
        struct
        {
            bool invert        : 1;
            bool rot64         : 1;
            bool rot64_inv     : 1;
            bool rot64_fast    : 1;
            bool diff          : 1;
            bool button        : 1;    // Button Down (!=00) and Button Up (00) events happen together
            bool sw            : 1;    // button down (!=00) and button up (00) events happen separately
            bool spread64      : 1;    // accelerated difference from 64
            bool herc_jog      : 1;    // generic Hercules range correction 0x01 -> +1; 0x7f -> -1
            bool selectknob    : 1;    // relative knob which can be turned forever and outputs a signed value
            bool soft_takeover : 1;    // prevents sudden changes when hardware position differs from software value
            bool script        : 1;    // maps a MIDI control to a custom MixxxScript function
            // the message supplies the MSB of a 14-bit message
            bool fourteen_bit_msb : 1;
            // the message supplies the LSB of a 14-bit message
            bool fourteen_bit_lsb : 1;
            bool herc_jog_fast    : 1;  // generic Hercules range correction 0x01 -> +5; 0x7f -> -5
            // 19 more available for future expansion
        };
    };
};
Q_DECLARE_METATYPE(MidiOptions);

struct MidiOutput {
    MidiOutput()
            : message(0) {
        // MSVC gets confused and thinks min/max are macros so they can't appear
        // in the initializer list.
        min = 0.0;
        max = 0.0;
    }

    bool operator==(const MidiOutput& other) const {
        return min == other.min && max == other.max && message == other.message;
    }

    double min;
    double max;
    union
    {
        uint32_t    message;
        struct
        {
            unsigned char    status  : 8;
            unsigned char    control : 8;
            unsigned char    on      : 8;
            unsigned char    off     : 8;
        };
    };
};

struct MidiKey {
    MidiKey();
    MidiKey(unsigned char status, unsigned char control);

    bool operator==(const MidiKey& other) const {
        return key == other.key;
    }

    union
    {
        uint16_t    key;
        struct
        {
            unsigned char    status  : 8;
            unsigned char    control : 8;
        };
    };
};

struct MidiInputMapping {
    MidiInputMapping() {
    }

    MidiInputMapping(MidiKey key, MidiOptions options)
            : key(key),
              options(options) {
    }

    MidiInputMapping(MidiKey key, MidiOptions options, const ConfigKey& control)
            : key(key),
              options(options),
              control(control) {
    }

    MidiInputMapping(MidiKey key,
            MidiOptions options,
            const ConfigKey& control,
            const QString& description)
            : key(key),
              options(options),
              control(control),
              description(description) {
    }

    bool operator==(const MidiInputMapping& other) const {
        return key == other.key && options == other.options &&
                control == other.control && description == other.description;
    }

    MidiKey key;
    MidiOptions options;
    ConfigKey control;
    QString description;
};
typedef QList<MidiInputMapping> MidiInputMappings;

struct MidiOutputMapping {
    bool operator==(const MidiOutputMapping& other) const {
        return output == other.output && controlKey == other.controlKey &&
                description == other.description;
    }

    MidiOutput output;
    ConfigKey controlKey;
    QString description;
};
typedef QList<MidiOutputMapping> MidiOutputMappings;
