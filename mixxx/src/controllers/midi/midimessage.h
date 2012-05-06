#ifndef MIDIMESSAGE_H
#define MIDIMESSAGE_H

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

struct MidiOptions
{
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
            bool sw            : 1;    // button down (!=00) and button up (00) events happen seperately
            bool spread64      : 1;    // accelerated difference from 64
            bool herc_jog      : 1;    // generic Hercules wierd range correction
            bool selectknob    : 1;    // relative knob which can be turned forever and outputs a signed value
            bool soft_takeover : 1;    // prevents sudden changes when hardware position differs from software value
            bool script        : 1;    // maps a MIDI control to a custom MixxxScript function
            // 20 more available for future expansion
        };
    };
};

struct MidiOutput
{
    float       min;
    float       max;
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

struct MidiKey
{
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


#endif