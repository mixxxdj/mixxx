#if defined(_MSC_VER)
#pragma warning(push)
// https://github.com/taglib/taglib/issues/1185
// warning C4251: 'TagLib::FileName::m_wname': class
// 'std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>'
// needs to have dll-interface to be used by clients of class 'TagLib::FileName'
#pragma warning(disable : 4251)
#endif

#include "track/taglib/trackmetadata_xiph.h"

#include <flacpicture.h>

#include <array>

#include "track/taglib/trackmetadata_common.h"
#include "track/tracknumbers.h"
#include "util/logger.h"

namespace mixxx {

namespace {

Logger kLogger("TagLib");

} // anonymous namespace

namespace taglib {

namespace {

// Preferred picture types for cover art sorted by priority
const std::array<TagLib::FLAC::Picture::Type, 4> kPreferredPictureTypes{{
        TagLib::FLAC::Picture::FrontCover,   // Front cover image of the album
        TagLib::FLAC::Picture::Media,        // Image from the album itself
        TagLib::FLAC::Picture::Illustration, // Illustration related to the track
        TagLib::FLAC::Picture::Other,
}};

const TagLib::String kCommentFieldKeySeratoBeatGrid = "SERATO_BEATGRID";
const TagLib::String kCommentFieldKeySeratoMarkers2FLAC = "SERATO_MARKERS_V2";
const TagLib::String kCommentFieldKeySeratoMarkers2Ogg = "SERATO_MARKERS2";

bool readCommentField(
        const TagLib::Ogg::XiphComment& tag,
        const TagLib::String& key,
        TagLib::String* pValue = nullptr) {
    const TagLib::Ogg::FieldListMap::ConstIterator it(
            tag.fieldListMap().find(key));
    if (it == tag.fieldListMap().end()) {
        return false;
    }
    if (pValue) {
        *pValue = firstNonEmptyStringListItem(
                (*it).second);
    }
    return true;
}

bool readCommentField(
        const TagLib::Ogg::XiphComment& tag,
        const TagLib::String& key,
        QString* pValue) {
    if (!pValue) {
        return readCommentField(tag, key);
    }
    TagLib::String value;
    if (!readCommentField(tag, key, &value)) {
        return false;
    }
    *pValue = toQString(value);
    return true;
}

inline bool hasCommentField(
        const TagLib::Ogg::XiphComment& tag,
        const TagLib::String& key) {
    return readCommentField(tag, key);
}

// Unconditionally write the field
void writeCommentField(
        TagLib::Ogg::XiphComment* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (value.isEmpty()) {
        // Purge empty fields
        pTag->removeFields(key);
    } else {
        const bool replace = true;
        pTag->addField(key, value, replace);
    }
}

// Conditionally write the field if it already exists
void updateCommentField(
        TagLib::Ogg::XiphComment* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (hasCommentField(*pTag, key)) {
        writeCommentField(pTag, key, value);
    }
}

inline QImage loadImageFromPicture(
        const TagLib::FLAC::Picture& picture) {
    return loadImageFromByteVector(picture.data(), picture.mimeType().toCString());
}

bool parseBase64EncodedPicture(
        TagLib::FLAC::Picture* pPicture,
        const TagLib::String& base64Encoded) {
    DEBUG_ASSERT(pPicture);
    const QByteArray decodedData(QByteArray::fromBase64(base64Encoded.toCString()));
    const TagLib::ByteVector rawData(decodedData.data(), decodedData.size());
    TagLib::FLAC::Picture picture;
    return pPicture->parse(rawData);
}

inline QImage parseBase64EncodedImage(
        const TagLib::String& base64Encoded) {
    const QByteArray decodedData(QByteArray::fromBase64(base64Encoded.toCString()));
    return QImage::fromData(decodedData);
}

} // anonymous namespace

namespace xiph {

QImage importCoverImageFromPictureList(
        const TagLib::List<TagLib::FLAC::Picture*>& pictures) {
    if (pictures.isEmpty()) {
        if (kLogger.debugEnabled()) {
            kLogger.debug() << "VorbisComment picture list is empty";
        }
        return QImage();
    }

    for (const auto coverArtType : kPreferredPictureTypes) {
        for (auto* const pPicture : pictures) {
            DEBUG_ASSERT(pPicture); // trust TagLib
            if (pPicture->type() == coverArtType) {
                const QImage image(loadImageFromPicture(*pPicture));
                if (image.isNull()) {
                    kLogger.warning()
                            << "Failed to load image from VorbisComment picture of type"
                            << pPicture->type();
                    continue;
                } else {
                    return image; // success
                }
            }
        }
    }

    // Fallback: No best match -> Create image from first loadable picture of any type
    for (auto* const pPicture : pictures) {
        DEBUG_ASSERT(pPicture); // trust TagLib
        const QImage image(loadImageFromPicture(*pPicture));
        if (image.isNull()) {
            kLogger.warning()
                    << "Failed to load image from VorbisComment picture of type"
                    << pPicture->type();
            continue;
        } else {
            return image; // success
        }
    }

    kLogger.warning()
            << "Failed to load cover art image from VorbisComment pictures";
    return QImage();
}

bool importCoverImageFromTag(
        QImage* pCoverArt,
        TagLib::Ogg::XiphComment& tag) {
    if (!pCoverArt) {
        return false; // nothing to do
    }

    const QImage image =
            importCoverImageFromPictureList(tag.pictureList());
    if (!image.isNull()) {
        *pCoverArt = image;
        return false; // done
    }

    // NOTE(uklotzde, 2016-07-13): Legacy code for parsing cover art (part 1)
    //
    // The following code is needed for TagLib versions <= 1.10 and as a workaround
    // for an incompatibility between TagLib 1.11 and puddletag 1.1.1.
    //
    // puddletag 1.1.1 seems to generate an incompatible METADATA_BLOCK_PICTURE
    // field that is not recognized by XiphComment::pictureList() by TagLib 1.11.
    // In this case XiphComment::pictureList() returns an empty list while the
    // raw data of the pictures can still be found in XiphComment::fieldListMap().
    if (tag.fieldListMap().contains("METADATA_BLOCK_PICTURE")) {
        // https://wiki.xiph.org/VorbisComment#METADATA_BLOCK_PICTURE
        const TagLib::StringList& base64EncodedList =
                tag.fieldListMap()["METADATA_BLOCK_PICTURE"];
        if (!base64EncodedList.isEmpty()) {
            kLogger.warning()
                    << "Taking legacy code path for reading cover art from VorbisComment field METADATA_BLOCK_PICTURE";
        }
        for (const auto& base64Encoded : base64EncodedList) {
            TagLib::FLAC::Picture picture;
            if (parseBase64EncodedPicture(&picture, base64Encoded)) {
                const QImage image(loadImageFromPicture(picture));
                if (image.isNull()) {
                    kLogger.warning()
                            << "Failed to load image from VorbisComment picture of type"
                            << picture.type();
                    continue;
                } else {
                    *pCoverArt = image;
                    return true; // done
                }
            } else {
                kLogger.warning()
                        << "Failed to parse picture from VorbisComment metadata block";
                continue;
            }
        }
    }

    // NOTE(uklotzde, 2016-07-13): Legacy code for parsing cover art (part 2)
    //
    // The unofficial COVERART field in a VorbisComment tag is deprecated:
    // https://wiki.xiph.org/VorbisComment#Unofficial_COVERART_field_.28deprecated.29
    if (tag.fieldListMap().contains("COVERART")) {
        const TagLib::StringList& base64EncodedList =
                tag.fieldListMap()["COVERART"];
        if (!base64EncodedList.isEmpty()) {
            kLogger.warning()
                    << "Fallback: Trying to parse image from deprecated VorbisComment field COVERART";
        }
        for (const auto& base64Encoded : base64EncodedList) {
            const QImage image(parseBase64EncodedImage(base64Encoded));
            if (image.isNull()) {
                kLogger.warning()
                        << "Failed to parse image from deprecated VorbisComment field COVERART";
                continue;
            } else {
                *pCoverArt = image;
                return true; // done
            }
        }
    }

    if (kLogger.debugEnabled()) {
        kLogger.debug() << "No cover art found in VorbisComment tag";
    }
    return false;
}

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag,
        FileType fileType,
        bool resetMissingTagMetadata) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    // Omit to read comments with the default implementation provided
    // by TagLib. The implementation is inconsistent with the handling
    // proposed by MusicBrainz (see below).
    taglib::importTrackMetadataFromTag(
            pTrackMetadata,
            tag,
            ReadTagFlag::OmitComment);

