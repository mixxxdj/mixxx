#pragma once

#include <QObject>
#include <QLinkedList>
#include <list>

#include "broadcast/scrobblingservice.h"
#include "track/track.h"
#include "track/trackplaytimers.h"

class MetadataBroadcaster : public QObject {
    Q_OBJECT
  public:
    MetadataBroadcaster(TrackTimers::ElapsedTimer *timer);
    QLinkedList<TrackPointer> getTrackedTracks();
    void addNewScrobblingService(ScrobblingService *service);

  public slots:
    void slotReadyToBeScrobbled(TrackPointer pTrack);
    void slotNowListening(TrackPointer pTrack);
  private:
    QLinkedList<TrackPointer> m_trackedTracks;    
    std::unique_ptr<TrackTimers::ElapsedTimer> m_pElapsedTimer;
    std::list<std::unique_ptr<ScrobblingService>> m_scrobblingServices;    
};