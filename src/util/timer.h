#pragma once

#include <QObject>

#include "control/controlproxy.h"
#include "util/cmdlineargs.h"
#include "util/duration.h"
#include "util/parented_ptr.h"
#include "util/performancetimer.h"
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
    ScopedTimer(const char* key, int i,
                Stat::ComputeFlags compute = kDefaultComputeFlags)
            : m_pTimer(NULL),
              m_cancel(false) {
        if (CmdlineArgs::Instance().getDeveloper()) {
            initialize(QString(key), QString::number(i), compute);
        }
    }

    ScopedTimer(const char* key, const char *arg = NULL,
                Stat::ComputeFlags compute = kDefaultComputeFlags)
            : m_pTimer(NULL),
              m_cancel(false) {
        if (CmdlineArgs::Instance().getDeveloper()) {
            initialize(QString(key), arg ? QString(arg) : QString(), compute);
        }
    }

    ScopedTimer(const char* key, const QString& arg,
                Stat::ComputeFlags compute = kDefaultComputeFlags)
            : m_pTimer(NULL),
              m_cancel(false) {
        if (CmdlineArgs::Instance().getDeveloper()) {
            initialize(QString(key), arg, compute);
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

    inline void initialize(const QString& key, const QString& arg,
                Stat::ComputeFlags compute = kDefaultComputeFlags) {
        QString strKey;
        if (arg.isEmpty()) {
            strKey = key;
        } else {
            strKey = key.arg(arg);
        }
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
