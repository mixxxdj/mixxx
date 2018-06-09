#pragma once

#include "track/track.h"

class ScrobblingService {
  public:
    virtual ~ScrobblingService() = default;
    virtual void broadcastCurrentTrack(TrackPointer pTrack) = 0;
    virtual void scrobbleTrack(TrackPointer pTrack) = 0;
};