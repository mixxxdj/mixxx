#include "track/trackplaytimers.h"

TimerQt::TimerQt(QObject *parent = nullptr) :
    m_Timer(parent) 
{
    connect(&m_Timer,SIGNAL(timeout()),
            this,SIGNAL(timeout()));
}

TimerQt::~TimerQt() {}

ElapsedTimerQt::~ElapsedTimerQt() {}

void TimerQt::start(int msec) {
    m_Timer.start(msec);
}

bool TimerQt::isActive() {
    return m_Timer.isActive();
}

void TimerQt::stop() {
    m_Timer.stop();
}

void ElapsedTimerQt::invalidate() {
    m_elapsedTimer.invalidate();
}

bool ElapsedTimerQt::isValid() {
    return m_elapsedTimer.isValid();
}

void ElapsedTimerQt::start() {
    m_elapsedTimer.start();
}

qint64 ElapsedTimerQt::elapsed() {
    return m_elapsedTimer.elapsed();
}