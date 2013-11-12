#ifndef TRACE_H
#define TRACE_H

#include <QString>
#include <QTime>

#include "util/cmdlineargs.h"
#include "util/stat.h"

class Trace {
  public:
    explicit Trace(const QString& tag, bool stdout=true, bool time=false)
            : m_tag(tag),
              m_stdout(stdout),
              m_time(time) {
        if (m_time) {
            m_timer.start();
        }
        if (m_stdout) {
            qDebug() << "START [" << m_tag << "]";
        }
        Stat::track(m_tag + "_enter", Stat::TRACE_START, Stat::COUNT, 0);

    }
    virtual ~Trace() {
        int elapsed = m_time ? m_timer.elapsed() : 0;
        if (m_stdout) {
            if (m_time) {
                qDebug() << "END [" << m_tag << "]"
                         << QString("elapsed: %1ms").arg(elapsed);
            } else {
                qDebug() << "END [" << m_tag << "]";
            }
        }

        Stat::track(m_tag + "_exit", Stat::TRACE_FINISH, Stat::COUNT, 0);
        if (m_time) {
            Stat::track(
                m_tag + "_duration",
                Stat::DURATION_MSEC,
                Stat::COUNT | Stat::AVERAGE | Stat::SAMPLE_VARIANCE | Stat::MAX | Stat::MIN,
                elapsed);
        }
    }

  private:
    const QString m_tag;
    const bool m_stdout, m_time;
    QTime m_timer;

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
