#include "broadcast/metadatabroadcast.h"

#include "mixer/playerinfo.h"
#include "moc_metadatabroadcast.cpp"

MetadataBroadcaster::MetadataBroadcaster()
        : m_gracePeriodSeconds(5 * 60) {
}

void MetadataBroadcaster::slotAttemptScrobble(TrackPointer pTrack) {
    for (auto it = m_trackedTracks.begin();
            it != m_trackedTracks.end();
            ++it) {
        if (*it == GracePeriod(0, pTrack)) {
            GracePeriod& trackPeriod = *it;
            if ((trackPeriod.hasBeenEjected &&
                        (trackPeriod.m_msElapsed > m_gracePeriodSeconds * 1000.0)) ||
                    trackPeriod.firstTimeLoaded) {
                for (auto& service : m_scrobblingServices) {
                    service->slotScrobbleTrack(pTrack);
                }
                trackPeriod.hasBeenEjected = false;
                trackPeriod.firstTimeLoaded = false;
                trackPeriod.m_numberOfScrobbles++;
            }
            break;
        }
    }
}

void MetadataBroadcaster::slotNowListening(TrackPointer pTrack) {
    for (auto& service : m_scrobblingServices) {
        service->slotBroadcastCurrentTrack(pTrack);
    }
}

void MetadataBroadcaster::slotAllTracksPaused() {
    for (auto& service : m_scrobblingServices) {
        service->slotAllTracksPaused();
    }
}

const QList<TrackId> MetadataBroadcaster::getTrackedTracks() {
    return {};
}

MetadataBroadcasterInterface& MetadataBroadcaster::addNewScrobblingService(const ScrobblingServicePtr& newService) {
    m_scrobblingServices.push_back(newService);
    return *this;
}

void MetadataBroadcaster::newTrackLoaded(TrackPointer pTrack) {
    QListIterator<GracePeriod> it(m_trackedTracks);
    if (!it.findNext(GracePeriod(0, pTrack))) {
        GracePeriod newPeriod(0, pTrack);
        m_trackedTracks.append(newPeriod);
    }
}

void MetadataBroadcaster::trackUnloaded(TrackPointer pTrack) {
    for (auto it = m_trackedTracks.begin();
            it != m_trackedTracks.end();
            ++it) {
        if (*it == GracePeriod(0, pTrack)) {
            it->firstTimeLoaded = false;
            it->hasBeenEjected = true;
            it->m_msElapsed = 0;
            break;
        }
    }
}

void MetadataBroadcaster::guiTick(double timeSinceLastTick) {
    for (auto it = m_trackedTracks.begin();
            it != m_trackedTracks.end();
            ++it) {
        if (it->hasBeenEjected) {
            it->m_msElapsed += timeSinceLastTick;
        }
    }
}
