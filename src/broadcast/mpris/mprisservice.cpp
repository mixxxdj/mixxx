#include "broadcast/mpris/mprisservice.h"

#include "moc_mprisservice.cpp"

MprisService::MprisService(PlayerManagerInterface* pPlayer,
        UserSettingsPointer pSettings)
        : m_mpris(pPlayer, pSettings) {
}

void MprisService::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void MprisService::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void MprisService::slotAllTracksPaused() {
}
