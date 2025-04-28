#include "track/serato/markers.h"

#include <QtEndian>

#include "track/serato/tags.h"
#include "util/logger.h"

namespace {

mixxx::Logger kLogger("SeratoMarkers");

// The number of entries are fixed, the "SeratoMarkers_" tag always contains
// the first 5 cues and 9 loops. Additional cues points can be stored in the
// "SeratoMarkers2" tag.
constexpr int kNumCueEntries = 5;
constexpr int kNumLoopEntries = 9;
constexpr int kNumEntries = kNumCueEntries + kNumLoopEntries;

constexpr int kEntrySizeID3 = 22;
constexpr int kEntrySizeMP4 = 19;
constexpr quint32 kNoPosition = 0x7F7F7F7F;
constexpr quint16 kVersion = 0x0205;

constexpr char kSeratoMarkersBase64EncodedPrefixStr[] =
        "application/octet-stream\0\0Serato Markers_";
const QByteArray kSeratoMarkersBase64EncodedPrefix = QByteArray::fromRawData(
        kSeratoMarkersBase64EncodedPrefixStr,
        sizeof(kSeratoMarkersBase64EncodedPrefixStr));

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
// https://github.com/Holzhaus/serato-tags/blob/main/docs/serato_markers_.md#custom-serato32-binary-format

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

QByteArray SeratoMarkersEntry::dumpID3() const {
    QByteArray data;
    data.resize(kEntrySizeID3);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint8>((m_hasStartPosition ? 0x00 : 0x7F))
           << static_cast<quint32>(
                      (m_hasStartPosition ? serato32fromUint24(m_startPosition)
                                          : kNoPosition))
           << static_cast<quint8>((m_hasEndPosition ? 0x00 : 0x7F))
           << static_cast<quint32>(
                      (m_hasEndPosition ? serato32fromUint24(m_endPosition)
                                        : kNoPosition));
    stream.writeRawData("\x00\x7F\x7F\x7F\x7F\x7F", 6);
    stream << serato32fromUint24(m_color.toQRgb())
           << static_cast<quint8>(m_type) << static_cast<quint8>(m_isLocked);
    return data;
}

QByteArray SeratoMarkersEntry::dumpMP4() const {
    QByteArray data;
    data.resize(kEntrySizeMP4);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(m_startPosition)
           << static_cast<quint32>(m_endPosition);
    stream.writeRawData("\x00\xFF\xFF\xFF\xFF\x00", 6);
    stream << static_cast<quint8>(qRed(m_color.toQRgb()))
           << static_cast<quint8>(qGreen(m_color.toQRgb()))
           << static_cast<quint8>(qBlue(m_color.toQRgb()))
           << static_cast<quint8>(m_type)
           << static_cast<quint8>(m_isLocked);
    return data;
}

SeratoMarkersEntryPointer SeratoMarkersEntry::parseID3(const QByteArray& data) {
    if (data.length() != kEntrySizeID3) {
        kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                          << "Length" << data.length() << "!=" << kEntrySizeID3;
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
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> startPositionStatus >> startPositionSerato32 >>
            endPositionStatus >> endPositionSerato32;

    if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
        kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                          << "unable to read bytes 10..16";
        return nullptr;
    }

    stream >> colorSerato32 >> type >> isLocked;

    if (isLocked && type != static_cast<quint8>(TypeId::Loop)) {
        // This should never happen with Metadata exported from Serato. We fail
        // here because this case is unexpected and should not be corrected
        // silently without knowing what we do. Write a bug if this assumption
        // is wrong.
        kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                          << "isLocked field is not false for non-loop type"
                          << static_cast<quint8>(TypeId::Loop);
        return nullptr;
    }

    const auto color = SeratoStoredHotcueColor(serato32toUint24(colorSerato32));

    // Parse Start Position
    bool hasStartPosition = (startPositionStatus != 0x7F);
    quint32 startPosition = kNoPosition;
    if (!hasStartPosition) {
        // Start position not set
        if (startPositionSerato32 != kNoPosition) {
            // This should never happen with Metadata exported from Serato. We fail
            // here because this case is unexpected and should not be corrected
            // silently without knowing what we do. Write a bug if this assumption
            // is wrong.
            kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                              << "startPosition"
                              << startPosition
                              << "!="
                              << QString::number(kNoPosition, 16);

            return nullptr;
        }
    } else {
        startPosition = serato32toUint24(startPositionSerato32);
    }

    // Parse End Position
    bool hasEndPosition = (endPositionStatus != 0x7F);
    quint32 endPosition = kNoPosition;
    if (!hasEndPosition) {
        // End position not set
        if (endPositionSerato32 != kNoPosition) {
            // This should never happen with Metadata exported from Serato. We fail
            // here because this case is unexpected and should not be corrected
            // silently without knowing what we do. Write a bug if this assumption
            // is wrong.
            kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                              << "endPosition"
                              << endPositionSerato32
                              << "!="
                              << QString::number(kNoPosition, 16);

            return nullptr;
        }
    } else {
        endPosition = serato32toUint24(endPositionSerato32);
    }

    // Make sure that the unknown (and probably unused) bytes have the expected value
    if (strncmp(buffer, "\x00\x7F\x7F\x7F\x7F\x7F", sizeof(buffer)) != 0) {
        // This should never happen with Metadata exported from Serato. We fail
        // here because this case is unexpected and should not be corrected
        // silently without knowing what we do. Write a bug if this assumption
        // is wrong.
        kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                          << "Unexpected value at offset 10"
                          << QByteArray::fromRawData(buffer, sizeof(buffer));
        return nullptr;
    }

    if (stream.status() != QDataStream::Status::Ok) {
        kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                          << "Stream read failed with status" << stream.status();
        return nullptr;
    }

    if (!stream.atEnd()) {
        kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                          << "Unexpected trailing data"
                          << stream.device()->readAll();
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
    kLogger.trace() << "SeratoMarkersEntry (ID3)" << *pEntry;
    return pEntry;
}

SeratoMarkersEntryPointer SeratoMarkersEntry::parseMP4(const QByteArray& data) {
    if (data.length() != kEntrySizeMP4) {
        kLogger.warning() << "Parsing SeratoMarkersEntry (MP4) failed:"
                          << "Length" << data.length() << "!=" << kEntrySizeMP4;
        return nullptr;
    }

    quint32 startPosition;
    quint32 endPosition;
    char buffer[6];
    quint8 colorRed;
    quint8 colorGreen;
    quint8 colorBlue;
    quint8 type;
    bool isLocked;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> startPosition >> endPosition;

    if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
        kLogger.warning() << "Parsing SeratoMarkersEntry (MP4) failed:"
                          << "unable to read bytes 8..14";
        return nullptr;
    }

    stream >> colorRed >> colorGreen >> colorBlue >> type >> isLocked;

    if (isLocked && type != static_cast<quint8>(TypeId::Loop)) {
        // This should never happen with Metadata exported from Serato. We fail
        // here because this case is unexpected and should not be corrected
        // silently without knowing what we do. Write a bug if this assumption
        // is wrong.
        kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                          << "isLocked field is not false for non-loop type"
                          << static_cast<quint8>(TypeId::Loop);
        return nullptr;
    }

    const auto color = SeratoStoredHotcueColor(qRgb(colorRed, colorGreen, colorBlue));

    // Make sure that the unknown (and probably unused) bytes have the expected value
    if (strncmp(buffer, "\x00\xFF\xFF\xFF\xFF\x00", sizeof(buffer)) != 0) {
        // This should never happen with Metadata exported from Serato. We fail
        // here because this case is unexpected and should not be corrected
        // silently without knowing what we do. Write a bug if this assumption
        // is wrong.
        kLogger.warning() << "Parsing SeratoMarkersEntry (MP4) failed:"
                          << "Unexpected value at offset 8"
                          << QByteArray::fromRawData(buffer, sizeof(buffer));
        return nullptr;
    }

    if (stream.status() != QDataStream::Status::Ok) {
        kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                          << "Stream read failed with status" << stream.status();
        return nullptr;
    }

    if (!stream.atEnd()) {
        kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                          << "Unexpected trailing data"
                          << stream.device()->readAll();
        return nullptr;
    }

    SeratoMarkersEntryPointer pEntry =
            SeratoMarkersEntryPointer(new SeratoMarkersEntry(
                    true,
                    startPosition,
                    type == static_cast<quint8>(TypeId::Loop),
                    endPosition,
                    color,
                    type,
                    isLocked));
    kLogger.trace() << "SeratoMarkersEntry (MP4)" << *pEntry;
    return pEntry;
}

// static
bool SeratoMarkers::parse(
        SeratoMarkers* seratoMarkers, const QByteArray& data, taglib::FileType fileType) {
    VERIFY_OR_DEBUG_ASSERT(seratoMarkers) {
        return false;
    }

    switch (fileType) {
    case taglib::FileType::MPEG:
    case taglib::FileType::AIFF:
        return parseID3(seratoMarkers, data);
    case taglib::FileType::MP4:
        return parseMP4(seratoMarkers, data);
    default:
        return false;
    }
}

// static
bool SeratoMarkers::parseID3(
        SeratoMarkers* seratoMarkers, const QByteArray& data) {
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);

