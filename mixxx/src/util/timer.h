#ifndef TIMER_H
#define TIMER_H

#include "util/stat.h"
#include "util/performancetimer.h"

const Stat::ComputeFlags kDefaultComputeFlags = Stat::COUNT | Stat::SUM | Stat::AVERAGE |
        Stat::MAX | Stat::MIN | Stat::SAMPLE_VARIANCE;

// A Timer that is instrumented for reporting elapsed times to StatsManager
// under a certain key. Construct with custom compute flags to get custom values
// computed for the times.
class Timer {
  public:
    Timer(const QString& key,
          Stat::ComputeFlags compute = kDefaultComputeFlags);
    void start();

    // Restart the timer returning the nanoseconds since it was last
    // started/restarted. If report is true, reports the elapsed time to the
    // associated Stat key.
    int restart(bool report);

    // Returns nanoseconds since start/restart was called. If report is true,
    // reports the elapsed time to the associated Stat key.
    int elapsed(bool report);

  private:
    QString m_key;
    Stat::ComputeFlags m_compute;
    bool m_running;
    PerformanceTimer m_time;
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
