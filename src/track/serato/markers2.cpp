#include "track/serato/markers2.h"

#include <QtEndian>

#include "util/logger.h"

namespace {

mixxx::Logger kLogger("SeratoMarkers2");

constexpr quint32 kLoopUnknownField2ExpectedValue = 0xFFFFFFFF;
constexpr quint8 kLoopUnknownField3ExpectedValue = 0x00;
constexpr quint8 kLoopUnknownField4ExpectedValue = 0x00;

constexpr char kSeratoMarkers2Base64EncodedPrefixStr[] =
        "application/octet-stream\0\0Serato Markers2";
const QByteArray kSeratoMarkers2Base64EncodedPrefix = QByteArray::fromRawData(
        kSeratoMarkers2Base64EncodedPrefixStr,
        sizeof(kSeratoMarkers2Base64EncodedPrefixStr));

QString zeroTerminatedUtf8StringtoQString(QDataStream* stream) {
    DEBUG_ASSERT(stream);

    QByteArray data;
    quint8 byte = '\xFF';
    while (byte != '\x00') {
        *stream >> byte;
        if (byte != '\x00') {
            data.append(byte);
        }
        if (stream->status() != QDataStream::Status::Ok) {
            return QString();
        }
    }
    return QString::fromUtf8(data);
}

QByteArray base64encode(const QByteArray& data, bool chopPadding) {
    QByteArray dataBase64;

    // A newline char is inserted at every 72 bytes of base64-encoded content.
    // Hence, we can split the data into blocks of 72 bytes * 3/4 = 54 bytes
    // and base64-encode them one at a time:
    int offset = 0;
    while (offset < data.size()) {
        if (offset > 0) {
            dataBase64.append('\n');
        }
        QByteArray block = data.mid(offset, 54);
        dataBase64.append(block.toBase64(
                QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals));
        offset += block.size();

        if (chopPadding) {
            // In case that the last block would require padding, Serato seems to
            // chop off the last byte of the base64-encoded data
            if (block.size() % 3) {
                dataBase64.chop(1);
            }
        }
    }

    return dataBase64;
}

} // namespace

namespace mixxx {

SeratoMarkers2EntryPointer SeratoMarkers2BpmLockEntry::parse(const QByteArray& data) {
    if (data.length() != 1) {
        kLogger.warning() << "Parsing SeratoMarkers2BpmLockEntry failed:"
                          << "Length" << data.length() << "!= 1";
        return nullptr;
    }

    const bool locked = data.at(0);
    SeratoMarkers2BpmLockEntry* pEntry = new SeratoMarkers2BpmLockEntry(locked);
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SeratoMarkers2BpmLockEntry" << *pEntry;
    }
    return SeratoMarkers2EntryPointer(pEntry);
}

QByteArray SeratoMarkers2BpmLockEntry::dump() const {
    QByteArray data;
    data.resize(length());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint8>(m_locked);

    return data;
}

quint32 SeratoMarkers2BpmLockEntry::length() const {
    return 1;
}

SeratoMarkers2EntryPointer SeratoMarkers2ColorEntry::parse(const QByteArray& data) {
    if (data.length() != 4) {
        kLogger.warning() << "Parsing SeratoMarkers2ColorEntry failed:"
                          << "Length" << data.length() << "!= 4";
        return nullptr;
    }

    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (data.at(0) != '\x00') {
        kLogger.warning() << "Parsing SeratoMarkers2ColorEntry failed:"
                          << "Byte 0: " << data.at(0) << "!= '\\0'";
        return nullptr;
    }

    const auto color = SeratoStoredTrackColor(qRgb(
            static_cast<quint8>(data.at(1)),
            static_cast<quint8>(data.at(2)),
            static_cast<quint8>(data.at(3))));

    SeratoMarkers2ColorEntry* pEntry = new SeratoMarkers2ColorEntry(color);
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SeratoMarkers2ColorEntry" << *pEntry;
    }
    return SeratoMarkers2EntryPointer(pEntry);
}

QByteArray SeratoMarkers2ColorEntry::dump() const {
    QByteArray data;
    data.resize(length());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint8>('\x00')
           << static_cast<quint8>(qRed(m_color.toQRgb()))
           << static_cast<quint8>(qGreen(m_color.toQRgb()))
           << static_cast<quint8>(qBlue(m_color.toQRgb()));

    return data;
}

