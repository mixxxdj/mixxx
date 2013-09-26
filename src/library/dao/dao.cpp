#include "dao.h"
#include <QDebug>
#include <QApplication>
#include <QThread>

#include "util/performancetimer.h"

// lock/unlock db access with durable transactions
Semaphore DAO::s_semaphorePause(1);
Semaphore DAO::s_semaphoreTransaction(1);
qint32 DAO::s_semaphoresLockedCount = 0;


void DAO::enterLockState(const QString& where) {
    qDebug() << "DAO::enterLockState from " << where;
    if (s_semaphoresLockedCount == 0) {
        PerformanceTimer t;
        t.start();
        s_semaphorePause.acquire("DAO::enterLockState");
        qDebug() << "\t\t entering LockState pause time = " << QString::number( static_cast<double>(t.elapsed()) / 1e9, 'f', 4);
        t.restart();
        s_semaphoreTransaction.acquire("DAO::enterLockState");
        qDebug() << "\t\t entering LockState transaction time = " << QString::number( static_cast<double>(t.elapsed()) / 1e9, 'f', 4);
    }
    ++s_semaphoresLockedCount;
}

void DAO::exitLockState(const QString &where) {
    qDebug() << "DAO::exitLockState from " << where;
    --s_semaphoresLockedCount;

    if (s_semaphoresLockedCount == 0) {
        PerformanceTimer t;
        t.start();
        s_semaphoreTransaction.release();
        qDebug() << "\t\t releasing LockState transaction time = " << QString::number( static_cast<double>(t.elapsed()) / 1e9, 'f', 4);
        t.restart();
        s_semaphorePause.release();
        qDebug() << "\t\t releasing LockState pause time = " << QString::number( static_cast<double>(t.elapsed()) / 1e9, 'f', 4);
    }

    if (s_semaphoresLockedCount < 0) {
        s_semaphoresLockedCount = 0;
    }
}

void DAO::acquirePause(const QString &where) {
    s_semaphorePause.acquire(where);
}

void DAO::acquireTransaction(const QString &where) {
    s_semaphoreTransaction.acquire(where);
}

void DAO::releaseTransaction() {
    if (s_semaphoreTransaction.available()>0)
        return;
    s_semaphoreTransaction.release();
}

void DAO::releasePause() {
    if (s_semaphorePause.available()>0)
        return;
    s_semaphorePause.release();
}