    quint16 version;
    stream >> version;
    if (version != kVersion) {
        kLogger.warning() << "Parsing SeratoMarkers_ failed:"
                          << "Unknown Serato Markers_ tag version";
        return false;
    }

    quint32 numEntries;
    stream >> numEntries;

    if (numEntries != kNumEntries) {
        kLogger.warning() << "Parsing SeratoMarkers_ failed:"
                          << "Expected" << kNumEntries << "entries but found"
                          << numEntries;
        return false;
    }

    char buffer[kEntrySizeID3];
    QList<SeratoMarkersEntryPointer> entries;
    for (quint32 i = 0; i < numEntries; i++) {
        if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
            kLogger.warning() << "Parsing SeratoMarkersEntry failed:"
                              << "unable to read entry data";
            return false;
        }

        QByteArray entryData = QByteArray(buffer, kEntrySizeID3);
        SeratoMarkersEntryPointer pEntry =
                SeratoMarkersEntryPointer(SeratoMarkersEntry::parseID3(entryData));
        if (!pEntry) {
            kLogger.warning() << "Parsing SeratoMarkers_ failed:"
                              << "Unable to parse entry!";
            return false;
        }

        if (i < kNumCueEntries &&
                pEntry->typeId() != SeratoMarkersEntry::TypeId::Cue) {
            // This should never happen with Metadata exported from Serato. We fail
            // here because this case is unexpected and should not be corrected
            // silently without knowing what we do. Write a bug if this assumption
            // is wrong.
            kLogger.warning() << "Parsing SeratoMarkers_ failed:"
                              << "Expected cue entry but found type" << pEntry->type();
            return false;
        }

