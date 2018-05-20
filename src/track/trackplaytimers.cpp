#include "track/trackplaytimers.h"

TrackTimers::TimerQt::TimerQt()
{
    connect(&m_Timer,SIGNAL(timeout()),
            this,SIGNAL(timeout()));
}

void TrackTimers::TimerQt::start(int msec) {
    m_Timer.start(msec);
}

bool TrackTimers::TimerQt::isActive() {
    return m_Timer.isActive();
}

void TrackTimers::TimerQt::stop() {
    m_Timer.stop();
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