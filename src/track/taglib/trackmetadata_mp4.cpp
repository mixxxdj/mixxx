#if defined(_MSC_VER)
#pragma warning(push)
// https://github.com/taglib/taglib/issues/1185
// warning C4251: 'TagLib::FileName::m_wname': class
// 'std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>'
// needs to have dll-interface to be used by clients of class 'TagLib::FileName'
#pragma warning(disable : 4251)
#endif

#include "track/taglib/trackmetadata_mp4.h"

#include "track/taglib/trackmetadata_common.h"
#include "track/tracknumbers.h"
#include "util/logger.h"

namespace mixxx {

namespace {

Logger kLogger("TagLib");

} // anonymous namespace

namespace taglib {

namespace {

// Freeform MP4 atom key format: "----:<mean>:<name>"

// Apple iTunes atom keys (informal standard)
const TagLib::String kAtomKeyBpm = "----:com.apple.iTunes:BPM";
const TagLib::String kAtomKeyInitialKey = "----:com.apple.iTunes:initialkey"; // official/preferred
const TagLib::String kAtomKeyAlternativeKey = "----:com.apple.iTunes:KEY";    // alternative (conforms to Rapid Evolution)
const TagLib::String kAtomKeyReplayGainTrackGain = "----:com.apple.iTunes:replaygain_track_gain";
const TagLib::String kAtomKeyReplayGainTrackPeak = "----:com.apple.iTunes:replaygain_track_peak";
const TagLib::String kAtomKeyReplayGainAlbumGain = "----:com.apple.iTunes:replaygain_album_gain";
const TagLib::String kAtomKeyReplayGainAlbumPeak = "----:com.apple.iTunes:replaygain_album_peak";

// Serato atom keys
const TagLib::String kAtomKeySeratoBeatGrid = "----:com.serato.dj:beatgrid";
const TagLib::String kAtomKeySeratoMarkers = "----:com.serato.dj:markers";
const TagLib::String kAtomKeySeratoMarkers2 = "----:com.serato.dj:markersv2";


bool readAtom(
        const TagLib::MP4::Tag& tag,
        const TagLib::String& key,
        TagLib::String* pValue = nullptr) {
    const auto itemMap = tag.itemMap();
    const TagLib::MP4::ItemMap::ConstIterator it = itemMap.find(key);
    if (it == itemMap.end()) {
        return false;
    }
    if (pValue) {
        *pValue = firstNonEmptyStringListItem(
                (*it).second.toStringList());
    }
    return true;
}

bool readAtom(
        const TagLib::MP4::Tag& tag,
        const TagLib::String& key,
        QString* pValue) {
    if (!pValue) {
        return readAtom(tag, key);
    }
    TagLib::String value;
    if (!readAtom(tag, key, &value)) {
        return false;
    }
    *pValue = toQString(value);
    return true;
}

// Unconditionally write the atom
void writeAtom(
        TagLib::MP4::Tag* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (value.isEmpty()) {
        // Purge empty atoms
        pTag->removeItem(key);
    } else {
        TagLib::StringList strList(value);
        pTag->setItem(key, std::move(strList));
    }
}

// Conditionally write the atom only if it already exists
inline void updateAtom(
        TagLib::MP4::Tag* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (readAtom(*pTag, key)) {
        writeAtom(pTag, key, value);
    }
}

} // anonymous namespace

namespace mp4 {

bool importCoverImageFromTag(
        QImage* pCoverArt,
        const TagLib::MP4::Tag& tag) {
    if (!pCoverArt) {
        return false; // nothing to do
    }

    if (tag.contains("covr")) {
        TagLib::MP4::CoverArtList coverArtList =
                tag.item("covr").toCoverArtList();
        for (const auto& coverArt : coverArtList) {
            const QImage image(loadImageFromByteVector(coverArt.data()));
            if (image.isNull()) {
                kLogger.warning()
                        << "Failed to load image from MP4 atom covr";
                continue;
            } else {
                *pCoverArt = image;
                return true; // done
            }
        }
    }

    if (kLogger.debugEnabled()) {
        kLogger.debug() << "No cover art found in MP4 tag";
    }
    return false;
}

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::MP4::Tag& tag,
        bool resetMissingTagMetadata) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    taglib::importTrackMetadataFromTag(
            pTrackMetadata,
            tag);