        if (i >= kNumCueEntries &&
                pEntry->typeId() != SeratoMarkersEntry::TypeId::Loop) {
            // This should never happen with Metadata exported from Serato. We fail
            // here because this case is unexpected and should not be corrected
            // silently without knowing what we do. Write a bug if this assumption
            // is wrong.
            kLogger.warning() << "Parsing SeratoMarkers_ failed:"
                              << "Expected loop entry but found type"
                              << pEntry->type();
            return false;
        }

        entries.append(pEntry);
    }

    quint32 trackColorSerato32;
    stream >> trackColorSerato32;
    const auto trackColor = SeratoStoredTrackColor(serato32toUint24(trackColorSerato32));

    if (stream.status() != QDataStream::Status::Ok) {
        kLogger.warning() << "Parsing SeratoMarkers_ failed:"
                          << "Stream read failed with status" << stream.status();
        return false;
    }

    if (!stream.atEnd()) {
        kLogger.warning() << "Parsing SeratoMarkers_ failed:"
                          << "Unexpected trailing data"
                          << stream.device()->readAll();
        return false;
    }
    seratoMarkers->setEntries(std::move(entries));
    seratoMarkers->setTrackColor(trackColor);

    return true;
}

// static
bool SeratoMarkers::parseMP4(
        SeratoMarkers* seratoMarkers,
        const QByteArray& base64EncodedData) {
    const auto decodedData = QByteArray::fromBase64(base64EncodedData);
    if (!decodedData.startsWith(kSeratoMarkersBase64EncodedPrefix)) {
        kLogger.warning() << "Decoding SeratoMarkers_ from base64 failed:"
                          << "Unexpected prefix"
                          << decodedData.left(kSeratoMarkersBase64EncodedPrefix.size())
                          << "!="
                          << kSeratoMarkersBase64EncodedPrefix;
        return false;
    }

    QDataStream stream(decodedData.mid(kSeratoMarkersBase64EncodedPrefix.length()));
    stream.setByteOrder(QDataStream::BigEndian);

    quint16 version;
    stream >> version;
    if (version != kVersion) {
        kLogger.warning() << "Parsing SeratoMarkers_ (MP4) failed:"
                          << "Unknown Serato Markers_ tag version" << QString::number(version, 16);
        return false;
    }

    quint32 numEntries;
    stream >> numEntries;

    if (numEntries != kNumEntries) {
        kLogger.warning() << "Parsing SeratoMarkers_ (MP4) failed:"
                          << "Expected" << kNumEntries << "entries but found"
                          << numEntries;
        return false;
    }

    char buffer[kEntrySizeMP4];
    QList<SeratoMarkersEntryPointer> entries;
    for (quint32 i = 0; i < numEntries; i++) {
        if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
            kLogger.warning() << "Parsing SeratoMarkersEntry (MP4) failed:"
                              << "unable to read entry data";
            return false;
        }

        QByteArray entryData = QByteArray(buffer, kEntrySizeMP4);
        SeratoMarkersEntryPointer pEntry =
                SeratoMarkersEntryPointer(SeratoMarkersEntry::parseMP4(entryData));
        if (!pEntry) {
            kLogger.warning() << "Parsing SeratoMarkers_ (MP4) failed:"
                              << "Unable to parse entry!";
            return false;
        }

        if (i < kNumCueEntries &&
                pEntry->typeId() != SeratoMarkersEntry::TypeId::Cue) {
            // This should never happen with Metadata exported from Serato. We fail
            // here because this case is unexpected and should not be corrected
            // silently without knowing what we do. Write a bug if this assumption
            // is wrong.
            kLogger.warning() << "Parsing SeratoMarkers_ (MP4) failed:"
                              << "Expected cue entry but found type" << pEntry->type();
            return false;
        }

        if (i >= kNumCueEntries &&
                pEntry->typeId() != SeratoMarkersEntry::TypeId::Loop) {
            // This should never happen with Metadata exported from Serato. We fail
            // here because this case is unexpected and should not be corrected
            // silently without knowing what we do. Write a bug if this assumption
            // is wrong.
            kLogger.warning() << "Parsing SeratoMarkers_ (MP4) failed:"
                              << "Expected loop entry but found type"
                              << pEntry->type();
            return false;
        }

        entries.append(pEntry);
    }

    quint8 field1;
    quint8 colorRed;
    quint8 colorGreen;
    quint8 colorBlue;
    stream >> field1 >> colorRed >> colorGreen >> colorBlue;
    const auto trackColor = SeratoStoredTrackColor(qRgb(colorRed, colorGreen, colorBlue));

    if (field1 != 0x00) {
        // This should never happen with Metadata exported from Serato. We fail
        // here because this case is unexpected and should not be corrected
        // silently without knowing what we do. Write a bug if this assumption
        // is wrong.
        kLogger.warning() << "Parsing SeratoMarkers_ (MP4) failed:"
                          << "Unexpected value before track color"
                          << field1;
        return false;
    }

    if (stream.status() != QDataStream::Status::Ok) {
        kLogger.warning() << "Parsing SeratoMarkers_ failed:"
                          << "Stream read failed with status" << stream.status();
        return false;
    }

    if (!stream.atEnd()) {
        kLogger.warning() << "Parsing SeratoMarkers_ failed:"
                          << "Unexpected trailing data"
                          << stream.device()->readAll();
        return false;
    }
    seratoMarkers->setEntries(std::move(entries));
    seratoMarkers->setTrackColor(trackColor);

    return true;
}

