#include "track/serato/markers.h"

#include <QtEndian>

#include "track/serato/tags.h"

namespace {

const int kNumEntries = 14;
const int kLoopEntryStartIndex = 5;
const int kEntrySize = 22;
const quint16 kVersion = 0x0205;

// These functions conversion between the 4-byte "Serato Markers_" color format
// and RgbColor (3-Byte RGB, transparency disabled).
//
// Serato's custom color format that is used here also represents RGB colors,
// but inserts a single null bit after every 7 payload bits, starting from the
// rightmost bit.
//
// Here's an example:
//
//                   | Hex          Binary
//     ------------- | -----------  --------------------------------
//     3-byte RGB    |    00 00 cc       000 0000000 0000001 1001100
//     Serato format | 00 00 01 4c  00000000000000000000000101001100
//                   |
//     3-byte RGB    |    cc 88 00       110 0110010 0010000 0000000
//     Serato format | 06 32 10 00  00000110001100100001000000000000
//
// See this for details:
// https://github.com/Holzhaus/serato-tags/blob/master/docs/serato_markers_.md#color-format

mixxx::RgbColor seratoColorToRgb(quint8 w, quint8 x, quint8 y, quint8 z) {
    quint8 b = (z & 0x7F) | ((y & 0x01) << 7);
    quint8 g = ((y & 0x7F) >> 1) | ((x & 0x03) << 6);
    quint8 r = ((x & 0x7F) >> 2) | ((w & 0x07) << 5);
    return mixxx::RgbColor((r << 16) | (g << 8) | b);
}

mixxx::RgbColor seratoColorToRgb(quint32 color) {
    return seratoColorToRgb(
            (color >> 24) & 0xFF,
            (color >> 16) & 0xFF,
            (color >> 8) & 0xFF,
            color & 0xFF);
}

quint32 seratoColorFromRgb(quint8 r, quint8 g, quint8 b) {
    quint8 z = b & 0x7F;
    quint8 y = ((b >> 7) | (g << 1)) & 0x7F;
    quint8 x = ((g >> 6) | (r << 2)) & 0x7F;
    quint8 w = (r >> 5);
    return (static_cast<quint32>(w) << 24) |
            (static_cast<quint32>(x) << 16) |
            (static_cast<quint32>(y) << 8) |
            static_cast<quint32>(z);
}

quint32 seratoColorFromRgb(mixxx::RgbColor rgb) {
    return seratoColorFromRgb(
            (rgb >> 16) & 0xFF,
            (rgb >> 8) & 0xFF,
            rgb & 0xFF);
}
}

namespace mixxx {

QByteArray SeratoMarkersEntry::dump() const {
    QByteArray data;
    data.resize(kEntrySize);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint8>((m_hasStartPosition ? 0x00 : 0x7F))
           << static_cast<quint32>((m_hasStartPosition ? m_startPosition : 0x7F7F7F7F))
           << static_cast<quint8>((m_hasEndPosition ? 0x00 : 0x7F))
           << static_cast<quint32>((m_hasEndPosition ? m_endPosition : 0x7F7F7F7F));
    stream.writeRawData("\x00\x7F\x7F\x7F\x7F\x7F", 6);
    stream << static_cast<quint32>(seratoColorFromRgb(m_color))
           << static_cast<quint8>(m_type)
           << static_cast<quint8>(m_isLocked);
    return data;
}

SeratoMarkersEntryPointer SeratoMarkersEntry::parse(const QByteArray& data) {
    if (data.length() != kEntrySize) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "Length" << data.length() << "!=" << kEntrySize;
        return nullptr;
    }

    quint8 type;
    quint8 startPositionStatus;
    quint8 endPositionStatus;
    quint32 startPosition;
    quint32 endPosition;
    quint32 colorRaw;
    bool isLocked;
    char buffer[6];

    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> startPositionStatus >> startPosition >> endPositionStatus >> endPosition;

