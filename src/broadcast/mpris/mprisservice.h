#pragma once

#include "broadcast/mpris/mpris.h"
#include "broadcast/scrobblingservice.h"
#include "mixxxmainwindow.h"

class MprisService : public ScrobblingService {
    Q_OBJECT
  public:
    explicit MprisService(MixxxMainWindow* pWindow);
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;

  private:
    Mpris m_mpris;
};
