#pragma once

#include <QObject>

#include "track/track.h"
#include "track/trackplaytimers.h"
#include "util/parented_ptr.h"

/// Manages different timing infos required for the scrobbling
/// services
class TrackTimingInfo : public QObject {
    Q_OBJECT
  public:
    explicit TrackTimingInfo(TrackPointer pTrack, QObject* parent = nullptr);
    void pausePlayedTime();
    void resumePlayedTime();
    void resetPlayedTime();
    // Take ownership of the pointer
    void setElapsedTimer(TrackTimers::ElapsedTimer* elapsedTimer);
    void setTimer(parented_ptr<TrackTimers::RegularTimer> timer);
    void setMsPlayed(qint64 ms);
    bool isScrobbable() const;
    void setTrackPointer(TrackPointer pTrack);
    bool isTimerPaused() const;
  public slots:
    void slotCheckIfScrobbable();
    void slotGuiTick(double timeSinceLastTick);
  signals:
    void readyToBeScrobbled(TrackPointer pTrack);

  private:
    std::unique_ptr<TrackTimers::ElapsedTimer> m_pElapsedTimer;
    parented_ptr<TrackTimers::RegularTimer> m_pTimer;
    TrackPointer m_pTrack;
    qint64 m_playedMs;
    bool m_isTrackScrobbable;
    bool m_isTimerPaused;
};
