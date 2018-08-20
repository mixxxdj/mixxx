#pragma once


#include "broadcast/scrobblingservice.h"

class LastFMService : public ScrobblingService {

  public:
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
};
