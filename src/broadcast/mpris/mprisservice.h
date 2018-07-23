#pragma once


#include "broadcast/scrobblingservice.h"
#include "broadcast/mpris/mpris.h"
#include "mixxx.h"

class MprisService : public ScrobblingService {
    Q_OBJECT
  public:
    explicit MprisService(MixxxMainWindow *pWindow,
                          PlayerManager *pPlayer,
                          UserSettingsPointer pSettings);
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
  private:
    Mpris m_mpris;
};