    // The original specification only defines a "DESCRIPTION" field,
    // while MusicBrainz recommends to use "COMMENT". Mixxx follows
    // MusicBrainz.
    // http://www.xiph.org/vorbis/doc/v-comment.html
    // https://picard.musicbrainz.org/docs/mappings
    //
    // We are not relying on  TagLib (1.11.1) with a somehow inconsistent
    // handling. It prefers "DESCRIPTION" for reading, but adds a "COMMENT"
    // field upon writing when no "DESCRIPTION" field exists.
    QString comment;
    if ((readCommentField(tag, "COMMENT", &comment) && !comment.isEmpty()) ||
            // Fallback to the the original "DESCRIPTION" field only if the
            // "COMMENT" field is either missing or empty
            (readCommentField(tag, "DESCRIPTION", &comment) && !comment.isEmpty()) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setComment(comment);
    }

    QString albumArtist;
    if (readCommentField(tag, "ALBUMARTIST", &albumArtist) ||      // recommended field
            readCommentField(tag, "ALBUM_ARTIST", &albumArtist) || // alternative field (with underscore character)
            readCommentField(tag, "ALBUM ARTIST", &albumArtist) || // alternative field (with space character)
            readCommentField(tag, "ENSEMBLE", &albumArtist) ||     // alternative field
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setArtist(albumArtist);
    }

