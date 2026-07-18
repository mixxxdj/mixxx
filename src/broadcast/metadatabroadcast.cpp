#include "broadcast/metadatabroadcast.h"

#include "mixer/playerinfo.h"
#include "moc_metadatabroadcast.cpp"

MetadataBroadcaster::MetadataBroadcaster()
        : m_gracePeriodSeconds(5 * 60) {
}

void MetadataBroadcaster::slotAttemptScrobble(TrackPointer pTrack) {
    if (m_trackedTracks.contains(pTrack->getId())) {
        GracePeriod trackPeriod = m_trackedTracks.value(pTrack->getId());
        if ((trackPeriod.m_hasBeenEjected &&
                    trackPeriod.m_msSinceEjection >
                            m_gracePeriodSeconds * 1000.0) ||
                trackPeriod.m_firstTimeLoaded) {
            for (auto& service : m_scrobblingServices) {
                service->slotScrobbleTrack(pTrack);
            }
            trackPeriod.m_hasBeenEjected = false;
            trackPeriod.m_firstTimeLoaded = false;
            trackPeriod.m_timesTrackHasBeenScrobbled++;
        }
    }
}

void MetadataBroadcaster::slotNowListening(TrackPointer pTrack) {
    for (const auto& service : std::as_const(m_scrobblingServices)) {
        service->slotBroadcastCurrentTrack(pTrack);
    }
}

void MetadataBroadcaster::slotAllTracksPaused() {
    for (const auto& service : std::as_const(m_scrobblingServices)) {
        service->slotAllTracksPaused();
    }
}

void MetadataBroadcaster::newTrackLoaded(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    if (!m_trackedTracks.contains(pTrack->getId())) {
        m_trackedTracks.insert(pTrack->getId(), GracePeriod());
    }
}

void MetadataBroadcaster::trackUnloaded(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    if (m_trackedTracks.contains(pTrack->getId())) {
        m_trackedTracks[pTrack->getId()].m_firstTimeLoaded = false;
        m_trackedTracks[pTrack->getId()].m_hasBeenEjected = true;
        m_trackedTracks[pTrack->getId()].m_msSinceEjection = 0.0;
    }
}

void MetadataBroadcaster::guiTick(double timeSinceLastTick) {
    for (auto&& it : m_trackedTracks) {
        if (it.m_hasBeenEjected) {
            it.m_msSinceEjection += timeSinceLastTick;
        }
    }
}
