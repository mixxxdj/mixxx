#pragma once

#include <QLinkedList>
#include <QMutex>
#include <QObject>

#include "track/track.h"

class BaseTrackPlayer;

class ScrobblingManager : public QObject {
    Q_OBJECT
  public:
    ScrobblingManager();

  private:
    struct trackPlayerPair {
        TrackPointer pTrack;
        BaseTrackPlayer* pPlayer;
        trackPlayerPair(TrackPointer pTrack, BaseTrackPlayer* pPlayer)
                : pTrack(pTrack), pPlayer(pPlayer) {
        }
    };
    QLinkedList<trackPlayerPair> m_tracksToBeReset;
    QList<BaseTrackPlayer*> m_players;
    QMutex m_mutex;
    void resetTrack(BaseTrackPlayer* player);

  private slots:
    void slotTrackPaused(TrackPointer pPausedTrack);
    void slotTrackResumed(TrackPointer pResumedTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotNewTrackLoaded(TrackPointer pNewTrack);
    void slotPlayerEmpty();
};
