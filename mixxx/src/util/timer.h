#ifndef TIMER_H
#define TIMER_H

#include "util/stat.h"

static const Stat::ComputeFlags kDefaultComputeFlags = Stat::COUNT | Stat::SUM | Stat::AVERAGE |
        Stat::MAX | Stat::MIN | Stat::SAMPLE_VARIANCE;

// A Timer that is instrumented for reporting elapsed times to StatsManager
// under a certain key. Construct with custom compute flags to get custom values
// computed for the times.
// TODO(rryan) use a more accurate timer than QTimer.
class Timer {
  public:
    Timer(const QString& key,
          Stat::ComputeFlags compute = kDefaultComputeFlags)
            : m_key(key),
              m_compute(compute),
              m_running(false) {
    }

    void start() {
        m_time.start();
        m_running = true;
    }

    // Restart the timer returning the milliseconds since it was last
    // started/restarted. If report is true, reports the elapsed time to the
    // associated Stat key.
    int restart(bool report) {
        if (m_running) {
            int msec = m_time.restart();
            if (report) {
                Stat::track(m_key, Stat::DURATION_MSEC, m_compute, msec);
            }
            return msec;
        } else {
            start();
            return 0;
        }
    }

    // Returns msec since start/restart was called. If report is true, reports
    // the elapsed time to the associated Stat key.
    int elapsed(bool report) {
        int msec = m_time.elapsed();
        if (report) {
            Stat::track(m_key, Stat::DURATION_MSEC, m_compute, msec);
        }
        return msec;
    }

  private:
    QString m_key;
    Stat::ComputeFlags m_compute;
    bool m_running;
    QTime m_time;
};

class ScopedTimer : public Timer {
  public:
    ScopedTimer(const QString& key,
                Stat::ComputeFlags compute = kDefaultComputeFlags)
            : Timer(key, compute),
              m_cancel(false) {
        start();
    }
    virtual ~ScopedTimer() {
        if (!m_cancel) {
            elapsed(true);
        }
    }
    void cancel() {
        m_cancel = true;
    }
  private:
    bool m_cancel;
};

#endif /* TIMER_H */
