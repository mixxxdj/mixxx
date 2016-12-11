#include "analyzer/analyzermanager.h"

#include <typeinfo>
#include <QThread>

#include <QtDebug>
#include <QMutexLocker>
#include <QListIterator>

#include "library/trackcollection.h"
#include "mixer/playerinfo.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/event.h"
#include "util/timer.h"
#include "util/trace.h"

AnalyzerManager::AnalyzerManager(UserSettingsPointer pConfig) :
    m_pConfig(pConfig),
    m_nextWorkerId(0),
    m_batchTrackQueue(),
    m_prioTrackQueue(),
    m_backgroundWorkers(),
    m_foregroundWorkers(),
    m_pausedWorkers() {

    int maxThreads = m_pConfig->getValue<int>(ConfigKey("[Library]", "MaxAnalysisThreads"));
    int ideal = QThread::idealThreadCount();
    if (QThread::idealThreadCount() < 1) {
        if (maxThreads > 0 && maxThreads <= 32) {
            qDebug() << "Cannot detect idealThreadCount. maxThreads is: " << maxThreads;
            ideal = maxThreads;
        }
        else {
            qWarning() << "Cannot detect idealThreadCount. Using the sane value of 1";
            ideal = 1;
        }
    }
    if (maxThreads <= 0 || maxThreads > ideal) {
        qWarning() << "maxThreads value is incorrect. Changing it to " << ideal;
        //Assume the value is incorrect, so fix it.
        maxThreads = ideal;
    }
    m_MaxThreads = maxThreads;
}

AnalyzerManager::~AnalyzerManager() {
    stop(true);
}

bool AnalyzerManager::isActive() {
    int total = m_foregroundWorkers.size() +
        m_backgroundWorkers.size() + m_pausedWorkers.size();
    return total > 0;
}
bool AnalyzerManager::isBackgroundWorkerActive() {
    int total = m_backgroundWorkers.size() + m_pausedWorkers.size();
    return total > 0;
}

void AnalyzerManager::stop(bool shutdown) {
    m_batchTrackQueue.clear();
    QListIterator<AnalyzerWorker*> it(m_backgroundWorkers);
    while (it.hasNext()) {
        AnalyzerWorker* worker = it.next();
        worker->endProcess();
        m_endingWorkers.append(worker);
    }
    QListIterator<AnalyzerWorker*> it3(m_pausedWorkers);
    while (it3.hasNext()) {
        AnalyzerWorker* worker = it3.next();
        worker->endProcess();
        m_endingWorkers.append(worker);
    }
    if (shutdown) {
        m_prioTrackQueue.clear();
        QListIterator<AnalyzerWorker*> it2(m_foregroundWorkers);
        while (it2.hasNext()) {
            AnalyzerWorker* worker = it2.next();
            worker->endProcess();
            m_endingWorkers.append(worker);
        }
        //TODO: ensure that they are all forcibly stopped.
    }
}
// Analyze it with a foreground worker. (foreground as in interactive, i.e. not a batch worker).
void AnalyzerManager::analyseTrackNow(TrackPointer tio) {
    if (m_batchTrackQueue.contains(tio)) {
        m_batchTrackQueue.removeAll(tio);
    }
    //TODO: There's one scenario that we still miss: load on a deck a track that is currently
    //being analyzed by the background worker. We cannot reuse the background worker, but we should discard its work.
    if (!m_prioTrackQueue.contains(tio)) {
        m_prioTrackQueue.append(tio);
        if (m_foregroundWorkers.size() < m_MaxThreads) {
            createNewWorker(false);
            if (m_foregroundWorkers.size() + m_backgroundWorkers.size() > m_MaxThreads) {
                AnalyzerWorker * backwork = m_backgroundWorkers.first();
                backwork->pause();
                //Ideally i would have done this on the slotPaused slot, but then i cannot 
                //ensure i won't call pause twice for the same worker.
                m_pausedWorkers.append(backwork);
                m_backgroundWorkers.removeAll(backwork);
            }
        }
    }
}
// This is called from the GUI for batch analysis.
void AnalyzerManager::queueAnalyseTrack(TrackPointer tio) {
    if (!m_batchTrackQueue.contains(tio)) {
        m_batchTrackQueue.append(tio);
        if (m_pausedWorkers.size() + m_backgroundWorkers.size() < m_MaxThreads) {
            createNewWorker(true);
        }
    }
}

