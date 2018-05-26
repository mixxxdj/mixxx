#pragma once

#include <QObject>
#include <QLinkedList>
#include "track/track.h"

class MetadataBroadcast : public QObject {
    Q_OBJECT
  public:
    MetadataBroadcast();
    QLinkedList<TrackPointer> getTrackedTracks();
  public slots:
    void slotReadyToBeScrobbled(Track *pTrack);
    void slotNowListening(Track *pTrack);
  private:
    QLinkedList<TrackPointer> m_trackedTracks;
};