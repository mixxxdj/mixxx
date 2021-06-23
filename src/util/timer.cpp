#include "util/timer.h"

#include "moc_timer.cpp"
#include "util/experiment.h"
#include "util/time.h"
#include "waveform/guitick.h"

Timer::Timer(const QString& key, Stat::ComputeFlags compute)
        : m_key(key),
          m_compute(Stat::experimentFlags(compute)),
          m_running(false) {
}

void Timer::start() {
    m_running = true;
    m_time.start();
}

mixxx::Duration Timer::restart(bool report) {
    if (m_running) {
        mixxx::Duration elapsed = m_time.restart();
        if (report) {
            // Ignore the report if it crosses the experiment boundary.
            Experiment::Mode oldMode = Stat::modeFromFlags(m_compute);
            if (oldMode == Experiment::mode()) {
                Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute,
                            elapsed.toIntegerNanos());
            }
        }
        return elapsed;
    } else {
        start();
        return mixxx::Duration::fromNanos(0);
    }
}

mixxx::Duration Timer::elapsed(bool report) {
    mixxx::Duration elapsedTime = m_time.elapsed();
    if (report) {
        // Ignore the report if it crosses the experiment boundary.
        Experiment::Mode oldMode = Stat::modeFromFlags(m_compute);
        if (oldMode == Experiment::mode()) {
            Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute,
                        elapsedTime.toIntegerNanos());
        }
    }
    return elapsedTime;
}


SuspendableTimer::SuspendableTimer(const QString& key,
                                   Stat::ComputeFlags compute)
        : Timer(key, compute) {
}

void SuspendableTimer::start() {
    m_leapTime = mixxx::Duration::fromSeconds(0);
    Timer::start();
}

mixxx::Duration SuspendableTimer::suspend() {
    m_leapTime += m_time.elapsed();
    m_running = false;
    return m_leapTime;
}

void SuspendableTimer::go() {
    Timer::start();
}

mixxx::Duration SuspendableTimer::elapsed(bool report) {
    m_leapTime += m_time.elapsed();
    if (report) {
        // Ignore the report if it crosses the experiment boundary.
        Experiment::Mode oldMode = Stat::modeFromFlags(m_compute);
        if (oldMode == Experiment::mode()) {
            Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute,
                        m_leapTime.toIntegerNanos());
        }
    }
    return m_leapTime;
}

GuiTickTimer::GuiTickTimer(QObject* pParent)
        : QObject(pParent),
          m_pGuiTick(make_parented<ControlProxy>(
              "[Master]", "guiTickTime", this)),
          m_bActive(false) {
}

void GuiTickTimer::start(mixxx::Duration duration) {
    m_pGuiTick->connectValueChanged(this, &GuiTickTimer::slotGuiTick);
    m_interval = duration;
    m_lastUpdate = mixxx::Duration::fromSeconds(0);
    m_bActive = true;
}

void GuiTickTimer::stop() {
    m_pGuiTick->disconnect();
    m_bActive = false;
    m_interval = mixxx::Duration::fromSeconds(0);
    m_lastUpdate = mixxx::Duration::fromSeconds(0);
}

void GuiTickTimer::slotGuiTick(double) {
    if (m_bActive) {
        auto time = mixxx::Time::elapsed();
        if (time - m_lastUpdate >= m_interval) {
            m_lastUpdate = time;
            emit timeout();
        }
    }
}
