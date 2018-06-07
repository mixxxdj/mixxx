#include "broadcast/metadatabroadcast.h"

#include "mixer/playerinfo.h"
#include "moc_metadatabroadcast.cpp"

MetadataBroadcaster::MetadataBroadcaster() {
}

void MetadataBroadcaster::slotAttemptScrobble(TrackPointer pTrack) {
    for (auto it = m_trackedTracks.begin();
            it != m_trackedTracks.end();
            ++it) {
        if (*it == GracePeriod(0, pTrack)) {
            GracePeriod& trackPeriod = *it;
            if (trackPeriod.hasBeenEjected &&
                    trackPeriod.m_msElapsed >
                            static_cast<double>(m_gracePeriodSeconds) * 1000.0) {
                for (auto& service : m_scrobblingServices) {
                    service->scrobbleTrack(pTrack);
                }
                trackPeriod.hasBeenEjected = false;
                trackPeriod.m_numberOfScrobbles++;
            }
            break;
        }
    }
}

void MetadataBroadcaster::slotNowListening(TrackPointer pTrack) {
    for (auto& service : m_scrobblingServices) {
        service->broadcastCurrentTrack(pTrack);
    }
}

const QList<TrackId> MetadataBroadcaster::getTrackedTracks() {
    return {};
}

MetadataBroadcasterInterface& MetadataBroadcaster::addNewScrobblingService(ScrobblingService* service) {
    m_scrobblingServices.push_back(
            std::move(std::unique_ptr<ScrobblingService>(service)));
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
            it->hasBeenEjected = true;
            it->m_msElapsed = 0;
        }
        break;
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