quint32 SeratoMarkers2ColorEntry::length() const {
    return 4;
}

SeratoMarkers2EntryPointer SeratoMarkers2CueEntry::parse(const QByteArray& data) {
    if (data.length() < 13) {
        kLogger.warning() << "Parsing SeratoMarkers2CueEntry failed:"
                          << "Length" << data.length() << "< 13";
        return nullptr;
    }

    // CUE entry fields in order of appearance
    quint8 unknownField1;
    quint8 index;
    quint32 position;
    quint8 unknownField2;
    quint8 rawRgbRed;
    quint8 rawRgbGreen;
    quint8 rawRgbBlue;
    quint16 unknownField3;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);

    stream >> unknownField1;

    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (unknownField1 != '\x00') {
        kLogger.warning() << "Parsing SeratoMarkers2CueEntry failed:"
                          << "Byte 0: " << data.at(0) << "!= '\\0'";
        return nullptr;
    }

    stream >> index >> position >> unknownField2;

    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (unknownField2 != '\x00') {
        kLogger.warning() << "Parsing SeratoMarkers2CueEntry failed:"
                          << "Byte 6: " << data.at(6) << "!= '\\0'";
        return nullptr;
    }

    stream >> rawRgbRed >> rawRgbGreen >> rawRgbBlue >> unknownField3;
    const auto color = SeratoStoredHotcueColor(qRgb(rawRgbRed, rawRgbGreen, rawRgbBlue));

    // Unknown field(s), make sure it's 0 in case it's a
    // null-terminated string
    if (unknownField3 != 0x0000) {
        kLogger.warning() << "Parsing SeratoMarkers2CueEntry failed:"
                          << "Bytes 10-11:" << unknownField3 << "!= \"\\0\\0\"";
        return nullptr;
    }

    QString label = zeroTerminatedUtf8StringtoQString(&stream);

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

    SeratoMarkers2CueEntry* pEntry = new SeratoMarkers2CueEntry(index, position, color, label);
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SeratoMarkers2CueEntry" << *pEntry;
    }
    return SeratoMarkers2EntryPointer(pEntry);
}

QByteArray SeratoMarkers2CueEntry::dump() const {
    QByteArray data;
    data.resize(length());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint8>('\x00')
           << m_index
           << m_position
           << static_cast<quint8>('\x00')
           << static_cast<quint8>(qRed(m_color.toQRgb()))
           << static_cast<quint8>(qGreen(m_color.toQRgb()))
           << static_cast<quint8>(qBlue(m_color.toQRgb()))
           << static_cast<quint8>('\x00')
           << static_cast<quint8>('\x00');

    QByteArray labelData = m_label.toUtf8();
    stream.writeRawData(labelData.constData(), labelData.length());
    stream << static_cast<quint8>('\x00'); // terminating null-byte

    return data;
}

quint32 SeratoMarkers2CueEntry::length() const {
    return 13 + m_label.toUtf8().length();
}

SeratoMarkers2EntryPointer SeratoMarkers2LoopEntry::parse(const QByteArray& data) {
    if (data.length() < 21) {
        kLogger.warning() << "Parsing SeratoMarkers2LoopEntry failed:"
                          << "Length" << data.length() << "< 21";
        return nullptr;
    }

    // LOOP entry fields in order of appearance
    quint8 unknownField1;
    quint8 index;
    quint32 startPosition;
    quint32 endPosition;
    quint32 unknownField2;
    quint8 unknownField3;
    quint8 colorRed;
    quint8 colorGreen;
    quint8 colorBlue;
    quint8 unknownField4;
    bool locked;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);

    stream >> unknownField1;
    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (unknownField1 != '\x00') {
        kLogger.warning() << "Parsing SeratoMarkers2LoopEntry failed:"
                          << "Byte 0: " << unknownField1 << "!= '\\0'";
        return nullptr;
    }

    stream >> index >> startPosition >> endPosition >> unknownField2;
    // Unknown field, make sure it contains the expected "default" value
    if (unknownField2 != kLoopUnknownField2ExpectedValue) {
        kLogger.warning() << "Parsing SeratoMarkers2LoopEntry failed:"
                          << "Invalid magic value" << unknownField2
                          << "!=" << kLoopUnknownField2ExpectedValue << "at offset 10";
        return nullptr;
    }

    stream >> unknownField3;
    // Unknown field, make sure it contains the expected "default" value
    if (unknownField3 != kLoopUnknownField3ExpectedValue) {
        kLogger.warning() << "Parsing SeratoMarkers2LoopEntry failed:"
                          << "Invalid magic value" << unknownField3
                          << "!=" << kLoopUnknownField3ExpectedValue << "at offset 14";
        return nullptr;
    }

    stream >> colorRed >> colorGreen >> colorBlue;
    const auto color = SeratoStoredHotcueColor(qRgb(colorRed, colorGreen, colorBlue));

    stream >> unknownField4;
    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (unknownField4 != kLoopUnknownField4ExpectedValue) {
        kLogger.warning() << "Parsing SeratoMarkers2LoopEntry failed:"
                          << "Byte 18:" << unknownField4 << "!=" << kLoopUnknownField4ExpectedValue;
        return nullptr;
    }

    stream >> locked;
    QString label = zeroTerminatedUtf8StringtoQString(&stream);

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

    SeratoMarkers2LoopEntry* pEntry = new SeratoMarkers2LoopEntry(
            index, startPosition, endPosition, color, locked, label);
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SeratoMarkers2LoopEntry" << *pEntry;
    }
    return SeratoMarkers2EntryPointer(pEntry);
}

