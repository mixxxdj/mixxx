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

#include "util/fifo.h"
#include "singleton.h"
#include "util/stat.h"

class StatsManager : public QThread, public Singleton<StatsManager> {
  public:
    explicit StatsManager();
    virtual ~StatsManager();

    // Returns true if write succeeds.
    bool maybeWriteReport(const StatReport& report);

  protected:
    virtual void run();

  private:
    void processIncomingStatReports();

    FIFO<StatReport> m_statsPipe;
    QWaitCondition m_statsPipeCondition;
    QMutex m_statsPipeLock;
    QAtomicInt m_quit;
    QMap<QString, Stat> m_stats;
};


#endif /* STATSMANAGER_H */
