#include "track/serato/beatgrid.h"

#include <QtEndian>

#include "util/logger.h"

namespace {

mixxx::Logger kLogger("SeratoBeatGrid");
constexpr quint16 kVersion = 0x0100;
constexpr int kEntrySizeID3 = 8;

} // namespace

namespace mixxx {

QByteArray SeratoBeatGridNonTerminalEntry::dumpID3() const {
    QByteArray data;
    data.resize(kEntrySizeID3);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << m_positionMillis
           << m_beatTillNextMarker;
    return data;
}

SeratoBeatGridEntryPointer SeratoBeatGridNonTerminalEntry::parseID3(const QByteArray& data) {
    if (data.length() != kEntrySizeID3) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalEntry failed:"
                          << "Length" << data.length() << "!=" << kEntrySizeID3;
        return nullptr;
    }

    float positionMillis;
    uint32_t beatsTillNextMarker;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream >> positionMillis >> beatsTillNextMarker;

    if (positionMillis < 0) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalEntry failed:"
                          << "Position value" << positionMillis << "is negative";
        return nullptr;
    }

    if (stream.status() != QDataStream::Status::Ok) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalEntry failed:"
                          << "Stream read failed with status" << stream.status();
        return nullptr;
    }

    if (!stream.atEnd()) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalEntry failed:"
                          << "Unexpected trailing data";
        return nullptr;
    }

    SeratoBeatGridEntryPointer pEntry =
            SeratoBeatGridEntryPointer(new SeratoBeatGridNonTerminalEntry(
                    positionMillis,
                    beatsTillNextMarker));
    kLogger.trace() << "SeratoBeatGridNonTerminalEntry" << *pEntry;
    return pEntry;
}

QByteArray SeratoBeatGridTerminalEntry::dumpID3() const {
    QByteArray data;
    data.resize(kEntrySizeID3);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << m_positionMillis
           << m_bpm;
    return data;
}

SeratoBeatGridEntryPointer SeratoBeatGridTerminalEntry::parseID3(const QByteArray& data) {
    if (data.length() != kEntrySizeID3) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalEntry failed:"
                          << "Length" << data.length() << "!=" << kEntrySizeID3;
        return nullptr;
    }

    float positionMillis;
    float bpm;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream >> positionMillis >> bpm;

    if (positionMillis < 0) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalEntry failed:"
                          << "Position value" << positionMillis << "is negative";
        return nullptr;
    }

    if (bpm < 0) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalEntry failed:"
                          << "BPM value" << bpm << "is negative";
        return nullptr;
    }

    if (stream.status() != QDataStream::Status::Ok) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalEntry failed:"
                          << "Stream read failed with status" << stream.status();
        return nullptr;
    }

    if (!stream.atEnd()) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalEntry failed:"
                          << "Unexpected trailing data";
        return nullptr;
    }

    SeratoBeatGridEntryPointer pEntry =
            SeratoBeatGridEntryPointer(new SeratoBeatGridTerminalEntry(
                    positionMillis,
                    bpm));
    kLogger.trace() << "SeratoBeatGridTerminalEntry" << *pEntry;
    return pEntry;
}

// static
bool SeratoBeatGrid::parse(
        SeratoBeatGrid* seratoBeatGrid, const QByteArray& data, taglib::FileType fileType) {
    VERIFY_OR_DEBUG_ASSERT(seratoBeatGrid) {
        return false;
    }

    switch (fileType) {
    case taglib::FileType::MP3:
    case taglib::FileType::AIFF:
        return parseID3(seratoBeatGrid, data);
    default:
        return false;
    }
}

// static
bool SeratoBeatGrid::parseID3(
        SeratoBeatGrid* seratoBeatGrid, const QByteArray& data) {
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    quint16 version;
    stream >> version;
    if (version != kVersion) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Unknown Serato BeatGrid tag version";
        return false;
    }

    quint32 numEntries;
    stream >> numEntries;

    if (numEntries <= 0) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Expected at leat one entry, but found"
                          << numEntries;
        return false;
    }

    char buffer[kEntrySizeID3];
    QList<SeratoBeatGridEntryPointer> entries;

    // Read non-terminal beatgrid markers
    for (quint32 i = 0; i < numEntries - 1; i++) {
        if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "unable to read entry data";
            return false;
        }

        QByteArray entryData = QByteArray(buffer, kEntrySizeID3);
        SeratoBeatGridEntryPointer pEntry =
                SeratoBeatGridEntryPointer(SeratoBeatGridNonTerminalEntry::parseID3(entryData));
        if (!pEntry) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "Unable to parse entry!";
            return false;
        }
        entries.append(pEntry);
    }

    // Read last (terminal) beatgrid entry
    if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "unable to read entry data";
        return false;
    }

    QByteArray entryData = QByteArray(buffer, kEntrySizeID3);
    SeratoBeatGridEntryPointer pEntry =
            SeratoBeatGridEntryPointer(SeratoBeatGridTerminalEntry::parseID3(entryData));
    if (!pEntry) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Unable to parse entry!";
        return false;
    }
    entries.append(pEntry);

    // Read footer
    //
    // FIXME: This byte has caused some headache because I have not the
    // slightest idea what this value could be. Apparently it's random, because
    // it changes even when entering Serato's "Edit Grid" mode and then leaving
    // it immediately without making any changes.
    // For now, we only read it to be able to dump the exact same byte sequence
    // later on.
    quint8 footer;
    stream >> footer;

    if (stream.status() != QDataStream::Status::Ok) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Stream read failed with status" << stream.status();
        return false;
    }

    if (!stream.atEnd()) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Unexpected trailing data";
        return false;
    }
    seratoBeatGrid->setEntries(std::move(entries));
    seratoBeatGrid->setFooter(footer);

    return true;
}

QByteArray SeratoBeatGrid::dump(taglib::FileType fileType) const {
    switch (fileType) {
    case taglib::FileType::MP3:
    case taglib::FileType::AIFF:
        return dumpID3();
    default:
        DEBUG_ASSERT(false);
        return {};
    }
}

QByteArray SeratoBeatGrid::dumpID3() const {
    QByteArray data;
    if (isEmpty()) {
        // Return empty QByteArray
        return data;
    }

    data.resize(sizeof(quint16) + sizeof(quint32) + sizeof(quint8) +
            kEntrySizeID3 * m_entries.size());

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << kVersion << m_entries.size();
    for (const SeratoBeatGridEntryPointer pEntry : m_entries) {
        stream.writeRawData(pEntry->dumpID3(), kEntrySizeID3);
    }
    stream << m_footer;
    return data;
}

} // namespace mixxx
