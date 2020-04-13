#include "track/trackrecord.h"

#include "track/keyfactory.h"

namespace mixxx {

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

bool TrackRecord::mergeImportedMetadata(
        const TrackMetadata& importedFromFile) {
    bool modified = false;
    TrackInfo* pMergedTrackInfo = m_metadata.ptrTrackInfo();
    const TrackInfo& importedTrackInfo = importedFromFile.getTrackInfo();
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
    const AlbumInfo& importedAlbumInfo = importedFromFile.getAlbumInfo();
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

} //namespace mixxx
