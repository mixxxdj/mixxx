#pragma once

#include "track/track.h"

class ScrobblingService : public QObject {
  public:
    ~ScrobblingService() override = default;
  public slots:
    virtual void slotBroadcastCurrentTrack(TrackPointer pTrack) = 0;
    virtual void slotScrobbleTrack(TrackPointer pTrack) = 0;
    virtual void slotAllTracksPaused() = 0;
};

typedef std::shared_ptr<ScrobblingService> ScrobblingServicePtr;
