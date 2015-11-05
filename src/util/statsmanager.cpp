#include <QtDebug>
#include <QMutexLocker>
#include <QTextStream>
#include <QFile>
#include <QMetaType>

#include "util/statsmanager.h"
#include "util/compatibility.h"
#include "util/cmdlineargs.h"

// In practice we process stats pipes about once a minute @1ms latency.
const int kStatsPipeSize = 1 << 20;
const int kProcessLength = kStatsPipeSize * 4 / 5;

// static
bool StatsManager::s_bStatsManagerEnabled = false;

StatsPipe::StatsPipe(StatsManager* pManager)
        : FIFO<StatReport>(kStatsPipeSize),
          m_pManager(pManager) {
    qRegisterMetaType<Stat>("Stat");
}

StatsPipe::~StatsPipe() {
    if (m_pManager) {
        m_pManager->onStatsPipeDestroyed(this);
    }
}

StatsManager::StatsManager()
        : QThread(),
          m_quit(0) {
    s_bStatsManagerEnabled = true;
    setObjectName("StatsManager");
    moveToThread(this);
    start(QThread::LowPriority);
}

StatsManager::~StatsManager() {
    s_bStatsManagerEnabled = false;
    m_quit = 1;
    m_statsPipeCondition.wakeAll();
    wait();
    qDebug() << "StatsManager shutdown report:";
    qDebug() << "=====================================";
    qDebug() << "ALL STATS";
    qDebug() << "=====================================";
    for (QMap<QString, Stat>::const_iterator it = m_stats.begin();
         it != m_stats.end(); ++it) {
        qDebug() << it.value();
    }

    if (!m_baseStats.isEmpty()) {
        qDebug() << "=====================================";
        qDebug() << "BASE STATS";
        qDebug() << "=====================================";
        for (QMap<QString, Stat>::const_iterator it = m_baseStats.begin();
             it != m_baseStats.end(); ++it) {
            qDebug() << it.value();
        }
    }

    if (!m_experimentStats.isEmpty()) {
        qDebug() << "=====================================";
        qDebug() << "EXPERIMENT STATS";
        qDebug() << "=====================================";
        for (QMap<QString, Stat>::const_iterator it = m_experimentStats.begin();
             it != m_experimentStats.end(); ++it) {
            qDebug() << it.value();
        }
    }
    qDebug() << "=====================================";

    if (CmdlineArgs::Instance().getTimelineEnabled()) {
        writeTimeline(CmdlineArgs::Instance().getTimelinePath());
    }
}

class OrderByTime {
  public:
    inline bool operator()(const Event& e1, const Event& e2) {
        return e1.m_time < e2.m_time;
    }
};

QString humanizeNanos(qint64 nanos) {
    double seconds = static_cast<double>(nanos) / 1e9;
    if (seconds > 1) {
        return QString("%1s").arg(QString::number(seconds));
    }

    double millis = static_cast<double>(nanos) / 1e6;
    if (millis > 1) {
        return QString("%1ms").arg(QString::number(millis));
    }

    double micros = static_cast<double>(nanos) / 1e3;
    if (micros > 1) {
        return QString("%1us").arg(QString::number(micros));
    }

    return QString("%1ns").arg(QString::number(nanos));
}

