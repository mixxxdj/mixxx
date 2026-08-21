#include "track/trackplaytimers.h"

#include "moc_trackplaytimers.cpp"

TrackTimers::GUITickTimer::GUITickTimer(QObject* parent)
        : RegularTimer(parent),
          m_msSoFar(0.0),
          m_msTarget(0.0),
          m_isActive(false),
          m_timeoutSent(false) {
}

void TrackTimers::GUITickTimer::start(double msec) {
    m_msTarget = msec;
    m_msSoFar = 0.0;
    m_isActive = true;
    m_timeoutSent = false;
}

bool TrackTimers::GUITickTimer::isActive() const {
    return m_isActive;
}

void TrackTimers::GUITickTimer::stop() {
    m_isActive = false;
}

void TrackTimers::GUITickTimer::slotTick(double timeSinceLastTick) {
    if (!m_timeoutSent && m_isActive) {
        m_msSoFar += timeSinceLastTick;
        if (m_msSoFar >= m_msTarget) {
            m_timeoutSent = true;
            emit timeout();
        }
    }
}

void TrackTimers::ElapsedTimerQt::invalidate() {
    m_elapsedTimer.invalidate();
}

bool TrackTimers::ElapsedTimerQt::isValid() const {
    return m_elapsedTimer.isValid();
}

void TrackTimers::ElapsedTimerQt::start() {
    m_elapsedTimer.start();
}

qint64 TrackTimers::ElapsedTimerQt::elapsed() const {
    return m_elapsedTimer.elapsed();
}
