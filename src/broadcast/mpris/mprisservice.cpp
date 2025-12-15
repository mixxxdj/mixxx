#include "broadcast/mpris/mprisservice.h"

#include "moc_mprisservice.cpp"

MprisService::MprisService(PlayerManagerInterface* pPlayer,
        UserSettingsPointer pSettings)
        : m_mpris(pPlayer, pSettings) {
}

void MprisService::slotBroadcastCurrentTrack(TrackPointer) {
}

void MprisService::slotScrobbleTrack(TrackPointer) {
}

void MprisService::slotAllTracksPaused() {
}