    QString albumArtist;
    if (readAtom(tag, "aART", &albumArtist) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setArtist(albumArtist);
    }

    QString composer;
    if (readAtom(tag, "\251wrt", &composer) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setComposer(composer);
    }

    QString grouping;
    if (readAtom(tag, "\251grp", &grouping) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setGrouping(grouping);
    }

    QString year;
    if (readAtom(tag, "\251day", &year) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setYear(year);
    }

    // Read track number/total pair
    if (tag.contains("trkn")) {
        const TagLib::MP4::Item trknItem = tag.item("trkn");
        const TagLib::MP4::Item::IntPair trknPair = trknItem.toIntPair();
        const TrackNumbers trackNumbers(trknPair.first, trknPair.second);
        QString trackNumber;
        QString trackTotal;
        trackNumbers.toStrings(&trackNumber, &trackTotal);
        pTrackMetadata->refTrackInfo().setTrackNumber(trackNumber);
        pTrackMetadata->refTrackInfo().setTrackTotal(trackTotal);
    } else if (resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setTrackNumber(QString{});
        pTrackMetadata->refTrackInfo().setTrackTotal(QString{});
    }

#if defined(__EXTRA_METADATA__)
    // Read disc number/total pair
    if (tag.contains("disk")) {
        const TagLib::MP4::Item trknItem = tag.item("disk");
        const TagLib::MP4::Item::IntPair trknPair = trknItem.toIntPair();
        const TrackNumbers discNumbers(trknPair.first, trknPair.second);
        QString discNumber;
        QString discTotal;
        discNumbers.toStrings(&discNumber, &discTotal);
        pTrackMetadata->refTrackInfo().setDiscNumber(discNumber);
        pTrackMetadata->refTrackInfo().setDiscTotal(discTotal);
    } else if (resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setDiscNumber(QString{});
        pTrackMetadata->refTrackInfo().setDiscTotal(QString{});
    }
#endif // __EXTRA_METADATA__

    QString bpm;
    if (readAtom(tag, kAtomKeyBpm, &bpm)) {
        // This is the preferred field for storing the BPM
        // with fractional digits as a floating-point value.
        // If this field contains a valid value the integer
        // BPM value that might have been read before is
        // overwritten.
        parseBpm(pTrackMetadata, bpm, resetMissingTagMetadata);
    } else if (tag.contains("tmpo")) {
        // Read the BPM as an integer value.
        const TagLib::MP4::Item tmpoItem = tag.item("tmpo");
        double bpmValue = tmpoItem.toInt();
        if (Bpm::isValidValue(bpmValue)) {
            pTrackMetadata->refTrackInfo().setBpm(Bpm(bpmValue));
        } else if (resetMissingTagMetadata) {
            pTrackMetadata->refTrackInfo().setBpm(Bpm{});
        }
    } else if (resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setBpm(Bpm{});
    }

    QString keyText;
    if (readAtom(tag,
                kAtomKeyInitialKey,
                &keyText) || // preferred (conforms to MixedInKey, Serato, Traktor)
            readAtom(tag,
                    kAtomKeyAlternativeKey,
                    &keyText) || // alternative (conforms to Rapid Evolution)
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setKeyText(keyText);
    }

    QString trackGain;
    if (readAtom(tag, kAtomKeyReplayGainTrackGain, &trackGain) || resetMissingTagMetadata) {
        parseTrackGain(pTrackMetadata, trackGain, resetMissingTagMetadata);
    }
    QString trackPeak;
    if (readAtom(tag, kAtomKeyReplayGainTrackPeak, &trackPeak) || resetMissingTagMetadata) {
        parseTrackPeak(pTrackMetadata, trackPeak, resetMissingTagMetadata);
    }

#if defined(__EXTRA_METADATA__)
    QString albumGain;
    if (readAtom(tag, kAtomKeyReplayGainAlbumGain, &albumGain) || resetMissingTagMetadata) {
        parseAlbumGain(pTrackMetadata, albumGain, resetMissingTagMetadata);
    }
    QString albumPeak;
    if (readAtom(tag, kAtomKeyReplayGainAlbumPeak, &albumPeak) || resetMissingTagMetadata) {
        parseAlbumPeak(pTrackMetadata, albumPeak, resetMissingTagMetadata);
    }

