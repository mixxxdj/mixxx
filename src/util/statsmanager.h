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

#include "util/fifo.h"
#include "singleton.h"
#include "util/stat.h"
#include "util/event.h"

class StatsManager;

class StatsPipe : public FIFO<StatReport> {
  public:
    StatsPipe(StatsManager* pManager);
    virtual ~StatsPipe();
  private:
    StatsManager* m_pManager;
};

class StatsManager : public QThread, public Singleton<StatsManager> {
  public:
    explicit StatsManager();
    virtual ~StatsManager();

    // Returns true if write succeeds.
    bool maybeWriteReport(const StatReport& report);

    static bool s_bStatsManagerEnabled;
  protected:
    virtual void run();

  private:
    void processIncomingStatReports();
    StatsPipe* getStatsPipeForThread();
    void onStatsPipeDestroyed(StatsPipe* pPipe);
    void writeTimeline(const QString& filename);

    QAtomicInt m_quit;
    QMap<QString, Stat> m_stats;
    QList<Event> m_events;

    QWaitCondition m_statsPipeCondition;
    QMutex m_statsPipeLock;
    QList<StatsPipe*> m_statsPipes;
    QThreadStorage<StatsPipe*> m_threadStatsPipes;

    friend class StatsPipe;
};


#endif /* STATSMANAGER_H */
