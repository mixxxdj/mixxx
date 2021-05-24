#include "track/trackrecord.h"

#include "track/keyfactory.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("TrackRecord");

} // anonymous namespace

/*static*/ const QString TrackRecord::kTrackTotalPlaceholder = QStringLiteral("//");

TrackRecord::TrackRecord(TrackId id)
        : m_id(std::move(id)),
          m_metadataSynchronized(false),
          m_rating(0),
          m_bpmLocked(false) {
}

void TrackRecord::setKeys(const Keys& keys) {
    refMetadata().refTrackInfo().setKey(KeyUtils::getGlobalKeyText(keys));
    m_keys = std::move(keys);
}

bool TrackRecord::updateGlobalKey(
        track::io::key::ChromaticKey key,
        track::io::key::Source keySource) {
    if (key == track::io::key::INVALID) {
        return false;
    } else {
        Keys keys = KeyFactory::makeBasicKeys(key, keySource);
        if (m_keys.getGlobalKey() != keys.getGlobalKey()) {
            setKeys(keys);
            return true;
        }
    }
    return false;
}

bool TrackRecord::updateGlobalKeyText(
        const QString& keyText,
        track::io::key::Source keySource) {
    Keys keys = KeyFactory::makeBasicKeysFromText(keyText, keySource);
    if (keys.getGlobalKey() == track::io::key::INVALID) {
        return false;
    } else {
        if (m_keys.getGlobalKey() != keys.getGlobalKey()) {
            setKeys(keys);
            return true;
        }
    }
    return false;
}

namespace {

#if defined(__EXTRA_METADATA__)
bool mergeReplayGainMetadataProperty(
        ReplayGain* pMergedReplayGain,
        const ReplayGain& importedReplayGain) {
    bool modified = false;
    // Preserve the values calculated by Mixxx and only merge missing
    // values from the imported replay gain.
    if (!pMergedReplayGain->hasRatio() &&
            importedReplayGain.hasRatio()) {
        pMergedReplayGain->setRatio(importedReplayGain.getRatio());
        modified = true;
    }
    if (!pMergedReplayGain->hasPeak() &&
            importedReplayGain.hasPeak()) {
        pMergedReplayGain->setPeak(importedReplayGain.getPeak());
        modified = true;
    }
    return modified;
}
#endif // __EXTRA_METADATA__

// This conditional copy operation only works for nullable properties
// like QString or QUuid.
template<typename T>
bool copyIfNotNull(
        T* pMergedProperty,
        const T& importedProperty) {
    if (pMergedProperty->isNull() &&
            *pMergedProperty != importedProperty) {
        *pMergedProperty = importedProperty;
        return true;
    }
    return false;
}

// This conditional copy operation only works for properties where
// empty = missing.
template<typename T>
bool copyIfNotEmpty(
        T* pMergedProperty,
        const T& importedProperty) {
    if (pMergedProperty->isEmpty() &&
            *pMergedProperty != importedProperty) {
        *pMergedProperty = importedProperty;
        return true;
    }
    return false;
}

} // anonymous namespace

bool TrackRecord::replaceMetadataFromSource(
        TrackMetadata&& importedMetadata,
        const QDateTime& metadataSynchronized) {
    bool modified = false;
    if (getMetadata() != importedMetadata) {
        setMetadata(std::move(importedMetadata));
        modified = true;
    }
    // Only set the metadata synchronized flag (column `header_parsed`
    // in the database) from false to true, but never reset it back to
    // false. Otherwise file tags would be re-imported and overwrite
    // the metadata stored in the database, e.g. after retrieving metadata
    // from MusicBrainz!
    // TODO: In the future this flag should become a time stamp
    // to detect updates of files and then decide based on time
    // stamps if file tags need to be re-imported.
    if (!getMetadataSynchronized() && !metadataSynchronized.isNull()) {
        setMetadataSynchronized(true);
        modified = true;
    }
    return modified;
}

