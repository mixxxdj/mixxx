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
QString MidiUtils::formatByteAsHex(unsigned char value) {
    // Construct a hex string formatted like 0xFF.
    return QString("0x%1").arg(QString::number(value, 16)
                               .toUpper().rightJustified(2,'0'));
}

// static
QString MidiUtils::midiOptionToTranslatedString(MidiOption option) {
    switch (option) {
        case MIDI_OPTION_NONE:
            return QObject::tr("Normal");
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
        case MIDI_OPTION_14BIT_LSB:
            return QObject::tr("14-bit (LSB)");
        case MIDI_OPTION_14BIT_MSB:
            return QObject::tr("14-bit (MSB)");
        default:
            return QObject::tr("Unknown (0x%1)")
                    .arg(option, 4, 16, QLatin1Char('0'));
    }
}

// static
QString MidiUtils::formatMidiMessage(const QString& controllerName,
                                          unsigned char status, unsigned char control,
                                          unsigned char value, unsigned char channel,
                                          unsigned char opCode, mixxx::Duration timestamp) {
    QString msg2;
    if (timestamp == mixxx::Duration::fromMillis(0)) {
      msg2 = "outgoing:";
    } else {
      msg2 = QString("t:%1").arg(timestamp.formatMillisWithUnit());
    }
    switch (opCode) {
        case MIDI_PITCH_BEND:
            return QString("%1: %2 status 0x%3: pitch bend ch %4, value 0x%5")
                    .arg(controllerName, msg2,
                         QString::number(status, 16).toUpper(),
                         QString::number(channel+1, 10),
                         QString::number((value << 7) | control, 16).toUpper().rightJustified(4,'0'));
        case MIDI_SONG_POS:
            return QString("%1: %2 status 0x%3: song position 0x%4")
                    .arg(controllerName, msg2,
                         QString::number(status, 16).toUpper(),
                         QString::number((value << 7) | control, 16).toUpper().rightJustified(4,'0'));
        case MIDI_PROGRAM_CH:
        case MIDI_CH_AFTERTOUCH:
            return QString("%1: %2 status 0x%3 (ch %4, opcode 0x%5), value 0x%6")
                    .arg(controllerName, msg2,
                         QString::number(status, 16).toUpper(),
                         QString::number(channel+1, 10),
                         QString::number((status & 255)>>4, 16).toUpper(),
                         QString::number(control, 16).toUpper().rightJustified(2,'0'));
        case MIDI_SONG:
            return QString("%1: %2 status 0x%3: select song #%4")
                    .arg(controllerName, msg2,
                         QString::number(status, 16).toUpper(),
                         QString::number(control+1, 10));
        case MIDI_NOTE_OFF:
        case MIDI_NOTE_ON:
        case MIDI_AFTERTOUCH:
        case MIDI_CC:
            return QString("%1: %2 status 0x%3 (ch %4, opcode 0x%5), ctrl 0x%6, val 0x%7")
                    .arg(controllerName, msg2,
                         QString::number(status, 16).toUpper(),
                         QString::number(channel+1, 10),
                         QString::number((status & 255)>>4, 16).toUpper(),
                         QString::number(control, 16).toUpper().rightJustified(2,'0'),
                         QString::number(value, 16).toUpper().rightJustified(2,'0'));
        default:
            return QString("%1: %2 status 0x%3")
                    .arg(controllerName, msg2,
                         QString::number(status, 16).toUpper());
    }
}

// static
QString MidiUtils::formatSysexMessage(const QString& controllerName, const QByteArray& data,
                                           mixxx::Duration timestamp) {
    QString msg2;
    if (timestamp == mixxx::Duration::fromMillis(0)) {
      msg2 = "outgoing:";
    } else {
      msg2 = QString("t:%1").arg(timestamp.formatMillisWithUnit());
    }
    QString message =
            QString("%1: %2 %3 byte sysex: [")
                    .arg(controllerName, msg2, QString::number(data.size()));
    for (int i = 0; i < data.size(); ++i) {
        message += QString("%1%2").arg(
            QString("%1").arg((unsigned char)(data.at(i)), 2, 16, QChar('0')).toUpper(),
            QString("%1").arg((i < (data.size()-1)) ? ' ' : ']'));
    }
    return message;
}
