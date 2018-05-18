#pragma once

#include <QList>
#include <QObject>

#include "track/track_decl.h"

class MetadataBroadcast : public QObject {
    Q_OBJECT
  public:
    MetadataBroadcast();
    const QList<TrackPointer>& getTrackedTracks();
  public slots:
    void slotReadyToBeScrobbled(Track* pTrack);
    void slotNowListening(Track* pTrack);

  private:
    QList<TrackPointer> m_trackedTracks;
};
