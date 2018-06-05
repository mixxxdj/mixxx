#pragma once

#include <QObject>
#include <QLinkedList>
#include <list>

#include "broadcast/scrobblingservice.h"
#include "track/track.h"
#include "track/trackid.h"
#include "track/trackplaytimers.h"

class MetadataBroadcaster : public QObject {
    Q_OBJECT    
  private:
    struct GracePeriod {
        double m_msElapsed;
        unsigned int m_numberOfScrobbles = 0;
        TrackId m_trackId;
        bool hasBeenEjected = false;      
        GracePeriod(double msElapsed,TrackPointer pTrack) :
        m_msElapsed(msElapsed),m_trackId(pTrack->getId()) {}
        bool operator==(const GracePeriod &other) const {
          return m_trackId == other.m_trackId;
        }
    };  
  public:    

    MetadataBroadcaster();
    QLinkedList<TrackId> getTrackedTracks();
    MetadataBroadcaster& addNewScrobblingService(ScrobblingService *service);
    void newTrackLoaded(TrackPointer pTrack);
    void trackUnloaded(TrackPointer pTrack);
    void setGracePeriod(unsigned int seconds);

  public slots:
    void slotAttemptScrobble(TrackPointer pTrack);
    void slotNowListening(TrackPointer pTrack);
    void slotGuiTick(double timeSinceLastTick);
  private:    
    unsigned int m_gracePeriodSeconds;
    QLinkedList<GracePeriod> m_trackedTracks;
    std::list<std::unique_ptr<ScrobblingService>> m_scrobblingServices;    
};