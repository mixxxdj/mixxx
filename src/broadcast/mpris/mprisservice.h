#pragma once

#include "broadcast/mpris/mpris.h"
#include "broadcast/scrobblingservice.h"

class MixxxMainWindow;

class PlayerManagerInterface;

class MprisService : public ScrobblingService {
    Q_OBJECT
  public:
    MprisService(PlayerManagerInterface* pPlayer,
            UserSettingsPointer pSettings);
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;

  private:
    Mpris m_mpris;
};