bool TrackRecord::mergeExtraMetadataFromSource(
        const TrackMetadata& importedMetadata) {
    bool modified = false;
    TrackInfo* pMergedTrackInfo = m_metadata.ptrTrackInfo();
    const TrackInfo& importedTrackInfo = importedMetadata.getTrackInfo();
    if (pMergedTrackInfo->getTrackTotal() == kTrackTotalPlaceholder) {
        pMergedTrackInfo->setTrackTotal(importedTrackInfo.getTrackTotal());
        // Also set the track number if it is still empty due
        // to insufficient parsing capabilities of Mixxx in
        // previous versions.
        if (pMergedTrackInfo->getTrackNumber().isEmpty() &&
                !importedTrackInfo.getTrackNumber().isEmpty()) {
            pMergedTrackInfo->setTrackNumber(importedTrackInfo.getTrackNumber());
            modified = true;
        }
    }
#if defined(__EXTRA_METADATA__)
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrConductor(),
            importedTrackInfo.getConductor());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrDiscNumber(),
            importedTrackInfo.getDiscNumber());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrDiscTotal(),
            importedTrackInfo.getDiscTotal());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrEncoder(),
            importedTrackInfo.getEncoder());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrEncoderSettings(),
            importedTrackInfo.getEncoderSettings());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrISRC(),
            importedTrackInfo.getISRC());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrLanguage(),
            importedTrackInfo.getLanguage());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrLyricist(),
            importedTrackInfo.getLyricist());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrMood(),
            importedTrackInfo.getMood());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrMovement(),
            importedTrackInfo.getMovement());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrMusicBrainzArtistId(),
            importedTrackInfo.getMusicBrainzArtistId());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrMusicBrainzRecordingId(),
            importedTrackInfo.getMusicBrainzRecordingId());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrMusicBrainzReleaseId(),
            importedTrackInfo.getMusicBrainzReleaseId());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrMusicBrainzWorkId(),
            importedTrackInfo.getMusicBrainzWorkId());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrRemixer(),
            importedTrackInfo.getRemixer());
    modified |= copyIfNotEmpty(
            pMergedTrackInfo->ptrSeratoTags(),
            importedTrackInfo.getSeratoTags());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrSubtitle(),
            importedTrackInfo.getSubtitle());
    modified |= copyIfNotNull(
            pMergedTrackInfo->ptrWork(),
            importedTrackInfo.getWork());
    AlbumInfo* pMergedAlbumInfo = refMetadata().ptrAlbumInfo();
    const AlbumInfo& importedAlbumInfo = importedMetadata.getAlbumInfo();
    modified |= mergeReplayGainMetadataProperty(
            pMergedAlbumInfo->ptrReplayGain(),
            importedAlbumInfo.getReplayGain());
    modified |= copyIfNotNull(
            pMergedAlbumInfo->ptrCopyright(),
            importedAlbumInfo.getCopyright());
    modified |= copyIfNotNull(
            pMergedAlbumInfo->ptrLicense(),
            importedAlbumInfo.getLicense());
    modified |= copyIfNotNull(
            pMergedAlbumInfo->ptrMusicBrainzArtistId(),
            importedAlbumInfo.getMusicBrainzArtistId());
    modified |= copyIfNotNull(
            pMergedAlbumInfo->ptrMusicBrainzReleaseGroupId(),
            importedAlbumInfo.getMusicBrainzReleaseGroupId());
    modified |= copyIfNotNull(
            pMergedAlbumInfo->ptrMusicBrainzReleaseId(),
            importedAlbumInfo.getMusicBrainzReleaseId());
    modified |= copyIfNotNull(
            pMergedAlbumInfo->ptrRecordLabel(),
            importedAlbumInfo.getRecordLabel());
#endif // __EXTRA_METADATA__
    return modified;
}

bool TrackRecord::updateStreamInfoFromSource(
        mixxx::audio::StreamInfo streamInfoFromSource) {
    // Complete missing properties from metadata. Some properties
    // are mandatory while others like the bitrate might not be
    // reported by all decoders.
    VERIFY_OR_DEBUG_ASSERT(streamInfoFromSource.getSignalInfo().getChannelCount().isValid()) {
        streamInfoFromSource.refSignalInfo().setChannelCount(
                getMetadata().getStreamInfo().getSignalInfo().getChannelCount());
    }
    VERIFY_OR_DEBUG_ASSERT(streamInfoFromSource.getSignalInfo().getSampleRate().isValid()) {
        streamInfoFromSource.refSignalInfo().setSampleRate(
                getMetadata().getStreamInfo().getSignalInfo().getSampleRate());
    }
    VERIFY_OR_DEBUG_ASSERT(streamInfoFromSource.getDuration() > Duration::empty()) {
        streamInfoFromSource.setDuration(
                getMetadata().getStreamInfo().getDuration());
    }
    if (!streamInfoFromSource.getBitrate().isValid()) {
        // The bitrate might not be reported by the SoundSource
        streamInfoFromSource.setBitrate(
                getMetadata().getStreamInfo().getBitrate());
    }
    // Stream properties are not expected to vary during a session
    VERIFY_OR_DEBUG_ASSERT(!m_streamInfoFromSource ||
            *m_streamInfoFromSource == streamInfoFromSource) {
        kLogger.warning()
                << "Varying stream properties:"
                << *m_streamInfoFromSource
                << "->"
                << streamInfoFromSource;
    }
    m_streamInfoFromSource = streamInfoFromSource;
    // Stream info from source is always propagated to metadata and
    // unconditionally overwrites any properties that have once been
    // parsed from file tags! This is required to store the most
    // accurate information about the audio stream in the database.
    const bool metadataUpdated = refMetadata().updateStreamInfoFromSource(streamInfoFromSource);
    DEBUG_ASSERT(getMetadata().getStreamInfo() == streamInfoFromSource);
    return metadataUpdated;
}

bool operator==(const TrackRecord& lhs, const TrackRecord& rhs) {
    return lhs.getMetadata() == rhs.getMetadata() &&
            lhs.getCoverInfo() == rhs.getCoverInfo() &&
            lhs.getId() == rhs.getId() &&
            lhs.getMetadataSynchronized() == rhs.getMetadataSynchronized() &&
            lhs.getDateAdded() == rhs.getDateAdded() &&
            lhs.getFileType() == rhs.getFileType() &&
            lhs.getUrl() == rhs.getUrl() &&
            lhs.getPlayCounter() == rhs.getPlayCounter() &&
            lhs.getColor() == rhs.getColor() &&
            lhs.getCuePoint() == rhs.getCuePoint() &&
            lhs.getBpmLocked() == rhs.getBpmLocked() &&
            lhs.getKeys() == rhs.getKeys() &&
            lhs.getRating() == rhs.getRating();
}

} // namespace mixxx
