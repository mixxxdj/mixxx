#include "metadatabroadcast.h"

#include "mixer/playerinfo.h"
#include "moc_metadatabroadcast.cpp"
#include "track/track.h"

MetadataBroadcast::MetadataBroadcast() {
}

void MetadataBroadcast::slotReadyToBeScrobbled(Track* pTrack) {
}

void MetadataBroadcast::slotNowListening(Track* pTrack) {
}

const QList<TrackPointer>& MetadataBroadcast::getTrackedTracks() {
    return m_trackedTracks;
}