    QString trackArtistId;
    if (readAtom(tag,
                "----:com.apple.iTunes:MusicBrainz Artist Id",
                &trackArtistId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzArtistId(QUuid(trackArtistId));
    }
    QString trackRecordingId;
    if (readAtom(tag,
                "----:com.apple.iTunes:MusicBrainz Track Id",
                &trackRecordingId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzRecordingId(QUuid(trackRecordingId));
    }
    QString trackReleaseId;
    if (readAtom(tag,
                "----:com.apple.iTunes:MusicBrainz Release Track Id",
                &trackReleaseId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzReleaseId(QUuid(trackReleaseId));
    }
    QString trackWorkId;
    if (readAtom(tag,
                "----:com.apple.iTunes:MusicBrainz Work Id",
                &trackWorkId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMusicBrainzWorkId(QUuid(trackWorkId));
    }
    QString albumArtistId;
    if (readAtom(tag,
                "----:com.apple.iTunes:MusicBrainz Album Artist Id",
                &albumArtistId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzArtistId(QUuid(albumArtistId));
    }
    QString albumReleaseId;
    if (readAtom(tag,
                "----:com.apple.iTunes:MusicBrainz Album Id",
                &albumReleaseId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseId(QUuid(albumReleaseId));
    }
    QString albumReleaseGroupId;
    if (readAtom(tag,
                "----:com.apple.iTunes:MusicBrainz Release Group Id",
                &albumReleaseGroupId) ||
            resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseGroupId(QUuid(albumReleaseGroupId));
    }

    QString conductor;
    if (readAtom(tag, "----:com.apple.iTunes:CONDUCTOR", &conductor) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setConductor(conductor);
    }
    QString isrc;
    if (readAtom(tag, "----:com.apple.iTunes:ISRC", &isrc) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setISRC(isrc);
    }
    QString language;
    if (readAtom(tag, "----:com.apple.iTunes:LANGUAGE", &language) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setLanguage(language);
    }
    QString lyricist;
    if (readAtom(tag, "----:com.apple.iTunes:LYRICIST", &lyricist) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setLyricist(lyricist);
    }
    QString mood;
    if (readAtom(tag, "----:com.apple.iTunes:MOOD", &mood) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMood(mood);
    }
    QString copyright;
    if (readAtom(tag, "cprt", &copyright) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setCopyright(copyright);
    }
    QString license;
    if (readAtom(tag, "----:com.apple.iTunes:LICENSE", &license) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setLicense(license);
    }
    QString recordLabel;
    if (readAtom(tag, "----:com.apple.iTunes:LABEL", &recordLabel) || resetMissingTagMetadata) {
        pTrackMetadata->refAlbumInfo().setRecordLabel(recordLabel);
    }
    QString remixer;
    if (readAtom(tag, "----:com.apple.iTunes:REMIXER", &remixer) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setRemixer(remixer);
    }
    QString subtitle;
    if (readAtom(tag, "----:com.apple.iTunes:SUBTITLE", &subtitle) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setSubtitle(subtitle);
    }
    QString encoder;
    if (readAtom(tag, "\251too", &encoder) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setEncoder(encoder);
    }
    QString work;
    if (readAtom(tag, "\251wrk", &work) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setWork(work);
    }
    QString movement;
    if (readAtom(tag, "\251mvn", &movement) || resetMissingTagMetadata) {
        pTrackMetadata->refTrackInfo().setMovement(movement);
    }
#endif // __EXTRA_METADATA__

    // Serato tags
    TagLib::String seratoBeatGridData;
    if (readAtom(
                tag,
                kAtomKeySeratoBeatGrid,
                &seratoBeatGridData)) {
        parseSeratoBeatGrid(
                pTrackMetadata,
                seratoBeatGridData,
                FileType::MP4);
    }
    TagLib::String seratoMarkersData;
    if (readAtom(
                tag,
                kAtomKeySeratoMarkers,
                &seratoMarkersData)) {
        parseSeratoMarkers(
                pTrackMetadata,
                seratoMarkersData,
                FileType::MP4);
    }
    TagLib::String seratoMarkers2Data;
    if (readAtom(
                tag,
                kAtomKeySeratoMarkers2,
                &seratoMarkers2Data)) {
        parseSeratoMarkers2(
                pTrackMetadata,
                seratoMarkers2Data,
                FileType::MP4);
    }
}

bool exportTrackMetadataIntoTag(
        TagLib::MP4::Tag* pTag,
        const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    taglib::exportTrackMetadataIntoTag(
            pTag,
            trackMetadata,
            WriteTagFlag::OmitTrackNumber | WriteTagFlag::OmitYear);

    // Write track number/total pair
    TrackNumbers parsedTrackNumbers;
    const TrackNumbers::ParseResult trackParseResult =
            TrackNumbers::parseFromStrings(
                    trackMetadata.getTrackInfo().getTrackNumber(),
                    trackMetadata.getTrackInfo().getTrackTotal(),
                    &parsedTrackNumbers);
    switch (trackParseResult) {
    case TrackNumbers::ParseResult::EMPTY:
        pTag->removeItem("trkn");
        break;
    case TrackNumbers::ParseResult::VALID:
        pTag->setItem("trkn",
                TagLib::MP4::Item(parsedTrackNumbers.getActual(),
                        parsedTrackNumbers.getTotal()));
        break;
    default:
        kLogger.warning() << "Invalid track numbers in MP4 atom:"
                          << TrackNumbers::joinAsString(
                                     trackMetadata.getTrackInfo().getTrackNumber(),
                                     trackMetadata.getTrackInfo().getTrackTotal());
    }

    writeAtom(pTag, "\251day", toTString(trackMetadata.getTrackInfo().getYear()));

    writeAtom(pTag, "aART", toTString(trackMetadata.getAlbumInfo().getArtist()));
    writeAtom(pTag, "\251wrt", toTString(trackMetadata.getTrackInfo().getComposer()));
    writeAtom(pTag, "\251grp", toTString(trackMetadata.getTrackInfo().getGrouping()));

    // Write both BPM fields (just in case)
    if (trackMetadata.getTrackInfo().getBpm().isValid()) {
        // 16-bit integer value
        const int tmpoValue =
                Bpm::valueToInteger(trackMetadata.getTrackInfo().getBpm().value());
        pTag->setItem("tmpo", tmpoValue);
    } else {
        pTag->removeItem("tmpo");
    }
    writeAtom(pTag, kAtomKeyBpm, toTString(formatBpm(trackMetadata)));

    const TagLib::String keyText =
            toTString(trackMetadata.getTrackInfo().getKeyText());
    writeAtom(pTag, kAtomKeyInitialKey, keyText);      // preferred
    updateAtom(pTag, kAtomKeyAlternativeKey, keyText); // alternative

    writeAtom(pTag, kAtomKeyReplayGainTrackGain, toTString(formatTrackGain(trackMetadata)));
    writeAtom(pTag, kAtomKeyReplayGainTrackPeak, toTString(formatTrackPeak(trackMetadata)));

#if defined(__EXTRA_METADATA__)
    // Write disc number/total pair
    QString discNumberText;
    QString discTotalText;
    TrackNumbers parsedDiscNumbers;
    const TrackNumbers::ParseResult discParseResult = TrackNumbers::parseFromStrings(
            trackMetadata.getTrackInfo().getDiscNumber(),
            trackMetadata.getTrackInfo().getDiscTotal(),
            &parsedDiscNumbers);
    switch (discParseResult) {
    case TrackNumbers::ParseResult::EMPTY:
        pTag->removeItem("disk");
        break;
    case TrackNumbers::ParseResult::VALID:
        pTag->setItem("disk",
                TagLib::MP4::Item(parsedDiscNumbers.getActual(),
                        parsedDiscNumbers.getTotal()));
        break;
    default:
        kLogger.warning() << "Invalid disc numbers in MP4 atom:"
                          << TrackNumbers::joinAsString(
                                     trackMetadata.getTrackInfo().getDiscNumber(),
                                     trackMetadata.getTrackInfo().getDiscTotal());
    }

    writeAtom(pTag, kAtomKeyReplayGainAlbumGain, toTString(formatAlbumGain(trackMetadata)));
    writeAtom(pTag, kAtomKeyReplayGainAlbumPeak, toTString(formatAlbumPeak(trackMetadata)));

    writeAtom(pTag, "----:com.apple.iTunes:MusicBrainz Artist Id", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzArtistId()));
    writeAtom(pTag, "----:com.apple.iTunes:MusicBrainz Track Id", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzRecordingId()));
    writeAtom(pTag, "----:com.apple.iTunes:MusicBrainz Release Track Id", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzReleaseId()));
    writeAtom(pTag, "----:com.apple.iTunes:MusicBrainz Work Id", uuidToTString(trackMetadata.getTrackInfo().getMusicBrainzWorkId()));
    writeAtom(pTag, "----:com.apple.iTunes:MusicBrainz Album Artist Id", uuidToTString(trackMetadata.getAlbumInfo().getMusicBrainzArtistId()));
    writeAtom(pTag, "----:com.apple.iTunes:MusicBrainz Album Id", uuidToTString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseId()));
    writeAtom(pTag, "----:com.apple.iTunes:MusicBrainz Release Group Id", uuidToTString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId()));

    writeAtom(pTag, "----:com.apple.iTunes:CONDUCTOR", toTString(trackMetadata.getTrackInfo().getConductor()));
    writeAtom(pTag, "----:com.apple.iTunes:ISRC", toTString(trackMetadata.getTrackInfo().getISRC()));
    writeAtom(pTag, "----:com.apple.iTunes:LANGUAGE", toTString(trackMetadata.getTrackInfo().getLanguage()));
    writeAtom(pTag, "----:com.apple.iTunes:LYRICIST", toTString(trackMetadata.getTrackInfo().getLyricist()));
    writeAtom(pTag, "----:com.apple.iTunes:MOOD", toTString(trackMetadata.getTrackInfo().getMood()));
    writeAtom(pTag, "cprt", toTString(trackMetadata.getAlbumInfo().getCopyright()));
    writeAtom(pTag, "----:com.apple.iTunes:LICENSE", toTString(trackMetadata.getAlbumInfo().getLicense()));
    writeAtom(pTag, "----:com.apple.iTunes:LABEL", toTString(trackMetadata.getAlbumInfo().getRecordLabel()));
    writeAtom(pTag, "----:com.apple.iTunes:REMIXER", toTString(trackMetadata.getTrackInfo().getRemixer()));
    writeAtom(pTag, "----:com.apple.iTunes:SUBTITLE", toTString(trackMetadata.getTrackInfo().getSubtitle()));
    writeAtom(pTag, "\251too", toTString(trackMetadata.getTrackInfo().getEncoder()));
    writeAtom(pTag, "\251wrk", toTString(trackMetadata.getTrackInfo().getWork()));
    writeAtom(pTag, "\251mvn", toTString(trackMetadata.getTrackInfo().getMovement()));
#endif // __EXTRA_METADATA__

    // Serato tags
    if (trackMetadata.getTrackInfo().getSeratoTags().status() != SeratoTags::ParserStatus::Failed) {
        writeAtom(
                pTag,
                kAtomKeySeratoBeatGrid,
                dumpSeratoBeatGrid(trackMetadata, FileType::MP4));
        writeAtom(
                pTag,
                kAtomKeySeratoMarkers,
                dumpSeratoMarkers(trackMetadata, FileType::MP4));
        writeAtom(
                pTag,
                kAtomKeySeratoMarkers2,
                dumpSeratoMarkers2(trackMetadata, FileType::MP4));
    }

    return true;
}

} // namespace mp4

} // namespace taglib

} // namespace mixxx

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