void StatsManager::writeTimeline(const QString& filename) {
    QFile timeline(filename);
    if (!timeline.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Could not open timeline file for writing:"
                 << timeline.fileName();
        return;
    }

    if (m_events.isEmpty()) {
        qDebug() << "No events recorded.";
        return;
    }

    // Sort by time.
    qSort(m_events.begin(), m_events.end(), OrderByTime());

    qint64 last_time = m_events[0].m_time;

    QMap<QString, qint64> startTimes;
    QMap<QString, qint64> endTimes;
    QMap<QString, Stat> tagStats;

    QTextStream out(&timeline);
    foreach (const Event& event, m_events) {
        qint64 last_start = startTimes.value(event.m_tag, -1);
        qint64 last_end = endTimes.value(event.m_tag, -1);

        qint64 duration_since_last_start = last_start == -1 ? 0 : event.m_time - last_start;
        qint64 duration_since_last_end = last_end == -1 ? 0 : event.m_time - last_end;

        if (event.m_type == Stat::EVENT_START) {
            // We last saw a start and we just saw another start.
            if (last_start > last_end) {
                qDebug() << "Mismatched start/end pair" << event.m_tag;
            }
            startTimes[event.m_tag] = event.m_time;
        } else if (event.m_type == Stat::EVENT_END) {
            // We last saw an end and we just saw another end.
            if (last_end > last_start) {
                qDebug() << "Mismatched start/end pair" << event.m_tag;
            }
            endTimes[event.m_tag] = event.m_time;
        }

        // TODO(rryan): CSV escaping
        qint64 elapsed = event.m_time - last_time;
        out << event.m_time << ","
            << "+" << humanizeNanos(elapsed) << ","
            << "+" << humanizeNanos(duration_since_last_start) << ","
            << "+" << humanizeNanos(duration_since_last_end) << ","
            << Stat::statTypeToString(event.m_type) << ","
            << event.m_tag << "\n";
        last_time = event.m_time;
    }

    timeline.close();
}

void StatsManager::onStatsPipeDestroyed(StatsPipe* pPipe) {
    QMutexLocker locker(&m_statsPipeLock);
    processIncomingStatReports();
    m_statsPipes.removeAll(pPipe);
}

StatsPipe* StatsManager::getStatsPipeForThread() {
    if (m_threadStatsPipes.hasLocalData()) {
        return m_threadStatsPipes.localData();
    }
    StatsPipe* pResult = new StatsPipe(this);
    m_threadStatsPipes.setLocalData(pResult);
    QMutexLocker locker(&m_statsPipeLock);
    m_statsPipes.push_back(pResult);
    return pResult;
}

bool StatsManager::maybeWriteReport(const StatReport& report) {
    StatsPipe* pStatsPipe = getStatsPipeForThread();
    if (pStatsPipe == NULL) {
        return false;
    }
    bool success = pStatsPipe->write(&report, 1) == 1;
    int space = pStatsPipe->writeAvailable();
    if (space < kProcessLength) {
        m_statsPipeCondition.wakeAll();
    }
    return success;
}

void StatsManager::processIncomingStatReports() {
    StatReport report;
    foreach (StatsPipe* pStatsPipe, m_statsPipes) {
        while (pStatsPipe->read(&report, 1) == 1) {
            QString tag = QString::fromUtf8(report.tag);
            Stat& info = m_stats[tag];
            info.m_tag = tag;
            info.m_type = report.type;
            info.m_compute = report.compute;
            info.processReport(report);
            emit(statUpdated(info));

            if (report.compute & Stat::STATS_EXPERIMENT) {
                Stat& experiment = m_experimentStats[tag];
                experiment.m_tag = tag;
                experiment.m_type = report.type;
                experiment.m_compute = report.compute;
                experiment.processReport(report);
            } else if (report.compute & Stat::STATS_BASE) {
                Stat& base = m_baseStats[tag];
                base.m_tag = tag;
                base.m_type = report.type;
                base.m_compute = report.compute;
                base.processReport(report);
            }

            if (CmdlineArgs::Instance().getTimelineEnabled() &&
                    (report.type == Stat::EVENT ||
                     report.type == Stat::EVENT_START ||
                     report.type == Stat::EVENT_END)) {
                Event event;
                event.m_tag = tag;
                event.m_type = report.type;
                event.m_time = report.time;
                m_events.append(event);
            }
            free(report.tag);
        }
    }
}

void StatsManager::run() {
    qDebug() << "StatsManager thread starting up.";
    while (true) {
        m_statsPipeLock.lock();
        m_statsPipeCondition.wait(&m_statsPipeLock);
        // We want to process reports even when we are about to quit since we
        // want to print the most accurate stat report on shutdown.
        processIncomingStatReports();
        m_statsPipeLock.unlock();

        if (load_atomic(m_emitAllStats) == 1) {
            for (QMap<QString, Stat>::const_iterator it = m_stats.begin();
                 it != m_stats.end(); ++it) {
                emit(statUpdated(it.value()));
            }
            m_emitAllStats = 0;
        }

        if (load_atomic(m_quit) == 1) {
            qDebug() << "StatsManager thread shutting down.";
            break;
        }
    }
}
