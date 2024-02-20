#include "track/taglib/trackmetadata_common.h"

#include "track/tracknumbers.h"
#include "util/assert.h"
#include "util/logger.h"

namespace mixxx {

namespace {

Logger kLogger("TagLib");

bool parseReplayGainGain(
        gsl::not_null<ReplayGain*> pReplayGain,
        const QString& dbGain,
        bool resetIfEmpty) {
    if (resetIfEmpty && dbGain.trimmed().isEmpty()) {
        pReplayGain->resetRatio();
        return true;
    }
    bool isValid = false;
    double ratio = ReplayGain::ratioFromString(dbGain, &isValid);
    if (isValid) {
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
    return isValid;
}

bool parseReplayGainPeak(
        gsl::not_null<ReplayGain*> pReplayGain,
        const QString& strPeak,
        bool resetIfEmpty) {
    if (resetIfEmpty && strPeak.trimmed().isEmpty()) {
        pReplayGain->resetPeak();
        return true;
    }
    bool isValid = false;
    const CSAMPLE peak = ReplayGain::peakFromString(strPeak, &isValid);
    if (isValid) {
        pReplayGain->setPeak(peak);
    }
    return isValid;
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
        const QString& sBpm,
        bool resetIfEmpty) {
    DEBUG_ASSERT(pTrackMetadata);
    if (resetIfEmpty && sBpm.trimmed().isEmpty()) {
        pTrackMetadata->refTrackInfo().setBpm(Bpm{});
        return true;
    }
    bool isBpmValid = false;
    const double bpmValue = Bpm::valueFromString(sBpm, &isBpmValid);
    if (isBpmValid) {
        pTrackMetadata->refTrackInfo().setBpm(Bpm(bpmValue));
    }
    return isBpmValid;
}

bool parseTrackGain(
        gsl::not_null<TrackMetadata*> pTrackMetadata,
        const QString& dbGain,
        bool resetIfEmpty) {
    return parseReplayGainGain(pTrackMetadata->refTrackInfo().ptrReplayGain(),
            dbGain,
            resetIfEmpty);
}

bool parseTrackPeak(
        gsl::not_null<TrackMetadata*> pTrackMetadata,
        const QString& strPeak,
        bool resetIfEmpty) {
    return parseReplayGainPeak(pTrackMetadata->refTrackInfo().ptrReplayGain(),
            strPeak,
            resetIfEmpty);
}

#if defined(__EXTRA_METADATA__)
bool parseAlbumGain(
        gsl::not_null<TrackMetadata*> pTrackMetadata,
        const QString& dbGain,
        bool resetIfEmpty) {
    return parseReplayGainGain(pTrackMetadata->refAlbumInfo().ptrReplayGain(),
            dbGain,
            resetIfEmpty);
}

bool parseAlbumPeak(
        gsl::not_null<TrackMetadata*> pTrackMetadata,
        const QString& strPeak,
        bool resetIfEmpty) {
    return parseReplayGainPeak(pTrackMetadata->refAlbumInfo().ptrReplayGain(),
            strPeak,
            resetIfEmpty);
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

bool isMultiValueTagEqual(const TagLib::String& taglibVal, QString mixxxVal) {
    // Taglib 2 uses " / " instead of " " as a multi value separator.
    // We may have read or write with either TagLib 1 or 2.
    QString taglibValStripped = toQString(taglibVal).remove(" /");
    QString mixxxValStripped = mixxxVal.remove(" /");
    return taglibValStripped == mixxxValStripped;
}

void exportTrackMetadataIntoTag(
        TagLib::Tag* pTag,
        const TrackMetadata& trackMetadata,
        WriteTagMask writeMask) {
    DEBUG_ASSERT(pTag); // already validated before

    // The mapping of multi-valued fields in TagLib is not bijective.
    // We don't want to overwrite existing values if the corresponding
    // field has not been modified in Mixxx.
    //
    // See also: <https://github.com/mixxxdj/mixxx/issues/12587>
    if (!isMultiValueTagEqual(pTag->title(), trackMetadata.getTrackInfo().getTitle())) {
        pTag->setTitle(toTString(trackMetadata.getTrackInfo().getTitle()));
    }
    if (!isMultiValueTagEqual(pTag->album(), trackMetadata.getAlbumInfo().getTitle())) {
        pTag->setAlbum(toTString(trackMetadata.getAlbumInfo().getTitle()));
    }
    if (!isMultiValueTagEqual(pTag->artist(), trackMetadata.getTrackInfo().getArtist())) {
        pTag->setArtist(toTString(trackMetadata.getTrackInfo().getArtist()));
    }
    if (!isMultiValueTagEqual(pTag->genre(), trackMetadata.getTrackInfo().getGenre())) {
        pTag->setGenre(toTString(trackMetadata.getTrackInfo().getGenre()));
    }

    // Using setComment() from TagLib::Tag might have undesirable
    // effects if the tag type supports multiple comment fields for
    // different purposes, e.g. ID3v2. In this case setting the
    // comment here should be omitted.
    if (0 == (writeMask & WriteTagFlag::OmitComment)) {
        if (!isMultiValueTagEqual(pTag->comment(), trackMetadata.getTrackInfo().getComment())) {
            pTag->setComment(toTString(trackMetadata.getTrackInfo().getComment()));
        }
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