QByteArray SeratoMarkers::dump(taglib::FileType fileType) const {
    switch (fileType) {
    case taglib::FileType::MPEG:
    case taglib::FileType::AIFF:
        return dumpID3();
    case taglib::FileType::MP4:
        return dumpMP4();
    default:
        DEBUG_ASSERT(false);
        return {};
    }
}

QByteArray SeratoMarkers::dumpID3() const {
    QByteArray data;
    if (isEmpty()) {
        // Return empty QByteArray
        return data;
    }

    data.resize(sizeof(quint16) + 2 * sizeof(quint32) +
            kEntrySizeID3 * m_entries.size());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << kVersion << static_cast<quint32>(m_entries.size());
    for (int i = 0; i < m_entries.size(); i++) {
        SeratoMarkersEntryPointer pEntry = m_entries.at(i);
        stream.writeRawData(pEntry->dumpID3(), kEntrySizeID3);
    }

    DEBUG_ASSERT(m_pTrackColor);
    SeratoStoredTrackColor trackColor = m_pTrackColor.value_or(
            SeratoStoredTrackColor(SeratoStoredColor::kNoColor));
    stream << serato32fromUint24(trackColor.toQRgb());

    return data;
}

QByteArray SeratoMarkers::dumpMP4() const {
    if (isEmpty()) {
        // Return empty QByteArray
        return {};
    }

    QByteArray data;
    data.resize(kSeratoMarkersBase64EncodedPrefix.length() +
            sizeof(quint16) + 2 * sizeof(quint32) +
            kEntrySizeMP4 * m_entries.size());
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.writeRawData(kSeratoMarkersBase64EncodedPrefix.constData(),
            kSeratoMarkersBase64EncodedPrefix.length());
    stream << kVersion << static_cast<quint32>(m_entries.size());
    for (int i = 0; i < m_entries.size(); i++) {
        SeratoMarkersEntryPointer pEntry = m_entries.at(i);
        stream.writeRawData(pEntry->dumpMP4(), kEntrySizeMP4);
    }

    DEBUG_ASSERT(m_pTrackColor);
    SeratoStoredTrackColor trackColor = m_pTrackColor.value_or(
            SeratoStoredTrackColor(SeratoStoredColor::kNoColor));
    stream << static_cast<quint8>(0x00)
           << static_cast<quint8>(qRed(trackColor.toQRgb()))
           << static_cast<quint8>(qGreen(trackColor.toQRgb()))
           << static_cast<quint8>(qBlue(trackColor.toQRgb()));

    // A newline char is inserted at every 72 bytes of base64-encoded content.
    // Hence, we can split the data into blocks of 72 bytes * 3/4 = 54 bytes
    // and base64-encode them one at a time:
    const int base64Size = (data.size() * 4 + 2) / 3;
    QByteArray base64Data;
    base64Data.reserve(base64Size + base64Size / 72);
    int offset = 0;
    while (offset < data.size()) {
        if (offset > 0) {
            base64Data.append('\n');
        }
        QByteArray block = data.mid(offset, 54);
        base64Data.append(block.toBase64(
                QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals));
        offset += block.size();
    }

    // FIXME: Why do we need to append another "A" here?
    base64Data.append('A');

    return base64Data;
}

