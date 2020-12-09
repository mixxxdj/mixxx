#include "track/serato/beatgrid.h"

#include <QtEndian>

#include "util/logger.h"

namespace {

mixxx::Logger kLogger("SeratoBeatGrid");
constexpr quint16 kVersion = 0x0100;
constexpr int kMarkerSizeID3 = 8;
constexpr char kSeratoBeatGridBase64EncodedPrefixStr[] =
        "application/octet-stream\0\0Serato BeatGrid";
const QByteArray kSeratoBeatGridBase64EncodedPrefix = QByteArray::fromRawData(
        kSeratoBeatGridBase64EncodedPrefixStr,
        sizeof(kSeratoBeatGridBase64EncodedPrefixStr));

QByteArray base64encode(const QByteArray& data, bool chopPadding) {
    QByteArray dataBase64;

    // Serato inserts a newline char after every 72 bytes of base64-encoded
    // content.  To mirror that behaviour, we can split the data into blocks of
    // 72 bytes * 3/4 = 54 bytes and base64-encode them one at a time.
    int offset = 0;
    while (offset < data.size()) {
        if (offset > 0) {
            // Add newline char after previous block of 54 raw bytes.
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

QByteArray SeratoBeatGridNonTerminalMarker::dumpID3() const {
    QByteArray data;
    data.reserve(kMarkerSizeID3);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << m_positionSecs
           << m_beatsTillNextMarker;
    return data;
}

// static
SeratoBeatGridNonTerminalMarkerPointer
SeratoBeatGridNonTerminalMarker::parseID3(const QByteArray& data) {
    if (data.length() != kMarkerSizeID3) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalMarker failed:"
                          << "Length" << data.length()
                          << "!=" << kMarkerSizeID3;
        return nullptr;
    }

    float positionSecs;
    quint32 beatsTillNextMarker;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream >> positionSecs >> beatsTillNextMarker;

    if (positionSecs < 0) {
        kLogger.warning() << "Parsing SeratoBeatGridNonTerminalMarker failed:"
                          << "Position value" << positionSecs
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
                    positionSecs, beatsTillNextMarker);
    kLogger.trace() << "SeratoBeatGridNonTerminalMarker" << *pMarker;
    return pMarker;
}

QByteArray SeratoBeatGridTerminalMarker::dumpID3() const {
    QByteArray data;
    data.reserve(kMarkerSizeID3);

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << m_positionSecs << m_bpm;
    return data;
}

// static
SeratoBeatGridTerminalMarkerPointer SeratoBeatGridTerminalMarker::parseID3(
        const QByteArray& data) {
    if (data.length() != kMarkerSizeID3) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalMarker failed:"
                          << "Length" << data.length()
                          << "!=" << kMarkerSizeID3;
        return nullptr;
    }

    float positionSecs;
    float bpm;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream >> positionSecs >> bpm;

    if (positionSecs < 0) {
        kLogger.warning() << "Parsing SeratoBeatGridTerminalMarker failed:"
                          << "Position value" << positionSecs
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
            std::make_shared<SeratoBeatGridTerminalMarker>(positionSecs, bpm);
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
    case taglib::FileType::MP4:
    case taglib::FileType::FLAC:
        return parseBase64Encoded(seratoBeatGrid, data);
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
                          << "Expected at least one marker, but found"
                          << numMarkers;
        return false;
    }

    char buffer[kMarkerSizeID3];
    double previousBeatPositionSecs = -1;

    // Read non-terminal beatgrid markers
    QList<SeratoBeatGridNonTerminalMarkerPointer> nonTerminalMarkers;
    for (quint32 i = 0; i < numMarkers - 1; i++) {
        if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "unable to read non-terminal marker data";
            return false;
        }

        QByteArray markerData = QByteArray::fromRawData(buffer, kMarkerSizeID3);
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

        if (pNonTerminalMarker->positionSecs() < 0) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "Non-terminal marker has invalid position"
                              << pNonTerminalMarker->positionSecs()
                              << "< 0";
            return false;
        }

        if (pNonTerminalMarker->positionSecs() <= previousBeatPositionSecs) {
            kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                              << "Non-terminal marker's position"
                              << pNonTerminalMarker->positionSecs()
                              << "must be greater than the previous marker's position"
                              << previousBeatPositionSecs;
            return false;
        }
        previousBeatPositionSecs = pNonTerminalMarker->positionSecs();

        nonTerminalMarkers.append(pNonTerminalMarker);
    }

