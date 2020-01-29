#include "track/seratomarkers2.h"

#include <QtEndian>

#include "engine/engine.h"
#include "track/cue.h"
#include "track/track.h"

namespace mixxx {

const QRgb kSeratoDefaultTrackColor = 0xFF9999;

SeratoMarkers2EntryPointer SeratoMarkers2BpmlockEntry::parse(const QByteArray& data) {
    if (data.length() != 1) {
        qWarning() << "Parsing SeratoMarkers2BpmlockEntry failed:"
                   << "Length" << data.length() << "!= 1";
        return nullptr;
    }

    const bool locked = data.at(0);
    SeratoMarkers2BpmlockEntry* pEntry = new SeratoMarkers2BpmlockEntry(locked);
    qDebug() << "SeratoMarkers2BpmlockEntry" << *pEntry;
    return SeratoMarkers2EntryPointer(pEntry);
}

QByteArray SeratoMarkers2BpmlockEntry::data() const {
    QByteArray data;
    data.resize(length());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (quint8)m_locked;

    return data;
}

quint32 SeratoMarkers2BpmlockEntry::length() const {
    return 1;
}

SeratoMarkers2EntryPointer SeratoMarkers2ColorEntry::parse(const QByteArray& data) {
    if (data.length() != 4) {
        qWarning() << "Parsing SeratoMarkers2ColorEntry failed:"
                   << "Length" << data.length() << "!= 4";
        return nullptr;
    }

    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (data.at(0) != '\x00') {
        qWarning() << "Parsing SeratoMarkers2ColorEntry failed:"
                   << "Byte 0: " << data.at(0) << "!= '\\0'";
        return nullptr;
    }

    QColor color(static_cast<quint8>(data.at(1)),
            static_cast<quint8>(data.at(2)),
            static_cast<quint8>(data.at(3)));

    SeratoMarkers2ColorEntry* pEntry = new SeratoMarkers2ColorEntry(color);
    qDebug() << "SeratoMarkers2ColorEntry" << *pEntry;
    return SeratoMarkers2EntryPointer(pEntry);
}

QByteArray SeratoMarkers2ColorEntry::data() const {
    QByteArray data;
    data.resize(length());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (quint8)0
           << (quint8)m_color.red()
           << (quint8)m_color.green()
           << (quint8)m_color.blue();

    return data;
}

quint32 SeratoMarkers2ColorEntry::length() const {
    return 4;
}

SeratoMarkers2EntryPointer SeratoMarkers2CueEntry::parse(const QByteArray& data) {
    if (data.length() < 13) {
        qWarning() << "Parsing SeratoMarkers2CueEntry failed:"
                   << "Length" << data.length() << "< 13";
        return nullptr;
    }

    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (data.at(0) != '\x00') {
        qWarning() << "Parsing SeratoMarkers2CueEntry failed:"
                   << "Byte 0: " << data.at(0) << "!= '\\0'";
        return nullptr;
    }

    const quint8 index = data.at(1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    const auto position = qFromBigEndian<quint32>(data.mid(2, 6));
#else
    const auto position = qFromBigEndian<quint32>(
            reinterpret_cast<const uchar*>(data.mid(2, 6).constData()));
#endif

    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (data.at(6) != '\x00') {
        qWarning() << "Parsing SeratoMarkers2CueEntry failed:"
                   << "Byte 6: " << data.at(6) << "!= '\\0'";
        return nullptr;
    }

    QColor color(static_cast<quint8>(data.at(7)),
            static_cast<quint8>(data.at(8)),
            static_cast<quint8>(data.at(9)));

    // Unknown field(s), make sure it's 0 in case it's a
    // null-terminated string
    if (data.at(10) != '\x00' || data.at(11) != '\x00') {
        qWarning() << "Parsing SeratoMarkers2CueEntry failed:"
                   << "Bytes 10-11:" << data.mid(10, 2) << "!= \"\\0\\0\"";
        return nullptr;
    }

    int labelEndPos = data.indexOf('\x00', 12);
    if (labelEndPos < 0) {
        qWarning() << "Parsing SeratoMarkers2CueEntry failed:"
                   << "Label end position not found";
        return nullptr;
    }
    QString label(data.mid(12, labelEndPos - 12));

    if (data.length() > labelEndPos + 1) {
        qWarning() << "Parsing SeratoMarkers2CueEntry failed:"
                   << "Trailing content" << data.mid(labelEndPos + 1);
        return nullptr;
    }

    SeratoMarkers2CueEntry* pEntry = new SeratoMarkers2CueEntry(index, position, color, label);
    qDebug() << "SeratoMarkers2CueEntry" << *pEntry;
    return SeratoMarkers2EntryPointer(pEntry);
}

QByteArray SeratoMarkers2CueEntry::data() const {
    QByteArray data;
    data.resize(length());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (quint8)0
           << m_index
           << m_position
           << (quint8)0
           << (quint8)m_color.red()
           << (quint8)m_color.green()
           << (quint8)m_color.blue()
           << (quint8)0
           << (quint8)0;

    QByteArray labelData = m_label.toUtf8();
    stream.writeRawData(labelData.constData(), labelData.length());
    stream << (qint8)'\0'; // terminating null-byte

    return data;
}

quint32 SeratoMarkers2CueEntry::length() const {
    return 13 + m_label.toUtf8().length();
}

SeratoMarkers2EntryPointer SeratoMarkers2LoopEntry::parse(const QByteArray& data) {
    if (data.length() < 21) {
        qWarning() << "Parsing SeratoMarkers2LoopEntry failed:"
                   << "Length" << data.length() << "< 21";
        return nullptr;
    }

    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (data.at(0) != '\x00') {
        qWarning() << "Parsing SeratoMarkers2LoopEntry failed:"
                   << "Byte 0: " << data.at(0) << "!= '\\0'";
        return nullptr;
    }

    const quint8 index = data.at(1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    const auto startposition = qFromBigEndian<quint32>(data.mid(2, 6));
    const auto endposition = qFromBigEndian<quint32>(data.mid(6, 10));
#else
    const auto startposition = qFromBigEndian<quint32>(
            reinterpret_cast<const uchar*>(data.mid(2, 6).constData()));
    const auto endposition = qFromBigEndian<quint32>(
            reinterpret_cast<const uchar*>(data.mid(6, 10).constData()));
#endif
    // Unknown field, make sure it contains the expected "default" value
    if (data.at(10) != '\xff' ||
            data.at(11) != '\xff' ||
            data.at(12) != '\xff' ||
            data.at(13) != '\xff' ||
            data.at(14) != '\x00' ||
            data.at(15) != '\x27' ||
            data.at(16) != '\xaa' ||
            data.at(17) != '\xe1') {
        qWarning() << "Parsing SeratoMarkers2LoopEntry failed:"
                   << "Invalid magic value " << data.mid(10, 16);
        return nullptr;
    }

    // Unknown field, make sure it's 0 in case it's a
    // null-terminated string
    if (data.at(18) != '\x00') {
        qWarning() << "Parsing SeratoMarkers2LoopEntry failed:"
                   << "Byte 18:" << data.at(18) << "!= '\\0'";
        return nullptr;
    }

    const bool locked = data.at(19);

    int labelEndPos = data.indexOf('\x00', 20);
    if (labelEndPos < 0) {
        qWarning() << "Parsing SeratoMarkers2LoopEntry failed:"
                   << "Label end position not found";
        return nullptr;
    }
    QString label(data.mid(20, labelEndPos - 20));

    if (data.length() > labelEndPos + 1) {
        qWarning() << "Parsing SeratoMarkers2LoopEntry failed:"
                   << "Trailing content" << data.mid(labelEndPos + 1);
        return nullptr;
    }

    SeratoMarkers2LoopEntry* pEntry = new SeratoMarkers2LoopEntry(index, startposition, endposition, locked, label);
    qDebug() << "SeratoMarkers2LoopEntry" << *pEntry;
    return SeratoMarkers2EntryPointer(pEntry);
}

QByteArray SeratoMarkers2LoopEntry::data() const {
    QByteArray data;
    data.resize(length());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (quint8)0
           << m_index
           << m_startposition
           << m_endposition;

    stream.writeRawData("\xff\xff\xff\xff\x00\x27\xaa\xe1", 8);

    stream << (quint8)0
           << (quint8)m_locked;

    QByteArray labelData = m_label.toUtf8();
    stream.writeRawData(labelData.constData(), labelData.length());
    stream << (qint8)'\0'; // terminating null-byte

    return data;
}

quint32 SeratoMarkers2LoopEntry::length() const {
    return 21 + m_label.toUtf8().length();
}

bool SeratoMarkers2::parse(SeratoMarkers2* seratoMarkers2, const QByteArray& outerData) {
    if (!outerData.startsWith("\x01\x01")) {
        qWarning() << "Parsing SeratoMarkers2 failed:"
                   << "Unknown outer Serato Markers2 tag version";
        return false;
    }

    const auto data = QByteArray::fromBase64(outerData.mid(2));

    if (!data.startsWith("\x01\x01")) {
        qWarning() << "Parsing SeratoMarkers2 failed:"
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
                qWarning() << "Parsing SeratoMarkers2 failed:"
                           << "Trailing content" << data.mid(offset);
                return false;
            }
            break;
        }

        // Entry Size
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        auto entrySize = qFromBigEndian<quint32>(data.mid(offset, 4));
#else
        auto entrySize = qFromBigEndian<quint32>(
                reinterpret_cast<const uchar*>(data.mid(offset, 4).constData()));
#endif
        offset += 4;

        QByteArray entryData = data.mid(offset, entrySize);
        offset += entrySize;

        // Entry Content
        SeratoMarkers2EntryPointer pEntry;
        if (entryType.compare("BPMLOCK") == 0) {
            pEntry = SeratoMarkers2BpmlockEntry::parse(entryData);
        } else if (entryType.compare("COLOR") == 0) {
            pEntry = SeratoMarkers2ColorEntry::parse(entryData);
        } else if (entryType.compare("CUE") == 0) {
            pEntry = SeratoMarkers2CueEntry::parse(entryData);
        } else if (entryType.compare("LOOP") == 0) {
            pEntry = SeratoMarkers2LoopEntry::parse(entryData);
        } else {
            pEntry = SeratoMarkers2EntryPointer(new SeratoMarkers2UnknownEntry(entryType, entryData));
            qDebug() << "SeratoMarkers2UnknownEntry" << *pEntry;
        }

        if (!pEntry) {
            qWarning() << "Parsing SeratoMarkers2 failed:"
                       << "Unable to parse entry of type " << entryType;
            return false;
        }
        entries.append(pEntry);
    }

    seratoMarkers2->setAllocatedSize(outerData.size());
    seratoMarkers2->setEntries(entries);
    return true;
}

QByteArray SeratoMarkers2::data() const {
    QByteArray data("\x01\x01", 2);

    for (int i = 0; i < m_entries.size(); i++) {
        SeratoMarkers2EntryPointer entry = m_entries.at(i);
        quint32 lengthBE = qToBigEndian(entry->length());
        data.append(entry->type().toUtf8());
        data.append('\0');
        data.append((const char*)&lengthBE, 4);
        data.append(entry->data());
    }
    data.append('\0');

    QByteArray outerData("\x01\x01", 2);

    // A newline char is inserted at every 72 bytes of base64-encoded content.
    // Hence, we can split the data into blocks of 72 bytes * 3/4 = 54 bytes
    // and base64-encode them one at a time:
    int offset = 0;
    while (offset < data.size()) {
        if (offset > 0) {
            outerData.append('\n');
        }
        QByteArray block = data.mid(offset, 54);
        outerData.append(block.toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals));
        offset += block.size();

        // In case that the last block would require padding, Serato seems to
        // chop off the last byte of the base64-encoded data
        if (block.size() % 3) {
            outerData.chop(1);
        }
    }

