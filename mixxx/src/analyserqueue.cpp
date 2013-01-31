#include <QtDebug>
#include <QMutexLocker>

#include "trackinfoobject.h"
#include "playerinfo.h"
#include "analyserqueue.h"
#include "soundsourceproxy.h"
#include "playerinfo.h"
#include "util/timer.h"
#include "library/trackcollection.h"

#ifdef __TONAL__
#include "tonal/tonalanalyser.h"
#endif

#include "analyserwaveform.h"
#include "analyserbpm.h"
#include "analyserrg.h"
#ifdef __VAMP__
#include "analyserbeats.h"
#include "vamp/vampanalyser.h"
#endif

#include <typeinfo>

#define FINALISE_PERCENT 100 // in 0.1%,
                           // 0 for no progress during finalize
                           // 100 for 10% step after finalise


AnalyserQueue::AnalyserQueue(TrackCollection* pTrackCollection) :
        m_aq(),
        m_exit(false),
        m_aiCheckPriorities(false),
        m_tioq(),
        m_qm(),
        m_qwait(),
        m_queue_size(0) {
    connect(this, SIGNAL(updateProgress()),
            this, SLOT(slotUpdateProgress()));
    connect(this, SIGNAL(trackDone(TrackPointer)),
            &pTrackCollection->getTrackDAO(), SLOT(saveTrack(TrackPointer)));
}

void AnalyserQueue::addAnalyser(Analyser* an) {
    m_aq.push_back(an);
}

// This is called from the AnalyserQueue thread
bool AnalyserQueue::isLoadedTrackWaiting(TrackPointer tio) {
    QMutexLocker queueLocker(&m_qm);

    const PlayerInfo& info = PlayerInfo::Instance();
    TrackPointer pTrack;
    bool trackWaiting = false;
    QMutableListIterator<TrackPointer> it(m_tioq);
    while (it.hasNext()) {
        TrackPointer& pTrack = it.next();
        if (!pTrack) {
            it.remove();
            continue;
        }
        if (!trackWaiting) {
            trackWaiting = info.isTrackLoaded(pTrack);
        }
        // try to load waveforms for all new tracks first
        // and remove them from queue if already analysed
        // This avoids waiting for a running analysis for those tracks.
        int progress = pTrack->getAnalyserProgress();
        if (progress < 0) {
            // Load stored analysis
            QListIterator<Analyser*> ita(m_aq);
            bool processTrack = false;
            while (ita.hasNext()) {
                if (!ita.next()->loadStored(pTrack)) {
                    processTrack = true;
                }
            }
            if (!processTrack) {
                emitUpdateProgress(pTrack, 1000);
                it.remove();
            } else {
                emitUpdateProgress(pTrack, 0);
            }
        } else if (progress == 1000) {
            it.remove();
        }
    }
    if (info.isTrackLoaded(tio)) {
        return false;
    }
    return trackWaiting;
}

// This is called from the AnalyserQueue thread
TrackPointer AnalyserQueue::dequeueNextBlocking() {
    m_qm.lock();
    if (m_tioq.isEmpty()) {
        m_qwait.wait(&m_qm);

        if (m_exit) {
            m_qm.unlock();
            return TrackPointer();
        }
    }

    const PlayerInfo& info = PlayerInfo::Instance();
    TrackPointer pLoadTrack;
    QMutableListIterator<TrackPointer> it(m_tioq);
    while (it.hasNext()) {
        TrackPointer& pTrack = it.next();
        if (!pTrack) {
            it.remove();
            continue;
        }
        // Prioritize tracks that are loaded.
        if (info.isTrackLoaded(pTrack)) {
            qDebug() << "Prioritizing" << pTrack->getTitle() << pTrack->getLocation();
            pLoadTrack = pTrack;
            it.remove();
            break;
        }
    }

    if (!pLoadTrack && !m_tioq.isEmpty()) {
        pLoadTrack = m_tioq.dequeue();
    }

    m_qm.unlock();

    if (pLoadTrack) {
        qDebug() << "Analyzing" << pLoadTrack->getTitle() << pLoadTrack->getLocation();
    }
    // pTrack might be NULL, up to the caller to check.
    return pLoadTrack;
}

