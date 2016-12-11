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
    m_defaultTrackQueue(),
    m_prioTrackQueue(),
    m_defaultWorkers(),
    m_priorityWorkers(),
    m_pausedWorkers() {

    int maxThreads = m_pConfig->getValue<int>(ConfigKey("[Library]", "MaxAnalysisThreads"));
    int ideal = QThread::idealThreadCount();
    if (ideal < 1) {
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
    int total = m_priorityWorkers.size() +
        m_defaultWorkers.size() + m_pausedWorkers.size();
    return total > 0;
}
bool AnalyzerManager::isDefaultQueueActive() {
    int total = m_defaultWorkers.size() + m_pausedWorkers.size();
    return total > 0;
}

void AnalyzerManager::stop(bool shutdown) {
    m_defaultTrackQueue.clear();
    QListIterator<AnalyzerWorker*> it(m_defaultWorkers);
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
        QListIterator<AnalyzerWorker*> it2(m_priorityWorkers);
        while (it2.hasNext()) {
            AnalyzerWorker* worker = it2.next();
            worker->endProcess();
            m_endingWorkers.append(worker);
        }
        //TODO: ensure that they are all forcibly stopped.
    }
}
//Add a track to be analyzed with a priority worker. (Like those required by loading a track into a player).
void AnalyzerManager::analyseTrackNow(TrackPointer tio) {
    if (m_defaultTrackQueue.contains(tio)) {
        m_defaultTrackQueue.removeAll(tio);
    }
    //TODO: There's one scenario that we still miss: load on a deck a track that is currently
    //being analyzed by the background worker. We cannot reuse the background worker, but we should discard its work.
    if (!m_prioTrackQueue.contains(tio)) {
        m_prioTrackQueue.append(tio);
        if (m_priorityWorkers.size() < m_MaxThreads) {
            createNewWorker(WorkerType::priorityWorker);
            if (m_priorityWorkers.size() + m_defaultWorkers.size() > m_MaxThreads) {
                AnalyzerWorker * backwork = m_defaultWorkers.first();
                backwork->pause();
                //Ideally i would have done this on the slotPaused slot, but then i cannot 
                //ensure i won't call pause twice for the same worker.
                m_pausedWorkers.append(backwork);
                m_defaultWorkers.removeAll(backwork);
            }
        }
    }
}
// This is called from the GUI for the analysis feature of the library.
void AnalyzerManager::queueAnalyseTrack(TrackPointer tio) {
    if (!m_defaultTrackQueue.contains(tio)) {
        m_defaultTrackQueue.append(tio);
        if (m_pausedWorkers.size() + m_defaultWorkers.size() < m_MaxThreads) {
            createNewWorker(WorkerType::defaultWorker);
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
        emit(trackFinished(m_defaultWorkers.size() + m_defaultTrackQueue.size() - 1));
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
    TrackPointer track;
    AnalyzerWorker* forepaused=nullptr;
    foreach(AnalyzerWorker* worker, m_pausedWorkers) {
        if (worker->isPriorized()) { forepaused=worker; break; }
    }
    if (!forepaused) {
        if (worker->isPriorized()) {
            while (!track && !m_prioTrackQueue.isEmpty()) {
                track = m_prioTrackQueue.dequeue();
            }
        }
        else {
            if (m_defaultWorkers.size()  + m_pausedWorkers.size() <= m_MaxThreads) {
                //The while loop is done in the event that the track which was added to the queue is no 
                //longer available.
                while (!track && !m_defaultTrackQueue.isEmpty()) {
                    track = m_defaultTrackQueue.dequeue();
                }
            }
        }
    }
    if (track) {
        worker->nextTrack(track);
    }
    else {
        worker->endProcess();
        //Removing from active lists, so that "isActive" can return the correct value.
        m_defaultWorkers.removeAll(worker);
        m_priorityWorkers.removeAll(worker);
        m_endingWorkers.append(worker);

        if (forepaused) {
            forepaused->resume();
            m_pausedWorkers.removeOne(forepaused);
            m_priorityWorkers.append(forepaused);
        }
        else if (!m_pausedWorkers.isEmpty()) {
            AnalyzerWorker* otherworker = m_pausedWorkers.first();
            otherworker->resume();
            m_pausedWorkers.removeOne(otherworker);
            if (otherworker->isPriorized()) {
                m_priorityWorkers.append(otherworker);
            }
            else {
                m_defaultWorkers.append(otherworker);
            }
        }
    }
    //Check if background workers are empty.
    if (!isDefaultQueueActive()) {
        emit(queueEmpty());
    }
}
void AnalyzerManager::slotWorkerFinished(AnalyzerWorker* worker) {
    m_endingWorkers.removeAll(worker);
    m_defaultWorkers.removeAll(worker);
    m_priorityWorkers.removeAll(worker);
    m_pausedWorkers.removeAll(worker);
    if (!isDefaultQueueActive()) {
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
        while (!m_defaultWorkers.isEmpty() 
            && m_priorityWorkers.size() + m_defaultWorkers.size() > threads) {
                AnalyzerWorker * backwork = m_defaultWorkers.first();
                backwork->pause();
                //Ideally i would have done this on the slotPaused slot, but then i cannot 
                //ensure i won't call pause twice for the same worker.
                m_pausedWorkers.append(backwork);
                m_defaultWorkers.removeAll(backwork);
        }
        while (m_priorityWorkers.size() > threads) {
                AnalyzerWorker * backwork = m_priorityWorkers.first();
                backwork->pause();
                //Ideally i would have done this on the slotPaused slot, but then i cannot 
                //ensure i won't call pause twice for the same worker.
                m_pausedWorkers.append(backwork);
                m_priorityWorkers.removeAll(backwork);
        }
    }
    else {
        //resume workers
        int pendingworkers=threads-m_MaxThreads;
        foreach(AnalyzerWorker* worker, m_pausedWorkers) {
            if (worker->isPriorized() && pendingworkers > 0) {
                worker->resume();
                m_pausedWorkers.removeOne(worker);
                m_priorityWorkers.append(worker);
                --pendingworkers;
            }
        }
        foreach(AnalyzerWorker* worker, m_pausedWorkers) {
            if (!worker->isPriorized() && pendingworkers > 0) {
                worker->resume();
                m_pausedWorkers.removeOne(worker);
                m_defaultWorkers.append(worker);
                --pendingworkers;
            }
        }
        //Create new workers, if tracks in queue.
        pendingworkers = math_min(pendingworkers,m_defaultTrackQueue.size());
        for ( ;pendingworkers > 0; --pendingworkers) {
            createNewWorker(WorkerType::defaultWorker);
        }
    }
    m_MaxThreads=threads;
}

AnalyzerWorker* AnalyzerManager::createNewWorker(WorkerType wtype) {
    bool priorized = (wtype == WorkerType::priorityWorker);
    QThread* thread = new QThread();
    AnalyzerWorker* worker = new AnalyzerWorker(m_pConfig, ++m_nextWorkerId, priorized);
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
    if (priorized) {
        m_priorityWorkers.append(worker);
    }
    else {
        m_defaultWorkers.append(worker);
    }
    return worker;
}



