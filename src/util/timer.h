#pragma once

#include <QString>
#include <QStringView>
#include <optional>

#include "util/cmdlineargs.h"
#include "util/duration.h"
#include "util/performancetimer.h"
#include "util/stat.h"
#include "util/stringformat.h"

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
    // Allows the timer to contain a format string which is only assembled
    // when we're not in `--developer` mode.
    /// @param compute Flags to use for the Stat::ComputeFlags (can be omitted)
    /// @param key The format string as QStringLiteral to identify the timer
    /// @param args The arguments to pass to the format string
    template<typename T, typename... Ts>
    ScopedTimer(Stat::ComputeFlags compute, T&& key, Ts&&... args)
            : m_maybeTimer(std::nullopt) {
        // we take a T here so we can detect the type and warn the user accordingly
        // instead of paying for an implicit runtime conversion at the call site
        static_assert(std::is_same_v<T, QString>,
                "only QString is supported as key type. Wrap it in u\"\""
                "_s or QStringLiteral() "
                "to avoid runtime UTF-16 conversion.");
        // we can now assume that T is a QString.
        if (!CmdlineArgs::Instance().getDeveloper()) {
            return; // leave timer in cancelled state
        }
        DEBUG_ASSERT(key.capacity() == 0);
        m_maybeTimer = std::make_optional<Timer>(([&]() {
            // only try to call QString::arg when we've been given parameters
            if constexpr (sizeof...(args) > 0) {
                return key.arg(convertToQStringConvertible(std::forward<Ts>(args))...);
            } else {
                return key;
            }
        })(),
                compute);
        m_maybeTimer->start();
    }

    template<typename T, typename... Ts>
    ScopedTimer(T&& key, Ts&&... args)
            : ScopedTimer(kDefaultComputeFlags, std::forward<T>(key), std::forward<Ts>(args)...) {
    }

    ~ScopedTimer() noexcept {
        if (m_maybeTimer) {
            m_maybeTimer->elapsed(true);
        }
    }

    // copying would technically be possible, but likely not intended
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
