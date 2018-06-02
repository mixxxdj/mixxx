#include "metadatabroadcast.h"
#include "mixer/playerinfo.h"

MetadataBroadcaster::MetadataBroadcaster(TrackTimers::ElapsedTimer *timer) :
m_pElapsedTimer(timer)
{

}

void MetadataBroadcaster::slotReadyToBeScrobbled(TrackPointer pTrack) {

}

void MetadataBroadcaster::slotNowListening(TrackPointer pTrack) {
    for (auto &service : m_scrobblingServices) {
        service->broadcastCurrentTrack(pTrack);
    }
}

QLinkedList<TrackPointer> MetadataBroadcaster::getTrackedTracks() {
    return m_trackedTracks;
}

void MetadataBroadcaster::addNewScrobblingService(ScrobblingService *service) {
    m_scrobblingServices.push_back(
        std::move(std::unique_ptr<ScrobblingService>(service)));    
}