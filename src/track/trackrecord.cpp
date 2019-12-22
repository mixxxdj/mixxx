#include "track/trackrecord.h"

#include "track/keyfactory.h"


namespace mixxx {

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

// Data migration: Reload track total from file tags if not initialized
// yet. The added column "tracktotal" has been initialized with the
// default value "//".
// See also: Schema revision 26 in schema.xml
const QString kReloadTrackTotal = QStringLiteral("//");

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

template<typename T>
void mergeNullableMetadataProperty(
        T& mergedProperty,
        const T& importedProperty) {
    if (mergedProperty.isNull()) {
        mergedProperty = importedProperty;
    }
}

} // anonymous namespace

void TrackRecord::mergeImportedMetadata(
        const TrackMetadata& importedFromFile) {
    TrackInfo& mergedTrackInfo = refMetadata().refTrackInfo();
    const TrackInfo& importedTrackInfo = importedFromFile.getTrackInfo();
    if (mergedTrackInfo.getTrackTotal() == kReloadTrackTotal) {
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
    mergeNullableMetadataProperty(mergedTrackInfo.refConductor(), importedTrackInfo.getConductor());
    mergeNullableMetadataProperty(mergedTrackInfo.refDiscNumber(), importedTrackInfo.getDiscNumber());
    mergeNullableMetadataProperty(mergedTrackInfo.refDiscTotal(), importedTrackInfo.getDiscTotal());
    mergeNullableMetadataProperty(mergedTrackInfo.refEncoder(), importedTrackInfo.getEncoder());
    mergeNullableMetadataProperty(mergedTrackInfo.refEncoderSettings(), importedTrackInfo.getEncoderSettings());
    mergeNullableMetadataProperty(mergedTrackInfo.refISRC(), importedTrackInfo.getISRC());
    mergeNullableMetadataProperty(mergedTrackInfo.refLanguage(), importedTrackInfo.getLanguage());
    mergeNullableMetadataProperty(mergedTrackInfo.refLyricist(), importedTrackInfo.getLyricist());
    mergeNullableMetadataProperty(mergedTrackInfo.refMood(), importedTrackInfo.getMood());
    mergeNullableMetadataProperty(mergedTrackInfo.refMovement(), importedTrackInfo.getMovement());
    mergeNullableMetadataProperty(mergedTrackInfo.refMusicBrainzArtistId(), importedTrackInfo.getMusicBrainzArtistId());
    mergeNullableMetadataProperty(mergedTrackInfo.refMusicBrainzRecordingId(), importedTrackInfo.getMusicBrainzRecordingId());
    mergeNullableMetadataProperty(mergedTrackInfo.refMusicBrainzReleaseId(), importedTrackInfo.getMusicBrainzReleaseId());
    mergeNullableMetadataProperty(mergedTrackInfo.refMusicBrainzWorkId(), importedTrackInfo.getMusicBrainzWorkId());
    mergeNullableMetadataProperty(mergedTrackInfo.refRemixer(), importedTrackInfo.getRemixer());
    mergeNullableMetadataProperty(mergedTrackInfo.refSubtitle(), importedTrackInfo.getSubtitle());
    mergeNullableMetadataProperty(mergedTrackInfo.refWork(), importedTrackInfo.getWork());
#endif // __EXTRA_METADATA__
    AlbumInfo& mergedAlbumInfo = refMetadata().refAlbumInfo();
    const AlbumInfo& importedAlbumInfo = importedFromFile.getAlbumInfo();
#if defined(__EXTRA_METADATA__)
    mergeReplayGainMetadataProperty(mergedAlbumInfo.refReplayGain(), importedAlbumInfo.getReplayGain());
    mergeNullableMetadataProperty(mergedAlbumInfo.refCopyright(), importedAlbumInfo.getCopyright());
    mergeNullableMetadataProperty(mergedAlbumInfo.refLicense(), importedAlbumInfo.getLicense());
    mergeNullableMetadataProperty(mergedAlbumInfo.refMusicBrainzArtistId(), importedAlbumInfo.getMusicBrainzArtistId());
    mergeNullableMetadataProperty(mergedAlbumInfo.refMusicBrainzReleaseGroupId(), importedAlbumInfo.getMusicBrainzReleaseGroupId());
    mergeNullableMetadataProperty(mergedAlbumInfo.refMusicBrainzReleaseId(), importedAlbumInfo.getMusicBrainzReleaseId());
    mergeNullableMetadataProperty(mergedAlbumInfo.refRecordLabel(), importedAlbumInfo.getRecordLabel());
#else
    Q_UNUSED(mergedAlbumInfo);
    Q_UNUSED(importedAlbumInfo);
#endif // __EXTRA_METADATA__
}

} //namespace mixxx