QByteArray SeratoMarkers2LoopEntry::dump() const {
    QByteArray data;
    data.resize(length());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint8>('\x00')
           << m_index
           << m_startposition
           << m_endposition;

    stream.writeRawData("\xff\xff\xff\xff\x00", 5);

    stream << static_cast<quint8>(qRed(m_color.toQRgb()))
           << static_cast<quint8>(qGreen(m_color.toQRgb()))
           << static_cast<quint8>(qBlue(m_color.toQRgb()))
           << static_cast<quint8>('\x00')
           << static_cast<quint8>(m_locked);

    QByteArray labelData = m_label.toUtf8();
    stream.writeRawData(labelData.constData(), labelData.length());
    stream << static_cast<quint8>('\x00'); // terminating null-byte

    return data;
}

quint32 SeratoMarkers2LoopEntry::length() const {
    return 21 + m_label.toUtf8().length();
}

bool SeratoMarkers2::parse(
        SeratoMarkers2* seratoMarkers2,
        const QByteArray& data,
        taglib::FileType fileType) {
    VERIFY_OR_DEBUG_ASSERT(seratoMarkers2) {
        return false;
    }

    switch (fileType) {
    case taglib::FileType::MPEG:
    case taglib::FileType::AIFF:
        return parseID3(seratoMarkers2, data);
    case taglib::FileType::MP4:
        return parseBase64Encoded(seratoMarkers2, data);
    case taglib::FileType::FLAC:
        return parseFLAC(seratoMarkers2, data);
    case taglib::FileType::OggVorbis:
        return parseCommon(seratoMarkers2, data);
    default:
        return false;
    }
}

bool SeratoMarkers2::parseID3(
        SeratoMarkers2* seratoMarkers2,
        const QByteArray& outerData) {
    if (!outerData.startsWith("\x01\x01")) {
        kLogger.warning() << "Parsing SeratoMarkers2 failed:"
                          << "Unknown outer Serato Markers2 tag version";
        return false;
    }

    if (!parseCommon(seratoMarkers2, outerData.mid(2))) {
        return false;
    }

    seratoMarkers2->setAllocatedSize(outerData.size());
    return true;
}

