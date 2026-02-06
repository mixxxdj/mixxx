#include "track/taglib/trackmetadata_ape.h"

#include <optional>

#include "track/taglib/trackmetadata_common.h"
#include "track/tracknumbers.h"
#include "util/logger.h"

namespace mixxx {

namespace {

Logger kLogger("TagLib");

} // anonymous namespace

namespace taglib {

namespace {

bool readItem(
        const TagLib::APE::Tag& tag,
        const TagLib::String& key,
        QString* pValue = nullptr) {
    const TagLib::APE::ItemListMap::ConstIterator it(
            tag.itemListMap().find(key));
    if (it != tag.itemListMap().end() && !(*it).second.values().isEmpty()) {
        if (pValue) {
            *pValue = toQString(
                    firstNonEmptyStringListItem(
                            (*it).second.values()));
        }
        return true;
    } else {
        return false;
    }
}

void writeItem(
        TagLib::APE::Tag* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (value.isEmpty()) {
        // Purge empty items
        pTag->removeItem(key);
    } else {
        const bool replace = true;
        pTag->addValue(key, value, replace);
    }
}

// FMPS Rating - APE item for cross-application rating compatibility
// https://www.freedesktop.org/wiki/Specifications/free-media-player-specs/
const TagLib::String kItemKeyFMPSRating = "FMPS_Rating";

// Rating conversion functions
// FMPS uses 0.0-1.0 scale, Mixxx uses 0-5 (with 0 meaning unrated)

/// Convert Mixxx rating (0-5) to FMPS rating (0.0-1.0)
double mixxxRatingToFMPS(int rating) {
    if (rating <= 0 || rating > 5) {
        return 0.0;
    }
    return rating / 5.0;
}

/// Convert FMPS rating (0.0-1.0) to Mixxx rating (0-5)
int fmpsRatingToMixxx(double fmps) {
    if (fmps < 0.1) {
        return 0; // Unrated
    } else if (fmps < 0.3) {
        return 1;
    } else if (fmps < 0.5) {
        return 2;
    } else if (fmps < 0.7) {
        return 3;
    } else if (fmps < 0.9) {
        return 4;
    } else {
        return 5;
    }
}

} // anonymous namespace

namespace ape {

std::optional<int> importRatingFromTag(const TagLib::APE::Tag& tag) {
    QString fmpsRatingStr;
    if (readItem(tag, kItemKeyFMPSRating, &fmpsRatingStr) &&
            !fmpsRatingStr.isEmpty()) {
        bool ok = false;
        double fmpsValue = fmpsRatingStr.toDouble(&ok);
        if (ok && fmpsValue >= 0.0 && fmpsValue <= 1.0) {
            int rating = fmpsRatingToMixxx(fmpsValue);
            kLogger.debug()
                    << "Imported FMPS_Rating from APE tag:"
                    << fmpsValue << "->" << rating;
            return rating;
        } else {
            kLogger.warning()
                    << "Invalid FMPS_Rating value in APE tag:"
                    << fmpsRatingStr;
        }
    }

    // No rating found
    return std::nullopt;
}

bool exportRatingIntoTag(
        TagLib::APE::Tag* pTag,
        int rating) {
    DEBUG_ASSERT(pTag);

    // Convert rating to FMPS format and write as APE item
    if (rating > 0 && rating <= 5) {
        double fmpsRating = mixxxRatingToFMPS(rating);
        QString fmpsRatingStr = QString::number(fmpsRating, 'f', 1);
        writeItem(
                pTag,
                kItemKeyFMPSRating,
                toTString(fmpsRatingStr));
        kLogger.debug()
                << "Exported rating to FMPS_Rating APE item:"
                << rating << "->" << fmpsRatingStr;
        return true;
    } else if (rating == 0) {
        // Remove existing FMPS_Rating item if rating is cleared
        pTag->removeItem(kItemKeyFMPSRating);
        kLogger.debug()
                << "Removed FMPS_Rating APE item (rating cleared)";
        return true;
    }

    // Invalid rating
    kLogger.warning()
            << "Invalid rating value for export:" << rating;
    return false;
}

bool importCoverImageFromTag(QImage* pCoverArt, const TagLib::APE::Tag& tag) {
    if (!pCoverArt) {
        return false; // nothing to do
    }

    if (tag.itemListMap().contains("COVER ART (FRONT)")) {
        const TagLib::ByteVector nullStringTerminator(1, 0);
        TagLib::ByteVector item =
                tag.itemListMap()["COVER ART (FRONT)"].binaryData();
        int pos = item.find(nullStringTerminator); // skip the filename
        if (++pos > 0) {
            const TagLib::ByteVector data(item.mid(pos));
            const QImage image(loadImageFromByteVector(data));
            if (image.isNull()) {
                kLogger.warning()
                        << "Failed to load image from APE tag";
            } else {
                *pCoverArt = image; // success
                return true;
            }
        }
    }

    if (kLogger.debugEnabled()) {
        kLogger.debug() << "No cover art found in APE tag";
    }
    return false;
}

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::APE::Tag& tag,
        bool resetMissingTagMetadata) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    taglib::importTrackMetadataFromTag(
            pTrackMetadata,
            tag);

