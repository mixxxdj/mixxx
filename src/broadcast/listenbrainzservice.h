#pragma once

#include "broadcast/scrobblingservice.h"

class ListenBrainzService : public ScrobblingService {
    Q_OBJECT
  public:
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
};