// This slot is called from the decks and samplers when the track is loaded.
void AnalyzerManager::slotAnalyseTrack(TrackPointer tio) {
    analyseTrackNow(tio);
}


//slot
void AnalyzerManager::slotUpdateProgress(int workerIdx, struct AnalyzerWorker::progress_info* progressInfo) {
    //Updates to wave overview and player status text comes from a signal emited from the track by calling setAnalyzerProgress.
    progressInfo->current_track->setAnalyzerProgress(progressInfo->track_progress);
    //These update the Analysis feature and analysis view.
    emit(trackProgress(workerIdx, progressInfo->track_progress / 10));
    if (progressInfo->track_progress == 1000) {
        //Right now no one is listening to trackDone, but it's here just in case.
        emit(trackDone(progressInfo->current_track));
        //Report that a track analysis has finished, and how many are still remaining.
        emit(trackFinished(m_backgroundWorkers.size() + m_batchTrackQueue.size() - 1));
    }
    //TODO: Which is the consequence of not calling reset?
    progressInfo->current_track.reset();
    progressInfo->sema.release();
}

void AnalyzerManager::slotNextTrack(AnalyzerWorker* worker) {
    //TODO: The old scan checked in isLoadedTrackWaiting for pTrack->getAnalyzerProgress()
    // and either tried to load a previuos scan, or discarded the track if it had already been
    // analyzed. I don't fully understand the scenario and I am not doing that right now.

    //This is used when the maxThreads change. Extra workers are paused until active workers end.
    //Then, those are terminated and the paused workers are resumed.
    TrackPointer track = TrackPointer();
    AnalyzerWorker* forepaused=nullptr;
    foreach(AnalyzerWorker* worker, m_pausedWorkers) {
        if (!worker->isBatch()) { forepaused=worker; break; }
    }
    if (!forepaused) {
        if (worker->isBatch()) {
            if (m_backgroundWorkers.size()  + m_pausedWorkers.size() <= m_MaxThreads) {
                //The while loop is done in the event that the track which was added to the queue is no 
                //longer available.
                while (!track && !m_batchTrackQueue.isEmpty()) {
                    track = m_batchTrackQueue.dequeue();
                }
            }
        }
        else {
            while (!track && !m_prioTrackQueue.isEmpty()) {
                track = m_prioTrackQueue.dequeue();
            }
        }
    }
    if (track) {
        worker->nextTrack(track);
    }
    else {
        worker->endProcess();
        //Removing from active lists, so that "isActive" can return the correct value.
        m_backgroundWorkers.removeAll(worker);
        m_foregroundWorkers.removeAll(worker);
        m_endingWorkers.append(worker);

        if (forepaused) {
            forepaused->resume();
            m_pausedWorkers.removeOne(forepaused);
            m_foregroundWorkers.append(forepaused);
        }
        else if (!m_pausedWorkers.isEmpty()) {
            AnalyzerWorker* otherworker = m_pausedWorkers.first();
            otherworker->resume();
            m_pausedWorkers.removeOne(otherworker);
            if (otherworker->isBatch()) {
                m_backgroundWorkers.append(otherworker);
            }
            else {
                m_foregroundWorkers.append(otherworker);
            }
        }
    }
    //Check if background workers are empty.
    if (!isBackgroundWorkerActive()) {
        emit(queueEmpty());
    }
}
void AnalyzerManager::slotWorkerFinished(AnalyzerWorker* worker) {
    m_endingWorkers.removeAll(worker);
    m_backgroundWorkers.removeAll(worker);
    m_foregroundWorkers.removeAll(worker);
    m_pausedWorkers.removeAll(worker);
    if (!isBackgroundWorkerActive()) {
        emit(queueEmpty());
    }
}
void AnalyzerManager::slotPaused(AnalyzerWorker* worker) {
    //No useful code to execute right now.
}
void AnalyzerManager::slotErrorString(QString errMsg) {
    //TODO: This is currently unused.
    qWarning() << "Testing with :" << errMsg;
}


