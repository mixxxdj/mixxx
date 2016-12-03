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

AnalyzerManager* AnalyzerManager::m_pAnalyzerManager = NULL;

//static
AnalyzerManager& AnalyzerManager::getInstance(UserSettingsPointer pConfig) {
    if (!m_pAnalyzerManager) {
        // There exists only one UserSettingsPointer in the app, so it doens't matter
        // if we only assign it once.
        m_pAnalyzerManager = new AnalyzerManager(pConfig);
    }
    int maxThreads = m_pAnalyzerManager->m_pConfig->getValue<int>(ConfigKey("[Library]", "MaxAnalysisThreads"));
    if (maxThreads < 0 || maxThreads > 2 * QThread::idealThreadCount()) {
        //Assume the value is incorrect, so fix it.
        maxThreads = QThread::idealThreadCount();
        m_pAnalyzerManager->m_pConfig->setValue<int>(ConfigKey("[Library]", "MaxAnalysisThreads"), maxThreads);
    }
    if (maxThreads == 0) {
        maxThreads = QThread::idealThreadCount();
    }
    m_pAnalyzerManager->m_MaxThreads = maxThreads;
    return *m_pAnalyzerManager;
}

AnalyzerManager::AnalyzerManager(UserSettingsPointer pConfig) :
    m_pConfig(pConfig),
    m_batchTrackQueue(),
    m_prioTrackQueue(),
    m_backgroundWorkers(),
    m_foregroundWorkers(),
    m_pausedWorkers() {
}

AnalyzerManager::~AnalyzerManager() {
    stop(true);
}

bool AnalyzerManager::isActive(bool includeForeground) {
    //I don't count foregroundWorkers because
    int total = (includeForeground ? m_foregroundWorkers.size() : 0) +
        m_backgroundWorkers.size() + m_pausedWorkers.size();
    return total > 0;
}

void AnalyzerManager::stop(bool shutdown) {
    QListIterator<AnalyzerWorker*> it(m_backgroundWorkers);
    while (it.hasNext()) {
        AnalyzerWorker* worker = it.next();
        worker->endProcess();
    }
    QListIterator<AnalyzerWorker*> it3(m_pausedWorkers);
    while (it3.hasNext()) {
        AnalyzerWorker* worker = it3.next();
        worker->endProcess();
    }
    if (shutdown) {
        QListIterator<AnalyzerWorker*> it2(m_foregroundWorkers);
        while (it2.hasNext()) {
            AnalyzerWorker* worker = it2.next();
            worker->endProcess();
        }
        //TODO: ensure that they are all forcibly stopped.
    }
}
// Analyze it with a foreground worker. (foreground as in interactive, i.e. not a batch worker).
void AnalyzerManager::analyseTrackNow(TrackPointer tio) {
    if (m_batchTrackQueue.contains(tio)) {
        m_batchTrackQueue.removeAll(tio);
    }
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
void AnalyzerManager::slotUpdateProgress(struct AnalyzerWorker::progress_info* progressInfo) {
    if (progressInfo->current_track) {
        progressInfo->current_track->setAnalyzerProgress(
            progressInfo->track_progress);
        if (progressInfo->track_progress == 1000) {
            emit(trackDone(progressInfo->current_track));
        }
        progressInfo->current_track.clear();
    }
    emit(trackProgress(progressInfo->track_progress / 10));
    if (progressInfo->track_progress == 1000) {
        emit(trackFinished(m_backgroundWorkers.size() + m_batchTrackQueue.size() - 1));
    }
    progressInfo->sema.release();
}

void AnalyzerManager::slotNextTrack(AnalyzerWorker* worker) {
    //The while loop is done in the event that the track which was added to the queue is no 
    //longer available.

    //TODO: The old scan checked in isLoadedTrackWaiting for pTrack->getAnalyzerProgress()
    // and either tried to load a previuos scan, or discarded the track if it had already been
    // analyzed. I don't fully understand the scenario and I am not doing that right now.
    TrackPointer track = TrackPointer();
    if (worker->isBatch()) {
        while (!track && !m_batchTrackQueue.isEmpty()) {
            track = m_batchTrackQueue.dequeue();
        }
    }
    else {
        while (!track && !m_prioTrackQueue.isEmpty()) {
            track = m_prioTrackQueue.dequeue();
        }
    }
    if (track) {
        worker->nextTrack(track);
    }
    else {
        worker->endProcess();
        if (!m_pausedWorkers.isEmpty()) {
            AnalyzerWorker* otherworker = m_pausedWorkers.first();
            otherworker->resume();
            m_pausedWorkers.removeOne(otherworker);
        }
    }
    //Check if all queues are empty.
    if (!isActive(false)) {
        emit(queueEmpty());
    }
}
void AnalyzerManager::slotWorkerFinished(AnalyzerWorker* worker) {
    m_backgroundWorkers.removeAll(worker);
    m_foregroundWorkers.removeAll(worker);
    m_pausedWorkers.removeAll(worker);
}
void AnalyzerManager::slotPaused(AnalyzerWorker* worker) {
    //No useful code to execute right now.
}
void AnalyzerManager::slotErrorString(QString errMsg) {
    //TODO: This is currently unused.
    qWarning() << "Testing with :" << errMsg;
}

AnalyzerWorker* AnalyzerManager::createNewWorker(bool batchJob) {
    QThread* thread = new QThread();
    AnalyzerWorker* worker = new AnalyzerWorker(m_pConfig, batchJob);
    worker->moveToThread(thread);
    //Auto startup and auto cleanup of worker and thread.
    connect(thread, SIGNAL(started()), worker, SLOT(slotProcess()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    //Connect with manager.
    connect(worker, SIGNAL(updateProgress(struct AnalyzerWorker::progress_info*)), this, SLOT(slotUpdateProgress(struct AnalyzerWorker::progress_info*)));
    connect(worker, SIGNAL(waitingForNextTrack(AnalyzerWorker*)), this, SLOT(slotNextTrack(AnalyzerWorker*)));
    connect(worker, SIGNAL(paused(AnalyzerWorker*)), this, SLOT(slotPaused(AnalyzerWorker*)));
    connect(worker, SIGNAL(workerFinished(AnalyzerWorker*)), this, SLOT(slotWorkerFinished(AnalyzerWorker*)));
    connect(worker, SIGNAL(error(QString)), this, SLOT(slotErrorString(QString)));
    thread->start();
    if (batchJob) {
        m_backgroundWorkers.append(worker);
    }
    else {
        m_foregroundWorkers.append(worker);
    }
    return worker;
}