    // NOTE(uklotzde, 2018-01-28, https://github.com/mixxxdj/mixxx/issues/9112)
    // It turns out that the keys for APEv2 tags are case-sensitive and
    // some tag editors seem to write UPPERCASE Vorbis keys instead of
    // the CamelCase APEv2 keys suggested by the Picard Mapping table:
    // https://picard.musicbrainz.org/docs/mappings/

    QString albumArtist;
    if (readItem(tag, "Album Artist", &albumArtist) ||
            readItem(tag, "ALBUM ARTIST", &albumArtist) ||
            readItem(tag, "ALBUMARTIST", &albumArtist) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setArtist(albumArtist);
    }

    QString composer;
    if (readItem(tag, "Composer", &composer) ||
            readItem(tag, "COMPOSER", &composer) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setComposer(composer);
    }

    QString grouping;
    if (readItem(tag, "Grouping", &grouping) ||
            readItem(tag, "GROUPING", &grouping) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setGrouping(grouping);
    }

    // The release date (ISO 8601 without 'T' separator between date and time)
    // according to the mapping used by MusicBrainz Picard.
    // http://wiki.hydrogenaud.io/index.php?title=APE_date
    // https://picard.musicbrainz.org/docs/mappings
    QString year;
    if (readItem(tag, "Year", &year) ||
            readItem(tag, "YEAR", &year) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setYear(year);
    }

    QString trackNumber;
    if (readItem(tag, "Track", &trackNumber) ||
            readItem(tag, "TRACK", &trackNumber)) {
        QString trackTotal;
        TrackNumbers::splitString(
                trackNumber,
                &trackNumber,
                &trackTotal);
        pTrackMetadata->refTrackInfo().setTrackNumber(trackNumber);
        pTrackMetadata->refTrackInfo().setTrackTotal(trackTotal);
    } else if (resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setTrackNumber(QString{});
        pTrackMetadata->refTrackInfo().setTrackTotal(QString{});
    }

#if defined(__EXTRA_METADATA__)
    QString discNumber;
    if (readItem(tag, "Disc", &discNumber) ||
            readItem(tag, "DISC", &discNumber)) {
        QString discTotal;
        TrackNumbers::splitString(
                discNumber,
                &discNumber,
                &discTotal);
        pTrackMetadata->refTrackInfo().setDiscNumber(discNumber);
        pTrackMetadata->refTrackInfo().setDiscTotal(discTotal);
    } else if (resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setDiscNumber(QString{});
        pTrackMetadata->refTrackInfo().setDiscTotal(QString{});
    }
#endif // __EXTRA_METADATA__

    QString bpm;
    if (readItem(tag, "BPM", &bpm) || resetMissingTagMetadata) {
        parseBpm(pTrackMetadata, bpm, resetMissingTagMetadata);
    }