    QString composer;
    if (readCommentField(tag, "COMPOSER", &composer) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setComposer(composer);
    }

    QString grouping;
    if (readCommentField(tag, "GROUPING", &grouping) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setGrouping(grouping);
    }

    QString trackNumber;
    if (readCommentField(tag, "TRACKNUMBER", &trackNumber)) {
        QString trackTotal;
        // Split the string, because some applications might decide
        // to store "<trackNumber>/<trackTotal>" in "TRACKNUMBER"
        // even if this is not recommended.
        TrackNumbers::splitString(
                trackNumber,
                &trackNumber,
                &trackTotal);
        if (!readCommentField(tag, "TRACKTOTAL", &trackTotal)) {     // recommended field
            (void)readCommentField(tag, "TOTALTRACKS", &trackTotal); // alternative field
        }
        pTrackMetadata->refTrackInfo().setTrackNumber(trackNumber);
        pTrackMetadata->refTrackInfo().setTrackTotal(trackTotal);
    } else if (resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setTrackNumber(QString{});
        pTrackMetadata->refTrackInfo().setTrackTotal(QString{});
    }

#if defined(__EXTRA_METADATA__)
    QString discNumber;
    if (readCommentField(tag, "DISCNUMBER", &discNumber)) {
        QString discTotal;
        // Split the string, because some applications might decide
        // to store "<discNumber>/<discTotal>" in "DISCNUMBER"
        // even if this is not recommended.
        TrackNumbers::splitString(
                discNumber,
                &discNumber,
                &discTotal);
        if (!readCommentField(tag, "DISCTOTAL", &discTotal)) {     // recommended field
            (void)readCommentField(tag, "TOTALDISCS", &discTotal); // alternative field
        }
        pTrackMetadata->refTrackInfo().setDiscNumber(discNumber);
        pTrackMetadata->refTrackInfo().setDiscTotal(discTotal);
    } else if (resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setDiscNumber(QString{});
        pTrackMetadata->refTrackInfo().setDiscTotal(QString{});
    }
#endif // __EXTRA_METADATA__

    // The release date formatted according to ISO 8601. Might
    // be followed by a space character and arbitrary text.
    // http://age.hobba.nl/audio/mirroredpages/ogg-tagging.html
    QString date;
    if (readCommentField(tag, "DATE", &date) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setYear(date);
    }

    // MusicBrainz recommends "BPM": https://picard.musicbrainz.org/docs/mappings
    // Mixxx (<= 2.0) favored "TEMPO": https://www.mixxx.org/wiki/doku.php/library_metadata_rewrite_using_taglib
    QString bpm;
    if (readCommentField(tag, "BPM", &bpm) ||
            readCommentField(tag, "TEMPO", &bpm) ||
            resetMissingTagMetadata) {
        parseBpm(pTrackMetadata, bpm, resetMissingTagMetadata);
    }

    // Reading key code information
    // Unlike, ID3 tags, there's no standard or recommendation on how to store 'key' code
    //
    // Luckily, there are only a few tools for that, e.g., Rapid Evolution (RE).
    // Assuming no distinction between start and end key, RE uses a "INITIALKEY"
    // or a "KEY" vorbis comment.
    QString keyText;
    if (readCommentField(tag, "INITIALKEY", &keyText) || // recommended field
            readCommentField(tag, "KEY", &keyText) ||    // alternative field
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setKeyText(keyText);
    }