    if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "unable to read bytes 10..16";
        return nullptr;
    }

    stream >> colorRaw >> type >> isLocked;

    const RgbColor color = seratoColorToRgb(colorRaw);

    // Parse Start Position
    bool hasStartPosition = (startPositionStatus != 0x7F);
    if (!hasStartPosition) {
        // Start position not set
        if (startPosition != 0x7F7F7F7F) {
            qWarning() << "Parsing SeratoMarkersEntry failed:"
                       << "startPosition != 0x7F7F7F7F";

            return nullptr;
        }
    }

    // Parse End Position
    bool hasEndPosition = (endPositionStatus != 0x7F);
    if (!hasEndPosition) {
        // End position not set
        if (endPosition != 0x7F7F7F7F) {
            qWarning() << "Parsing SeratoMarkersEntry failed:"
                       << "endPosition != 0x7F7F7F7F";

            return nullptr;
        }
    }

    // Make sure that the unknown (and probably unused) bytes have the expected value
    if (strncmp(buffer, "\x00\x7F\x7F\x7F\x7F\x7F", sizeof(buffer)) != 0) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "Unexpected value at offset 10";
        return nullptr;
    }

    if (stream.status() != QDataStream::Status::Ok) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "Stream read failed with status" << stream.status();
        return nullptr;
    }

    if (!stream.atEnd()) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "Unexpected trailing data";
        return nullptr;
    }

    SeratoMarkersEntryPointer pEntry = SeratoMarkersEntryPointer(new SeratoMarkersEntry(
            hasStartPosition,
            startPosition,
            hasEndPosition,
            endPosition,
            color,
            type,
            isLocked));
    qDebug() << "SeratoMarkersEntry" << *pEntry;
    return pEntry;
}

bool SeratoMarkers::parse(SeratoMarkers* seratoMarkers, const QByteArray& data) {
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);

    quint16 version;
    stream >> version;
    if (version != kVersion) {
        qWarning() << "Parsing SeratoMarkers_ failed:"
                   << "Unknown Serato Markers_ tag version";
        return false;
    }

    quint32 numEntries;
    stream >> numEntries;

    if (numEntries != kNumEntries) {
        qWarning() << "Parsing SeratoMarkers_ failed:"
                   << "Expected" << kNumEntries << "entries but found" << numEntries;
        return false;
    }

    char buffer[kEntrySize];
    QList<SeratoMarkersEntryPointer> entries;
    for (quint32 i = 0; i < numEntries; i++) {
        if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
            qWarning() << "Parsing SeratoMarkersEntry failed:"
                       << "unable to read entry data";
            return false;
        }

        QByteArray entryData = QByteArray(buffer, kEntrySize);
        SeratoMarkersEntryPointer pEntry = SeratoMarkersEntryPointer(
                SeratoMarkersEntry::parse(entryData));
        if (!pEntry) {
            qWarning() << "Parsing SeratoMarkers_ failed:"
                       << "Unable to parse entry!";
            return false;
        }

        if (i < kLoopEntryStartIndex &&
                pEntry->typeId() != SeratoMarkersEntry::TypeId::Cue) {
            qWarning() << "Parsing SeratoMarkers_ failed:"
                       << "Expected cue entry but found type" << pEntry->type();
            return false;
        }

        if (i >= kLoopEntryStartIndex &&
                pEntry->typeId() != SeratoMarkersEntry::TypeId::Loop) {
            qWarning() << "Parsing SeratoMarkers_ failed:"
                       << "Expected loop entry but found type" << pEntry->type();
            return false;
        }

        entries.append(pEntry);
    }

    quint32 trackColorRaw;
    stream >> trackColorRaw;
    RgbColor trackColor = seratoColorToRgb(trackColorRaw);

    if (stream.status() != QDataStream::Status::Ok) {
        qWarning() << "Parsing SeratoMarkers_ failed:"
                   << "Stream read failed with status" << stream.status();
        return false;
    }

    if (!stream.atEnd()) {
        qWarning() << "Parsing SeratoMarkers_ failed:"
                   << "Unexpected trailing data";
        return false;
    }
    seratoMarkers->setEntries(std::move(entries));
    seratoMarkers->setTrackColor(trackColor);

    return true;
}

QByteArray SeratoMarkers::dump() const {
    QByteArray data;
    if (isEmpty()) {
        // Return empty QByteArray
        return data;
    }

    data.resize(sizeof(quint16) + 2 * sizeof(quint32) + kEntrySize * m_entries.size());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << kVersion << m_entries.size();
    for (int i = 0; i < m_entries.size(); i++) {
        SeratoMarkersEntryPointer pEntry = m_entries.at(i);
        stream.writeRawData(pEntry->dump(), kEntrySize);
    }
    stream << seratoColorFromRgb(m_trackColor.value_or(SeratoTags::kDefaultTrackColor));
    return data;
}

} //namespace mixxx
