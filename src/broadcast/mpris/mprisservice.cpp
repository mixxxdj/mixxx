
#include "broadcast/mpris/mprisservice.h"

#include "mixxxmainwindow.h"
#include "moc_mprisservice.cpp"

MprisService::MprisService(MixxxMainWindow* pWindow,
        PlayerManager* pPlayer,
        UserSettingsPointer pSettings)
        : m_mpris(pWindow, pPlayer, pSettings),
          m_pCPAutoDJEnabled(nullptr) {
    connect(pWindow, &MixxxMainWindow::componentsInitialized, this, &MprisService::slotComponentsInitialized);
}

void MprisService::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (m_pCPAutoDJEnabled && !m_pCPAutoDJEnabled->toBool()) {
        m_mpris.broadcastCurrentTrack();
    }
}

void MprisService::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void MprisService::slotAllTracksPaused() {
}

void MprisService::slotComponentsInitialized() {
    m_pCPAutoDJEnabled = new ControlProxy(ConfigKey("[AutoDJ]", "enabled"), this);
}