bool SeratoMarkers2::parseCommon(
        SeratoMarkers2* seratoMarkers2,
        const QByteArray& outerData) {
    const auto data = QByteArray::fromBase64(outerData);

    if (!data.startsWith("\x01\x01")) {
        kLogger.warning() << "Parsing SeratoMarkers2 failed:"
                          << "Unknown inner Serato Markers2 tag version";
        return false;
    }

    QList<std::shared_ptr<SeratoMarkers2Entry>> entries;

    int offset = 2;
    int entryTypeEndPos;
    while ((entryTypeEndPos = data.indexOf('\x00', offset)) >= 0) {
        // Entry Name
        QString entryType(data.mid(offset, entryTypeEndPos - offset));
        offset = entryTypeEndPos + 1;

        if (entryType.isEmpty()) {
            // We reached the end of the markers
            if (offset != data.size()) {
                kLogger.warning() << "Parsing SeratoMarkers2 failed:"
                                  << "Trailing content" << data.mid(offset);
                return false;
            }
            break;
        }

        // Entry Size
        auto entrySize = qFromBigEndian<quint32>(data.mid(offset, 4));
        offset += 4;

        QByteArray entryData = data.mid(offset, entrySize);
        offset += entrySize;

        // Entry Content
        SeratoMarkers2EntryPointer pEntry;
        if (entryType.compare("BPMLOCK") == 0) {
            pEntry = SeratoMarkers2BpmLockEntry::parse(entryData);
        } else if (entryType.compare("COLOR") == 0) {
            pEntry = SeratoMarkers2ColorEntry::parse(entryData);
        } else if (entryType.compare("CUE") == 0) {
            pEntry = SeratoMarkers2CueEntry::parse(entryData);
        } else if (entryType.compare("LOOP") == 0) {
            pEntry = SeratoMarkers2LoopEntry::parse(entryData);
        } else {
            pEntry = SeratoMarkers2EntryPointer(new SeratoMarkers2UnknownEntry(entryType, entryData));
            if (kLogger.traceEnabled()) {
                kLogger.trace() << "SeratoMarkers2UnknownEntry" << *pEntry;
            }
        }

        if (!pEntry) {
            kLogger.warning() << "Parsing SeratoMarkers2 failed:"
                              << "Unable to parse entry of type " << entryType;
            return false;
        }
        entries.append(pEntry);
    }

    seratoMarkers2->setAllocatedSize(outerData.size());
    seratoMarkers2->setEntries(entries);
    return true;
}

//static
bool SeratoMarkers2::parseBase64Encoded(
        SeratoMarkers2* seratoMarkers2,
        const QByteArray& base64EncodedData) {
    const auto decodedData = QByteArray::fromBase64(base64EncodedData);
    if (!decodedData.startsWith(kSeratoMarkers2Base64EncodedPrefix)) {
        kLogger.warning() << "Decoding SeratoMarkers2 from base64 failed:"
                          << "Unexpected prefix"
                          << decodedData.left(kSeratoMarkers2Base64EncodedPrefix.size())
                          << "!="
                          << kSeratoMarkers2Base64EncodedPrefix;
        return false;
    }
    DEBUG_ASSERT(decodedData.size() >= kSeratoMarkers2Base64EncodedPrefix.size());
    if (!parseID3(
                seratoMarkers2,
                decodedData.mid(kSeratoMarkers2Base64EncodedPrefix.size()))) {
        kLogger.warning() << "Parsing base64encoded SeratoMarkers2 failed!";
        return false;
    }

    seratoMarkers2->setAllocatedSize(decodedData.size());
    return true;
}

//static
bool SeratoMarkers2::parseFLAC(
        SeratoMarkers2* seratoMarkers2,
        const QByteArray& base64EncodedData) {
    if (!base64EncodedData.isEmpty() && parseBase64Encoded(seratoMarkers2, base64EncodedData)) {
        seratoMarkers2->setLastBase64ByteFLAC(base64EncodedData.at(base64EncodedData.size() - 1));
        return true;
    };

    return false;
}

QByteArray SeratoMarkers2::dump(taglib::FileType fileType) const {
    switch (fileType) {
    case taglib::FileType::MPEG:
    case taglib::FileType::AIFF:
        return dumpID3();
    case taglib::FileType::MP4:
        return dumpBase64Encoded();
    case taglib::FileType::FLAC:
        return dumpFLAC();
    case taglib::FileType::OggVorbis:
        return dumpCommon();
    default:
        DEBUG_ASSERT(false);
        return {};
    }
}

QByteArray SeratoMarkers2::dumpCommon() const {
    QByteArray data;

    // To reduce disk fragmentation, Serato pre-allocates at least 470 bytes
    // for the "Markers2" tag. Unused bytes are filled with null-bytes.
    // Hence, it's possible to have a valid tag that does not contain actual
    // marker information. The allocated size is set after successfully parsing
    // the tag, so if the tag is valid but does not contain entries we
    // shouldn't delete the tag content.
    if (isEmpty() && getAllocatedSize() == 0) {
        // Return empty QByteArray
        return data;
    }

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint16>(0x0101);

    for (int i = 0; i < m_entries.size(); i++) {
        SeratoMarkers2EntryPointer entry = m_entries.at(i);
        QByteArray entryName = entry->type().toUtf8();
        QByteArray entryData = entry->dump();
        stream.writeRawData(entryName.constData(), entryName.length());
        stream << static_cast<quint8>('\x00') // terminating null-byte
               << static_cast<quint32>(entryData.length());
        stream.writeRawData(entryData.constData(), entryData.length());
    }
    data.append('\0');
    return base64encode(data, true);
}

