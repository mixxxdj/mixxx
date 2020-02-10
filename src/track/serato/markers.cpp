#include "track/serato/markers.h"

#include <QtEndian>
#include <QStringLiteral>

namespace {

const int kEntrySize = 22;
const char* kVersion = "\x02\x05";
const int kVersionSize = 2;

inline
quint32 bytesToUInt32(const QByteArray& data) {
    DEBUG_ASSERT(data.size() == sizeof(quint32));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    return qFromBigEndian<quint32>(data.constData());
#else
    return qFromBigEndian<quint32>(
            reinterpret_cast<const uchar*>(data.constData()));
#endif
}

// These functions conversion between the 4-byte "Serato Markers_" color format
// and QRgb (3-Byte RGB, transparency disabled).
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

QRgb colorToRgb(quint8 w, quint8 x, quint8 y, quint8 z) {
    quint8 b = (z & 0x7F) | ((y & 0x01) << 7);
    quint8 g = ((y & 0x7F) >> 1) | ((x & 0x03) << 6);
    quint8 r = ((x & 0x7F) >> 2) | ((w & 0x07) << 5);
    return QRgb(r << 16) | (g << 8) | b;
}

QRgb colorToRgb(const QByteArray& data) {
    DEBUG_ASSERT(data.size() == 4);
    return colorToRgb(data.at(0), data.at(1), data.at(2), data.at(3));
}

quint32 colorFromRgb(quint8 r, quint8 g, quint8 b) {
    quint8 z = b & 0x7F;
    quint8 y = ((b >> 7) | (g << 1)) & 0x7F;
    quint8 x = ((g >> 6) | (r << 2)) & 0x7F;
    quint8 w = (r >> 5);
    return (w << 24) | (x << 16) | (y << 8) | z;
}

quint32 colorFromRgb(QRgb rgb) {
    return colorFromRgb(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

}

namespace mixxx {

QByteArray SeratoMarkersEntry::data() const {
    QByteArray data;
    data.resize(kEntrySize);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (quint8)(m_isSet ? '\x00' : '\x7F')
           << (quint32)((m_startPosition == -1) ? 0x7F7F7F7F : m_startPosition)
           << (quint8)((!m_isSet || m_type == 1) ? '\x7F' : m_isEnabled)
           << (quint32)((m_endPosition == -1) ? 0x7F7F7F7F : m_endPosition);
    stream.writeRawData("\x00\x7F\x7F\x7F\x7F\x7F", 6);
    stream << (quint32)colorFromRgb(m_color)
           << (quint8)m_type
           << (quint8)m_isLocked;
    return data;
}

SeratoMarkersEntryPointer SeratoMarkersEntry::parse(const QByteArray& data) {
    if (data.length() != kEntrySize) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "Length" << data.length() << "!=" << kEntrySize;
        return nullptr;
    }

    const bool isSet = (data.at(0) == '\x7F') ? false : true;
    const quint32 uStartPosition = bytesToUInt32(data.mid(1, 4));
    if (uStartPosition > 0x7FFFFFFF) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "startPosition > 0x7FFFFFF";
        return nullptr;
    }
    const int startPosition = (uStartPosition == 0x7F7F7F7F) ? -1 : static_cast<int>(uStartPosition);

    const bool isEnabled = static_cast<bool>(data.at(5));

    const quint32 uEndPosition = bytesToUInt32(data.mid(6, 4));
    if (uEndPosition > 0x7FFFFFFF) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "endPosition > 0x7FFFFFF";
        return nullptr;
    }
    const int endPosition = (uEndPosition == 0x7F7F7F7F) ? -1 : static_cast<int>(uEndPosition);

    if (data.mid(10, 6).compare("\x00\x7F\x7F\x7F\x7F") == 0) {
        qWarning() << "Parsing SeratoMarkersEntry failed:"
                   << "Unexpected value at offset 10";
        return nullptr;
    }

    const QRgb color = colorToRgb(data.mid(16, 4));
    const quint8 type = data.at(20);
    const bool isLocked = static_cast<bool>(data.at(21));

    SeratoMarkersEntryPointer pEntry = SeratoMarkersEntryPointer(new SeratoMarkersEntry(
            isSet,
            startPosition,
            isEnabled,
            endPosition,
            color,
            type,
            isLocked));
    qDebug() << "SeratoMarkersEntry" << *pEntry;
    return pEntry;
}

bool SeratoMarkers::parse(SeratoMarkers* seratoMarkers, const QByteArray& data) {
    if (!data.startsWith(kVersion)) {
        qWarning() << "Parsing SeratoMarkers_ failed:"
                   << "Unknown Serato Markers_ tag version";
        return false;
    }

    quint32 numEntries = bytesToUInt32(data.mid(2, 4));
    QList<std::shared_ptr<SeratoMarkersEntry>> entries;

    int offset = 6;
    for (quint32 i = 0; i < numEntries; i++) {
        if (data.size() <= (offset + kEntrySize)) {
            qWarning() << "Parsing SeratoMarkers_ failed:"
                       << "Incomplete entry" << data.mid(offset);
            return false;
        }

        QByteArray entryData = data.mid(offset, kEntrySize);
        SeratoMarkersEntryPointer pEntry = SeratoMarkersEntryPointer(
                SeratoMarkersEntry::parse(entryData));
        if (!pEntry) {
            qWarning() << "Parsing SeratoMarkers_ failed:"
                       << "Unable to parse entry!";
            return false;
        }
        entries.append(pEntry);

        offset += kEntrySize;
    }

    if (data.mid(offset).size() != 4) {
        qWarning() << "Parsing SeratoMarkers_ failed:"
                   << "Invalid footer" << data.mid(offset);
        return false;
    }

    QRgb color = colorToRgb(data.mid(offset));

    seratoMarkers->setEntries(entries);
    seratoMarkers->setTrackColor(color);

    return true;
}

QByteArray SeratoMarkers::data() const {
    QByteArray data;
    data.resize(kVersionSize + 2 * sizeof(quint32) + kEntrySize * m_entries.size());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.writeRawData(kVersion, kVersionSize);
    stream << m_entries.size();
    for (int i = 0; i < m_entries.size(); i++) {
        SeratoMarkersEntryPointer pEntry = m_entries.at(i);
        stream.writeRawData(pEntry->data(), kEntrySize);
    }
    stream << colorFromRgb(m_trackColor);
    return data;
}

} //namespace mixxx
