
#include "broadcast/mpris/mprisservice.h"

#include "moc_mprisservice.cpp"

MprisService::MprisService(MixxxMainWindow* pWindow,
        PlayerManager* pPlayer,
        UserSettingsPointer pSettings)
        : m_mpris(pWindow, pPlayer, pSettings) {
    connect(pWindow, &MixxxMainWindow::componentsInitialized, this, &MprisService::slotComponentsInitialized);
}

void MprisService::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (!m_CPAutoDJEnabled.toBool()) {
        m_mpris.broadcastCurrentTrack();
    }
}

void MprisService::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void MprisService::slotAllTracksPaused() {
}

void MprisService::slotComponentsInitialized() {
    m_CPAutoDJEnabled.initialize(ConfigKey("[AutoDJ]", "enabled"));
}
