#include "track/tracktiminginfo.h"

#include "moc_tracktiminginfo.cpp"

TrackTimingInfo::TrackTimingInfo(TrackPointer pTrack)
        : m_pElapsedTimer(new TrackTimers::ElapsedTimerQt()),
          m_pTimer(new TrackTimers::GUITickTimer()),
          m_pTrackPtr(pTrack),
          m_playedMs(0),
          m_isTrackScrobbable(false) {
    connect(m_pTimer.get(), SIGNAL(timeout()), this, SLOT(slotCheckIfScrobbable()));
}

void TrackTimingInfo::pausePlayedTime() {
    if (m_pElapsedTimer->isValid()) {
        m_playedMs += m_pElapsedTimer->elapsed();
        m_pElapsedTimer->invalidate();
        QObject::disconnect(m_pTimer.get(), SIGNAL(timeout()), this, SLOT(slotCheckIfScrobbable()));
    }
}

void TrackTimingInfo::resumePlayedTime() {
    if (!m_pElapsedTimer->isValid()) {
        connect(m_pTimer.get(), SIGNAL(timeout()), SLOT(slotCheckIfScrobbable()));
        m_pElapsedTimer->start();
        m_pTimer->start(1000);
    }
}

void TrackTimingInfo::resetPlayedTime() {
    m_pElapsedTimer->invalidate();
    disconnect(m_pTimer.get(), SIGNAL(timeout()), this, SLOT(slotCheckIfScrobbable()));
    m_playedMs = 0;
}

void TrackTimingInfo::setElapsedTimer(TrackTimers::ElapsedTimer* elapsedTimer) {
    m_pElapsedTimer.reset(elapsedTimer);
}

void TrackTimingInfo::setTimer(TrackTimers::RegularTimer* timer) {
    m_pTimer.reset(timer);
}

void TrackTimingInfo::slotCheckIfScrobbable() {
    qint64 msInTimer = 0;
    if (m_pElapsedTimer->isValid())
        msInTimer = m_pElapsedTimer->elapsed();
    if (!m_pTrackPtr) {
        qDebug() << "Track pointer is null when checking if track is scrobbable";
        return;
    }
    if (static_cast<double>(msInTimer + m_playedMs) / 1000.0 >=
            m_pTrackPtr->getDuration() / 2.0) {
        m_isTrackScrobbable = true;
        emit(readyToBeScrobbled(m_pTrackPtr));
    } else {
        m_pTimer->start(1000);
    }
}

void TrackTimingInfo::setMsPlayed(qint64 ms) {
    m_playedMs = ms;
}

bool TrackTimingInfo::isScrobbable() {
    return m_isTrackScrobbable;
}

void TrackTimingInfo::setTrackPointer(TrackPointer pTrack) {
    m_pTrackPtr = pTrack;
}
