#pragma once

#include <QLinkedList>
#include <QMutex>
#include <QObject>

#include "track/track.h"
#include "track/tracktiminginfo.h"

class BaseTrackPlayer;

class ScrobblingManager : public QObject {
    Q_OBJECT
  public:
    ScrobblingManager();

  private:
    struct TrackInfo {
        TrackPointer m_pTrack;
        TrackTimingInfo m_trackInfo;
        QLinkedList<BaseTrackPlayer*> m_players;
        TrackInfo(TrackPointer pTrack, BaseTrackPlayer* pPlayer) :
        m_pTrack(pTrack), m_trackInfo(pTrack)
        {} 
    };
    struct TrackToBeReset {
        TrackPointer m_pTrack;
        BaseTrackPlayer* m_pPlayer;
        TrackToBeReset(TrackPointer pTrack, BaseTrackPlayer* pPlayer)
                : m_pTrack(pTrack), m_pPlayer(pPlayer) {
        }
    };
    QMutex m_mutex;
    QLinkedList<TrackInfo*> m_trackList;
    QLinkedList<TrackToBeReset> m_tracksToBeReset;
    ControlProxy m_CPGuiTick;
    ControlProxy m_CPCrossfader;
    ControlProxy m_CPXFaderCurve;
    ControlProxy m_CPXFaderCalibration;
    ControlProxy m_CPXFaderMode;
    ControlProxy m_CPXFaderReverse;

    void resetTracks();
    bool isTrackAudible(TrackPointer pTrack, BaseTrackPlayer* pPlayer);
    double getPlayerVolume(BaseTrackPlayer* pPlayer);

  protected:
    void timerEvent(QTimerEvent* timerEvent) override;
  public slots:
    void slotTrackPaused(TrackPointer pPausedTrack);
    void slotTrackResumed(TrackPointer pResumedTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotNewTrackLoaded(TrackPointer pNewTrack);
    void slotPlayerEmpty();
  private slots:
    void slotGuiTick(double timeSinceLastTick);
    void slotReadyToBeScrobbled(TrackPointer pTrack);
};
