#include "track/tracktiminginfo.h"

#include "moc_tracktiminginfo.cpp"

TrackTimingInfo::TrackTimingInfo(TrackPointer pTrack)
        : m_pElapsedTimer(new TrackTimers::ElapsedTimerQt()),
          m_pTimer(new TrackTimers::GUITickTimer()),
          m_pTrackPtr(pTrack),
          m_playedMs(0),
          m_isTrackScrobbable(false),
          m_isTimerPaused(true) {
    connect(m_pTimer.get(), SIGNAL(timeout()), this, SLOT(slotCheckIfScrobbable()));
    m_pElapsedTimer->invalidate();
}

void TrackTimingInfo::pausePlayedTime() {
    if (m_pElapsedTimer->isValid()) {
        m_playedMs += m_pElapsedTimer->elapsed();
        m_pElapsedTimer->invalidate();
        m_isTimerPaused = true;
    }
}

void TrackTimingInfo::resumePlayedTime() {
    if (!m_pElapsedTimer->isValid()) {
        m_pElapsedTimer->start();
        m_pTimer->start(1000);
        m_isTimerPaused = false;
    }
}

bool TrackTimingInfo::isTimerPaused() const {
    return m_isTimerPaused;
}

void TrackTimingInfo::resetPlayedTime() {
    m_pElapsedTimer->invalidate();
    m_isTimerPaused = true;
    m_playedMs = 0;
}

void TrackTimingInfo::setElapsedTimer(TrackTimers::ElapsedTimer* elapsedTimer) {
    m_pElapsedTimer.reset(elapsedTimer);
    m_pElapsedTimer->invalidate();
}

void TrackTimingInfo::setTimer(TrackTimers::RegularTimer* timer) {
    m_pTimer.reset(timer);
}

void TrackTimingInfo::slotCheckIfScrobbable() {
    if (m_isTrackScrobbable) {
        return;
    }
    qint64 msInTimer = 0;
    if (m_pElapsedTimer->isValid()) {
        msInTimer = m_pElapsedTimer->elapsed();
    } else {
        return;
    }
    if (!m_pTrackPtr) {
        qDebug() << "Track pointer is null when checking if track is scrobbable";
        return;
    }
    if ((msInTimer + m_playedMs) / 1000.0 >=
                    m_pTrackPtr->getDuration() / 2.0 ||
            (msInTimer + m_playedMs) / 1000.0 >= 240.0) {
        m_isTrackScrobbable = true;
        emit readyToBeScrobbled(m_pTrackPtr);
    } else {
        m_pTimer->start(1000);
    }
}

void TrackTimingInfo::setMsPlayed(qint64 ms) {
    m_playedMs = ms;
}

bool TrackTimingInfo::isScrobbable() const {
    return m_isTrackScrobbable;
}

void TrackTimingInfo::setTrackPointer(TrackPointer pTrack) {
    m_pTrackPtr = pTrack;
}

void TrackTimingInfo::slotGuiTick(double timeSinceLastTick) {
    TrackTimers::GUITickTimer* pTimer =
            qobject_cast<TrackTimers::GUITickTimer*>(m_pTimer.get());
    pTimer->slotTick(timeSinceLastTick);
}