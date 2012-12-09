#include <QtDebug>
#include <QMutexLocker>

#include "util/statsmanager.h"

// In practice we process stats pipes about once a minute @1ms latency.
const int kStatsPipeSize = 1 << 20;
const int kProcessLength = kStatsPipeSize * 4 / 5;

StatsPipe::StatsPipe(StatsManager* pManager)
        : FIFO<StatReport>(kStatsPipeSize),
          m_pManager(pManager) {
}

StatsPipe::~StatsPipe() {
    if (m_pManager) {
        m_pManager->onStatsPipeDestroyed(this);
    }
}

StatsManager::StatsManager()
    : QThread(),
      m_quit(0) {
    setObjectName("StatsManager");
    moveToThread(this);
    start(QThread::LowPriority);
}

StatsManager::~StatsManager() {
    m_quit = 1;
    m_statsPipeCondition.wakeAll();
    wait();
    qDebug() << "StatsManager shutdown report:";
    qDebug() << "=====================================";
    for (QMap<QString, Stat>::const_iterator it = m_stats.begin();
         it != m_stats.end(); ++it) {
        qDebug() << it.value();
    }
    qDebug() << "=====================================";
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
            QString tag(report.tag);
            Stat& info = m_stats[tag];
            info.m_tag = tag;
            info.m_type = report.type;
            info.m_compute = report.compute;
            info.processReport(report);
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

        if (m_quit == 1) {
            qDebug() << "StatsManager thread shutting down.";
            break;
        }
    }
}
