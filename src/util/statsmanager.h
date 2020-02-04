#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QThread>
#include <QAtomicInt>
#include <QtDebug>
#include <QMutex>
#include <QWaitCondition>
#include <QThreadStorage>
#include <QList>

#include "rigtorp/SPSCQueue.h"

#include "util/singleton.h"
#include "util/stat.h"
#include "util/event.h"

class StatsManager;

class StatsPipe final {
  public:
    explicit StatsPipe(StatsManager* pManager);
    ~StatsPipe();

    bool enqueue(StatReport report) {
        return m_queue.try_emplace(std::move(report));
    }

    bool dequeue(StatReport* pReport) {
        auto pFront = m_queue.front();
        if (!pFront) {
            return false;
        }
        *pReport = *pFront;
        m_queue.pop();
        return true;
    }

    size_t remainingCapacity() const {
        return m_queue.capacity() - m_queue.size();
    }

  private:
    StatsManager* m_pManager;
    rigtorp::SPSCQueue<StatReport> m_queue;
};

class StatsManager : public QThread, public Singleton<StatsManager> {
    Q_OBJECT
  public:
    explicit StatsManager();
    virtual ~StatsManager();

    // Returns true if write succeeds.
    bool maybeWriteReport(StatReport report);

    static bool s_bStatsManagerEnabled;

    // Tell the StatsManager to emit statUpdated for every stat that exists.
    void emitAllStats() {
        m_emitAllStats = 1;
    }

    void updateStats() {
        m_statsPipeCondition.wakeAll();
    }

  signals:
    void statUpdated(const Stat& stat);

  protected:
    virtual void run();

  private:
    void processIncomingStatReports();
    StatsPipe* getStatsPipeForThread();
    void onStatsPipeDestroyed(StatsPipe* pPipe);
    void writeTimeline(const QString& filename);

    QAtomicInt m_emitAllStats;
    QAtomicInt m_quit;
    QMap<QString, Stat> m_stats;
    QMap<QString, Stat> m_baseStats;
    QMap<QString, Stat> m_experimentStats;
    QList<Event> m_events;

    QWaitCondition m_statsPipeCondition;
    QMutex m_statsPipeLock;
    QList<StatsPipe*> m_statsPipes;
    QThreadStorage<StatsPipe*> m_threadStatsPipes;

    friend class StatsPipe;
};
