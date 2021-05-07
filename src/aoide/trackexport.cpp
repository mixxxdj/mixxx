#include "aoide/trackexport.h"

#include <QMimeDatabase>

#include "track/trackrecord.h"
#include "util/encodedurl.h"
#include "util/fileinfo.h"
#include "util/logger.h"
#include "util/math.h"
#include "util/quuid.h"

namespace {

const mixxx::Logger kLogger("aoide TrackExporter");

const QString MBID_ARTIST_UUID_PREFIX = "artist/";
const QString MBID_RECORDING_UUID_PREFIX = "recording/";
const QString MBID_TRACK_UUID_PREFIX = "track/";
const QString MBID_WORK_UUID_PREFIX = "work/";
const QString MBID_RELEASE_UUID_PREFIX = "release/";
const QString MBID_RELEASE_GROUP_UUID_PREFIX = "release-group/";

inline void appendUri(
        QStringList* uris, const QString& uri, const QString& prefix = QString()) {
    if (!uri.isEmpty()) {
        *uris += prefix + uri;
    }
}

inline void appendUri(
        QStringList* uris,
        const QUuid& uuid, // e.g. MusicBrainz MBIDs
        const QString& prefix = QString()) {
    appendUri(uris, uuidToNullableStringWithoutBraces(uuid), prefix);
}

inline void addOrReplaceTagWithUuidLabel(
        mixxx::Facets* pFacets,
        const QUuid& uuid,
        const mixxx::TagFacetId& facetId = mixxx::TagFacetId{}) {
    auto label = mixxx::TagLabel{uuidToNullableStringWithoutBraces(uuid)};
    if (!label.isEmpty()) {
        pFacets->addOrReplaceTagsWithSingleTag(mixxx::Tag{std::move(label)}, facetId);
    }
}

} // anonymous namespace