QByteArray SeratoMarkers2::dumpID3() const {
    if (isEmpty() && getAllocatedSize() == 0) {
        // Return empty QByteArray
        return {};
    }

    QByteArray outerData("\x01\x01", 2);
    outerData.append(dumpCommon());

    int size = getAllocatedSize();
    if (size <= outerData.size()) {
        // TODO: Find out how Serato chooses the allocation sizes
        size = outerData.size() + 1;
        if (size < 470) {
            size = 470;
        }
    }

    return outerData.leftJustified(size, '\0');
}

QList<SeratoMarkers2EntryPointer> SeratoMarkers2::findEntriesByType(
        SeratoMarkers2Entry::TypeId typeId) const {
    QList<SeratoMarkers2EntryPointer> entriesFound;
    for (const auto& pEntry : std::as_const(m_entries)) {
        VERIFY_OR_DEBUG_ASSERT(pEntry) {
            continue;
        }

        if (pEntry->typeId() == typeId) {
            entriesFound.append(pEntry);
        }
    }
    return entriesFound;
}

SeratoMarkers2EntryPointer SeratoMarkers2::findEntryByType(
        SeratoMarkers2Entry::TypeId typeId) const {
    for (const auto& pEntry : std::as_const(m_entries)) {
        VERIFY_OR_DEBUG_ASSERT(pEntry) {
            continue;
        }

        if (pEntry->typeId() == typeId) {
            return pEntry;
        }
    }

    return nullptr;
}

QList<CueInfo> SeratoMarkers2::getCues() const {
    // qDebug() << "Reading cues from 'Serato Markers2' tag data...";

    QList<CueInfo> cueInfos;

    const QList<SeratoMarkers2EntryPointer> cueEntries =
            findEntriesByType(SeratoMarkers2Entry::TypeId::Cue);
    for (const auto& pEntry : cueEntries) {
        VERIFY_OR_DEBUG_ASSERT(pEntry) {
            continue;
        }

        VERIFY_OR_DEBUG_ASSERT(pEntry->typeId() == SeratoMarkers2Entry::TypeId::Cue) {
            continue;
        }

        const auto pCueEntry = std::static_pointer_cast<SeratoMarkers2CueEntry>(pEntry);
        CueInfo cueInfo(
                CueType::HotCue,
                pCueEntry->getPosition(),
                std::nullopt,
                pCueEntry->getIndex(),
                pCueEntry->getLabel(),
                pCueEntry->getColor().toDisplayedColor(),
                CueFlag::None);
        cueInfos.append(cueInfo);
    }

    const QList<SeratoMarkers2EntryPointer> loopEntries =
            findEntriesByType(SeratoMarkers2Entry::TypeId::Loop);
    for (const auto& pEntry : loopEntries) {
        VERIFY_OR_DEBUG_ASSERT(pEntry) {
            continue;
        }

        VERIFY_OR_DEBUG_ASSERT(pEntry->typeId() == SeratoMarkers2Entry::TypeId::Loop) {
            continue;
        }

        const auto pLoopEntry = std::static_pointer_cast<SeratoMarkers2LoopEntry>(pEntry);
        CueInfo loopInfo = CueInfo(
                CueType::Loop,
                pLoopEntry->getStartPosition(),
                pLoopEntry->getEndPosition(),
                pLoopEntry->getIndex(),
                pLoopEntry->getLabel(),
                std::nullopt, // Serato's Loops don't have a color
                pLoopEntry->isLocked() ? CueFlag::Locked : CueFlag::None);
        // TODO: Add support for "locked" loops
        cueInfos.append(loopInfo);
    }

    // TODO: Add support for FLIP entries

    return cueInfos;
}

