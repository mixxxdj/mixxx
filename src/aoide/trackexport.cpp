#include "aoide/trackexport.h"

#include <QMimeDatabase>

#include "track/trackrecord.h"
#include "util/encodedurl.h"
#include "util/fileinfo.h"
#include "util/logger.h"
#include "util/math.h"

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
            json::Artwork artwork;
            artwork.setDigest(trackRecord.getCoverInfo().imageDigest());
            // TODO: Generate 4x4 thumbnail from actual image? This
            // is computationally expensive.
            if (!artwork.thumbnail().isNull()) {
                // If no thumbnail is available then simply generate one
                // from the solid color. But don't overwrite an existing
                // thumbnail with such an artificial pseudo-thumbnail!
                const auto thumbnail =
                        json::Artwork::generateThumbnailFromRgbColor(
                                trackRecord.getCoverInfo().color);
                artwork.setThumbnail(thumbnail);
            }
            // TODO: How to obtain artwork size without loading the
            // actual image?
            //artwork.setSize(...);
            if (trackRecord.getCoverInfo().type == CoverInfoRelative::FILE) {
                const auto coverUrl = mixxx::FileInfo(
                        trackRecord.getCoverInfo().coverLocation)
                                              .toQUrl();
                DEBUG_ASSERT(coverUrl.isValid());
                artwork.setUri(mixxx::EncodedUrl::fromQUrl(coverUrl));
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
    auto customTags = trackMetadata.getCustomTags();
    auto tags = json::Tags::fromCustomTags(trackMetadata.getCustomTags());
    // The labels for the faceted tags mixxx::CustomTags::kFacetGenre
    // an mixxx::CustomTags::kFacetMood are implicitly synchronized.
    // We can only perform a basic consistency check without the
    // mixxx::TaggingConfig at hand.
    DEBUG_ASSERT(trackInfo.getGenre().isEmpty() ==
            (customTags.countFacetedTags(mixxx::CustomTags::kFacetGenre) == 0));
    DEBUG_ASSERT(trackInfo.getMood().isEmpty() ==
            (customTags.countFacetedTags(mixxx::CustomTags::kFacetMood) == 0));
    const auto commentLabel =
            mixxx::TagLabel(mixxx::TagLabel::clampValue(trackInfo.getComment()));
    if (!commentLabel.isEmpty()) {
        DEBUG_ASSERT(commentLabel.isValid());
        DEBUG_ASSERT(!customTags.containsTag(
                commentLabel, mixxx::CustomTags::kReservedFacetComment));
        customTags.addOrReplaceTag(mixxx::Tag(commentLabel),
                mixxx::CustomTags::kReservedFacetComment);
    }
    const auto groupingLabel =
            mixxx::TagLabel(mixxx::TagLabel::clampValue(trackInfo.getGrouping()));
    if (!groupingLabel.isEmpty()) {
        DEBUG_ASSERT(groupingLabel.isValid());
        DEBUG_ASSERT(!customTags.containsTag(
                groupingLabel, mixxx::CustomTags::kReservedFacetGrouping));
        customTags.addOrReplaceTag(mixxx::Tag(groupingLabel),
                mixxx::CustomTags::kReservedFacetGrouping);
    }
    const auto languageLabel =
            mixxx::TagLabel(mixxx::TagLabel::clampValue(trackInfo.getLanguage()));
    if (!languageLabel.isEmpty()) {
        DEBUG_ASSERT(languageLabel.isValid());
        DEBUG_ASSERT(!customTags.containsTag(languageLabel, mixxx::CustomTags::kFacetLanguage));
        customTags.addOrReplaceTag(mixxx::Tag(languageLabel), mixxx::CustomTags::kFacetLanguage);
    }

    // ISRC
    const auto isrcLabel =
            mixxx::TagLabel(mixxx::TagLabel::clampValue(trackInfo.getISRC()));
    if (!isrcLabel.isEmpty()) {
        DEBUG_ASSERT(isrcLabel.isValid());
        DEBUG_ASSERT(!customTags.containsTag(isrcLabel, mixxx::CustomTags::kReservedFacetISRC));
        customTags.addOrReplaceTag(mixxx::Tag(isrcLabel), mixxx::CustomTags::kReservedFacetISRC);
    }

    // MusicBrainz
    QStringList musicBrainzLabels;
    appendUri(&musicBrainzLabels,
            trackInfo.getMusicBrainzRecordingId(),
            MBID_RECORDING_UUID_PREFIX);
    appendUri(&musicBrainzLabels,
            trackInfo.getMusicBrainzReleaseId(),
            MBID_TRACK_UUID_PREFIX);
    appendUri(&musicBrainzLabels,
            trackInfo.getMusicBrainzWorkId(),
            MBID_WORK_UUID_PREFIX);
    appendUri(&musicBrainzLabels,
            trackInfo.getMusicBrainzArtistId(),
            MBID_ARTIST_UUID_PREFIX);
    appendUri(&musicBrainzLabels,
            albumInfo.getMusicBrainzReleaseGroupId(),
            MBID_RELEASE_GROUP_UUID_PREFIX);
    appendUri(&musicBrainzLabels,
            albumInfo.getMusicBrainzReleaseId(),
            MBID_RELEASE_UUID_PREFIX);
    if (albumInfo.getMusicBrainzArtistId() !=
            trackInfo.getMusicBrainzArtistId()) {
        appendUri(&musicBrainzLabels,
                albumInfo.getMusicBrainzArtistId(),
                MBID_ARTIST_UUID_PREFIX);
    }
    for (auto&& musicBrainzLabel : musicBrainzLabels) {
        const auto label = mixxx::TagLabel(mixxx::TagLabel::clampValue(musicBrainzLabel));
        if (!label.isEmpty()) {
            DEBUG_ASSERT(label.isValid());
            DEBUG_ASSERT(!customTags.containsTag(
                    label, mixxx::CustomTags::kReservedFacetMusicBrainz));
            customTags.addOrReplaceTag(
                    mixxx::Tag(label), mixxx::CustomTags::kReservedFacetMusicBrainz);
        }
    }

    jsonTrack.setTags(
            json::Tags::fromCustomTags(customTags));

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
