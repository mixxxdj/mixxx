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
    Trace(const char* tag, const char* arg=NULL,
          bool writeToStdout=false, bool time=true)
            : m_writeToStdout(writeToStdout),
              m_time(time) {
        if (writeToStdout || CmdlineArgs::Instance().getDeveloper()) {
            initialize(tag, arg);
        }
    }

    Trace(const char* tag, int arg,
          bool writeToStdout=false, bool time=true)
            : m_writeToStdout(writeToStdout),
              m_time(time) {
        if (writeToStdout || CmdlineArgs::Instance().getDeveloper()) {
            initialize(tag, QString::number(arg));
        }
    }

    Trace(const char* tag, const QString& arg,
          bool writeToStdout=false, bool time=true)
            : m_writeToStdout(writeToStdout),
              m_time(time) {
        if (writeToStdout || CmdlineArgs::Instance().getDeveloper()) {
            initialize(tag, arg);
        }
    }

    virtual ~Trace() {
        // Proxy for whether initialize was called.
        if (!m_tag.isEmpty()) {
            Event::end(m_tag);

            qint64 elapsed = m_time ? m_timer.elapsed() : 0;
            if (m_writeToStdout) {
                if (m_time) {
                    qDebug() << "END [" << m_tag << "]"
                             << QString("elapsed: %1ns").arg(elapsed);
                } else {
                    qDebug() << "END [" << m_tag << "]";
                }
            }
            if (m_time) {
                // NOTE(rryan) do we need to do this string append? We could add
                // a check in StatsManager to infer that a DURATION_NANOSEC
                // event for the same tag that has an EVENT_START/EVENT_END is a
                // duration instead of changing the tag.
                Stat::track(
                        m_tag + "_duration",
                        Stat::DURATION_NANOSEC,
                        Stat::COUNT | Stat::AVERAGE | Stat::SAMPLE_VARIANCE |
                        Stat::MAX | Stat::MIN,
                        elapsed);
            }
        }
    }

  private:
    void initialize(const QString& key, const QString& arg) {
        if (arg.isEmpty()) {
            m_tag = key;
        } else {
            m_tag = key.arg(arg);
        }

        Event::start(m_tag);
        if (m_time) {
            m_timer.start();
        }
        if (m_writeToStdout) {
            qDebug() << "START [" << m_tag << "]";
        }
    }

    QString m_tag;
    const bool m_writeToStdout, m_time;
    PerformanceTimer m_timer;

};

class DebugTrace : public Trace {
  public:
    DebugTrace(const char* tag, bool time=true)
            : Trace(tag, "", CmdlineArgs::Instance().getDeveloper(), time) {
    }
    virtual ~DebugTrace() {
    }
};

#endif /* TRACE_H */
