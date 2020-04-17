#include "track/serato/markers.h"

#include <QtEndian>

#include "track/serato/tags.h"

namespace {

const int kNumEntries = 14;
const int kLoopEntryStartIndex = 5;
const int kEntrySize = 22;
const quint16 kVersion = 0x0205;

// These functions convert between a custom 4-byte format (that we'll call
// "serato32" for brevity) and 3-byte plaintext (both quint32).
// Serato's custom format inserts a single null bit after every 7 payload
// bits, starting from the rightmost bit.
//
// Here's an example:
//
//                      | Hex          Binary
//     ---------------- | -----------  --------------------------------
//     3-byte plaintext |    00 00 cc       000 0000000 0000001 1001100
//     serato32 value   | 00 00 01 4c  00000000000000000000000101001100
//                      |
//     3-byte plaintext |    cc 88 00       110 0110010 0010000 0000000
//     serato32 value   | 06 32 10 00  00000110001100100001000000000000
//
// See this for details:
// https://github.com/Holzhaus/serato-tags/blob/master/docs/serato_markers_.md#custom-serato32-binary-format

/// Decode value from Serato's 32-bit custom format to 24-bit plaintext.
quint32 serato32toUint24(quint8 w, quint8 x, quint8 y, quint8 z) {
    quint8 c = (z & 0x7F) | ((y & 0x01) << 7);
    quint8 b = ((y & 0x7F) >> 1) | ((x & 0x03) << 6);
    quint8 a = ((x & 0x7F) >> 2) | ((w & 0x07) << 5);
    return ((static_cast<quint32>(a) << 16) | (static_cast<quint32>(b) << 8) |
            static_cast<quint32>(c));
}

/// Decode value from Serato's 32-bit custom format to 24-bit plaintext.
quint32 serato32toUint24(quint32 value) {
    return serato32toUint24((value >> 24) & 0xFF,
            (value >> 16) & 0xFF,
            (value >> 8) & 0xFF,
            value & 0xFF);
}

/// Encode a 24-bit plaintext value into Serato's 32-bit custom format.
quint32 serato32fromUint24(quint8 a, quint8 b, quint8 c) {
    quint8 z = c & 0x7F;
    quint8 y = ((c >> 7) | (b << 1)) & 0x7F;
    quint8 x = ((b >> 6) | (a << 2)) & 0x7F;
    quint8 w = (a >> 5);
    return (static_cast<quint32>(w) << 24) | (static_cast<quint32>(x) << 16) |
            (static_cast<quint32>(y) << 8) | static_cast<quint32>(z);
}

/// Encode a 24-bit plaintext value into Serato's 32-bit custom format. The 8
/// most significant bits of the quint32 will be ignored.
quint32 serato32fromUint24(quint32 value) {
    return serato32fromUint24(
            (value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
}
} // namespace

namespace mixxx {

QByteArray SeratoMarkersEntry::dump() const {
    QByteArray data;
    data.resize(kEntrySize);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint8>((m_hasStartPosition ? 0x00 : 0x7F))
           << static_cast<quint32>(
                      (m_hasStartPosition ? serato32fromUint24(m_startPosition)
                                          : 0x7F7F7F7F))
           << static_cast<quint8>((m_hasEndPosition ? 0x00 : 0x7F))
           << static_cast<quint32>(
                      (m_hasEndPosition ? serato32fromUint24(m_endPosition)
                                        : 0x7F7F7F7F));
    stream.writeRawData("\x00\x7F\x7F\x7F\x7F\x7F", 6);
    stream << serato32fromUint24(static_cast<quint32>(m_color))
           << static_cast<quint8>(m_type) << static_cast<quint8>(m_isLocked);
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
    quint32 startPositionSerato32;
    quint32 endPositionSerato32;
    quint32 colorSerato32;
    bool isLocked;
    char buffer[6];

    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> startPositionStatus >> startPositionSerato32 >>
            endPositionStatus >> endPositionSerato32;

    if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "unable to read bytes 10..16";
        return nullptr;
    }

    stream >> colorSerato32 >> type >> isLocked;

    const RgbColor color = RgbColor(serato32toUint24(colorSerato32));

    // Parse Start Position
    bool hasStartPosition = (startPositionStatus != 0x7F);
    quint32 startPosition = 0x7F7F7F7F;
    if (!hasStartPosition) {
        // Start position not set
        if (startPositionSerato32 != 0x7F7F7F7F) {
            qWarning() << "Parsing SeratoMarkersEntry failed:"
                       << "startPosition != 0x7F7F7F7F";

            return nullptr;
        }
    } else {
        startPosition = serato32toUint24(startPositionSerato32);
    }

    // Parse End Position
    bool hasEndPosition = (endPositionStatus != 0x7F);
    quint32 endPosition = 0x7F7F7F7F;
    if (!hasEndPosition) {
        // End position not set
        if (endPositionSerato32 != 0x7F7F7F7F) {
            qWarning() << "Parsing SeratoMarkersEntry failed:"
                       << "endPosition != 0x7F7F7F7F";

            return nullptr;
        }
    } else {
        endPosition = serato32toUint24(endPositionSerato32);
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

    SeratoMarkersEntryPointer pEntry =
            SeratoMarkersEntryPointer(new SeratoMarkersEntry(hasStartPosition,
                    startPosition,
                    hasEndPosition,
                    endPosition,
                    color,
                    type,
                    isLocked));
    qDebug() << "SeratoMarkersEntry" << *pEntry;
    return pEntry;
}

bool SeratoMarkers::parse(
        SeratoMarkers* seratoMarkers, const QByteArray& data) {
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
                   << "Expected" << kNumEntries << "entries but found"
                   << numEntries;
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
        SeratoMarkersEntryPointer pEntry =
                SeratoMarkersEntryPointer(SeratoMarkersEntry::parse(entryData));
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
                       << "Expected loop entry but found type"
                       << pEntry->type();
            return false;
        }

        entries.append(pEntry);
    }

    quint32 trackColorSerato32;
    stream >> trackColorSerato32;
    RgbColor trackColor = RgbColor(serato32toUint24(trackColorSerato32));

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

    data.resize(sizeof(quint16) + 2 * sizeof(quint32) +
            kEntrySize * m_entries.size());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << kVersion << m_entries.size();
    for (int i = 0; i < m_entries.size(); i++) {
        SeratoMarkersEntryPointer pEntry = m_entries.at(i);
        stream.writeRawData(pEntry->dump(), kEntrySize);
    }
    stream << serato32fromUint24(static_cast<quint32>(
            m_trackColor.value_or(SeratoTags::kDefaultTrackColor)));
    return data;
}

QList<CueInfo> SeratoMarkers::getCues(double timingOffsetMillis) const {
    qDebug() << "Reading cues from 'Serato Markers_' tag data...";

    QList<CueInfo> cueInfos;
    int cueIndex = 0;
    for (const auto& pEntry : m_entries) {
        DEBUG_ASSERT(pEntry);
        switch (pEntry->typeId()) {
        case SeratoMarkersEntry::TypeId::Cue: {
            if (pEntry->hasStartPosition()) {
                CueInfo cueInfo(
                        CueType::HotCue,
                        pEntry->getStartPosition() + timingOffsetMillis,
                        std::nullopt,
                        cueIndex,
                        "",
                        pEntry->getColor());
                cueInfos.append(cueInfo);
            }
            cueIndex++;
            break;
        }
        // TODO: Add support for Loops
        default:
            break;
        }
    }

    return cueInfos;
}

} //namespace mixxx
