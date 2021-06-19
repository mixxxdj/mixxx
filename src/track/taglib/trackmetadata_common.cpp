#include "track/taglib/trackmetadata_common.h"

#include <taglib/tmap.h>

#include "track/tracknumbers.h"
#include "util/assert.h"
#include "util/duration.h"
#include "util/logger.h"

// TagLib has support for length in milliseconds since version 1.10
#define TAGLIB_HAS_LENGTH_IN_MILLISECONDS \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 10))

namespace mixxx {

namespace {

Logger kLogger("TagLib");

bool parseReplayGainGain(
        ReplayGain* pReplayGain,
        const QString& dbGain) {
    DEBUG_ASSERT(pReplayGain);

    bool isRatioValid = false;
    double ratio = ReplayGain::ratioFromString(dbGain, &isRatioValid);
    if (isRatioValid) {
        // Some applications (e.g. Rapid Evolution 3) write a replay gain
        // of 0 dB even if the replay gain is undefined. To be safe we
        // ignore this special value and instead prefer to recalculate
        // the replay gain.
        if (ratio == ReplayGain::kRatio0dB) {
            // special case
            kLogger.info() << "Ignoring possibly undefined gain:" << dbGain;
            ratio = ReplayGain::kRatioUndefined;
        }
        pReplayGain->setRatio(ratio);
    }
    return isRatioValid;
}

bool parseReplayGainPeak(
        ReplayGain* pReplayGain,
        const QString& strPeak) {
    DEBUG_ASSERT(pReplayGain);

    bool isPeakValid = false;
    const CSAMPLE peak = ReplayGain::peakFromString(strPeak, &isPeakValid);
    if (isPeakValid) {
        pReplayGain->setPeak(peak);
    }
    return isPeakValid;
}

} // anonymous namespace

namespace taglib {

QString toQString(
        const TagLib::String& tString) {
    if (tString.isEmpty()) {
        // TagLib::null/isNull() is deprecated so we cannot distinguish
        // between null and empty strings.
        return QString();
    }
    return TStringToQString(tString);
}

TagLib::String toTString(
        const QString& qString) {
    if (qString.isEmpty()) {
        // TagLib::null/isNull() is deprecated so we cannot distinguish
        // between null and empty strings.
        return TagLib::String();
    }
    const QByteArray qba(qString.toUtf8());
    return TagLib::String(qba.constData(), TagLib::String::UTF8);
}

TagLib::String firstNonEmptyStringListItem(
        const TagLib::StringList& strList) {
    for (const auto& str : strList) {
        if (!str.isEmpty()) {
            return str;
        }
    }
    return TagLib::String();
}

bool parseBpm(
        TrackMetadata* pTrackMetadata,
        const QString& sBpm) {
    DEBUG_ASSERT(pTrackMetadata);
    bool isBpmValid = false;
    const double bpmValue = Bpm::valueFromString(sBpm, &isBpmValid);
    if (isBpmValid) {
        pTrackMetadata->refTrackInfo().setBpm(Bpm(bpmValue));
    }
    return isBpmValid;
}

bool parseTrackGain(
        TrackMetadata* pTrackMetadata,
        const QString& dbGain) {
    DEBUG_ASSERT(pTrackMetadata);

    ReplayGain replayGain(pTrackMetadata->getTrackInfo().getReplayGain());
    bool isRatioValid = parseReplayGainGain(&replayGain, dbGain);
    if (isRatioValid) {
        pTrackMetadata->refTrackInfo().setReplayGain(replayGain);
    }
    return isRatioValid;
}

bool parseTrackPeak(
        TrackMetadata* pTrackMetadata,
        const QString& strPeak) {
    DEBUG_ASSERT(pTrackMetadata);

    ReplayGain replayGain(pTrackMetadata->getTrackInfo().getReplayGain());
    bool isPeakValid = parseReplayGainPeak(&replayGain, strPeak);
    if (isPeakValid) {
        pTrackMetadata->refTrackInfo().setReplayGain(replayGain);
    }
    return isPeakValid;
}

#if defined(__EXTRA_METADATA__)
bool parseAlbumGain(
        TrackMetadata* pTrackMetadata,
        const QString& dbGain) {
    DEBUG_ASSERT(pTrackMetadata);

    ReplayGain replayGain(pTrackMetadata->getAlbumInfo().getReplayGain());
    bool isRatioValid = parseReplayGainGain(&replayGain, dbGain);
    if (isRatioValid) {
        pTrackMetadata->refAlbumInfo().setReplayGain(replayGain);
    }
    return isRatioValid;
}

bool parseAlbumPeak(
        TrackMetadata* pTrackMetadata,
        const QString& strPeak) {
    DEBUG_ASSERT(pTrackMetadata);

    ReplayGain replayGain(pTrackMetadata->getAlbumInfo().getReplayGain());
    bool isPeakValid = parseReplayGainPeak(&replayGain, strPeak);
    if (isPeakValid) {
        pTrackMetadata->refAlbumInfo().setReplayGain(replayGain);
    }
    return isPeakValid;
}
#endif // __EXTRA_METADATA__

bool parseSeratoBeatGrid(
        TrackMetadata* pTrackMetadata,
        const QByteArray& data,
        FileType fileType) {
    DEBUG_ASSERT(pTrackMetadata);

    SeratoTags seratoTags(pTrackMetadata->getTrackInfo().getSeratoTags());
    bool isValid = seratoTags.parseBeatGrid(data, fileType);
    if (isValid) {
        pTrackMetadata->refTrackInfo().setSeratoTags(seratoTags);
    }
    return isValid;
}

bool parseSeratoBeatGrid(
        TrackMetadata* pTrackMetadata,
        const TagLib::String& data,
        FileType fileType) {
    const TagLib::ByteVector byteVec =
            data.data(TagLib::String::UTF8);
    return parseSeratoBeatGrid(pTrackMetadata, toQByteArrayRaw(byteVec), fileType);
}

bool parseSeratoMarkers(
        TrackMetadata* pTrackMetadata,
        const QByteArray& data,
        FileType fileType) {
    DEBUG_ASSERT(pTrackMetadata);

    SeratoTags seratoTags(pTrackMetadata->getTrackInfo().getSeratoTags());
    bool isValid = seratoTags.parseMarkers(data, fileType);
    if (isValid) {
        pTrackMetadata->refTrackInfo().setSeratoTags(seratoTags);
    }
    return isValid;
}

bool parseSeratoMarkers(
        TrackMetadata* pTrackMetadata,
        const TagLib::String& data,
        FileType fileType) {
    const TagLib::ByteVector byteVec =
            data.data(TagLib::String::UTF8);
    return parseSeratoMarkers(pTrackMetadata, toQByteArrayRaw(byteVec), fileType);
}

bool parseSeratoMarkers2(
        TrackMetadata* pTrackMetadata,
        const QByteArray& data,
        FileType fileType) {
    DEBUG_ASSERT(pTrackMetadata);

    SeratoTags seratoTags(pTrackMetadata->getTrackInfo().getSeratoTags());
    bool isValid = seratoTags.parseMarkers2(data, fileType);
    if (isValid) {
        pTrackMetadata->refTrackInfo().setSeratoTags(seratoTags);
    }
    return isValid;
}

bool parseSeratoMarkers2(
        TrackMetadata* pTrackMetadata,
        const TagLib::String& data,
        FileType fileType) {
    const TagLib::ByteVector byteVec =
            data.data(TagLib::String::UTF8);
    return parseSeratoMarkers2(pTrackMetadata, toQByteArrayRaw(byteVec), fileType);
}

TagLib::String dumpSeratoBeatGrid(
        const TrackMetadata& trackMetadata,
        FileType fileType) {
    const QByteArray seratoBeatGridData =
            trackMetadata.getTrackInfo().getSeratoTags().dumpBeatGrid(fileType);
    return TagLib::String(
            seratoBeatGridData.constData(),
            TagLib::String::UTF8);
}

TagLib::String dumpSeratoMarkers(
        const TrackMetadata& trackMetadata,
        FileType fileType) {
    const QByteArray seratoMarkersData =
            trackMetadata.getTrackInfo().getSeratoTags().dumpMarkers(fileType);
    return TagLib::String(
            seratoMarkersData.constData(),
            TagLib::String::UTF8);
}

TagLib::String dumpSeratoMarkers2(
        const TrackMetadata& trackMetadata,
        FileType fileType) {
    const QByteArray seratoMarkers2Data =
            trackMetadata.getTrackInfo().getSeratoTags().dumpMarkers2(fileType);
    return TagLib::String(
            seratoMarkers2Data.constData(),
            TagLib::String::UTF8);
}

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::Tag& tag,
        ReadTagMask readMask) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    pTrackMetadata->refTrackInfo().setTitle(toQString(tag.title()));
    pTrackMetadata->refTrackInfo().setArtist(toQString(tag.artist()));
    pTrackMetadata->refTrackInfo().setGenre(toQString(tag.genre()));
    pTrackMetadata->refAlbumInfo().setTitle(toQString(tag.album()));

