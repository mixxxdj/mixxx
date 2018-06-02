#pragma once

#include <QList>
#include <QObject>
#include <list>

#include "broadcast/scrobblingservice.h"
#include "track/track_decl.h"
#include "track/trackplaytimers.h"

class MetadataBroadcaster : public QObject {
    Q_OBJECT
  public:
    MetadataBroadcaster(TrackTimers::ElapsedTimer* timer);
    const QList<TrackPointer>& getTrackedTracks();
    void addNewScrobblingService(ScrobblingService* service);

  public slots:
    void slotReadyToBeScrobbled(TrackPointer pTrack);
    void slotNowListening(TrackPointer pTrack);

  private:
    QList<TrackPointer> m_trackedTracks;
    std::unique_ptr<TrackTimers::ElapsedTimer> m_pElapsedTimer;
    std::list<std::unique_ptr<ScrobblingService>> m_scrobblingServices;
};
