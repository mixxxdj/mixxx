#include "broadcast/mpris/mprisservice.h"

#include "moc_mprisservice.cpp"

MprisService::MprisService(MixxxMainWindow* pWindow,
        PlayerManagerInterface* pPlayer,
        UserSettingsPointer pSettings)
        : m_mpris(pWindow, pPlayer, pSettings) {
    m_pCPAutoDJEnabled = new ControlProxy(ConfigKey("[AutoDJ]", "enabled"), this);
}

void MprisService::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    if (m_pCPAutoDJEnabled && !m_pCPAutoDJEnabled->toBool()) {
        m_mpris.broadcastCurrentTrack();
    }
}

void MprisService::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void MprisService::slotAllTracksPaused() {
}
