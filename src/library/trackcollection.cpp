#include <QtSql>
#include <QtDebug>

#include "library/trackcollection.h"

#include "defs.h"
#include "library/librarytablemodel.h"
#include "library/schemamanager.h"
#include "library/queryutil.h"
#include "soundsourceproxy.h"
#include "trackinfoobject.h"
#include "xmlparse.h"
#include "util/sleepableqthread.h"

#include "controlobjectthread.h"
#include "controlobject.h"
#include "configobject.h"

#define MAX_LAMBDA_COUNT 8

TrackCollection::TrackCollection(ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_stop(false),
          m_semLambdasReadyToCall(0),
          m_semLambdasFree(MAX_LAMBDA_COUNT),
          m_pCOTPlaylistIsBusy(NULL),
          m_inCallSyncCount(0){
    DBG() << "TrackCollection constructor \tfrom thread id="
          << QThread::currentThreadId() << "name="
          << QThread::currentThread()->objectName();
    m_pTrackCollectionPrivate = new TrackCollectionPrivate(pConfig);
}

TrackCollection::~TrackCollection() {
    DBG() << "~TrackCollection()";
    delete m_pCOTPlaylistIsBusy;
}

void TrackCollection::run() {
    QThread::currentThread()->setObjectName("TrackCollection");
    DBG() << "id=" << QThread::currentThreadId()
          << "name=" << QThread::currentThread()->objectName();

    m_pTrackCollectionPrivate->initialize();

    emit(initialized()); // to notify that Daos can be used

    func lambda;
    // main TrackCollection's loop
    while (!m_stop) {
        if (!m_semLambdasReadyToCall.tryAcquire(1)) {
            setUiEnabled(true); // no Lambda available, so unlock GUI.
            m_semLambdasReadyToCall.acquire(1); // Sleep until new Lambdas have arrived
        }
        // got lambda on queue (or needness to stop thread)
        if (m_stop) {
            // m_stop == true
            DBG() << "Need to stop thread";
            while (!m_lambdas.isEmpty()) {
                // There are remaining lambdas in queue. Must execute them all.
                m_lambdasQueueMutex.lock();
                lambda = m_lambdas.dequeue();
                m_lambdasQueueMutex.unlock();
                lambda(m_pTrackCollectionPrivate);
                m_semLambdasFree.release(1);
            }
            break;
        } else {
            if (!m_lambdas.isEmpty()) {
                m_lambdasQueueMutex.lock();
                lambda = m_lambdas.dequeue();
                m_lambdasQueueMutex.unlock();
                lambda(m_pTrackCollectionPrivate);
                m_semLambdasFree.release(1);
            }
        }
    }
    //TODO(MK) TearDown TrackCollectionPrivate
    DBG() << " ### Thread ended ###";
}

// callAsync can be called from any thread. Be careful: callAsync runs asynchonously.
//    @param: lambda function, string (for debug purposes).
//    Catched values in lambda must be guarantly alive until end of execution lambda
//    (if catched by value) or you can catch by value.
void TrackCollection::callAsync(func lambda, QString where) {
    qDebug() << "callAsync from" << where;
    if (lambda == NULL || m_stop) return;
    const bool inMainThread = QThread::currentThread() == qApp->thread();
    if (inMainThread) {
        setUiEnabled(false);
    }
    addLambdaToQueue(lambda);
}

// callAsync can be called from any thread
//    @param: lambda function, string (for debug purposes).
void TrackCollection::callSync(func lambda, QString where) {
    qDebug() << "callSync BEGIN from" << where;
    auto waitCycles = 0;
    if (QThread::currentThread() == this){
        // we are already in TrackCollection-Thread. Just execute the lambda
        qDebug() << "!!! WARNING: CallSync from ThreadCollection Thread. Executing without locks";
        lambda(m_pTrackCollectionPrivate);
    } else {
        if (m_inCallSyncCount > 0) { // callSync inside callSync is workable, but we must avoid it
            DBG() << "nested CallSync";
            Q_ASSERT(false); // just stop here in debug
        }
        qDebug() << "callSync from" << where;
        ++m_inCallSyncCount;

        if (lambda == NULL || m_stop) return;

        const bool inMainThread = QThread::currentThread() == qApp->thread();
        if (inMainThread) {
            setUiEnabled(false);
        }
        QMutex mutex;
        mutex.lock();
        func lambdaWithMutex =
                [&mutex, &lambda] (TrackCollectionPrivate* pTrackCollectionPrivate) {
                    lambda(pTrackCollectionPrivate);
                    mutex.unlock();
                };
        addLambdaToQueue(lambdaWithMutex);

        while (!mutex.tryLock(1)) { // timeout 1 ms
            MainExecuter::getInstance().call();
            //if (inMainThread) {
            //    // Check if we are NOT in nested callSync
            //    if (m_inCallSyncCount == 1) {
            //        // This calls the pending slots, which can result in stacking up
            //        // recursive callSync() calls
            //        qApp->processEvents(QEventLoop::AllEvents);
            //    }
            //}
            //DBG() << "Start animation";
            //animationIsShowed = true;
            waitCycles++;
        }
        // If we are here, the lamda is done.
        mutex.unlock(); // QMutexes should be always destroyed in unlocked state.
        if (inMainThread) {
            setUiEnabled(true);
        }

        --m_inCallSyncCount;
    }
    qDebug() << "callSync END from" << where << waitCycles << "waitCycles";
}

void TrackCollection::addLambdaToQueue(func lambda) {
    m_semLambdasFree.acquire(1); // we'll wait here if lambdas count in queue is greater then MAX_LAMBDA_COUNT
    m_lambdasQueueMutex.lock();
    m_lambdas.enqueue(lambda);
    m_lambdasQueueMutex.unlock();
    m_semLambdasReadyToCall.release(1);
}

void TrackCollection::setupControlObject() {
    m_pCOTPlaylistIsBusy = new ControlObjectThread(ConfigKey("[Playlist]", "isBusy"));
}

void TrackCollection::setUiEnabled(const bool enabled) {
// lock/unlock GUI elements by setting [playlist] "isBusy
//TODO() causes an endless loop. can be re-enabled after The track cash is moved
//    if (m_pCOTPlaylistIsBusy)
//        m_pCOTPlaylistIsBusy->set(enabled ? 0.0 : 1.0);
}

void TrackCollection::stopThread() {
    DBG() << "Stopping thread";
		//Todo(MK): TearDown TrackCollectionPrivate here
/*
    callSync([this](void) {
        DBG() << "Closing database connection";
        m_pTrackDao->finish();
        delete m_pTrackDao; // do it here becouse it uses DB connection
    }, "TrackCollection::stopThread, delete m_pTrackDao");

    callSync([this](void) {
        DBG() << "Closing database connection";
        if (m_pDatabase->isOpen()) {
            // There should never be an outstanding transaction when this code is
            // called. If there is, it means we probably aren't committing a
            // transaction somewhere that should be.
            if (m_pDatabase->rollback()) {
                qDebug() << "ERROR: There was a transaction in progress on the main database connection while shutting down."
                         << "There is a logic error somewhere.";
            }
            m_pDatabase->close();
        } else {
            qDebug() << "ERROR: The main database connection was closed before TrackCollection closed it."
                     << "There is a logic error somewhere.";
        }
    }, "TrackCollection::stopThread, closing DB connection");
*/
    m_semLambdasReadyToCall.release(1);
    m_stop = true;
}

QSharedPointer<BaseTrackCache> TrackCollection::getTrackSource() {
    return m_defaultTrackSource;
}

void TrackCollection::setTrackSource(QSharedPointer<BaseTrackCache> trackSource) {
    Q_ASSERT(m_defaultTrackSource.isNull());
    m_defaultTrackSource = trackSource;
}

ConfigObject<ConfigValue>* TrackCollection::getConfig() {
    return m_pConfig;
}
