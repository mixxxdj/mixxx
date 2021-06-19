#pragma once

#include <QFlags>
#include <QList>
#include <QMetaType>
#include <QPair>
#include <QtDebug>
#include <cstdint>

#include "preferences/usersettings.h"

// The second value of each OpCode will be the channel number the message
// corresponds to.  So 0xB0 is a CC on the first channel, and 0xB1 is a CC
// on the second channel.  When working with incoming midi data, first call
// MidiUtils::opCodeFromStatus to translate from raw status values to opcodes,
// then compare to these enums.
enum class MidiOpCode : uint8_t {
    /// Note Off event.
    /// This message is sent when a note is released (ended).
    NoteOff = 0x80,
    /// Note On event.
    /// This message is sent when a note is depressed (start).
    NoteOn = 0x90,
    /// Polyphonic Key Pressure (Aftertouch). This message is most often sent
    /// by pressing down on the key after it "bottoms out".
    PolyphonicKeyPressure = 0xA0,
    /// Control Change. This message is sent when a controller value changes.
    /// Controllers include devices such as pedals and levers. Controller
    /// numbers 120-127 are reserved as "Channel Mode Messages" (below).
    ControlChange = 0xB0,
    /// Program Change. This message sent when the patch number changes.
    ProgramChange = 0xC0,
    /// Channel Pressure (After-touch). This message is most often sent by
    /// pressing down on the key after it "bottoms out". This message is
    /// different from polyphonic after-touch. Use this message to send the
    /// single greatest pressure value (of all the current depressed keys).
    ChannelPressure = 0xD0,
    /// Pitch Bend Change. This message is sent to indicate a change in the
    /// pitch bender (wheel or lever, typically). The pitch bender is measured
    /// by a fourteen bit value. Center (no pitch change) is 2000H. Sensitivity
    /// is a function of the receiver, but may be set using RPN 0.
    PitchBendChange = 0xE0,
    /// System Exclusive. This message type allows manufacturers to create
    /// their own messages (such as bulk dumps, patch parameters, and other
    /// non-spec data) and provides a mechanism for creating additional MIDI
    /// Specification messages. The Manufacturer's ID code (assigned by MMA or
    /// AMEI) is either 1 byte (0iiiiiii) or 3 bytes (0iiiiiii 0iiiiiii
    /// 0iiiiiii). Two of the 1 Byte IDs are reserved for extensions called
    /// Universal Exclusive Messages, which are not manufacturer-specific. If a
    /// device recognizes the ID code as its own (or as a supported Universal
    /// message) it will listen to the rest of the message (0ddddddd).
    /// Otherwise, the message will be ignored. (Note: Only Real-Time messages
    /// may be interleaved with a System Exclusive.)
    SystemExclusive = 0xF0,
    /// MIDI Time Code Quarter Frame.
    TimeCodeQuarterFrame = 0xF1,
    /// Song Position Pointer.
    /// This is an internal 14 bit register that holds the number of MIDI beats
    //  (1 beat = six MIDI clocks) since the start of the song.
    SongPosition = 0xF2,
    /// Song Select. The Song Select specifies which sequence or song is to be
    /// played.
    SongSelect = 0xF3,
    /// Undefined. (Reserved)
    Undefined1 = 0xF4,
    /// Undefined. (Reserved)
    Undefined2 = 0xF5,
    /// Tune Request. Upon receiving a Tune Request, all analog synthesizers
    /// should tune their oscillators.
    TuneRequest = 0xF6,
    /// End of Exclusive. Used to terminate a System Exclusive dump (see
    /// above).
    EndOfExclusive = 0xF7,
    /// Timing Clock. Sent 24 times per quarter note when synchronization
    /// is required.
    TimingClock = 0xF8,
    /// Undefined. (Reserved)
    Undefined3 = 0xF9,
    /// Start. Start the current sequence playing.
    /// (This message will be followed with Timing Clocks).
    Start = 0xFA,
    /// Continue. Continue at the point the sequence was stopped
    Continue = 0xFB,
    /// Stop. Stop the current sequence.
    Stop = 0xFC,
    /// Undefined. (Reserved)
    Undefined4 = 0xFD,
    /// Active Sensing. This message is intended to be sent repeatedly to tell the receiver that a
    /// connection is alive. Use of this message is optional. When initially received, the receiver
    /// will expect to receive another Active Sensing message each 300ms (max), and if it does not
    /// then it will assume that the connection has been terminated. At termination, the receiver
    /// will turn off all voices and return to normal (non- active sensing) operation.
    ActiveSensing = 0xFE,
    /// Reset. Reset all receivers in the system to power-up status. This should be used sparingly,
    /// preferably under manual control. In particular, it should not be sent on power-up.
    SystemReset = 0xFF,
};
QDebug operator<<(QDebug debug, MidiOpCode midiOpCode);
uint qHash(MidiOpCode key, uint seed);

enum class MidiOption : uint16_t {
    None = 0x0000,
    Invert = 0x0001,
    Rot64 = 0x0002,
    Rot64Invert = 0x0004,
    Rot64Fast = 0x0008,
    Diff = 0x0010,
    /// Button Down (!=00) and Button Up (00) events happen together
    Button = 0x0020,
    /// Button Down (!=00) and Button Up (00) events happen separately
    Switch = 0x0040,
    /// Accelerated Difference from 64
    Spread64 = 0x0080,
    /// Generic Hercules Range Correction (0x01 -> +1; 0x7f -> -1)
    HercJog = 0x0100,
    /// Relative Knob which can be turned forever and outputs a signed value
    SelectKnob = 0x0200,
    /// Prevents Sudden Changes when hardware position differs from software value
    SoftTakeover = 0x0400,
    /// Maps a MIDI control to a custom JavaScript function
    Script = 0x0800,
    /// Nessage supplies the LSB of a 14-bit message
    FourteenBitLSB = 0x1000,
    /// Nessage supplies the MSB of a 14-bit message
    FourteenBitMSB = 0x2000,
    /// Generic Hercules Range Correction (0x01 -> +5; 0x7f -> -5)
    HercJogFast = 0x4000,
};
Q_DECLARE_FLAGS(MidiOptions, MidiOption);
Q_DECLARE_OPERATORS_FOR_FLAGS(MidiOptions);
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
