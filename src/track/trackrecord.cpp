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

void mergeReplayGainMetadataProperty(
        ReplayGain& mergedReplayGain,
        const ReplayGain& importedReplayGain) {
    // Preserve the values calculated by Mixxx and only merge missing
    // values from the imported replay gain.
    if (!mergedReplayGain.hasRatio()) {
        mergedReplayGain.setRatio(importedReplayGain.getRatio());
    }
    if (!mergedReplayGain.hasPeak()) {
        mergedReplayGain.setPeak(importedReplayGain.getPeak());
    }
}

// This conditional copy operation only works for nullable properties
// like QString or QUuid.
template<typename T>
void copyIfNotNull(
        T& mergedProperty,
        const T& importedProperty) {
    if (mergedProperty.isNull()) {
        mergedProperty = importedProperty;
    }
}

// This conditional copy operation only works for properties where
// empty = missing.
template<typename T>
void copyIfNotEmpty(
        T& mergedProperty,
        const T& importedProperty) {
    if (mergedProperty.isEmpty()) {
        mergedProperty = importedProperty;
    }
}

} // anonymous namespace

void TrackRecord::mergeImportedMetadata(
        const TrackMetadata& importedFromFile) {
    TrackInfo& mergedTrackInfo = refMetadata().refTrackInfo();
    const TrackInfo& importedTrackInfo = importedFromFile.getTrackInfo();
    if (mergedTrackInfo.getTrackTotal() == kTrackTotalPlaceholder) {
        mergedTrackInfo.setTrackTotal(importedTrackInfo.getTrackTotal());
        // Also set the track number if it is still empty due
        // to insufficient parsing capabilities of Mixxx in
        // previous versions.
        if (mergedTrackInfo.getTrackNumber().isEmpty() &&
                !importedTrackInfo.getTrackNumber().isEmpty()) {
            mergedTrackInfo.setTrackNumber(importedTrackInfo.getTrackNumber());
        }
    }
#if defined(__EXTRA_METADATA__)
    copyIfNotNull(mergedTrackInfo.refConductor(), importedTrackInfo.getConductor());
    copyIfNotNull(mergedTrackInfo.refDiscNumber(), importedTrackInfo.getDiscNumber());
    copyIfNotNull(mergedTrackInfo.refDiscTotal(), importedTrackInfo.getDiscTotal());
    copyIfNotNull(mergedTrackInfo.refEncoder(), importedTrackInfo.getEncoder());
    copyIfNotNull(mergedTrackInfo.refEncoderSettings(), importedTrackInfo.getEncoderSettings());
    copyIfNotNull(mergedTrackInfo.refISRC(), importedTrackInfo.getISRC());
    copyIfNotNull(mergedTrackInfo.refLanguage(), importedTrackInfo.getLanguage());
    copyIfNotNull(mergedTrackInfo.refLyricist(), importedTrackInfo.getLyricist());
    copyIfNotNull(mergedTrackInfo.refMood(), importedTrackInfo.getMood());
    copyIfNotNull(mergedTrackInfo.refMovement(), importedTrackInfo.getMovement());
    copyIfNotNull(mergedTrackInfo.refMusicBrainzArtistId(), importedTrackInfo.getMusicBrainzArtistId());
    copyIfNotNull(mergedTrackInfo.refMusicBrainzRecordingId(), importedTrackInfo.getMusicBrainzRecordingId());
    copyIfNotNull(mergedTrackInfo.refMusicBrainzReleaseId(), importedTrackInfo.getMusicBrainzReleaseId());
    copyIfNotNull(mergedTrackInfo.refMusicBrainzWorkId(), importedTrackInfo.getMusicBrainzWorkId());
    copyIfNotNull(mergedTrackInfo.refRemixer(), importedTrackInfo.getRemixer());
    copyIfNotEmpty(mergedTrackInfo.refSeratoMarkers2(), importedTrackInfo.getSeratoMarkers2());
    copyIfNotNull(mergedTrackInfo.refSubtitle(), importedTrackInfo.getSubtitle());
    copyIfNotNull(mergedTrackInfo.refWork(), importedTrackInfo.getWork());
    AlbumInfo& mergedAlbumInfo = refMetadata().refAlbumInfo();
    const AlbumInfo& importedAlbumInfo = importedFromFile.getAlbumInfo();
    mergeReplayGainMetadataProperty(mergedAlbumInfo.refReplayGain(), importedAlbumInfo.getReplayGain());
    copyIfNotNull(mergedAlbumInfo.refCopyright(), importedAlbumInfo.getCopyright());
    copyIfNotNull(mergedAlbumInfo.refLicense(), importedAlbumInfo.getLicense());
    copyIfNotNull(mergedAlbumInfo.refMusicBrainzArtistId(), importedAlbumInfo.getMusicBrainzArtistId());
    copyIfNotNull(mergedAlbumInfo.refMusicBrainzReleaseGroupId(), importedAlbumInfo.getMusicBrainzReleaseGroupId());
    copyIfNotNull(mergedAlbumInfo.refMusicBrainzReleaseId(), importedAlbumInfo.getMusicBrainzReleaseId());
    copyIfNotNull(mergedAlbumInfo.refRecordLabel(), importedAlbumInfo.getRecordLabel());
#endif // __EXTRA_METADATA__
}

} //namespace mixxx
