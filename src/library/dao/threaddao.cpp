#include <QDebug>
#include <QEventLoop>
#include <QApplication>
#include <QPushButton>

#include "threaddao.h"
#include "util/sleepableqthread.h"

const int SleepTimeMs = 10;
const int TooLong = 100;

ThreadDAO::ThreadDAO(QObject *parent) :
    QThread(parent),
    m_haveFunction(false),
    m_callFinished(false),
    m_stop(false) {
}


void ThreadDAO::run() {
    qDebug() << "ThreadDAO::run";
    while (!m_stop) {
        while (!m_haveFunction) {
            SleepableQThread::msleep(SleepTimeMs);
        }
        m_callFinished = false;

        m_lambda();

        m_haveFunction = false;
        m_callFinished = true;
    }
}

// tro: callSync must be executed in GUI thread. callSync waits while and
//      process application events while lambda is executing.
//      input: lambda - function that must be executed in ThredDAO's thread
//             second parameter - UI's control, what must be locked/unlocked
//             while lambda will be executed.
void ThreadDAO::callSync(func lambda, QWidget &w) {

    //TODO(tro) check lambda
    setLambda(lambda);

    bool animationIsShowed = false;
    int waitCycles = 0;

    while (m_haveFunction && !m_callFinished) {
        qApp->processEvents( QEventLoop::AllEvents );
        SleepableQThread::msleep(SleepTimeMs);
        ++waitCycles;
        if (waitCycles > TooLong && !animationIsShowed) {
            qDebug() << "Start animation";
            w.setEnabled(false);
//            m_progressindicator.startAnimation();
            animationIsShowed = true;
        }
    }

    if (animationIsShowed) {
        qDebug() << "Stop animation";
        w.setEnabled(true);
//        m_progressindicator.stopAnimation();
    }
}

void ThreadDAO::stopThread() {
    qDebug() << "Stopping thread";
    m_stop = true;
}

// store lambda to private member
void ThreadDAO::setLambda(func lambda) {
    m_lambda = lambda;
    m_haveFunction = true;
    m_callFinished = false;
}
