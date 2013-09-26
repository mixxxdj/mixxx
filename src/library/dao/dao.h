// dao.h
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#ifndef DAO_H
#define DAO_H

#include <QSemaphore>
#include <QDebug>
#include <QString>

// type lambda function -- to test C++0x
typedef std::function <void ()> func;

class Semaphore : public QSemaphore {

public:
    explicit Semaphore(int n = 1) : QSemaphore(n) {}
    ~Semaphore(){}
    void acquire(const QString who = QString(), int n = 1) {
        qDebug() << "\tin semaphore, acq by" << who;
        QSemaphore::acquire(n);
        qDebug() << "\tsemaphore is acq by" << who;
    }
};

class DAO {
    virtual void initialize() = 0;

public:
    static void enterLockState(const QString& where);
    static void exitLockState(const QString& where);
    static bool tryAcquirePause() { return s_semaphorePause.tryAcquire(); }
    static void acquirePause(const QString& where);
    static void releasePause();
    static void acquireTransaction(const QString &where);
    static void releaseTransaction();

protected:
    static Semaphore s_semaphorePause;
    static Semaphore s_semaphoreTransaction;
    static qint32 s_semaphoresLockedCount;
};

#endif /* DAO_H */