    // Exit early if outerData is empty
    if (outerData.isEmpty()) {
        return outerData;
    }

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

void SeratoMarkers2::syncFromTrackObject(const Track* track) {
    const double sampleRateKhz = track->getSampleRate() / 1000.0;
    const double samples = sampleRateKhz * mixxx::kEngineChannelCount;

    QMutableListIterator<std::shared_ptr<SeratoMarkers2Entry>> i(m_entries);

    // Keep potential unknown entries before the COLOR entry
    while (i.hasNext() && i.peekNext()->typeId() < SeratoMarkers2EntryTypeId::Color) {
        i.next();
    }

    // Add the COLOR entry
    // TODO: Add support for actually reading COLOR entries
    //QColor color = track->getColor();
    QColor color = QColor(kSeratoDefaultTrackColor);
    i.insert(SeratoMarkers2EntryPointer(new SeratoMarkers2ColorEntry(color)));

    // Keep potential unknown entries before the CUE entries
    while (i.hasNext() && i.peekNext()->typeId() < SeratoMarkers2EntryTypeId::Cue) {
        SeratoMarkers2EntryPointer pEntry = i.next();

        // We already added the COLOR entry, remove existing ones
        if (pEntry->typeId() == SeratoMarkers2EntryTypeId::Color) {
            i.remove();
        }
    }

    // Add the CUE entries
    for (const CuePointer& pCue : track->getCuePoints()) {
        if (pCue->getType() != Cue::Type::HotCue) {
            continue;
        }

        quint8 index = pCue->getHotCue();
        quint32 position = pCue->getPosition() / samples;
        QString label = pCue->getLabel();
        QColor color = QColor(0xFFFF0000); // TODO
        i.insert(SeratoMarkers2EntryPointer(new SeratoMarkers2CueEntry(index, position, color, label)));
    }

    // TODO: Add support for LOOP entries here

    // Keep potential unknown entries before the CUE entries
    while (i.hasNext() && i.peekNext()->typeId() < SeratoMarkers2EntryTypeId::Bpmlock) {
        SeratoMarkers2EntryPointer pEntry = i.next();

        // We already added the COLOR entry, remove existing ones
        if (pEntry->typeId() == SeratoMarkers2EntryTypeId::Color || pEntry->typeId() == SeratoMarkers2EntryTypeId::Cue) {
            i.remove();
        }
    }

    // Add the BPMLOCK entry
    bool bBpmLocked = track->isBpmLocked();
    m_entries.append(SeratoMarkers2EntryPointer(new SeratoMarkers2BpmlockEntry(bBpmLocked)));

    // Keep potential unknown entries after BPMLOCK entry
    while (i.hasNext()) {
        SeratoMarkers2EntryPointer pEntry = i.next();

        // We already added the COLOR entry, remove existing ones
        if (pEntry->typeId() == SeratoMarkers2EntryTypeId::Color || pEntry->typeId() == SeratoMarkers2EntryTypeId::Cue || pEntry->typeId() == SeratoMarkers2EntryTypeId::Bpmlock) {
            i.remove();
        }
    }
}

void SeratoMarkers2::syncToTrackObject(Track* track) const {
    QMap<int, const mixxx::SeratoMarkers2CueEntry*> cueMap;
    for (auto& pEntry : m_entries) {
        DEBUG_ASSERT(pEntry);
        switch (pEntry->typeId()) {
        case SeratoMarkers2EntryTypeId::Cue: {
            const mixxx::SeratoMarkers2CueEntry* pCueEntry = static_cast<mixxx::SeratoMarkers2CueEntry*>(pEntry.get());
            cueMap.insert(pCueEntry->getIndex(), pCueEntry);
            break;
        }
        case SeratoMarkers2EntryTypeId::Bpmlock: {
            const mixxx::SeratoMarkers2BpmlockEntry* pBpmlockEntry = static_cast<mixxx::SeratoMarkers2BpmlockEntry*>(pEntry.get());
            track->setBpmLocked(pBpmlockEntry->isLocked());
            break;
        }
        // TODO: Add support for COLOR/LOOP/FLIP
        default:
            break;
        }
    }

    const double sampleRateKhz = track->getSampleRate() / 1000.0;
    const double samples = sampleRateKhz * mixxx::kEngineChannelCount;

    // Update existing hotcues
    for (const CuePointer& pCue : track->getCuePoints()) {
        if (pCue->getType() != Cue::Type::HotCue) {
            continue;
        }

        const mixxx::SeratoMarkers2CueEntry* cueEntry = cueMap.take(pCue->getHotCue());
        if (!cueEntry) {
            qWarning() << "Removed existing hotcue" << pCue->getHotCue() << pCue->getLabel() << "at" << pCue->getPosition();
            track->removeCue(pCue);
            continue;
        }

        pCue->setType(Cue::Type::HotCue);
        pCue->setStartPosition(static_cast<double>(cueEntry->getPosition()) * samples);
        pCue->setLabel(cueEntry->getLabel());
        // TODO: Add support for hotcue colors
        qWarning() << "Updated existing hotcue" << pCue->getHotCue() << pCue->getLabel() << "at" << pCue->getPosition();
    }

    // Add new cues
    for (auto cueEntry : cueMap.values()) {
        DEBUG_ASSERT(cueEntry);
        CuePointer pCue = CuePointer(track->createAndAddCue());

        pCue->setType(Cue::Type::HotCue);
        pCue->setHotCue(cueEntry->getIndex());
        pCue->setStartPosition(static_cast<double>(cueEntry->getPosition()) * samples);
        pCue->setLabel(cueEntry->getLabel());
        // TODO: Add support for hotcue colors
        qWarning() << "Added new hotcue" << pCue->getHotCue() << pCue->getLabel() << "at" << pCue->getPosition();
    }
}

} //namespace mixxx
