#include "library/externaltrackcollection.h"


void ExternalTrackCollection::deduplicateTracks(
        const QList<DuplicateTrack>& duplicateTracks) {
    QList<QString> purgedTracks;
    QList<TrackRef> updatedTracks;
    purgedTracks.reserve(duplicateTracks.size());
    updatedTracks.reserve(duplicateTracks.size());
    for (const auto& duplicateTrack : duplicateTracks) {
        purgedTracks += duplicateTrack.removed.getLocation();
        updatedTracks += duplicateTrack.replacedBy;
    }
    purgeTracks(purgedTracks);
    updateTracks(updatedTracks);
}
