#include <QtDebug>

#include "util/statsmanager.h"

// Roughly a million.
const int kStatsPipeSize = 2 << 20;

// Process when half-full.
const int kProcessLength = kStatsPipeSize / 2;

StatsManager::StatsManager()
    : QThread(),
      m_statsPipe(kStatsPipeSize),
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

bool StatsManager::maybeWriteReport(const StatReport& report) {
    bool success = m_statsPipe.write(&report, 1) == 1;
    int space = m_statsPipe.writeAvailable();
    if (space < kProcessLength) {
        m_statsPipeCondition.wakeAll();
    }
    if (!success) {
        qDebug() << "Failed to write StatReport!";
    }
    return success;
}

void StatsManager::processIncomingStatReports() {
    StatReport report;
    while (m_statsPipe.read(&report, 1) == 1) {
        QString tag(report.tag);
        Stat& info = m_stats[tag];
        info.m_tag = tag;
        info.m_type = report.type;
        info.m_compute = report.compute;
        info.processReport(report);
        free(report.tag);
    }
}

void StatsManager::run() {
    qDebug() << "StatsManager thread starting up.";
    while (true) {
        // We want to process reports even when we are about to quit since we
        // want to print the most accurate stat report on shutdown.
        processIncomingStatReports();
        if (m_quit == 1) {
            qDebug() << "StatsManager thread shutting down.";
            break;
        }
        m_statsPipeLock.lock();
        m_statsPipeCondition.wait(&m_statsPipeLock);
        m_statsPipeLock.unlock();
    }
}
