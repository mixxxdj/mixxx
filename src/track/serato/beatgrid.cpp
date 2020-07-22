#include "track/serato/beatgrid.h"

#include <QtEndian>

#include "util/logger.h"

namespace {

mixxx::Logger kLogger("SeratoBeatGrid");
constexpr quint16 kVersion = 0x0100;
constexpr int kMarkerSizeID3 = 8;

} // namespace

namespace mixxx {

QByteArray SeratoBeatGridNonTerminalMarker::dumpID3() const {
    QByteArray data;
    data.resize(kMarkerSizeID3);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << m_positionMillis
           << m_beatsTillNextMarker;
    return data;
}

SeratoBeatGridNonTerminalMarkerPointer
SeratoBeatGridNonTerminalMarker::parseID3(const QByteArray& data) {
    if (data.length() != kMarkerSizeID3) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalMarker failed:"
                          << "Length" << data.length()
                          << "!=" << kMarkerSizeID3;
        return nullptr;
    }

    float positionMillis;
    uint32_t beatsTillNextMarker;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream >> positionMillis >> beatsTillNextMarker;

    if (positionMillis < 0) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalMarker failed:"
                          << "Position value" << positionMillis
                          << "is negative";
        return nullptr;
    }

    if (stream.status() != QDataStream::Status::Ok) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalMarker failed:"
                          << "Stream read failed with status"
                          << stream.status();
        return nullptr;
    }

    if (!stream.atEnd()) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalMarker failed:"
                          << "Unexpected trailing data";
        return nullptr;
    }

    SeratoBeatGridNonTerminalMarkerPointer pMarker =
            std::make_shared<SeratoBeatGridNonTerminalMarker>(
                    positionMillis, beatsTillNextMarker);
    kLogger.trace() << "SeratoBeatGridNonTerminalMarker" << *pMarker;
    return pMarker;
}

QByteArray SeratoBeatGridTerminalMarker::dumpID3() const {
    QByteArray data;
    data.resize(kMarkerSizeID3);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << m_positionMillis << m_bpm;
    return data;
}

SeratoBeatGridTerminalMarkerPointer SeratoBeatGridTerminalMarker::parseID3(
        const QByteArray& data) {
    if (data.length() != kMarkerSizeID3) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalMarker failed:"
                          << "Length" << data.length()
                          << "!=" << kMarkerSizeID3;
        return nullptr;
    }

    float positionMillis;
    float bpm;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream >> positionMillis >> bpm;

    if (positionMillis < 0) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalMarker failed:"
                          << "Position value" << positionMillis
                          << "is negative";
        return nullptr;
    }

    if (bpm < 0) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalMarker failed:"
                          << "BPM value" << bpm << "is negative";
        return nullptr;
    }

    if (stream.status() != QDataStream::Status::Ok) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalMarker failed:"
                          << "Stream read failed with status"
                          << stream.status();
        return nullptr;
    }

    if (!stream.atEnd()) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalMarker failed:"
                          << "Unexpected trailing data";
        return nullptr;
    }

    SeratoBeatGridTerminalMarkerPointer pMarker =
            std::make_shared<SeratoBeatGridTerminalMarker>(positionMillis, bpm);
    kLogger.trace() << "SeratoBeatGridTerminalMarker" << *pMarker;
    return pMarker;
}

// static
bool SeratoBeatGrid::parse(SeratoBeatGrid* seratoBeatGrid,
        const QByteArray& data,
        taglib::FileType fileType) {
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

    quint32 numMarkers;
    stream >> numMarkers;

    if (numMarkers <= 0) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Expected at leat one marker, but found"
                          << numMarkers;
        return false;
    }

    char buffer[kMarkerSizeID3];
    double previousBeatPositionMillis = -1;

    // Read non-terminal beatgrid markers
    QList<SeratoBeatGridNonTerminalMarkerPointer> nonTerminalMarkers;
    for (quint32 i = 0; i < numMarkers - 1; i++) {
        if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "unable to read non-terminal marker data";
            return false;
        }

        QByteArray markerData = QByteArray(buffer, kMarkerSizeID3);
        SeratoBeatGridNonTerminalMarkerPointer pNonTerminalMarker =
                SeratoBeatGridNonTerminalMarker::parseID3(markerData);
        if (!pNonTerminalMarker) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "Unable to parse non-terminal marker!";
            return false;
        }

        if (pNonTerminalMarker->beatsTillNextMarker() <= 0) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "Non-terminal marker's beatsTillNextMarker"
                              << pNonTerminalMarker->beatsTillNextMarker()
                              << "must be greater than 0";
            return false;
        }

        if (pNonTerminalMarker->positionMillis() < 0) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "Non-terminal marker has invalid position"
                              << pNonTerminalMarker->positionMillis()
                              << "< 0";
            return false;
        }

        if (pNonTerminalMarker->positionMillis() <= previousBeatPositionMillis) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "Non-terminal marker's position"
                              << pNonTerminalMarker->positionMillis()
                              << "must be greater than the previous marker's position"
                              << previousBeatPositionMillis;
            return false;
        }
        previousBeatPositionMillis = pNonTerminalMarker->positionMillis();

        nonTerminalMarkers.append(pNonTerminalMarker);
    }

    // Read last (terminal) beatgrid marker
    if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "unable to read terminal marker data";
        return false;
    }

    QByteArray markerData = QByteArray(buffer, kMarkerSizeID3);
    SeratoBeatGridTerminalMarkerPointer pTerminalMarker =
            SeratoBeatGridTerminalMarker::parseID3(markerData);
    if (!pTerminalMarker) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Unable to parse terminal marker!";
        return false;
    }

    if (pTerminalMarker->bpm() <= 0) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Terminal marker's BPM"
                          << pTerminalMarker->bpm()
                          << "must be greater than 0";
        return false;
    }

    if (pTerminalMarker->positionMillis() < 0) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Non-terminal marker has invalid position"
                          << pTerminalMarker->positionMillis()
                          << "< 0";
        return false;
    }

    if (pTerminalMarker->positionMillis() <= previousBeatPositionMillis) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Terminal marker's position"
                          << pTerminalMarker->positionMillis()
                          << "must be greater than the previous marker's position"
                          << previousBeatPositionMillis;
        return false;
    }

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
    seratoBeatGrid->setNonTerminalMarkers(std::move(nonTerminalMarkers));
    seratoBeatGrid->setTerminalMarker(pTerminalMarker);
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
    if (isEmpty() || !m_pTerminalMarker) {
        // Return empty QByteArray
        return data;
    }

    quint32 numMarkers = m_nonTerminalMarkers.size() + 1;
    data.resize(
            sizeof(quint16) + // Version
            sizeof(quint32) + // Number of Markers
            (kMarkerSizeID3 * numMarkers) +
            sizeof(quint8) // Footer
    );

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << kVersion << numMarkers;
    for (const SeratoBeatGridNonTerminalMarkerPointer pMarker : m_nonTerminalMarkers) {
        stream.writeRawData(pMarker->dumpID3(), kMarkerSizeID3);
    }
    stream.writeRawData(m_pTerminalMarker->dumpID3(), kMarkerSizeID3);
    stream << m_footer;
    return data;
}

} // namespace mixxx