// This is called from the AnalyserQueue thread
bool AnalyserQueue::doAnalysis(TrackPointer tio, SoundSourceProxy* pSoundSource) {
    // TonalAnalyser requires a block size of 65536. Using a different value
    // breaks the tonal analyser. We need to use a smaller block size becuase on
    // Linux, the AnalyserQueue can starve the CPU of its resources, resulting
    // in xruns.. A block size of 8192 seems to do fine.
    const int ANALYSISBLOCKSIZE = 8192;

    int totalSamples = pSoundSource->length();
    //qDebug() << tio->getFilename() << " has " << totalSamples << " samples.";
    int processedSamples = 0;

    SAMPLE* data16 = new SAMPLE[ANALYSISBLOCKSIZE];
    CSAMPLE* samples = new CSAMPLE[ANALYSISBLOCKSIZE];

    QTime progressUpdateInhibitTimer;
    progressUpdateInhibitTimer.start(); // Inhibit Updates for 60 milliseconds

    int read = 0;
    bool dieflag = false;
    bool cancelled = false;
    int progress; // progress in 0 ... 100

    do {
        ScopedTimer t("AnalyserQueue::doAnalysis block");
        read = pSoundSource->read(ANALYSISBLOCKSIZE, data16);

        // To compare apples to apples, let's only look at blocks that are the
        // full block size.
        if (read != ANALYSISBLOCKSIZE) {
            t.cancel();
        }

        // Safety net in case something later barfs on 0 sample input
        if (read == 0) {
            t.cancel();
            break;
        }

        // If we get more samples than length, ask the analysers to process
        // up to the number we promised, then stop reading - AD
        if (read + processedSamples > totalSamples) {
            qDebug() << "While processing track of length " << totalSamples << " actually got "
                     << read + processedSamples << " samples, truncating analysis at expected length";
            read = totalSamples - processedSamples;
            dieflag = true;
        }

        for (int i = 0; i < read; i++) {
            samples[i] = ((float)data16[i])/32767.0f;
        }

        QListIterator<Analyser*> it(m_aq);

        while (it.hasNext()) {
            Analyser* an =  it.next();
            //qDebug() << typeid(*an).name() << ".process()";
            an->process(samples, read);
            //qDebug() << "Done " << typeid(*an).name() << ".process()";
        }

        // emit progress updates
        // During the doAnalysis function it goes only to 100% - FINALISE_PERCENT
        // because the finalise functions will take also some time
        processedSamples += read;
        //fp div here prevents insane signed overflow
        progress = (int)(((float)processedSamples)/totalSamples *
                         (1000 - FINALISE_PERCENT));

        if (m_progressInfo.track_progress != progress) {
            if (progressUpdateInhibitTimer.elapsed() > 60) {
                // Inhibit Updates for 60 milliseconds
                emitUpdateProgress(tio, progress);
                progressUpdateInhibitTimer.start();
            }
        }

        // Since this is a background analysis queue, we should co-operatively
        // yield every now and then to try and reduce CPU contention. The
        // analyser queue is CPU intensive so we want to get out of the way of
        // the audio callback thread.
        //QThread::yieldCurrentThread();
        //QThread::usleep(10);

        //has something new entered the queue?
        if (m_aiCheckPriorities) {
            m_aiCheckPriorities = false;
            if (isLoadedTrackWaiting(tio)) {
                qDebug() << "Interrupting analysis to give preference to a loaded track.";
                dieflag = true;
                cancelled = true;
            }
        }

        if (m_exit) {
            dieflag = true;
            cancelled = true;
        }

        // Ignore blocks in which we decided to bail for stats purposes.
        if (dieflag || cancelled) {
            t.cancel();
        }
    } while(read == ANALYSISBLOCKSIZE && !dieflag);

    delete[] data16;
    delete[] samples;

    return !cancelled; //don't return !dieflag or we might reanalyze over and over
}

void AnalyserQueue::stop() {
    m_exit = true;
    m_qm.lock();
    m_qwait.wakeAll();
    m_qm.unlock();
}

void AnalyserQueue::run() {
    unsigned static id = 0; //the id of this thread, for debugging purposes
    QThread::currentThread()->setObjectName(QString("AnalyserQueue %1").arg(++id));

    // If there are no analyzers, don't waste time running.
    if (m_aq.size() == 0)
        return;

    m_progressInfo.current_track = TrackPointer();
    m_progressInfo.track_progress = 0;
    m_progressInfo.queue_size = 0;
    m_progressInfo.sema.release(); // Initalise with one

    while (!m_exit) {
        TrackPointer nextTrack = dequeueNextBlocking();

        // It's important to check for m_exit here in case we decided to exit
        // while blocking for a new track.
        if (m_exit)
            return;

        // If the track is NULL, try to get the next one.
        // Could happen if the track was queued but then deleted.
        // Or if dequeueNextBlocking is unblocked by exit == true
        if (!nextTrack) {
            continue;
        }

        // Get the audio
        SoundSourceProxy* pSoundSource = new SoundSourceProxy(nextTrack);
        pSoundSource->open(); //Open the file for reading
        int iNumSamples = pSoundSource->length();
        int iSampleRate = pSoundSource->getSampleRate();

        if (iNumSamples == 0 || iSampleRate == 0) {
            qDebug() << "Skipping invalid file:" << nextTrack->getLocation();
            continue;
        }

        QListIterator<Analyser*> it(m_aq);
        bool processTrack = false;
        while (it.hasNext()) {
            // Make sure not to short-circuit initialise(...)
            if (it.next()->initialise(nextTrack, iSampleRate, iNumSamples)) {
                processTrack = true;
            }
        }

        m_qm.lock();
        m_queue_size = m_tioq.size();
        m_qm.unlock();

        if (processTrack) {
            emitUpdateProgress(nextTrack, 0);
            bool completed = doAnalysis(nextTrack, pSoundSource);
            if (!completed) {
                //This track was cancelled
                QListIterator<Analyser*> itf(m_aq);
                while (itf.hasNext()) {
                    itf.next()->cleanup(nextTrack);
                }
                queueAnalyseTrack(nextTrack);
                emitUpdateProgress(nextTrack, 0);
            } else {
                // 100% - FINALISE_PERCENT finished
                // This takes around 3 sec on a Atom Netbook
                QListIterator<Analyser*> itf(m_aq);
                while (itf.hasNext()) {
                    itf.next()->finalise(nextTrack);
                }
                emit(trackDone(nextTrack));
                emitUpdateProgress(nextTrack, 1000); // 100%
            }
        } else {
            emitUpdateProgress(nextTrack, 1000); // 100%
            qDebug() << "Skipping track analysis because no analyzer initialized.";
        }

        delete pSoundSource;

        m_qm.lock();
        m_queue_size = m_tioq.size();
        m_qm.unlock();
        if (m_queue_size == 0) {
            emit(queueEmpty()); // emit asynchrony for no deadlock
        }
    }
    emit(queueEmpty()); // emit in case of exit;
}

