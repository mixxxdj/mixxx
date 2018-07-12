
#include "broadcast/mpris/mprisservice.h"

#include "moc_mprisservice.cpp"

MprisService::MprisService(MixxxMainWindow* pWindow)
        : m_mpris(pWindow) {
}

void MprisService::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    m_mpris.broadcastCurrentTrack();
}

void MprisService::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void MprisService::slotAllTracksPaused() {
}
