#pragma once

#include <QObject>
#include <QString>
#include <optional>
#include <type_traits>
#include <utility>

#include "control/controlproxy.h"
#include "util/cmdlineargs.h"
#include "util/duration.h"
#include "util/parented_ptr.h"
#include "util/performancetimer.h"
#include "util/qstringformat.h"
#include "util/stat.h"

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
    bool m_running;
    PerformanceTimer m_time;
};

class SuspendableTimer : public Timer {
  public:
    SuspendableTimer(const QString& key,
            Stat::ComputeFlags compute = kDefaultComputeFlags);
    void start();
    mixxx::Duration suspend();
    void go();
    mixxx::Duration elapsed(bool report);

  private:
    mixxx::Duration m_leapTime;
};

class ScopedTimer {
  public:
    // Allows the timer to contain a format string which is only assembled
    // when we're not in `--developer` mode.
    /// @param compute Flags to use for the Stat::ComputeFlags (can be omitted)
    /// @param key The format string for the key
    /// @param args The arguments to pass to the format string
    template<typename T, typename... Ts>
    ScopedTimer(Stat::ComputeFlags compute, T&& key, Ts&&... args)
            : m_timer(std::nullopt) {
        static_assert(!std::is_same_v<std::remove_cvref_t<std::decay_t<T>>, char*>,
                "string type likely not UTF-16, please pass QStringLiteral by "
                "means of u\"key text\"_s");
        if (!CmdlineArgs::Instance().getDeveloper()) {
            return; // leave timer in cancelled state
        }
        // key is explicitly `auto` to allow passing non-QString types such as `const char*`
        // its conversion is delayed until after we've checked if we're in developer mode
        auto assembledKey = QString(std::forward<T>(key));
        if constexpr (sizeof...(args) > 0) {
            // only try to call QString::arg when we've been given parameters
            assembledKey = assembledKey.arg(convertToQStringConvertible(std::forward<Ts>(args))...);
        }
        m_timer = std::make_optional<Timer>(assembledKey, compute);
        m_timer->start();
    }
    template<typename T, typename... Ts>
    ScopedTimer(T&& key, Ts&&... args)
            : ScopedTimer(kDefaultComputeFlags, std::forward<T>(key), std::forward<Ts>(args)...) {
    }

    ~ScopedTimer() {
        if (m_timer.has_value()) {
            m_timer->elapsed(true);
        }
    }

    void cancel() {
        m_timer.reset();
    }
  private:
    // nullopt also counts as cancelled
    std::optional<Timer> m_timer;
};

// A timer that provides a similar API to QTimer but uses render events from the
// VSyncThread as its source of timing events. This means the timer cannot fire
// at a rate faster than the user's configured waveform FPS.
class GuiTickTimer : public QObject {
    Q_OBJECT
  public:
    GuiTickTimer(QObject* pParent);

    void start(mixxx::Duration interval);
    bool isActive() const { return m_bActive; }
    void stop();

  signals:
    void timeout();

  private slots:
    void slotGuiTick(double v);

  private:
    parented_ptr<ControlProxy> m_pGuiTick;
    mixxx::Duration m_interval;
    mixxx::Duration m_lastUpdate;
    bool m_bActive;
};
