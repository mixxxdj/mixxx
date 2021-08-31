#include "library/tags/tagfacetid.h"

#include <QRegularExpression>

namespace {

const QRegularExpression kValidFacetStringNotEmpty(
        QStringLiteral("^[\\+\\-\\./0-9@a-z\\[\\]_]+"));
const QRegularExpression kInversekValidFacetStringNotEmpty(
        QStringLiteral("[^\\+\\-\\./0-9@a-z\\[\\]_]+"));

} // anonymous namespace

namespace mixxx {

namespace library {

namespace tags {

//static
bool TagFacetId::isValidValue(
        const value_t& value) {
    if (value.isNull()) {
        return true;
    }
    if (value.isEmpty()) {
        // for disambiguation with null
        return false;
    }
    const auto match = kValidFacetStringNotEmpty.match(value);
    DEBUG_ASSERT(match.isValid());
    DEBUG_ASSERT(value.length() > 0);
    // match = exact match
    return match.capturedLength() == value.length();
}

//static
TagFacetId::value_t TagFacetId::convertIntoValidValue(
        const value_t& value) {
    auto validValue = filterEmptyValue(value.toLower().remove(kInversekValidFacetStringNotEmpty));
    DEBUG_ASSERT(isValidValue(validValue));
    return validValue;
}

} // namespace tags

} // namespace library

} // namespace mixxx

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetAcoustidFingerprint =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("acoustid_fingerprint"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetAcoustidId =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("acoustid_id"));

// MusicBrainz: "comment:description"
const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetComment =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("comment"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetDecade =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("decade"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetGenre =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("genre"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetGrouping =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("grouping"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetIsrc =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("isrc"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetLanguage =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("language"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetMood =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("mood"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetMusicBrainzAlbumArtistId =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("musicbrainz_albumartistid"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetMusicBrainzAlbumId =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("musicbrainz_albumid"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetMusicBrainzArtistId =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("musicbrainz_artistid"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetMusicBrainzRecordingId =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("musicbrainz_recordingid"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetMusicBrainzReleaseGroupId =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("musicbrainz_releasegroupid"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetMusicBrainzReleaseTrackId =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("musicbrainz_releasetrackid"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetMusicBrainzTrackId =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("musicbrainz_trackid"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetMusicBrainzWorkId =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("musicbrainz_workid"));

// MusicBrainz: "_rating"
const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetRating =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("rating"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetArousal =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("arousal"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetValence =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("valence"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetAcousticness =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("acousticness"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetDanceability =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("danceability"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetEnergy =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("energy"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetInstrumentalness =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("instrumentalness"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetLiveness =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("liveness"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetPopularity =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("popularity"));

const mixxx::library::tags::TagFacetId mixxx::library::tags::kTagFacetSpeechiness =
        mixxx::library::tags::TagFacetId::staticConst(QStringLiteral("speechiness"));
