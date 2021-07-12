#pragma once

#include <QThread>

// Subclass of QThread exposing static sleep methods. Qt developers know
// better than us and therefore made these methods protected but I'm a
// rebel.
class SleepableQThread : public QThread {
    Q_OBJECT;
    Q_DISABLE_COPY(SleepableQThread);
public:
    explicit SleepableQThread(QObject *parent = NULL) : QThread(parent) { }
    virtual ~SleepableQThread() { }
    static void sleep(unsigned long secs) {
        QThread::sleep(secs);
    }

    static void msleep(unsigned long msecs) {
        QThread::msleep(msecs);
    }

    static void usleep(unsigned long usecs) {
        QThread::usleep(usecs);
    }
};
