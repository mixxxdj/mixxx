
#include "broadcast/mpris/mprisservice.h"

#include "moc_mprisservice.cpp"

MprisService::MprisService(MixxxMainWindow* pWindow,
        PlayerManager* pPlayer,
        UserSettingsPointer pSettings)
        : m_mpris(pWindow, pPlayer, pSettings) {
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
