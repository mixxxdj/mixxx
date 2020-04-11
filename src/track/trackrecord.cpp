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
void mergeReplayGainMetadataProperty(
        ReplayGain* pMergedReplayGain,
        const ReplayGain& importedReplayGain) {
    // Preserve the values calculated by Mixxx and only merge missing
    // values from the imported replay gain.
    if (!pMergedReplayGain->hasRatio()) {
        pMergedReplayGain->setRatio(importedReplayGain.getRatio());
    }
    if (!pMergedReplayGain->hasPeak()) {
        pMergedReplayGain->setPeak(importedReplayGain.getPeak());
    }
}
#endif // __EXTRA_METADATA__

// This conditional copy operation only works for nullable properties
// like QString or QUuid.
template<typename T>
void copyIfNotNull(
        T* pMergedProperty,
        const T& importedProperty) {
    if (pMergedProperty->isNull()) {
        *pMergedProperty = importedProperty;
    }
}

// This conditional copy operation only works for properties where
// empty = missing.
template<typename T>
void copyIfNotEmpty(
        T* pMergedProperty,
        const T& importedProperty) {
    if (pMergedProperty->isEmpty()) {
        *pMergedProperty = importedProperty;
    }
}

} // anonymous namespace

void TrackRecord::mergeImportedMetadata(
        const TrackMetadata& importedFromFile) {
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
        }
    }
#if defined(__EXTRA_METADATA__)
    copyIfNotNull(pMergedTrackInfo->ptrConductor(), importedTrackInfo.getConductor());
    copyIfNotNull(pMergedTrackInfo->ptrDiscNumber(), importedTrackInfo.getDiscNumber());
    copyIfNotNull(pMergedTrackInfo->ptrDiscTotal(), importedTrackInfo.getDiscTotal());
    copyIfNotNull(pMergedTrackInfo->ptrEncoder(), importedTrackInfo.getEncoder());
    copyIfNotNull(pMergedTrackInfo->ptrEncoderSettings(), importedTrackInfo.getEncoderSettings());
    copyIfNotNull(pMergedTrackInfo->ptrISRC(), importedTrackInfo.getISRC());
    copyIfNotNull(pMergedTrackInfo->ptrLanguage(), importedTrackInfo.getLanguage());
    copyIfNotNull(pMergedTrackInfo->ptrLyricist(), importedTrackInfo.getLyricist());
    copyIfNotNull(pMergedTrackInfo->ptrMood(), importedTrackInfo.getMood());
    copyIfNotNull(pMergedTrackInfo->ptrMovement(), importedTrackInfo.getMovement());
    copyIfNotNull(pMergedTrackInfo->ptrMusicBrainzArtistId(), importedTrackInfo.getMusicBrainzArtistId());
    copyIfNotNull(pMergedTrackInfo->ptrMusicBrainzRecordingId(), importedTrackInfo.getMusicBrainzRecordingId());
    copyIfNotNull(pMergedTrackInfo->ptrMusicBrainzReleaseId(), importedTrackInfo.getMusicBrainzReleaseId());
    copyIfNotNull(pMergedTrackInfo->ptrMusicBrainzWorkId(), importedTrackInfo.getMusicBrainzWorkId());
    copyIfNotNull(pMergedTrackInfo->ptrRemixer(), importedTrackInfo.getRemixer());
    copyIfNotEmpty(pMergedTrackInfo->ptrSeratoTags(), importedTrackInfo.getSeratoTags());
    copyIfNotNull(pMergedTrackInfo->ptrSubtitle(), importedTrackInfo.getSubtitle());
    copyIfNotNull(pMergedTrackInfo->ptrWork(), importedTrackInfo.getWork());
    AlbumInfo* pMergedAlbumInfo = refMetadata().ptrAlbumInfo();
    const AlbumInfo& importedAlbumInfo = importedFromFile.getAlbumInfo();
    mergeReplayGainMetadataProperty(pMergedAlbumInfo->ptrReplayGain(), importedAlbumInfo.getReplayGain());
    copyIfNotNull(pMergedAlbumInfo->ptrCopyright(), importedAlbumInfo.getCopyright());
    copyIfNotNull(pMergedAlbumInfo->ptrLicense(), importedAlbumInfo.getLicense());
    copyIfNotNull(pMergedAlbumInfo->ptrMusicBrainzArtistId(), importedAlbumInfo.getMusicBrainzArtistId());
    copyIfNotNull(pMergedAlbumInfo->ptrMusicBrainzReleaseGroupId(), importedAlbumInfo.getMusicBrainzReleaseGroupId());
    copyIfNotNull(pMergedAlbumInfo->ptrMusicBrainzReleaseId(), importedAlbumInfo.getMusicBrainzReleaseId());
    copyIfNotNull(pMergedAlbumInfo->ptrRecordLabel(), importedAlbumInfo.getRecordLabel());
#endif // __EXTRA_METADATA__
}

} //namespace mixxx
