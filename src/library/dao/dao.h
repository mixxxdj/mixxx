// dao.h
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#ifndef DAO_H
#define DAO_H

#include <QSemaphore>
#include <QDebug>
#include <QString>

class Semaphore : public QSemaphore {

public:
    explicit Semaphore(int n = 1) : QSemaphore(n) {}
    ~Semaphore(){}
    void acquire(const QString who, int n = 1) {
        // qDebug() << "\tin semaphore, acq by" << who;
        QSemaphore::acquire(n);
        // qDebug() << "\tsemaphore is acq by" << who;
    }
};

class DAO {
    virtual void initialize() = 0;

public:
    static void enterLockState(const QString where);
    static void exitLockState(const QString where);
    static Semaphore& pauseSemaphore() {
        return s_semaphorePause;
    }
    static Semaphore& transactionSemaphore() { return s_semaphoreTransaction; }

    static void pauseAcquire(const QString& who) { s_semaphorePause.acquire(who); }
    static void transactionAcquire(const QString& who) { s_semaphoreTransaction.acquire(who); }

protected:
    static Semaphore s_semaphorePause;
    static Semaphore s_semaphoreTransaction;
    static qint32 s_semaphoresLockedCount;
};

#endif /* DAO_H */
