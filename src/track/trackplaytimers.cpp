#include "track/trackplaytimers.h"

#include "moc_trackplaytimers.cpp"

TrackTimers::GUITickTimer::GUITickTimer() {
}

void TrackTimers::GUITickTimer::start(int msec) {
}

bool TrackTimers::GUITickTimer::isActive() {
    return false;
}

void TrackTimers::GUITickTimer::stop() {
}

void TrackTimers::ElapsedTimerQt::invalidate() {
    m_elapsedTimer.invalidate();
}

bool TrackTimers::ElapsedTimerQt::isValid() {
    return m_elapsedTimer.isValid();
}

void TrackTimers::ElapsedTimerQt::start() {
    m_elapsedTimer.start();
}

qint64 TrackTimers::ElapsedTimerQt::elapsed() {
    return m_elapsedTimer.elapsed();
}
