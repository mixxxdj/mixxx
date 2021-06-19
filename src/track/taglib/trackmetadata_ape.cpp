#include "track/taglib/trackmetadata_ape.h"

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

} // anonymous namespace

namespace ape {

bool importCoverImageFromTag(QImage* pCoverArt, const TagLib::APE::Tag& tag) {
    if (!pCoverArt) {
        return false; // nothing to do
    }

    if (tag.itemListMap().contains("COVER ART (FRONT)")) {
        const TagLib::ByteVector nullStringTerminator(1, 0);
        TagLib::ByteVector item =
                tag.itemListMap()["COVER ART (FRONT)"].value();
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

void importTrackMetadataFromTag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    taglib::importTrackMetadataFromTag(
            pTrackMetadata,
            tag);

    // NOTE(uklotzde, 2018-01-28, https://bugs.launchpad.net/mixxx/+bug/1745847)
    // It turns out that the keys for APEv2 tags are case-sensitive and
    // some tag editors seem to write UPPERCASE Vorbis keys instead of
    // the CamelCase APEv2 keys suggested by the Picard Mapping table:
    // https://picard.musicbrainz.org/docs/mappings/

    QString albumArtist;
    if (readItem(tag, "Album Artist", &albumArtist) ||
            readItem(tag, "ALBUM ARTIST", &albumArtist) ||
            readItem(tag, "ALBUMARTIST", &albumArtist)) {
        pTrackMetadata->refAlbumInfo().setArtist(albumArtist);
    }

    QString composer;
    if (readItem(tag, "Composer", &composer) ||
            readItem(tag, "COMPOSER", &composer)) {
        pTrackMetadata->refTrackInfo().setComposer(composer);
    }

    QString grouping;
    if (readItem(tag, "Grouping", &grouping) ||
            readItem(tag, "GROUPING", &grouping)) {
        pTrackMetadata->refTrackInfo().setGrouping(grouping);
    }

    // The release date (ISO 8601 without 'T' separator between date and time)
    // according to the mapping used by MusicBrainz Picard.
    // http://wiki.hydrogenaud.io/index.php?title=APE_date
    // https://picard.musicbrainz.org/docs/mappings
    QString year;
    if (readItem(tag, "Year", &year) ||
            readItem(tag, "YEAR", &year)) {
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
    }
#endif // __EXTRA_METADATA__

    QString bpm;
    if (readItem(tag, "BPM", &bpm)) {
        parseBpm(pTrackMetadata, bpm);
    }

    QString trackGain;
    if (readItem(tag, "REPLAYGAIN_TRACK_GAIN", &trackGain)) {
        parseTrackGain(pTrackMetadata, trackGain);
    }
    QString trackPeak;
    if (readItem(tag, "REPLAYGAIN_TRACK_PEAK", &trackPeak)) {
        parseTrackPeak(pTrackMetadata, trackPeak);
    }

#if defined(__EXTRA_METADATA__)
    QString albumGain;
    if (readItem(tag, "REPLAYGAIN_ALBUM_GAIN", &albumGain)) {
        parseTrackGain(pTrackMetadata, albumGain);
    }
    QString albumPeak;
    if (readItem(tag, "REPLAYGAIN_ALBUM_PEAK", &albumPeak)) {
        parseAlbumPeak(pTrackMetadata, albumPeak);
    }

    QString trackArtistId;
    if (readItem(tag, "MUSICBRAINZ_ARTISTID", &trackArtistId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzArtistId(QUuid(trackArtistId));
    }
    QString trackRecordingId;
    if (readItem(tag, "MUSICBRAINZ_TRACKID", &trackRecordingId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzRecordingId(QUuid(trackRecordingId));
    }
    QString trackReleaseId;
    if (readItem(tag, "MUSICBRAINZ_RELEASETRACKID", &trackReleaseId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzReleaseId(QUuid(trackReleaseId));
    }
    QString trackWorkId;
    if (readItem(tag, "MUSICBRAINZ_WORKID", &trackWorkId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzWorkId(QUuid(trackWorkId));
    }
    QString albumArtistId;
    if (readItem(tag, "MUSICBRAINZ_ALBUMARTISTID", &albumArtistId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzArtistId(QUuid(albumArtistId));
    }
    QString albumReleaseId;
    if (readItem(tag, "MUSICBRAINZ_ALBUMID", &albumReleaseId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseId(QUuid(albumReleaseId));
    }
    QString albumReleaseGroupId;
    if (readItem(tag, "MUSICBRAINZ_RELEASEGROUPID", &albumReleaseGroupId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseGroupId(QUuid(albumReleaseGroupId));
    }

    QString conductor;
    if (readItem(tag, "Conductor", &conductor) ||
            readItem(tag, "CONDUCTOR", &conductor)) {
        pTrackMetadata->refTrackInfo().setConductor(conductor);
    }
    QString isrc;
    if (readItem(tag, "ISRC", &isrc)) {
        pTrackMetadata->refTrackInfo().setISRC(isrc);
    }
    QString language;
    if (readItem(tag, "Language", &language) ||
            readItem(tag, "LANGUAGE", &language)) {
        pTrackMetadata->refTrackInfo().setLanguage(language);
    }
    QString lyricist;
    if (readItem(tag, "Lyricist", &lyricist) ||
            readItem(tag, "LYRICIST", &lyricist)) {
        pTrackMetadata->refTrackInfo().setLyricist(lyricist);
    }
    QString mood;
    if (readItem(tag, "Mood", &mood) ||
            readItem(tag, "MOOD", &mood)) {
        pTrackMetadata->refTrackInfo().setMood(mood);
    }
    QString remixer;
    if (readItem(tag, "MixArtist", &remixer) ||
            readItem(tag, "MIXARTIST", &remixer) ||
            readItem(tag, "REMIXER", &remixer)) {
        pTrackMetadata->refTrackInfo().setRemixer(remixer);
    }
    QString copyright;
    if (readItem(tag, "Copyright", &copyright) ||
            readItem(tag, "COPYRIGHT", &copyright)) {
        pTrackMetadata->refAlbumInfo().setCopyright(copyright);
    }
    QString license;
    if (readItem(tag, "License", &license) ||
            readItem(tag, "LICENSE", &license)) {
        pTrackMetadata->refAlbumInfo().setLicense(license);
    }
    QString recordLabel;
    if (readItem(tag, "Label", &recordLabel) ||
            readItem(tag, "LABEL", &recordLabel)) {
        pTrackMetadata->refAlbumInfo().setRecordLabel(recordLabel);
    }
    QString subtitle;
    if (readItem(tag, "Subtitle", &subtitle) ||
            readItem(tag, "SUBTITLE", &subtitle)) {
        pTrackMetadata->refTrackInfo().setSubtitle(subtitle);
    }
    QString encoder;
    if (readItem(tag, "EncodedBy", &encoder) ||
            readItem(tag, "ENCODEDBY", &encoder)) {
        pTrackMetadata->refTrackInfo().setEncoder(encoder);
    }
    QString encoderSettings;
    if (readItem(tag, "EncoderSettings", &encoderSettings) ||
            readItem(tag, "ENCODERSETTINGS", &encoderSettings)) {
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

    writeItem(pTag, "INITIALKEY", toTString(trackMetadata.getTrackInfo().getKey()));

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
