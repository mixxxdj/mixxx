#include "controllers/midi/midiutils.h"

// static
QString MidiUtils::opCodeToTranslatedString(MidiOpCode code) {
    switch (code) {
    case MidiOpCode::NoteOn:
        return QObject::tr("Note On");
    case MidiOpCode::NoteOff:
        return QObject::tr("Note Off");
    case MidiOpCode::ControlChange:
        return QObject::tr("CC");
    case MidiOpCode::PitchBendChange:
        return QObject::tr("Pitch Bend");
    default:
        return QObject::tr("Unknown (0x%1)")
                .arg(opCodeValue(code), 2, 16, QLatin1Char('0'));
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
    case MidiOption::None:
        return QObject::tr("Normal");
    case MidiOption::Invert:
        return QObject::tr("Invert");
    case MidiOption::Rot64:
        return QObject::tr("Rot64");
    case MidiOption::Rot64Invert:
        return QObject::tr("Rot64Inv");
    case MidiOption::Rot64Fast:
        return QObject::tr("Rot64Fast");
    case MidiOption::Diff:
        return QObject::tr("Diff");
    case MidiOption::Button:
        return QObject::tr("Button");
    case MidiOption::Switch:
        return QObject::tr("Switch");
    case MidiOption::Spread64:
        return QObject::tr("Spread64");
    case MidiOption::HercJog:
        return QObject::tr("HercJog");
    case MidiOption::SelectKnob:
        return QObject::tr("SelectKnob");
    case MidiOption::SoftTakeover:
        return QObject::tr("SoftTakeover");
    case MidiOption::Script:
        return QObject::tr("Script");
    case MidiOption::FourteenBitLSB:
        return QObject::tr("14-bit (LSB)");
    case MidiOption::FourteenBitMSB:
        return QObject::tr("14-bit (MSB)");
    default:
        return QObject::tr("Unknown (0x%1)")
                .arg(static_cast<uint16_t>(option), 4, 16, QLatin1Char('0'));
    }
}

// static
QString MidiUtils::formatMidiOpCode(const QString& controllerName,
        unsigned char status,
        unsigned char control,
        unsigned char value,
        unsigned char channel,
        MidiOpCode opCode,
        mixxx::Duration timestamp) {
    QString msg2;
    if (timestamp == mixxx::Duration::fromMillis(0)) {
      msg2 = "outgoing:";
    } else {
      msg2 = QString("t:%1").arg(timestamp.formatMillisWithUnit());
    }
    switch (opCode) {
    case MidiOpCode::PitchBendChange:
        return QString("%1: %2 status 0x%3: pitch bend ch %4, value 0x%5")
                .arg(controllerName,
                        msg2,
                        QString::number(status, 16).toUpper(),
                        QString::number(channel + 1, 10),
                        QString::number((value << 7) | control, 16)
                                .toUpper()
                                .rightJustified(4, '0'));
    case MidiOpCode::SongPosition:
        return QString("%1: %2 status 0x%3: song position 0x%4")
                .arg(controllerName,
                        msg2,
                        QString::number(status, 16).toUpper(),
                        QString::number((value << 7) | control, 16)
                                .toUpper()
                                .rightJustified(4, '0'));
    case MidiOpCode::ProgramChange:
    case MidiOpCode::ChannelPressure:
        return QString("%1: %2 status 0x%3 (ch %4, opcode 0x%5), value 0x%6")
                .arg(controllerName,
                        msg2,
                        QString::number(status, 16).toUpper(),
                        QString::number(channel + 1, 10),
                        QString::number((status & 255) >> 4, 16).toUpper(),
                        QString::number(control, 16)
                                .toUpper()
                                .rightJustified(2, '0'));
    case MidiOpCode::SongSelect:
        return QString("%1: %2 status 0x%3: select song #%4")
                .arg(controllerName,
                        msg2,
                        QString::number(status, 16).toUpper(),
                        QString::number(control + 1, 10));
    case MidiOpCode::NoteOff:
    case MidiOpCode::NoteOn:
    case MidiOpCode::PolyphonicKeyPressure:
    case MidiOpCode::ControlChange:
        return QString(
                "%1: %2 status 0x%3 (ch %4, opcode 0x%5), ctrl 0x%6, val 0x%7")
                .arg(controllerName,
                        msg2,
                        QString::number(status, 16).toUpper(),
                        QString::number(channel + 1, 10),
                        QString::number((status & 255) >> 4, 16).toUpper(),
                        QString::number(control, 16)
                                .toUpper()
                                .rightJustified(2, '0'),
                        QString::number(value, 16).toUpper().rightJustified(
                                2, '0'));
    default:
        return QString("%1: %2 status 0x%3")
                .arg(controllerName, msg2, QString::number(status, 16).toUpper());
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