QList<CueInfo> SeratoMarkers::getCues() const {
    // qDebug() << "Reading cues from 'Serato Markers_' tag data...";

    QList<CueInfo> cueInfos;
    int cueIndex = 0;
    int loopIndex = 0;
    for (const auto& pEntry : std::as_const(m_entries)) {
        VERIFY_OR_DEBUG_ASSERT(pEntry) {
            continue;
        }
        switch (pEntry->typeId()) {
        case SeratoMarkersEntry::TypeId::Cue: {
            if (pEntry->hasStartPosition()) {
                CueInfo cueInfo(
                        CueType::HotCue,
                        pEntry->getStartPosition(),
                        std::nullopt,
                        cueIndex,
                        QString(),
                        pEntry->getColor().toDisplayedColor(),
                        CueFlag::None);
                cueInfos.append(cueInfo);
            }
            cueIndex++;
            break;
        }
        case SeratoMarkersEntry::TypeId::Loop: {
            if (pEntry->hasStartPosition()) {
                if (!pEntry->hasEndPosition()) {
                    // Usually this can't happen unless Serato is buggy or the
                    // Metadata is broken. But we should never trust user data
                    // and better be safe than sorry.
                    qWarning() << "SeratoMarkers: Loop" << loopIndex << "has no end position!";
                    continue;
                }
                CueInfo loopInfo = CueInfo(
                        CueType::Loop,
                        pEntry->getStartPosition(),
                        pEntry->getEndPosition(),
                        loopIndex,
                        QString(),
                        std::nullopt,
                        pEntry->isLocked() ? CueFlag::Locked : CueFlag::None);
                cueInfos.append(loopInfo);
                // TODO: Add support for the "locked" attribute
            }
            loopIndex++;
            break;
        }
        default:
            break;
        }
    }

    return cueInfos;
}

