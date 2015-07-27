#ifndef TRACE_H
#define TRACE_H

#include <QString>
#include <QtDebug>

#include "util/cmdlineargs.h"
#include "util/stat.h"
#include "util/event.h"
#include "util/performancetimer.h"

class Trace {
  public:
    explicit Trace(const QString& tag, bool stdout=false, bool time=true)
            : m_tag(tag),
              m_stdout(stdout),
              m_time(time) {
        Event::start(m_tag);
        if (m_time) {
            m_timer.start();
        }
        if (m_stdout) {
            qDebug() << "START [" << m_tag << "]";
        }
    }
    virtual ~Trace() {
        Event::end(m_tag);
        qint64 elapsed = m_time ? m_timer.elapsed() : 0;
        if (m_stdout) {
            if (m_time) {
                qDebug() << "END [" << m_tag << "]"
                         << QString("elapsed: %1ns").arg(elapsed);
            } else {
                qDebug() << "END [" << m_tag << "]";
            }
        }
        if (m_time) {
            Stat::track(
                m_tag + "_duration",
                Stat::DURATION_NANOSEC,
                Stat::COUNT | Stat::AVERAGE | Stat::SAMPLE_VARIANCE | Stat::MAX | Stat::MIN,
                elapsed);
        }
    }

  private:
    const QString m_tag;
    const bool m_stdout, m_time;
    PerformanceTimer m_timer;

};

class DebugTrace : public Trace {
  public:
    DebugTrace(const QString& tag, bool time=true)
    : Trace(tag, CmdlineArgs::Instance().getDeveloper(), time) {
    }
    virtual ~DebugTrace() {
    }
};

#endif /* TRACE_H */