    QString trackGain;
    if (readItem(tag, "REPLAYGAIN_TRACK_GAIN", &trackGain) || resetMissingTagMetadata) {
        parseTrackGain(pTrackMetadata, trackGain, resetMissingTagMetadata);
    }
    QString trackPeak;
    if (readItem(tag, "REPLAYGAIN_TRACK_PEAK", &trackPeak) || resetMissingTagMetadata) {
        parseTrackPeak(pTrackMetadata, trackPeak, resetMissingTagMetadata);
    }

#if defined(__EXTRA_METADATA__)
    QString albumGain;
    if (readItem(tag, "REPLAYGAIN_ALBUM_GAIN", &albumGain) || resetMissingTagMetadata) {
        parseTrackGain(pTrackMetadata, albumGain, resetMissingTagMetadata);
    }
    QString albumPeak;
    if (readItem(tag, "REPLAYGAIN_ALBUM_PEAK", &albumPeak) || resetMissingTagMetadata) {
        parseAlbumPeak(pTrackMetadata, albumPeak, resetMissingTagMetadata);
    }

    QString trackArtistId;
    if (readItem(tag, "MUSICBRAINZ_ARTISTID", &trackArtistId) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzArtistId(QUuid(trackArtistId));
    }
    QString trackRecordingId;
    if (readItem(tag, "MUSICBRAINZ_TRACKID", &trackRecordingId) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzRecordingId(QUuid(trackRecordingId));
    }
    QString trackReleaseId;
    if (readItem(tag, "MUSICBRAINZ_RELEASETRACKID", &trackReleaseId) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzReleaseId(QUuid(trackReleaseId));
    }
    QString trackWorkId;
    if (readItem(tag, "MUSICBRAINZ_WORKID", &trackWorkId) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzWorkId(QUuid(trackWorkId));
    }
    QString albumArtistId;
    if (readItem(tag, "MUSICBRAINZ_ALBUMARTISTID", &albumArtistId) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzArtistId(QUuid(albumArtistId));
    }
    QString albumReleaseId;
    if (readItem(tag, "MUSICBRAINZ_ALBUMID", &albumReleaseId) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseId(QUuid(albumReleaseId));
    }
    QString albumReleaseGroupId;
    if (readItem(tag, "MUSICBRAINZ_RELEASEGROUPID", &albumReleaseGroupId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseGroupId(QUuid(albumReleaseGroupId));
    }

    QString conductor;
    if (readItem(tag, "Conductor", &conductor) ||
            readItem(tag, "CONDUCTOR", &conductor) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setConductor(conductor);
    }
    QString isrc;
    if (readItem(tag, "ISRC", &isrc) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setISRC(isrc);
    }
    QString language;
    if (readItem(tag, "Language", &language) ||
            readItem(tag, "LANGUAGE", &language) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setLanguage(language);
    }
    QString lyricist;
    if (readItem(tag, "Lyricist", &lyricist) ||
            readItem(tag, "LYRICIST", &lyricist) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setLyricist(lyricist);
    }
    QString mood;
    if (readItem(tag, "Mood", &mood) ||
            readItem(tag, "MOOD", &mood) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMood(mood);
    }
    QString remixer;
    if (readItem(tag, "MixArtist", &remixer) ||
            readItem(tag, "MIXARTIST", &remixer) ||
            readItem(tag, "REMIXER", &remixer) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setRemixer(remixer);
    }
    QString copyright;
    if (readItem(tag, "Copyright", &copyright) ||
            readItem(tag, "COPYRIGHT", &copyright) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setCopyright(copyright);
    }
    QString license;
    if (readItem(tag, "License", &license) ||
            readItem(tag, "LICENSE", &license) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setLicense(license);
    }
    QString recordLabel;
    if (readItem(tag, "Label", &recordLabel) ||
            readItem(tag, "LABEL", &recordLabel) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setRecordLabel(recordLabel);
    }
    QString subtitle;
    if (readItem(tag, "Subtitle", &subtitle) ||
            readItem(tag, "SUBTITLE", &subtitle) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setSubtitle(subtitle);
    }
    QString encoder;
    if (readItem(tag, "EncodedBy", &encoder) ||
            readItem(tag, "ENCODEDBY", &encoder) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setEncoder(encoder);
    }
    QString encoderSettings;
    if (readItem(tag, "EncoderSettings", &encoderSettings) ||
            readItem(tag, "ENCODERSETTINGS", &encoderSettings) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setEncoderSettings(encoderSettings);
    }
#endif // __EXTRA_METADATA__
}

bool exportTrackMetadataIntoTag(TagLib::APE::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    taglib::exportTrackMetadataIntoTag(
            pTag,
            trackMetadata,
            WriteTagFlag::OmitTrackNumber | WriteTagFlag::OmitYear);

    // NOTE(uklotzde): Overwrite the numeric track number in the common
    // part of the tag with the custom string from the track metadata
    // (pass-through without any further validation)
    writeItem(pTag, "Track", toTString(TrackNumbers::joinAsString(trackMetadata.getTrackInfo().getTrackNumber(), trackMetadata.getTrackInfo().getTrackTotal())));

    writeItem(pTag, "Year", toTString(trackMetadata.getTrackInfo().getYear()));

    writeItem(pTag, "Album Artist", toTString(trackMetadata.getAlbumInfo().getArtist()));
    writeItem(pTag, "Composer", toTString(trackMetadata.getTrackInfo().getComposer()));
    writeItem(pTag, "Grouping", toTString(trackMetadata.getTrackInfo().getGrouping()));

    writeItem(pTag, "BPM", toTString(formatBpm(trackMetadata)));

    writeItem(pTag, "INITIALKEY", toTString(trackMetadata.getTrackInfo().getKeyText()));

    writeItem(pTag, "REPLAYGAIN_TRACK_GAIN", toTString(formatTrackGain(trackMetadata)));
    writeItem(pTag, "REPLAYGAIN_TRACK_PEAK", toTString(formatTrackPeak(trackMetadata)));

#if defined(__EXTRA_METADATA__)
    auto discNumbers = TrackNumbers::joinAsString(
            trackMetadata.getTrackInfo().getDiscNumber(),
            trackMetadata.getTrackInfo().getDiscTotal());
    writeItem(pTag, "Disc", toTString(discNumbers));

    writeItem(pTag, "REPLAYGAIN_ALBUM_GAIN", toTString(formatAlbumGain(trackMetadata)));
    writeItem(pTag, "REPLAYGAIN_ALBUM_PEAK", toTString(formatAlbumPeak(trackMetadata)));

    writeItem(pTag, "MUSICBRAINZ_ARTISTID", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzArtistId()));
    writeItem(pTag, "MUSICBRAINZ_TRACKID", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzRecordingId()));
    writeItem(pTag, "MUSICBRAINZ_RELEASETRACKID", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzReleaseId()));
    writeItem(pTag, "MUSICBRAINZ_WORKID", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzWorkId()));
    writeItem(pTag, "MUSICBRAINZ_ALBUMARTISTID", uuidToTString(trackMetadata.getAlbumInfo().getMusicBrainzArtistId()));
    writeItem(pTag, "MUSICBRAINZ_ALBUMID", uuidToTString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseId()));
    writeItem(pTag, "MUSICBRAINZ_RELEASEGROUPID", uuidToTString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId()));

    writeItem(pTag, "Conductor", toTString(trackMetadata.getTrackInfo().getConductor()));
    writeItem(pTag, "ISRC", toTString(trackMetadata.getTrackInfo().getISRC()));
    writeItem(pTag, "Language", toTString(trackMetadata.getTrackInfo().getLanguage()));
    writeItem(pTag, "Lyricist", toTString(trackMetadata.getTrackInfo().getLyricist()));
    writeItem(pTag, "Mood", toTString(trackMetadata.getTrackInfo().getMood()));
    writeItem(pTag, "Copyright", toTString(trackMetadata.getAlbumInfo().getCopyright()));
    writeItem(pTag, "LICENSE", toTString(trackMetadata.getAlbumInfo().getLicense()));
    writeItem(pTag, "Label", toTString(trackMetadata.getAlbumInfo().getRecordLabel()));
    writeItem(pTag, "MixArtist", toTString(trackMetadata.getTrackInfo().getRemixer()));
    writeItem(pTag, "Subtitle", toTString(trackMetadata.getTrackInfo().getSubtitle()));
    writeItem(pTag, "EncodedBy", toTString(trackMetadata.getTrackInfo().getEncoder()));
    writeItem(pTag, "EncoderSettings", toTString(trackMetadata.getTrackInfo().getEncoderSettings()));
#endif // __EXTRA_METADATA__

    return true;
}

} // namespace ape

} // namespace taglib

} // namespace mixxx
