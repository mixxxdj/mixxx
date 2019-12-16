#include "library/externaltrackcollection.h"


void ExternalTrackCollection::relocateTracks(
        const QList<RelocatedTrack>& relocatedTracks) {
    QList<QString> purgedTracks;
    QList<TrackRef> updatedTracks;
    purgedTracks.reserve(relocatedTracks.size());
    updatedTracks.reserve(relocatedTracks.size());
    for (const auto& relocatedTrack : relocatedTracks) {
        if (!relocatedTrack.deletedTrackLocation().isEmpty()) {
            purgedTracks.append(relocatedTrack.deletedTrackLocation());
        }
        DEBUG_ASSERT(relocatedTrack.mergedTrackRef().isValid());
        updatedTracks.append(relocatedTrack.mergedTrackRef());
    }
    purgeTracks(purgedTracks);
    updateTracks(updatedTracks);
}