    // Read last (terminal) beatgrid marker
    if (stream.readRawData(buffer, sizeof(buffer)) != sizeof(buffer)) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "unable to read terminal marker data";
        return false;
    }

    QByteArray markerData = QByteArray::fromRawData(buffer, kMarkerSizeID3);
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

    if (pTerminalMarker->positionSecs() < 0) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Non-terminal marker has invalid position"
                          << pTerminalMarker->positionSecs()
                          << "< 0";
        return false;
    }

    if (pTerminalMarker->positionSecs() <= previousBeatPositionSecs) {
        kLogger.warning() << "Parsing SeratoBeatGrid failed:"
                          << "Terminal marker's position"
                          << pTerminalMarker->positionSecs()
                          << "must be greater than the previous marker's position"
                          << previousBeatPositionSecs;
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

bool SeratoBeatGrid::parseBase64Encoded(
        SeratoBeatGrid* seratoBeatGrid, const QByteArray& base64EncodedData) {
    if (base64EncodedData.isEmpty()) {
        kLogger.warning() << "Decoding SeratoBeatGrid from base64 failed:"
                          << "No data";
        return true;
    }
    char extraBase64Byte = base64EncodedData.at(base64EncodedData.size() - 1);
    const auto decodedData = QByteArray::fromBase64(
            base64EncodedData.left(base64EncodedData.size() - 1));
    if (!decodedData.startsWith(kSeratoBeatGridBase64EncodedPrefix)) {
        kLogger.warning() << "Decoding SeratoBeatGrid from base64 failed:"
                          << "Unexpected prefix"
                          << decodedData.left(kSeratoBeatGridBase64EncodedPrefix.size())
                          << "!="
                          << kSeratoBeatGridBase64EncodedPrefix;
        return false;
    }
    DEBUG_ASSERT(decodedData.size() >= kSeratoBeatGridBase64EncodedPrefix.size());
    if (!parseID3(
                seratoBeatGrid,
                decodedData.mid(kSeratoBeatGridBase64EncodedPrefix.size()))) {
        kLogger.warning() << "Parsing base64encoded SeratoBeatGrid failed!";
        return false;
    }

    seratoBeatGrid->setExtraBase64Byte(extraBase64Byte);

    return true;
}

QByteArray SeratoBeatGrid::dump(taglib::FileType fileType) const {
    switch (fileType) {
    case taglib::FileType::MP3:
    case taglib::FileType::AIFF:
        return dumpID3();
    case taglib::FileType::MP4:
    case taglib::FileType::FLAC:
        return dumpBase64Encoded();
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
    data.reserve(
            sizeof(quint16) + // Version
            sizeof(quint32) + // Number of Markers
            (kMarkerSizeID3 * numMarkers) +
            sizeof(quint8) // Footer
    );

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << kVersion << numMarkers;
    for (const SeratoBeatGridNonTerminalMarkerPointer& pMarker : m_nonTerminalMarkers) {
        stream.writeRawData(pMarker->dumpID3(), kMarkerSizeID3);
    }
    stream.writeRawData(m_pTerminalMarker->dumpID3(), kMarkerSizeID3);
    stream << m_footer;
    return data;
}

QByteArray SeratoBeatGrid::dumpBase64Encoded() const {
    if (isEmpty()) {
        // Return empty QByteArray
        return {};
    }

    QByteArray data = kSeratoBeatGridBase64EncodedPrefix;
    data.append(dumpID3());

    QByteArray base64EncodedData = base64encode(data, false);
    base64EncodedData.append(extraBase64Byte());
    return base64EncodedData;
}

void SeratoBeatGrid::setBeats(BeatsPointer pBeats,
        const audio::StreamInfo& streamInfo,
        double timingOffsetMillis) {
    VERIFY_OR_DEBUG_ASSERT(pBeats) {
        return;
    }

    const double timingOffsetSecs = timingOffsetMillis / 1000;

    // Find at least the track duration in samples. This value might be longer
    // than the actual track, to make sure we get all beats.
    const SINT trackDurationSamples = streamInfo.getSignalInfo().frames2samples(
            static_cast<SINT>(streamInfo.getSignalInfo().secs2frames(
                    std::ceil(streamInfo.getDuration().toDoubleSeconds()))));
    auto pBeatsIterator = pBeats->findBeats(0, trackDurationSamples);

    // This might be null if the track doesn't contain any beats
    if (!pBeatsIterator) {
        setTerminalMarker(nullptr);
        setNonTerminalMarkers({});
        return;
    }

    SINT currentBeatPositionSamples = 0;
    SINT previousBeatPositionSamples = 0;
    double previousDeltaSamples = -1;
    int beatsSinceLastMarker = 0;
    QList<SeratoBeatGridNonTerminalMarkerPointer> nonTerminalMarkers;
    while (pBeatsIterator->hasNext()) {
        previousBeatPositionSamples = currentBeatPositionSamples;
        currentBeatPositionSamples = static_cast<SINT>(pBeatsIterator->next());

        // Calculate the delta between the current beat and the previous beat.
        // If the distance is the same as the distance between the previous
        // beat and the beat before that, we can just increment
        // `beatsSinceLastMarker`. If not, we need to add a new marker.
        const double currentDeltaSamples = currentBeatPositionSamples - previousBeatPositionSamples;
        if (currentDeltaSamples != previousDeltaSamples) {
            // We are adding a new beat marker, therefore we need to update the
            // `beatsSinceLastMarker` variable of the last marker we added.
            if (!nonTerminalMarkers.isEmpty()) {
                DEBUG_ASSERT(beatsSinceLastMarker > 0);
                const auto pNonTerminalMarker =
                        nonTerminalMarkers.at(nonTerminalMarkers.size() - 1);
                DEBUG_ASSERT(pNonTerminalMarker);
                pNonTerminalMarker->setBeatsTillNextMarker(beatsSinceLastMarker);

                // After adding the first marker, the the sample delta won't
                // match, because it compares the distance between beats 1 and
                // two with the distance between the start of the track and the
                // first beat. This special case makes sure that we don't add
                // an unnecessary additional marker that has
                // beatsSinceLastMarker set to 1.
                if (nonTerminalMarkers.size() == 1 && beatsSinceLastMarker == 1) {
                    previousDeltaSamples = currentDeltaSamples;
                    beatsSinceLastMarker++;
                    continue;
                }
            }
            // Don't create a SeratoBeatGridNonTerminalMarker entry for the
            // last beat, this needs to be a terminal marker entry.
            if (pBeatsIterator->hasNext()) {
                const double positionSecs = streamInfo.getSignalInfo().frames2secs(
                                                    streamInfo.getSignalInfo().samples2frames(
                                                            currentBeatPositionSamples)) -
                        timingOffsetSecs;
                nonTerminalMarkers.append(
                        std::make_shared<SeratoBeatGridNonTerminalMarker>(positionSecs, 0));
            }
            beatsSinceLastMarker = 0;
        }
        beatsSinceLastMarker++;
        previousDeltaSamples = currentDeltaSamples;
    }

    // The track has no beats. This isn't possible because the `pBeatsIterator`
    // check above should have caught this case already.
    VERIFY_OR_DEBUG_ASSERT(currentBeatPositionSamples != -1) {
        return;
    }

    // Update the `beatsSinceLastMarker` of the last non-terminal marker we inserted.
    DEBUG_ASSERT(!nonTerminalMarkers.isEmpty());
    DEBUG_ASSERT(beatsSinceLastMarker > 0);
    const auto pNonTerminalMarker = nonTerminalMarkers.at(nonTerminalMarkers.size() - 1);
    DEBUG_ASSERT(pNonTerminalMarker);
    // We need to subtract 1 from `beatsSinceLastMarker`, because at the end of
    // the last iteration the counter is incremented even though we didn't move
    // a beat forwards.
    pNonTerminalMarker->setBeatsTillNextMarker(beatsSinceLastMarker - 1);

    // Finally, create the terminal marker.
    const double positionSecs =
            streamInfo.getSignalInfo().frames2secs(
                    streamInfo.getSignalInfo().samples2frames(
                            currentBeatPositionSamples)) -
            timingOffsetSecs;
    const double bpm = pBeats->getBpmAroundPosition(currentBeatPositionSamples, 1);

    setTerminalMarker(std::make_shared<SeratoBeatGridTerminalMarker>(positionSecs, bpm));
    setNonTerminalMarkers(nonTerminalMarkers);
}

QDebug operator<<(QDebug dbg, const SeratoBeatGridTerminalMarker& arg) {
    return dbg << "SeratoBeatGridTerminalMarker"
               << "PositionSecs =" << arg.positionSecs()
               << "BPM =" << arg.bpm();
}

QDebug operator<<(QDebug dbg, const SeratoBeatGridNonTerminalMarker& arg) {
    return dbg << "SeratoBeatGridNonTerminalMarker"
               << "PositionSecs =" << arg.positionSecs()
               << "BeatTillNextMarker = " << arg.beatsTillNextMarker();
}

QDebug operator<<(QDebug dbg, const SeratoBeatGrid& arg) {
    // TODO: Improve debug output
    return dbg << "number of markers ="
               << (arg.nonTerminalMarkers().length() +
                          (arg.terminalMarker() ? 1 : 0));
}

} // namespace mixxx
