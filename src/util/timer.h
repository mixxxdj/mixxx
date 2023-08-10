#pragma once

#include <QObject>
#include <QStringView>

#include "control/controlproxy.h"
#include "util/cmdlineargs.h"
#include "util/duration.h"
#include "util/parented_ptr.h"
#include "util/performancetimer.h"
#include "util/stat.h"

static constexpr Stat::ComputeFlags kDefaultComputeFlags = Stat::COUNT | Stat::SUM | Stat::AVERAGE |
        Stat::MAX | Stat::MIN | Stat::SAMPLE_VARIANCE;

// A Timer that is instrumented for reporting elapsed times to StatsManager
// under a certain key. Construct with custom compute flags to get custom values
// computed for the times.
class Timer {
  public:
    Timer(QString key,
            Stat::ComputeFlags compute = kDefaultComputeFlags);
    void start();

    // Restart the timer returning the time duration since it was last
    // started/restarted. If report is true, reports the elapsed time to the
    // associated Stat key.
    mixxx::Duration restart(bool report);

    // Returns time duration since start/restart was called. If report is true,
    // reports the elapsed time to the associated Stat key.
    mixxx::Duration elapsed(bool report);

  protected:
    QString m_key;
    Stat::ComputeFlags m_compute;
    PerformanceTimer m_time;
};

class ScopedTimer {
  public:
    ScopedTimer(QStringView key,
            Stat::ComputeFlags compute = kDefaultComputeFlags)
            : ScopedTimer(key, QStringView(), compute) {
    }
    ScopedTimer(QStringView key, int i, Stat::ComputeFlags compute = kDefaultComputeFlags)
            : ScopedTimer(key, QString::number(i), compute) {
    }

    ScopedTimer(QStringView key, QStringView arg, Stat::ComputeFlags compute = kDefaultComputeFlags)
            : m_pTimer(NULL),
              m_cancel(false) {
        if (CmdlineArgs::Instance().getDeveloper()) {
            initialize(key, arg, compute);
        }
    }

    virtual ~ScopedTimer() {
        if (m_pTimer) {
            if (!m_cancel) {
                m_pTimer->elapsed(true);
            }
            m_pTimer->~Timer();
        }
    }

    inline void initialize(QStringView key,
            QStringView arg,
            Stat::ComputeFlags compute = kDefaultComputeFlags) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QString strKey = arg.isEmpty() ? key.toString() : key.arg(arg);
#else
        QString strKey = arg.isEmpty() ? key.toString() : key.toString().arg(arg);
#endif
        m_pTimer = new(m_timerMem) Timer(strKey, compute);
        m_pTimer->start();
    }

    void cancel() {
        m_cancel = true;
    }
  private:
    Timer* m_pTimer;
    char m_timerMem[sizeof(Timer)];
    bool m_cancel;
};
