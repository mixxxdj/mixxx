#pragma once

#include <QList>
#include <QObject>
#include <list>

#include "broadcast/scrobblingservice.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "track/trackplaytimers.h"

class MetadataBroadcasterInterface : public QObject {
    Q_OBJECT
  public slots:
    virtual void slotNowListening(TrackPointer pTrack) = 0;
    virtual void slotAttemptScrobble(TrackPointer pTrack) = 0;
    virtual void slotAllTracksPaused() = 0;

  public:
    virtual ~MetadataBroadcasterInterface() = default;
    virtual MetadataBroadcasterInterface& 
        addNewScrobblingService(const ScrobblingServicePtr &newService) = 0;
    virtual void newTrackLoaded(TrackPointer pTrack) = 0;
    virtual void trackUnloaded(TrackPointer pTrack) = 0;
};

class MetadataBroadcaster : public MetadataBroadcasterInterface {
    Q_OBJECT
  private:
    struct GracePeriod {
        double m_msElapsed;
        unsigned int m_numberOfScrobbles = 0;
        TrackId m_trackId;
        bool hasBeenEjected = false;
        GracePeriod(double msElapsed, TrackPointer pTrack)
                : m_msElapsed(msElapsed), m_trackId(pTrack->getId()) {
        }
        bool operator==(const GracePeriod& other) const {
            return m_trackId == other.m_trackId;
        }
    };
  public:

    MetadataBroadcaster() = default;
    const QList<TrackId> getTrackedTracks();
    MetadataBroadcasterInterface& 
        addNewScrobblingService(const ScrobblingServicePtr &newService) override;
    void newTrackLoaded(TrackPointer pTrack) override;
    void trackUnloaded(TrackPointer pTrack) override;
    void setGracePeriod(unsigned int seconds);
    void slotNowListening(TrackPointer pTrack) override;
    void slotAttemptScrobble(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
    void guiTick(double timeSinceLastTick);

  private:
    unsigned int m_gracePeriodSeconds;
    QList<GracePeriod> m_trackedTracks;
    QList<ScrobblingServicePtr> m_scrobblingServices;
};