    if ((readMask & ReadTagFlag::OmitComment) == 0) {
        pTrackMetadata->refTrackInfo().setComment(toQString(tag.comment()));
    }

    int iYear = tag.year();
    if (iYear > 0) {
        pTrackMetadata->refTrackInfo().setYear(QString::number(iYear));
    }

    int iTrack = tag.track();
    if (iTrack > 0) {
        pTrackMetadata->refTrackInfo().setTrackNumber(QString::number(iTrack));
    }
}

void exportTrackMetadataIntoTag(
        TagLib::Tag* pTag,
        const TrackMetadata& trackMetadata,
        WriteTagMask writeMask) {
    DEBUG_ASSERT(pTag); // already validated before

    pTag->setArtist(toTString(trackMetadata.getTrackInfo().getArtist()));
    pTag->setTitle(toTString(trackMetadata.getTrackInfo().getTitle()));
    pTag->setAlbum(toTString(trackMetadata.getAlbumInfo().getTitle()));
    pTag->setGenre(toTString(trackMetadata.getTrackInfo().getGenre()));

    // Using setComment() from TagLib::Tag might have undesirable
    // effects if the tag type supports multiple comment fields for
    // different purposes, e.g. ID3v2. In this case setting the
    // comment here should be omitted.
    if (0 == (writeMask & WriteTagFlag::OmitComment)) {
        pTag->setComment(toTString(trackMetadata.getTrackInfo().getComment()));
    }

    // Specialized write functions for tags derived from Taglib::Tag might
    // be able to write the complete string from trackMetadata.getTrackInfo().getYear()
    // into the corresponding field. In this case parsing the year string
    // here should be omitted.
    if (0 == (writeMask & WriteTagFlag::OmitYear)) {
        // Set the numeric year if available
        const QDate yearDate(
                TrackMetadata::parseDateTime(trackMetadata.getTrackInfo().getYear()).date());
        if (yearDate.isValid()) {
            pTag->setYear(yearDate.year());
        }
    }

    // The numeric track number in TagLib::Tag does not reflect the total
    // number of tracks! Specialized write functions for tags derived from
    // Taglib::Tag might be able to handle both trackMetadata.getTrackInfo().getTrackNumber()
    // and trackMetadata.getTrackInfo().getTrackTotal(). In this case parsing the track
    // number string here is useless and should be omitted.
    if (0 == (writeMask & WriteTagFlag::OmitTrackNumber)) {
        // Set the numeric track number if available
        TrackNumbers parsedTrackNumbers;
        const TrackNumbers::ParseResult parseResult =
                TrackNumbers::parseFromString(trackMetadata.getTrackInfo().getTrackNumber(), &parsedTrackNumbers);
        if (TrackNumbers::ParseResult::VALID == parseResult) {
            pTag->setTrack(parsedTrackNumbers.getActual());
        }
    }
}

} // namespace taglib

} // namespace mixxx
