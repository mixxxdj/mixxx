#pragma once

#include <QString>
#include <QStringView>
#include <optional>

#include "util/cmdlineargs.h"
#include "util/duration.h"
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

// TODO: replace with std::experimental::scope_exit<Timer> once stabilized
class ScopedTimer {
  public:
    ScopedTimer(QStringView key,
            Stat::ComputeFlags compute = kDefaultComputeFlags)
            : ScopedTimer(key, QStringView(), compute) {
    }
    ScopedTimer(QStringView key,
            int i,
            Stat::ComputeFlags compute = kDefaultComputeFlags)
            : ScopedTimer(key,
                      CmdlineArgs::Instance().getDeveloper()
                              ? QString::number(i)
                              : QStringView(),
                      compute) {
    }

    ScopedTimer(QStringView key, QStringView arg, Stat::ComputeFlags compute = kDefaultComputeFlags)
            : m_maybeTimer(std::nullopt) {
        if (!CmdlineArgs::Instance().getDeveloper()) {
            return;
        }
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QString strKey = arg.isEmpty() ? key.toString() : key.arg(arg);
#else
        QString strKey = arg.isEmpty() ? key.toString() : key.toString().arg(arg);
#endif
        m_maybeTimer = std::make_optional<Timer>(std::move(strKey), compute);
        m_maybeTimer->start();
    }

    ~ScopedTimer() noexcept {
        if (m_maybeTimer) {
            m_maybeTimer->elapsed(true);
        }
    }

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

    ScopedTimer(ScopedTimer&&) = default;
    ScopedTimer& operator=(ScopedTimer&&) = default;

    void cancel() {
        m_maybeTimer.reset();
    }

  private:
    // use std::optional to avoid heap allocation which is frequent
    // because of ScopedTimer's temporary nature
    std::optional<Timer> m_maybeTimer;
};