namespace aoide {

namespace {

void appendTitle(json::TitleVector* titles, QString name, int kind) {
    if (!name.isEmpty()) {
        *titles += std::move(json::Title(name, kind));
    }
}

void appendActor(json::ActorVector* actors, QString name, int role) {
    if (!name.isEmpty()) {
        *actors += std::move(json::Actor(name, role));
    }
}

} // anonymous namespace

json::Track exportTrack(
        const json::MediaSourceConfig& mediaSourceConfig,
        const mixxx::FileInfo& trackFile,
        const mixxx::TrackRecord& trackRecord,
        const QList<CuePointer>& cuePoints) {
    // Might be needed in the future. Currently the percent-encoded
    // file URL is mapped to a virtual file path by aoide and not
    // by Mixxx.
    Q_UNUSED(mediaSourceConfig)

    json::Track jsonTrack;
    kLogger.debug() << "Exporting track"
                    << trackFile;

    const mixxx::TrackMetadata& trackMetadata = trackRecord.getMetadata();
    const mixxx::TrackInfo& trackInfo = trackMetadata.getTrackInfo();
    const mixxx::AlbumInfo& albumInfo = trackMetadata.getAlbumInfo();

    {
        json::AudioContent audioContent;
        audioContent.setChannelCount(trackMetadata.getStreamInfo()
                                             .getSignalInfo()
                                             .getChannelCount());
        audioContent.setSampleRate(trackMetadata.getStreamInfo().getSignalInfo().getSampleRate());
        audioContent.setBitrate(trackMetadata.getStreamInfo().getBitrate());
        audioContent.setDuration(trackMetadata.getStreamInfo().getDuration());
        audioContent.setReplayGain(trackInfo.getReplayGain());

        auto encoder = trackInfo.getEncoder().trimmed();
        const auto encoderSettings = trackInfo.getEncoderSettings().trimmed();
        if (encoder.isEmpty()) {
            encoder = encoderSettings;
        } else if (!encoderSettings.isEmpty()) {
            encoder += ' ';
            encoder += encoderSettings;
        }
        audioContent.setEncoder(encoder);

        // The media source property is only valid for existing tracks
        // with a valid URL and valid content type!
        json::MediaSource mediaSource;
        DEBUG_ASSERT(trackRecord.getDateAdded().isValid());
        mediaSource.setCollectedAt(trackRecord.getDateAdded());
        const auto pathUrl = trackFile.toQUrl();
        DEBUG_ASSERT(pathUrl.isValid());
        mediaSource.setPathUrl(mixxx::EncodedUrl::fromQUrl(pathUrl));
        mediaSource.setSynchronizedAt(trackRecord.getSourceSynchronizedAt());
        const auto trackLocation = trackFile.location();
        DEBUG_ASSERT(!trackLocation.isEmpty());
        const auto contentType = QMimeDatabase().mimeTypeForFile(trackLocation);
        DEBUG_ASSERT(contentType.isValid());
        mediaSource.setContentType(contentType);
        mediaSource.setAudioContent(std::move(audioContent));
        // Artwork
        if (trackRecord.getCoverInfo().type != CoverInfoRelative::NONE) {
            json::ArtworkImage artworkImage;
            artworkImage.setDigest(trackRecord.getCoverInfo().imageDigest());
            // TODO: Generate 4x4 thumbnail from actual image? This
            // is computationally expensive.
            if (!artworkImage.thumbnail().isNull()) {
                // If no thumbnail is available then simply generate one
                // from the solid color. But don't overwrite an existing
                // thumbnail with such an artificial pseudo-thumbnail!
                const auto thumbnail =
                        json::ArtworkImage::generateThumbnailFromRgbColor(
                                trackRecord.getCoverInfo().color);
                artworkImage.setThumbnail(thumbnail);
            }
            // TODO: How to obtain artwork picture type (APIC), media type,
            // and size without loading the actual image?
            json::Artwork artwork;
            artwork.setImage(std::move(artworkImage));
            if (trackRecord.getCoverInfo().type == CoverInfoRelative::FILE) {
                artwork.setSource(QStringLiteral("linked"));
                const auto coverUrl = mixxx::FileInfo(
                        trackRecord.getCoverInfo().coverLocation)
                                              .toQUrl();
                DEBUG_ASSERT(coverUrl.isValid());
                artwork.setUri(mixxx::EncodedUrl::fromQUrl(coverUrl));
            } else if (trackRecord.getCoverInfo().type == CoverInfoRelative::METADATA) {
                artwork.setSource(QStringLiteral("embedded"));
            }
            mediaSource.setArtwork(std::move(artwork));
        }
        jsonTrack.setMediaSource(std::move(mediaSource));
    }

    jsonTrack.setIndexNumbers(trackInfo);

    {
        json::MusicMetrics musicMetrics;
        bool musicMetricsFlag = false;
        if (trackInfo.getBpm().isValid()) {
            musicMetrics.setBpm(trackInfo.getBpm());
            musicMetrics.setBpmLocked(trackRecord.getBpmLocked());
            musicMetricsFlag = true;
        }
        if (trackRecord.getGlobalKey() != mixxx::track::io::key::ChromaticKey::INVALID) {
            musicMetrics.setKey(trackRecord.getGlobalKey());
            musicMetricsFlag = true;
        }
        if (musicMetricsFlag) {
            jsonTrack.setMusicMetrics(std::move(musicMetrics));
        }
    }

    jsonTrack.setColor(trackRecord.getColor());

    // Track titles
    json::TitleVector trackTitles;
    appendTitle(&trackTitles, trackInfo.getTitle(), json::Title::kKindMain);
    appendTitle(&trackTitles, trackInfo.getSubtitle(), json::Title::kKindSub);
    appendTitle(&trackTitles, trackInfo.getWork(), json::Title::kKindWork);
    appendTitle(&trackTitles, trackInfo.getMovement(), json::Title::kKindMovement);
    jsonTrack.addTitles(std::move(trackTitles));

    // Track actors
    json::ActorVector trackActors;
    appendActor(&trackActors, trackInfo.getArtist(), json::Actor::kRoleArtist);
    appendActor(&trackActors, trackInfo.getComposer(), json::Actor::kRoleComposer);
    appendActor(&trackActors, trackInfo.getConductor(), json::Actor::kRoleConductor);
    appendActor(&trackActors, trackInfo.getLyricist(), json::Actor::kRoleLyricist);
    appendActor(&trackActors, trackInfo.getRemixer(), json::Actor::kRoleRemixer);
    jsonTrack.addActors(std::move(trackActors));

    // Album {
    json::Album aoideAlbum = jsonTrack.album();

    // Album titles
    json::TitleVector albumTitles;
    appendTitle(&albumTitles, albumInfo.getTitle(), json::Title::kKindMain);
    aoideAlbum.addTitles(std::move(albumTitles));

    // Album actors
    json::ActorVector albumActors;
    appendActor(&albumActors, albumInfo.getArtist(), json::Actor::kRoleArtist);
    aoideAlbum.addActors(std::move(albumActors));

    jsonTrack.setAlbum(std::move(aoideAlbum));
    // } Album

    // Release
    json::Release release = jsonTrack.release();
    release.setReleasedAt(trackInfo.getYear());
    release.setReleasedBy(albumInfo.getRecordLabel());
    release.setCopyright(albumInfo.getCopyright());
    jsonTrack.setRelease(std::move(release));

    // Tags
    auto facets = trackMetadata.getFacets();
    auto tags = json::Tags::fromFacets(trackMetadata.getFacets());
    // The labels for the faceted tags mixxx::library::tags::kFacetGenre
    // an mixxx::library::tags::kFacetMood are implicitly synchronized.
    // We can only perform a basic consistency check without the
    // mixxx::TaggingConfig at hand.
    DEBUG_ASSERT(trackInfo.getGenre().isEmpty() ==
            (facets.countTags(mixxx::library::tags::kFacetGenre) == 0));
    DEBUG_ASSERT(trackInfo.getMood().isEmpty() ==
            (facets.countTags(mixxx::library::tags::kFacetMood) == 0));
    const auto commentLabel =
            mixxx::TagLabel(mixxx::TagLabel::convertIntoValidValue(trackInfo.getComment()));
    if (!commentLabel.isEmpty()) {
        DEBUG_ASSERT(commentLabel.isValid());
        DEBUG_ASSERT(!facets.containsTagLabeled(
                commentLabel, mixxx::library::tags::kFacetComment));
        facets.addOrReplaceTagsWithSingleTag(mixxx::Tag(commentLabel),
                mixxx::library::tags::kFacetComment);
    }
    const auto groupingLabel =
            mixxx::TagLabel(mixxx::TagLabel::convertIntoValidValue(trackInfo.getGrouping()));
    if (!groupingLabel.isEmpty()) {
        DEBUG_ASSERT(groupingLabel.isValid());
        DEBUG_ASSERT(!facets.containsTagLabeled(
                groupingLabel, mixxx::library::tags::kFacetGrouping));
        facets.addOrReplaceTagsWithSingleTag(mixxx::Tag(groupingLabel),
                mixxx::library::tags::kFacetGrouping);
    }
    const auto languageLabel =
            mixxx::TagLabel(mixxx::TagLabel::convertIntoValidValue(trackInfo.getLanguage()));
    if (!languageLabel.isEmpty()) {
        DEBUG_ASSERT(languageLabel.isValid());
        DEBUG_ASSERT(!facets.containsTagLabeled(
                languageLabel, mixxx::library::tags::kFacetLanguage));
        facets.addOrReplaceTagsWithSingleTag(mixxx::Tag(languageLabel),
                mixxx::library::tags::kFacetLanguage);
    }

    // ISRC
    const auto isrcLabel =
            mixxx::TagLabel(mixxx::TagLabel::convertIntoValidValue(trackInfo.getISRC()));
    if (!isrcLabel.isEmpty()) {
        DEBUG_ASSERT(isrcLabel.isValid());
        DEBUG_ASSERT(!facets.containsTagLabeled(isrcLabel, mixxx::library::tags::kFacetIsrc));
        facets.addOrReplaceTagsWithSingleTag(
                mixxx::Tag(isrcLabel), mixxx::library::tags::kFacetIsrc);
    }

    // MusicBrainz
    addOrReplaceTagWithUuidLabel(
            &facets,
            trackInfo.getMusicBrainzRecordingId(),
            mixxx::library::tags::kFacetMusicBrainzRecordingId);
    addOrReplaceTagWithUuidLabel(
            &facets,
            trackInfo.getMusicBrainzReleaseId(),
            mixxx::library::tags::kFacetMusicBrainzReleaseTrackId);
    addOrReplaceTagWithUuidLabel(
            &facets,
            trackInfo.getMusicBrainzWorkId(),
            mixxx::library::tags::kFacetMusicBrainzWorkId);
    addOrReplaceTagWithUuidLabel(
            &facets,
            trackInfo.getMusicBrainzArtistId(),
            mixxx::library::tags::kFacetMusicBrainzArtistId);
    addOrReplaceTagWithUuidLabel(
            &facets,
            albumInfo.getMusicBrainzReleaseGroupId(),
            mixxx::library::tags::kFacetMusicBrainzReleaseGroupId);
    addOrReplaceTagWithUuidLabel(
            &facets,
            albumInfo.getMusicBrainzReleaseId(),
            mixxx::library::tags::kFacetMusicBrainzAlbumId);
    addOrReplaceTagWithUuidLabel(
            &facets,
            albumInfo.getMusicBrainzArtistId(),
            mixxx::library::tags::kFacetMusicBrainzAlbumArtistId);

    jsonTrack.setTags(
            json::Tags::fromFacets(facets));

    // Cue markers
    if (trackMetadata.getStreamInfo().getSignalInfo().getSampleRate() > 0) {
        const auto sampleRate = trackMetadata.getStreamInfo().getSignalInfo().getSampleRate();
        json::CueMarkers cueMarkers;
        for (const auto& cuePoint : cuePoints) {
            auto cueMarker =
                    json::CueMarker::fromCueInfo(
                            cuePoint->getCueInfo(sampleRate));
            if (cueMarker) {
                cueMarkers.append(*cueMarker);
            } else {
                kLogger.warning()
                        << "Skipped unsupported cue point"
                        << cuePoint->getCueInfo(sampleRate);
            }
        }
        jsonTrack.setCueMarkers(std::move(cueMarkers));
    } else {
        kLogger.warning()
                << "Unable to export cue points of track"
                << trackFile.location();
    }

    {
        json::PlayCounter playCounter;
        playCounter.setLastPlayedAt(trackRecord.getPlayCounter().getLastPlayedAt());
        playCounter.setTimesPlayed(trackRecord.getPlayCounter().getTimesPlayed());
        jsonTrack.setPlayCounter(std::move(playCounter));
    }

    return jsonTrack;
}

} // namespace aoide