    // Only read track gain (not album gain)
    QString trackGain;
    if (readCommentField(tag, "REPLAYGAIN_TRACK_GAIN", &trackGain) || resetMissingTagMetadata) {
        parseTrackGain(pTrackMetadata, trackGain, resetMissingTagMetadata);
    }
    QString trackPeak;
    if (readCommentField(tag, "REPLAYGAIN_TRACK_PEAK", &trackPeak) || resetMissingTagMetadata) {
        parseTrackPeak(pTrackMetadata, trackPeak, resetMissingTagMetadata);
    }

#if defined(__EXTRA_METADATA__)
    QString albumGain;
    if (readCommentField(tag, "REPLAYGAIN_ALBUM_GAIN", &albumGain) || resetMissingTagMetadata) {
        parseAlbumGain(pTrackMetadata, albumGain, resetMissingTagMetadata);
    }
    QString albumPeak;
    if (readCommentField(tag, "REPLAYGAIN_ALBUM_PEAK", &albumPeak) || resetMissingTagMetadata) {
        parseAlbumPeak(pTrackMetadata, albumPeak, resetMissingTagMetadata);
    }

    QString trackArtistId;
    if (readCommentField(tag, "MUSICBRAINZ_ARTISTID", &trackArtistId) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzArtistId(QUuid(trackArtistId));
    }
    QString trackRecordingId;
    if (readCommentField(tag, "MUSICBRAINZ_TRACKID", &trackRecordingId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzRecordingId(QUuid(trackRecordingId));
    }
    QString trackReleaseId;
    if (readCommentField(tag, "MUSICBRAINZ_RELEASETRACKID", &trackReleaseId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzReleaseId(QUuid(trackReleaseId));
    }
    QString trackWorkId;
    if (readCommentField(tag, "MUSICBRAINZ_WORKID", &trackWorkId) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzWorkId(QUuid(trackWorkId));
    }
    QString albumArtistId;
    if (readCommentField(tag, "MUSICBRAINZ_ALBUMARTISTID", &albumArtistId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzArtistId(QUuid(albumArtistId));
    }
    QString albumReleaseId;
    if (readCommentField(tag, "MUSICBRAINZ_ALBUMID", &albumReleaseId) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseId(QUuid(albumReleaseId));
    }
    QString albumReleaseGroupId;
    if (readCommentField(
                tag, "MUSICBRAINZ_RELEASEGROUPID", &albumReleaseGroupId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseGroupId(QUuid(albumReleaseGroupId));
    }

    QString conductor;
    if (readCommentField(tag, "CONDUCTOR", &conductor) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setConductor(conductor);
    }
    QString isrc;
    if (readCommentField(tag, "ISRC", &isrc) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setISRC(isrc);
    }
    QString language;
    if (readCommentField(tag, "LANGUAGE", &language) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setLanguage(language);
    }
    QString lyricist;
    if (readCommentField(tag, "LYRICIST", &lyricist) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setLyricist(lyricist);
    }
    QString mood;
    if (readCommentField(tag, "MOOD", &mood) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMood(mood);
    }
    QString copyright;
    if (readCommentField(tag, "COPYRIGHT", &copyright) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setCopyright(copyright);
    }
    QString license;
    if (readCommentField(tag, "LICENSE", &license) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setLicense(license);
    }
    QString recordLabel;
    if (readCommentField(tag, "LABEL", &recordLabel) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setRecordLabel(recordLabel);
    }
    QString remixer;
    if (readCommentField(tag, "REMIXER", &remixer) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setRemixer(remixer);
    }
    QString subtitle;
    if (readCommentField(tag, "SUBTITLE", &subtitle) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setSubtitle(subtitle);
    }
    QString encoder;
    if (readCommentField(tag, "ENCODEDBY", &encoder) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setEncoder(encoder);
    }
    QString encoderSettings;
    if (readCommentField(tag, "ENCODERSETTINGS", &encoderSettings) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setEncoderSettings(encoderSettings);
    }
#endif // __EXTRA_METADATA__

    // Serato tags
    //
    // FIXME: We're only parsing FLAC tags for now, since the Ogg format is
    // different we don't support it yet.
    if (fileType == FileType::FLAC) {
        TagLib::String seratoBeatGridData;
        if (readCommentField(tag,
                    kCommentFieldKeySeratoBeatGrid,
                    &seratoBeatGridData)) {
            parseSeratoBeatGrid(
                    pTrackMetadata,
                    seratoBeatGridData,
                    fileType);
        }

        TagLib::String seratoMarkers2Data;
        if (readCommentField(tag,
                    kCommentFieldKeySeratoMarkers2FLAC,
                    &seratoMarkers2Data)) {
            parseSeratoMarkers2(
                    pTrackMetadata,
                    seratoMarkers2Data,
                    fileType);
        }
    }
}

bool exportTrackMetadataIntoTag(
        TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata,
        FileType fileType) {
    if (!pTag) {
        return false;
    }

    taglib::exportTrackMetadataIntoTag(
            pTag,
            trackMetadata,
            WriteTagFlag::OmitTrackNumber | WriteTagFlag::OmitYear | WriteTagFlag::OmitComment);

    // The original specification only defines a "DESCRIPTION" field,
    // while MusicBrainz recommends to use "COMMENT". Mixxx follows
    // MusicBrainz.
    // http://www.xiph.org/vorbis/doc/v-comment.html
    // https://picard.musicbrainz.org/docs/mappings
    if (hasCommentField(*pTag, "COMMENT") || !hasCommentField(*pTag, "DESCRIPTION")) {
        // MusicBrainz-style
        writeCommentField(pTag, "COMMENT", toTString(trackMetadata.getTrackInfo().getComment()));
    } else {
        // Preserve and update the "DESCRIPTION" field only if it already exists
        DEBUG_ASSERT(hasCommentField(*pTag, "DESCRIPTION"));
        writeCommentField(pTag, "DESCRIPTION", toTString(trackMetadata.getTrackInfo().getComment()));
    }

    // Write unambiguous fields
    writeCommentField(pTag, "DATE", toTString(trackMetadata.getTrackInfo().getYear()));
    writeCommentField(pTag, "COMPOSER", toTString(trackMetadata.getTrackInfo().getComposer()));
    writeCommentField(pTag, "GROUPING", toTString(trackMetadata.getTrackInfo().getGrouping()));
    writeCommentField(pTag, "TRACKNUMBER", toTString(trackMetadata.getTrackInfo().getTrackNumber()));

    // According to https://wiki.xiph.org/Field_names "TRACKTOTAL" is
    // the proposed field name, but some applications use "TOTALTRACKS".
    const TagLib::String trackTotal(
            toTString(trackMetadata.getTrackInfo().getTrackTotal()));
    writeCommentField(pTag, "TRACKTOTAL", trackTotal);   // recommended field
    updateCommentField(pTag, "TOTALTRACKS", trackTotal); // alternative field

    const TagLib::String albumArtist(
            toTString(trackMetadata.getAlbumInfo().getArtist()));
    writeCommentField(pTag, "ALBUMARTIST", albumArtist);   // recommended field
    updateCommentField(pTag, "ALBUM_ARTIST", albumArtist); // alternative field
    updateCommentField(pTag, "ALBUM ARTIST", albumArtist); // alternative field
    updateCommentField(pTag, "ENSEMBLE", albumArtist);     // alternative field

    const TagLib::String bpm(
            toTString(formatBpm(trackMetadata)));
    // MusicBrainz recommends "BPM": https://picard.musicbrainz.org/docs/mappings
    // Mixxx (<= 2.0) favored "TEMPO": https://www.mixxx.org/wiki/doku.php/library_metadata_rewrite_using_taglib
    if (hasCommentField(*pTag, "BPM") || !hasCommentField(*pTag, "TEMPO")) {
        // Update or add the recommended field for BPM values
        writeCommentField(pTag, "BPM", bpm);
    } else {
        // Update the legacy field for BPM values only if it already exists exclusively
        DEBUG_ASSERT(hasCommentField(*pTag, "TEMPO"));
        writeCommentField(pTag, "TEMPO", bpm);
    }

    // Write both INITIALKEY and KEY
    const TagLib::String keyText =
            toTString(trackMetadata.getTrackInfo().getKeyText());
    writeCommentField(pTag, "INITIALKEY", keyText); // recommended field
    updateCommentField(pTag, "KEY", keyText);       // alternative field

    writeCommentField(pTag, "REPLAYGAIN_TRACK_GAIN", toTString(formatTrackGain(trackMetadata)));
    // NOTE(uklotzde, 2018-04-22): The analyzers currently doesn't
    // calculate a peak value, so leave it untouched in the file if
    // the value is invalid/absent. Otherwise the comment field would
    // be deleted.
    writeCommentField(pTag, "REPLAYGAIN_TRACK_PEAK", toTString(formatTrackPeak(trackMetadata)));

#if defined(__EXTRA_METADATA__)
    // According to https://wiki.xiph.org/Field_names "DISCTOTAL" is
    // the proposed field name, but some applications use "TOTALDISCS".
    const TagLib::String discTotal(toTString(trackMetadata.getTrackInfo().getDiscTotal()));
    writeCommentField(pTag, "DISCTOTAL", discTotal);   // recommended field
    updateCommentField(pTag, "TOTALDISCS", discTotal); // alternative field

    writeCommentField(pTag, "REPLAYGAIN_ALBUM_GAIN", toTString(formatAlbumGain(trackMetadata)));
    writeCommentField(pTag, "REPLAYGAIN_ALBUM_PEAK", toTString(formatAlbumPeak(trackMetadata)));

    writeCommentField(pTag, "MUSICBRAINZ_ARTISTID", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzArtistId()));
    writeCommentField(pTag, "MUSICBRAINZ_TRACKID", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzRecordingId()));
    writeCommentField(pTag, "MUSICBRAINZ_RELEASETRACKID", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzReleaseId()));
    writeCommentField(pTag, "MUSICBRAINZ_WORKID", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzWorkId()));
    writeCommentField(pTag, "MUSICBRAINZ_ALBUMARTISTID", uuidToTString(trackMetadata.getAlbumInfo().getMusicBrainzArtistId()));
    writeCommentField(pTag, "MUSICBRAINZ_ALBUMID", uuidToTString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseId()));
    writeCommentField(pTag, "MUSICBRAINZ_RELEASEGROUPID", uuidToTString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId()));

    writeCommentField(pTag, "CONDUCTOR", toTString(trackMetadata.getTrackInfo().getConductor()));
    writeCommentField(pTag, "ISRC", toTString(trackMetadata.getTrackInfo().getISRC()));
    writeCommentField(pTag, "LANGUAGE", toTString(trackMetadata.getTrackInfo().getLanguage()));
    writeCommentField(pTag, "LYRICIST", toTString(trackMetadata.getTrackInfo().getLyricist()));
    writeCommentField(pTag, "MOOD", toTString(trackMetadata.getTrackInfo().getMood()));
    writeCommentField(pTag, "COPYRIGHT", toTString(trackMetadata.getAlbumInfo().getCopyright()));
    writeCommentField(pTag, "LICENSE", toTString(trackMetadata.getAlbumInfo().getLicense()));
    writeCommentField(pTag, "LABEL", toTString(trackMetadata.getAlbumInfo().getRecordLabel()));
    writeCommentField(pTag, "REMIXER", toTString(trackMetadata.getTrackInfo().getRemixer()));
    writeCommentField(pTag, "SUBTITLE", toTString(trackMetadata.getTrackInfo().getSubtitle()));
    writeCommentField(pTag, "ENCODEDBY", toTString(trackMetadata.getTrackInfo().getEncoder()));
    writeCommentField(pTag, "ENCODERSETTINGS", toTString(trackMetadata.getTrackInfo().getEncoderSettings()));
    writeCommentField(
            pTag, "DISCNUMBER", toTString(trackMetadata.getTrackInfo().getDiscNumber()));
#endif // __EXTRA_METADATA__

    // Serato tags
    //
    // FIXME: We're only dumping FLAC tags for now, since the Ogg format is
    // different we don't support it yet.
    if (fileType == FileType::FLAC &&
            trackMetadata.getTrackInfo().getSeratoTags().status() !=
                    SeratoTags::ParserStatus::Failed) {
        writeCommentField(
                pTag,
                kCommentFieldKeySeratoBeatGrid,
                dumpSeratoBeatGrid(trackMetadata, fileType));

        writeCommentField(
                pTag,
                kCommentFieldKeySeratoMarkers2FLAC,
                dumpSeratoMarkers2(trackMetadata, fileType));
    }

    return true;
}

} // namespace xiph

} // namespace taglib

} // namespace mixxx

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
