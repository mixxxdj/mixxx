#include "track/tracktiminginfo.h"

#include "moc_tracktiminginfo.cpp"
#include "util/parented_ptr.h"

namespace {
const double kMsInOneSecond = 1000;
const double kAbsoluteDurationInSecondBeforeScrobbling = 240;
} // namespace

TrackTimingInfo::TrackTimingInfo(TrackPointer pTrack, QObject* parent)
        : QObject(parent),
          m_pElapsedTimer(new TrackTimers::ElapsedTimerQt()),
          m_pTimer(make_parented<TrackTimers::GUITickTimer>(this)),
          m_pTrack(pTrack),
          m_playedMs(0),
          m_isTrackScrobbable(false),
          m_isTimerPaused(true) {
    connect(m_pTimer.get(),
            &TrackTimers::RegularTimer::timeout,
            this,
            &TrackTimingInfo::slotCheckIfScrobbable);
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

void TrackTimingInfo::setTimer(parented_ptr<TrackTimers::RegularTimer> timer) {
    m_pTimer = std::move(timer);
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
    if (!m_pTrack) {
        qDebug() << "Track pointer is null when checking if track is scrobbable";
        return;
    }
    bool overHalfTrackDuration = (msInTimer + m_playedMs) / kMsInOneSecond >=
            m_pTrack->getDuration() / 2.0;
    bool overAbsoluteMark = (msInTimer + m_playedMs) / kMsInOneSecond >=
            kAbsoluteDurationInSecondBeforeScrobbling;
    if (overHalfTrackDuration || overAbsoluteMark) {
        m_isTrackScrobbable = true;
        emit readyToBeScrobbled(m_pTrack);
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
    m_pTrack = pTrack;
}

void TrackTimingInfo::slotGuiTick(double timeSinceLastTick) {
    TrackTimers::GUITickTimer* timer =
            qobject_cast<TrackTimers::GUITickTimer*>(m_pTimer.get());
    timer->slotTick(timeSinceLastTick);
}
