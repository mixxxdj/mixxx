#ifndef STATSMANAGER_H
#define STATSMANAGER_H

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

#include "util/spscfifo.h"
#include "util/singleton.h"
#include "util/stat.h"
#include "util/event.h"

class StatsManager;

class StatsPipe final {
    typedef SpscFifo<StatReport> Fifo;

  public:
    typedef Fifo::size_type size_type;

    explicit StatsPipe(StatsManager* pManager);
    ~StatsPipe();

    size_type writeAvailable() {
        return m_fifo.push_peek();
    }

    bool write(const StatReport& report) {
        return m_fifo.push(&report) == 1;
    }

    bool read(StatReport* pReport) {
        return m_fifo.pop(pReport) == 1;
    }

  private:
    Fifo m_fifo;
    StatsManager* m_pManager;
};

class StatsManager : public QThread, public Singleton<StatsManager> {
    Q_OBJECT
  public:
    explicit StatsManager();
    virtual ~StatsManager();

    // Returns true if write succeeds.
    bool maybeWriteReport(const StatReport& report);

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


#endif /* STATSMANAGER_H */
