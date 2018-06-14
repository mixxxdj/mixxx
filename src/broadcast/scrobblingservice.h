#pragma once

#include "track/track.h"

class ScrobblingService : public QObject {
  public:
    virtual ~ScrobblingService() = default;
  public slots:
    virtual void slotBroadcastCurrentTrack(TrackPointer pTrack) = 0;
    virtual void slotScrobbleTrack(TrackPointer pTrack) = 0;
    virtual void slotAllTracksPaused() = 0;
};