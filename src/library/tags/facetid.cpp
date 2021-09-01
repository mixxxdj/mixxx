#include "library/tags/facetid.h"

#include <QRegularExpression>

namespace {

const QRegularExpression kValidFacetStringNotEmpty(
        QStringLiteral("^[\\+\\-\\./0-9@a-z\\[\\]_]+"));
const QRegularExpression kInverseValidFacetStringNotEmpty(
        QStringLiteral("[^\\+\\-\\./0-9@a-z\\[\\]_]+"));

} // anonymous namespace

namespace mixxx {

namespace library {

namespace tags {

//static
bool FacetId::isValidValue(
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
FacetId::value_t FacetId::convertIntoValidValue(
        const value_t& value) {
    auto validValue = filterEmptyValue(value.toLower().remove(kInverseValidFacetStringNotEmpty));
    DEBUG_ASSERT(isValidValue(validValue));
    return validValue;
}

} // namespace tags

} // namespace library

} // namespace mixxx

// MusicBrainz: "comment:description"
const mixxx::library::tags::FacetId mixxx::library::tags::kFacetComment =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("comment"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetDecade =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("decade"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetEthno =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("ethno"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetGenre =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("genre"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetGrouping =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("grouping"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetIsrc =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("isrc"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetLanguage =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("language"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetMood =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("mood"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetVenue =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("venue"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetVibe =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("vibe"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetAcoustidFingerprint =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("acoustid_fingerprint"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetAcoustidId =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("acoustid_id"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetMusicBrainzAlbumArtistId =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("musicbrainz_albumartistid"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetMusicBrainzAlbumId =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("musicbrainz_albumid"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetMusicBrainzArtistId =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("musicbrainz_artistid"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetMusicBrainzRecordingId =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("musicbrainz_recordingid"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetMusicBrainzReleaseGroupId =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("musicbrainz_releasegroupid"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetMusicBrainzReleaseTrackId =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("musicbrainz_releasetrackid"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetMusicBrainzTrackId =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("musicbrainz_trackid"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetMusicBrainzWorkId =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("musicbrainz_workid"));

// MusicBrainz: "_rating"
const mixxx::library::tags::FacetId mixxx::library::tags::kFacetRating =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("rating"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetArousal =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("arousal"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetValence =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("valence"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetAcousticness =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("acousticness"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetDanceability =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("danceability"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetEnergy =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("energy"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetInstrumentalness =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("instrumentalness"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetLiveness =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("liveness"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetPopularity =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("popularity"));

const mixxx::library::tags::FacetId mixxx::library::tags::kFacetSpeechiness =
        mixxx::library::tags::FacetId::staticConst(QStringLiteral("speechiness"));

const QVector<mixxx::library::tags::FacetId>
        mixxx::library::tags::kReservedFacetIds =
                QVector<mixxx::library::tags::FacetId>{
                        mixxx::library::tags::kFacetComment,
                        mixxx::library::tags::kFacetDecade,
                        mixxx::library::tags::kFacetEthno,
                        mixxx::library::tags::kFacetGenre,
                        mixxx::library::tags::kFacetGrouping,
                        mixxx::library::tags::kFacetIsrc,
                        mixxx::library::tags::kFacetLanguage,
                        mixxx::library::tags::kFacetMood,
                        mixxx::library::tags::kFacetVenue,
                        mixxx::library::tags::kFacetVibe,
                        mixxx::library::tags::kFacetAcoustidFingerprint,
                        mixxx::library::tags::kFacetAcoustidId,
                        mixxx::library::tags::kFacetMusicBrainzAlbumArtistId,
                        mixxx::library::tags::kFacetMusicBrainzAlbumId,
                        mixxx::library::tags::kFacetMusicBrainzArtistId,
                        mixxx::library::tags::kFacetMusicBrainzRecordingId,
                        mixxx::library::tags::kFacetMusicBrainzReleaseGroupId,
                        mixxx::library::tags::kFacetMusicBrainzReleaseTrackId,
                        mixxx::library::tags::kFacetMusicBrainzTrackId,
                        mixxx::library::tags::kFacetMusicBrainzWorkId,
                        mixxx::library::tags::kFacetRating,
                        mixxx::library::tags::kFacetArousal,
                        mixxx::library::tags::kFacetValence,
                        mixxx::library::tags::kFacetAcousticness,
                        mixxx::library::tags::kFacetDanceability,
                        mixxx::library::tags::kFacetEnergy,
                        mixxx::library::tags::kFacetInstrumentalness,
                        mixxx::library::tags::kFacetLiveness,
                        mixxx::library::tags::kFacetPopularity,
                        mixxx::library::tags::kFacetSpeechiness,
                };
