#include "library/externaltrackcollection.h"

#include "moc_externaltrackcollection.cpp"

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
        DEBUG_ASSERT(relocatedTrack.updatedTrackRef().isValid());
        updatedTracks.append(relocatedTrack.updatedTrackRef());
    }
    purgeTracks(purgedTracks);
    updateTracks(updatedTracks);
}