// This is called from the AnalyserQueue thread
void AnalyserQueue::emitUpdateProgress(TrackPointer tio, int progress) {
    if (!m_exit) {    
        // First tryAcqire will have always success because sema is initalised with on
        // The following tries will success if the previous signal was processed in the GUI Thread
        // This prevent the AnalysisQueue from filling up the GUI Thread event Queue
        // 100 % is emitted in any case
        if (progress < 1000 && progress > 0 ) {
            // Signals during processing are not required in any case
            if (!m_progressInfo.sema.tryAcquire()) {
               return;
            }
        } else {
            m_progressInfo.sema.acquire();
        }
        m_progressInfo.current_track = tio;
        m_progressInfo.track_progress = progress;
        m_progressInfo.queue_size = m_queue_size;
        emit(updateProgress());
    }
}

//slot
void AnalyserQueue::slotUpdateProgress() {
    if (m_progressInfo.current_track) {
        m_progressInfo.current_track->setAnalyserProgress(m_progressInfo.track_progress);
    }
    emit(trackProgress(m_progressInfo.track_progress/10));
    if (m_progressInfo.track_progress == 1000) {
        emit(trackFinished(m_progressInfo.queue_size));
    }
    m_progressInfo.sema.release();
}

//slot
void AnalyserQueue::slotAnalyseTrack(TrackPointer tio) {
    // This slot is called from the decks and and samplers when the track was loaded.
    m_aiCheckPriorities = true;
    queueAnalyseTrack(tio);
}

// This is called from the GUI and from the AnalyserQueue thread
void AnalyserQueue::queueAnalyseTrack(TrackPointer tio) {
    m_qm.lock();
    if (!m_tioq.contains(tio)) {
        m_tioq.enqueue(tio);
        m_qwait.wakeAll();
    }
    m_qm.unlock();
}

// static
AnalyserQueue* AnalyserQueue::createDefaultAnalyserQueue(
        ConfigObject<ConfigValue>* _config, TrackCollection* pTrackCollection) {
    AnalyserQueue* ret = new AnalyserQueue(pTrackCollection);

#ifdef __TONAL__
    ret->addAnalyser(new TonalAnalyser());
#endif

    ret->addAnalyser(new AnalyserWaveform(_config));
    ret->addAnalyser(new AnalyserGain(_config));
#ifdef __VAMP__
    VampAnalyser::initializePluginPaths();
    ret->addAnalyser(new AnalyserBeats(_config));
    //ret->addAnalyser(new AnalyserVampKeyTest(_config));
#else
    ret->addAnalyser(new AnalyserBPM(_config));
#endif

    ret->start(QThread::IdlePriority);
    return ret;
}

// static
AnalyserQueue* AnalyserQueue::createPrepareViewAnalyserQueue(
        ConfigObject<ConfigValue>* _config, TrackCollection* pTrackCollection) {
    AnalyserQueue* ret = new AnalyserQueue(pTrackCollection);

    ret->addAnalyser(new AnalyserWaveform(_config));
    ret->addAnalyser(new AnalyserGain(_config));
#ifdef __VAMP__
    VampAnalyser::initializePluginPaths();
    ret->addAnalyser(new AnalyserBeats(_config));
    //ret->addAnalyser(new AnalyserVampKeyTest(_config));
#else
    ret->addAnalyser(new AnalyserBPM(_config));
#endif

    ret->start(QThread::IdlePriority);
    return ret;
}

AnalyserQueue::~AnalyserQueue() {
    stop();
    m_progressInfo.sema.release();
    wait(); //Wait until thread has actually stopped before proceeding.

    QListIterator<Analyser*> it(m_aq);
    while (it.hasNext()) {
        Analyser* an = it.next();
        //qDebug() << "AnalyserQueue: deleting " << typeid(an).name();
        delete an;
    }
    //qDebug() << "AnalyserQueue::~AnalyserQueue()";
}
