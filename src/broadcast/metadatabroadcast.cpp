#include "metadatabroadcast.h"

#include "mixer/playerinfo.h"
#include "moc_metadatabroadcast.cpp"

MetadataBroadcaster::MetadataBroadcaster(TrackTimers::ElapsedTimer* pTimer)
        : m_pElapsedTimer(pTimer) {
}

void MetadataBroadcaster::slotReadyToBeScrobbled(TrackPointer pTrack) {
}

void MetadataBroadcaster::slotNowListening(TrackPointer pTrack) {
    for (auto& service : m_scrobblingServices) {
        service->broadcastCurrentTrack(pTrack);
    }
}

const QList<TrackPointer>& MetadataBroadcaster::getTrackedTracks() {
    return m_trackedTracks;
}

void MetadataBroadcaster::addNewScrobblingService(ScrobblingService* service) {
    m_scrobblingServices.push_back(
            std::move(std::unique_ptr<ScrobblingService>(service)));
}