void SeratoMarkers2::setCues(const QList<CueInfo>& cueInfos) {
    QMap<int, CueInfo> cueMap;
    QMap<int, CueInfo> loopMap;

    for (const CueInfo& cueInfo : std::as_const(cueInfos)) {
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

    QList<SeratoMarkers2EntryPointer> newEntries;

    // Append COLOR entry
    const SeratoMarkers2EntryPointer pColorEntry =
            findEntryByType(SeratoMarkers2Entry::TypeId::Color);
    if (pColorEntry) {
        newEntries.append(pColorEntry);
    }

    // Append CUE entries
    for (auto it = cueMap.constBegin(); it != cueMap.constEnd(); ++it) {
        const CueInfo& cueInfo = it.value();
        auto pEntry = std::make_shared<SeratoMarkers2CueEntry>(
                *cueInfo.getHotCueIndex(),
                *cueInfo.getStartPositionMillis(),
                SeratoStoredHotcueColor::fromDisplayedColor(cueInfo.getColor()),
                cueInfo.getLabel());
        newEntries.append(pEntry);
    }

    // Append LOOP entries
    for (auto it = loopMap.constBegin(); it != loopMap.constEnd(); ++it) {
        const CueInfo& cueInfo = it.value();
        auto pEntry = std::make_shared<SeratoMarkers2LoopEntry>(
                *cueInfo.getHotCueIndex(),
                *cueInfo.getStartPositionMillis(),
                *cueInfo.getEndPositionMillis(),
                SeratoStoredHotcueColor(SeratoStoredColor::kFixedLoopColor),
                cueInfo.isLocked(),
                cueInfo.getLabel());
        newEntries.append(pEntry);
    }

    // Append BPMLOCK entry
    const SeratoMarkers2EntryPointer pBpmLockEntry =
            findEntryByType(SeratoMarkers2Entry::TypeId::BpmLock);
    if (pBpmLockEntry) {
        newEntries.append(pBpmLockEntry);
    }

    // Append all existing entries of unknown type. This assumes that new entry
    // types are appended when Serato decides to add a new type. For FLIP
    // entries, this holds true.
    //
    // Since all entry types are named, it's still possible to parse everything
    // properly even if the assumption above (and hence the order of entries) is
    // wrong, so let's hope that the Serato developers implemented their parser in
    // a robust way.
    newEntries.append(findEntriesByType(SeratoMarkers2Entry::TypeId::Unknown));

    setEntries(std::move(newEntries));
}

QByteArray SeratoMarkers2::dumpBase64Encoded() const {
    QByteArray innerData;

    // To reduce disk fragmentation, Serato pre-allocates at least 470 bytes
    // for the "Markers2" tag. Unused bytes are filled with null-bytes.
    // Hence, it's possible to have a valid tag that does not contain actual
    // marker information. The allocated size is set after successfully parsing
    // the tag, so if the tag is valid but does not contain entries we
    // shouldn't delete the tag content.
    if (isEmpty() && getAllocatedSize() == 0) {
        // Return empty QByteArray
        return innerData;
    }

    QDataStream stream(&innerData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint16>(0x0101);

    for (int i = 0; i < m_entries.size(); i++) {
        SeratoMarkers2EntryPointer entry = m_entries.at(i);
        QByteArray entryName = entry->type().toUtf8();
        QByteArray entryData = entry->dump();
        stream.writeRawData(entryName.constData(), entryName.length());
        stream << static_cast<quint8>(0x00) // terminating null-byte
               << static_cast<quint32>(entryData.length());
        stream.writeRawData(entryData.constData(), entryData.length());
    }
    innerData.append('\0');

    QByteArray outerData;
    outerData.reserve(kSeratoMarkers2Base64EncodedPrefix.size() + 2 + innerData.size());
    outerData += kSeratoMarkers2Base64EncodedPrefix;
    outerData += QByteArray("\x01\x01", 2);
    outerData += base64encode(innerData, true);

    int size = getAllocatedSize();
    if (size <= outerData.size()) {
        // TODO: Find out how Serato chooses the allocation sizes
        size = outerData.size() + 1;
        if (size < 470) {
            size = 470;
        }
    }

    // Add padding and apply the weird double base64encoding
    return base64encode(outerData.leftJustified(size, '\0'), false);
}

QByteArray SeratoMarkers2::dumpFLAC() const {
    QByteArray data = dumpBase64Encoded();

    if (!data.isEmpty()) {
        data[data.size() - 1] = lastBase64ByteFLAC();
    }

    return data;
}

std::optional<SeratoStoredTrackColor> SeratoMarkers2::getTrackColor() const {
    // kLogger.debug() << "Reading track color from 'Serato Markers2' tag data...";

    for (const auto& pEntry : std::as_const(m_entries)) {
        VERIFY_OR_DEBUG_ASSERT(pEntry) {
            continue;
        }

        if (pEntry->typeId() != SeratoMarkers2Entry::TypeId::Color) {
            continue;
        }

        const auto pColorEntry = std::static_pointer_cast<SeratoMarkers2ColorEntry>(pEntry);
        return pColorEntry->getColor();
    }

    return std::nullopt;
}

void SeratoMarkers2::setTrackColor(SeratoStoredTrackColor color) {
    QList<SeratoMarkers2EntryPointer> newEntries;

    // Append COLOR entry
    SeratoMarkers2EntryPointer pEntry = std::make_shared<SeratoMarkers2ColorEntry>(color);
    newEntries.append(pEntry);

    // Append CUE/LOOP entries
    newEntries.append(findEntriesByType(SeratoMarkers2Entry::TypeId::Cue));
    newEntries.append(findEntriesByType(SeratoMarkers2Entry::TypeId::Loop));

    // Append BPMLOCK entry
    const SeratoMarkers2EntryPointer pBpmLockEntry =
            findEntryByType(SeratoMarkers2Entry::TypeId::BpmLock);
    if (pBpmLockEntry) {
        newEntries.append(pBpmLockEntry);
    }

    // Append all existing entries of unknown type. This assumes that new entry
    // types are appended when Serato decides to add a new type. For FLIP
    // entries, this holds true.
    //
    // Since all entry types are named, it's still possible to parse everything
    // properly even if the assumption above (and hence the order of entries) is
    // wrong, so let's hope that the Serato developers implemented their parser in
    // a robust way.
    newEntries.append(findEntriesByType(SeratoMarkers2Entry::TypeId::Unknown));

    setEntries(std::move(newEntries));
}

bool SeratoMarkers2::isBpmLocked() const {
    // kLogger.debug() << "Reading bpmlock state from 'Serato Markers2' tag data...";

    for (const auto& pEntry : std::as_const(m_entries)) {
        VERIFY_OR_DEBUG_ASSERT(pEntry) {
            continue;
        }

        if (pEntry->typeId() != SeratoMarkers2Entry::TypeId::BpmLock) {
            continue;
        }

        const auto pBpmLockEntry = std::static_pointer_cast<SeratoMarkers2BpmLockEntry>(pEntry);
        return pBpmLockEntry->isLocked();
    }

    return false;
}

void SeratoMarkers2::setBpmLocked(bool bpmLocked) {
    QList<SeratoMarkers2EntryPointer> newEntries;

    // Append COLOR entry
    const SeratoMarkers2EntryPointer pColorEntry =
            findEntryByType(SeratoMarkers2Entry::TypeId::Color);
    if (pColorEntry) {
        newEntries.append(pColorEntry);
    }

    // Append CUE/LOOP entries
    newEntries.append(findEntriesByType(SeratoMarkers2Entry::TypeId::Cue));
    newEntries.append(findEntriesByType(SeratoMarkers2Entry::TypeId::Loop));

    // Append BPMLOCK entry
    SeratoMarkers2EntryPointer pEntry = std::make_shared<SeratoMarkers2BpmLockEntry>(bpmLocked);
    newEntries.append(pEntry);

    // Append all existing entries of unknown type. This assumes that new entry
    // types are appended when Serato decides to add a new type. For FLIP
    // entries, this holds true.
    //
    // Since all entry types are named, it's still possible to parse everything
    // properly even if the assumption above (and hence the order of entries) is
    // wrong, so let's hope that the Serato developers implemented their parser in
    // a robust way.
    newEntries.append(findEntriesByType(SeratoMarkers2Entry::TypeId::Unknown));

    setEntries(std::move(newEntries));
}

int SeratoMarkers2::countEntriesByType(SeratoMarkers2Entry::TypeId typeId) const {
    int numEntries = 0;
    for (const auto& pEntry : std::as_const(m_entries)) {
        VERIFY_OR_DEBUG_ASSERT(pEntry) {
            continue;
        }

        if (pEntry->typeId() == typeId) {
            numEntries++;
        }
    }
    return numEntries;
}

} // namespace mixxx
