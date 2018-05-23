#include "metadatabroadcast.h"
#include "mixer/playerinfo.h"

MetadataBroadcast::MetadataBroadcast() {

}

void MetadataBroadcast::slotReadyToBeScrobbled(Track *pTrack) {

}

void MetadataBroadcast::slotNowListening(Track *pTrack) {

}

QLinkedList<TrackPointer> MetadataBroadcast::getTrackedTracks() {
    return m_trackedTracks;
}