void AnalyzerManager::slotMaxThreadsChanged(int threads) {
    // If it is Active, adapt the amount of workers. If it is not active, it will just update the variable.
    if (threads < m_MaxThreads) {
        //Pause workers
        while (!m_backgroundWorkers.isEmpty() 
            && m_foregroundWorkers.size() + m_backgroundWorkers.size() > threads) {
                AnalyzerWorker * backwork = m_backgroundWorkers.first();
                backwork->pause();
                //Ideally i would have done this on the slotPaused slot, but then i cannot 
                //ensure i won't call pause twice for the same worker.
                m_pausedWorkers.append(backwork);
                m_backgroundWorkers.removeAll(backwork);
        }
        while (m_foregroundWorkers.size() > threads) {
                AnalyzerWorker * backwork = m_foregroundWorkers.first();
                backwork->pause();
                //Ideally i would have done this on the slotPaused slot, but then i cannot 
                //ensure i won't call pause twice for the same worker.
                m_pausedWorkers.append(backwork);
                m_foregroundWorkers.removeAll(backwork);
        }
    }
    else {
        //resume workers
        int pendingworkers=threads-m_MaxThreads;
        foreach(AnalyzerWorker* worker, m_pausedWorkers) {
            if (!worker->isBatch() && pendingworkers > 0) {
                worker->resume();
                m_pausedWorkers.removeOne(worker);
                m_foregroundWorkers.append(worker);
                --pendingworkers;
            }
        }
        foreach(AnalyzerWorker* worker, m_pausedWorkers) {
            if (worker->isBatch() && pendingworkers > 0) {
                worker->resume();
                m_pausedWorkers.removeOne(worker);
                m_backgroundWorkers.append(worker);
                --pendingworkers;
            }
        }
        //Create new workers, if tracks in queue.
        pendingworkers = math_min(pendingworkers,m_batchTrackQueue.size());
        for ( ;pendingworkers > 0; --pendingworkers) {
            createNewWorker(true);
        }
    }
    m_MaxThreads=threads;
}

AnalyzerWorker* AnalyzerManager::createNewWorker(bool batchJob) {
    QThread* thread = new QThread();
    AnalyzerWorker* worker = new AnalyzerWorker(m_pConfig, ++m_nextWorkerId, batchJob);
    worker->moveToThread(thread);
    //Auto startup and auto cleanup of worker and thread.
    connect(thread, SIGNAL(started()), worker, SLOT(slotProcess()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    //Connect with manager.
    connect(worker, SIGNAL(updateProgress(int, struct AnalyzerWorker::progress_info*)), this, SLOT(slotUpdateProgress(int, struct AnalyzerWorker::progress_info*)));
    connect(worker, SIGNAL(waitingForNextTrack(AnalyzerWorker*)), this, SLOT(slotNextTrack(AnalyzerWorker*)));
    connect(worker, SIGNAL(paused(AnalyzerWorker*)), this, SLOT(slotPaused(AnalyzerWorker*)));
    connect(worker, SIGNAL(workerFinished(AnalyzerWorker*)), this, SLOT(slotWorkerFinished(AnalyzerWorker*)));
    connect(worker, SIGNAL(error(QString)), this, SLOT(slotErrorString(QString)));
    thread->start(QThread::LowPriority);
    if (batchJob) {
        m_backgroundWorkers.append(worker);
    }
    else {
        m_foregroundWorkers.append(worker);
    }
    return worker;
}



