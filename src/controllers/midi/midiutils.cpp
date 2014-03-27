#include "controllers/midi/midiutils.h"

// static
QString MidiUtils::opCodeToTranslatedString(MidiOpCode code) {
    if (code < 0x00 || code > 0xFF) {
        return QObject::tr("Invalid");
    }
    switch (code) {
        case MIDI_NOTE_ON:
            return QObject::tr("Note On");
        case MIDI_NOTE_OFF:
            return QObject::tr("Note Off");
        case MIDI_CC:
            return QObject::tr("CC");
        case MIDI_PITCH_BEND:
            return QObject::tr("Pitch Bend");
        default:
            return QObject::tr("Unknown (0x%1)").arg(
                code, 2, 16, QLatin1Char('0'));
    }
}

// static
QString MidiUtils::formatByte(unsigned char value) {
    // Construct a hex string formatted like 0xFF.
    return QString("0x") + QString("%1")
            .arg(value, 2, 16, QLatin1Char('0')).toUpper();
}

// static
QString MidiUtils::midiOptionToTranslatedString(MidiOption option) {
    switch (option) {
        case MIDI_OPTION_INVERT:
            return QObject::tr("Invert");
        case MIDI_OPTION_ROT64:
            return QObject::tr("Rot64");
        case MIDI_OPTION_ROT64_INV:
            return QObject::tr("Rot64Inv");
        case MIDI_OPTION_ROT64_FAST:
            return QObject::tr("Rot64Fast");
        case MIDI_OPTION_DIFF:
            return QObject::tr("Diff");
        case MIDI_OPTION_BUTTON:
            return QObject::tr("Button");
        case MIDI_OPTION_SWITCH:
            return QObject::tr("Switch");
        case MIDI_OPTION_SPREAD64:
            return QObject::tr("Spread64");
        case MIDI_OPTION_HERC_JOG:
            return QObject::tr("HercJog");
        case MIDI_OPTION_SELECTKNOB:
            return QObject::tr("SelectKnob");
        case MIDI_OPTION_SOFT_TAKEOVER:
            return QObject::tr("SoftTakeover");
        case MIDI_OPTION_SCRIPT:
            return QObject::tr("Script");
        default:
            return QObject::tr("Unknown (0x%1)")
                    .arg(option, 4, 16, QLatin1Char('0'));
    }
}