void SeratoMarkers::setCues(const QList<CueInfo>& cueInfos) {
    QMap<int, CueInfo> cueMap;
    QMap<int, CueInfo> loopMap;

    for (const CueInfo& cueInfo : cueInfos) {
        // All of these check can be debug assertions, as the list should be
        // pre-filtered by the seratoTags class.
        VERIFY_OR_DEBUG_ASSERT(cueInfo.getHotCueIndex()) {
            continue;
        }
        int hotcueIndex = *cueInfo.getHotCueIndex();

        VERIFY_OR_DEBUG_ASSERT(hotcueIndex >= kFirstHotCueIndex) {
            continue;
        }
        VERIFY_OR_DEBUG_ASSERT(cueInfo.getStartPositionMillis()) {
            continue;
        }

        switch (cueInfo.getType()) {
        case CueType::HotCue:
            cueMap.insert(hotcueIndex, cueInfo);
            break;
        case CueType::Loop:
            VERIFY_OR_DEBUG_ASSERT(cueInfo.getEndPositionMillis()) {
                continue;
            }
            loopMap.insert(hotcueIndex, cueInfo);
            break;
        default:
            DEBUG_ASSERT(!"Invalid cue type");
            continue;
        }
    }

    QList<SeratoMarkersEntryPointer> entries;
    for (int i = 0; i < kNumCueEntries; i++) {
        const CueInfo cueInfo = cueMap.value(i);

        SeratoMarkersEntryPointer pEntry;
        if (cueInfo.getStartPositionMillis() && cueInfo.getColor()) {
            pEntry = std::make_shared<SeratoMarkersEntry>(
                    true,
                    static_cast<int>(*cueInfo.getStartPositionMillis()),
                    false,
                    0,
                    SeratoStoredHotcueColor::fromDisplayedColor(cueInfo.getColor()),
                    static_cast<int>(SeratoMarkersEntry::TypeId::Cue),
                    false);
        } else {
            pEntry = std::make_shared<SeratoMarkersEntry>(
                    false,
                    0,
                    false,
                    0,
                    SeratoStoredHotcueColor(SeratoStoredColor::kFixedUnsetColor),
                    static_cast<int>(SeratoMarkersEntry::TypeId::Unknown),
                    false);
        }
        entries.append(pEntry);
    }

    for (int i = 0; i < kNumLoopEntries; i++) {
        const CueInfo cueInfo = loopMap.value(i);

        SeratoMarkersEntryPointer pEntry;
        if (cueInfo.getStartPositionMillis()) {
            pEntry = std::make_shared<SeratoMarkersEntry>(
                    true,
                    static_cast<int>(*cueInfo.getStartPositionMillis()),
                    true,
                    static_cast<int>(*cueInfo.getEndPositionMillis()),
                    // TODO: In Serato, saved loops always have a fixed color.
                    // We *could* export the actual color here if we also
                    // import the blue-ish default color in the code above, but
                    // it will not be used by Serato.
                    SeratoStoredHotcueColor(SeratoStoredColor::kFixedLoopColor),
                    static_cast<int>(SeratoMarkersEntry::TypeId::Loop),
                    cueInfo.isLocked());
        } else {
            pEntry = std::make_shared<SeratoMarkersEntry>(
                    false,
                    0,
                    false,
                    0,
                    SeratoStoredHotcueColor(SeratoStoredColor::kFixedUnsetColor),
                    // In contrast to cues, unset saved loop have the same type
                    // ID as set ones.
                    static_cast<int>(SeratoMarkersEntry::TypeId::Loop),
                    false);
        }
        entries.append(pEntry);
    }

    setEntries(std::move(entries));
}

} // namespace mixxx
