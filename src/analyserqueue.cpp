#include <typeinfo>

#include <QtDebug>
#include <QMutexLocker>

#include "trackinfoobject.h"
#include "playerinfo.h"
#include "analyserqueue.h"
#include "soundsourceproxy.h"
#include "playerinfo.h"
#include "util/timer.h"
#include "library/trackcollection.h"
#include "analyserwaveform.h"
#include "analyserrg.h"
#include "analyserbeats.h"
#include "analyserkey.h"
#include "vamp/vampanalyser.h"
#include "util/compatibility.h"
#include "util/event.h"
#include "util/trace.h"

// Measured in 0.1%
// 0 for no progress during finalize
// 1 to display the text "finalizing"
// 100 for 10% step after finalize
#define FINALIZE_PROMILLE 1

namespace {
    // Analysis is done in blocks.
    // We need to use a smaller block size, because on Linux the AnalyserQueue
    // can starve the CPU of its resources, resulting in xruns. A block size
    // of 4096 frames per block seems to do fine.
    const SINT kAnalysisChannels = Mixxx::AudioSource::kChannelCountStereo;
    const SINT kAnalysisFramesPerBlock = 4096;
    const SINT kAnalysisSamplesPerBlock =
            kAnalysisFramesPerBlock * kAnalysisChannels;
} // anonymous namespace

AnalyserQueue::AnalyserQueue(TrackCollection* pTrackCollection)
        : m_aq(),
          m_exit(false),
          m_aiCheckPriorities(false),
          m_sampleBuffer(kAnalysisSamplesPerBlock),
          m_tioq(),
          m_qm(),
          m_qwait(),
          m_queue_size(0) {
    Q_UNUSED(pTrackCollection);
    connect(this, SIGNAL(updateProgress()),
            this, SLOT(slotUpdateProgress()));
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

void AnalyserQueue::addAnalyser(Analyser* an) {
    m_aq.push_back(an);
}

// This is called from the AnalyserQueue thread
bool AnalyserQueue::isLoadedTrackWaiting(TrackPointer tio) {
    QMutexLocker queueLocker(&m_qm);

    const PlayerInfo& info = PlayerInfo::instance();
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
        Event::end("AnalyserQueue process");
        m_qwait.wait(&m_qm);
        Event::start("AnalyserQueue process");

        if (m_exit) {
            m_qm.unlock();
            return TrackPointer();
        }
    }

    const PlayerInfo& info = PlayerInfo::instance();
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
bool AnalyserQueue::doAnalysis(TrackPointer tio, Mixxx::AudioSourcePointer pAudioSource) {

    QTime progressUpdateInhibitTimer;
    progressUpdateInhibitTimer.start(); // Inhibit Updates for 60 milliseconds

    SINT frameIndex = pAudioSource->getMinFrameIndex();
    bool dieflag = false;
    bool cancelled = false;
    do {
        ScopedTimer t("AnalyserQueue::doAnalysis block");

        DEBUG_ASSERT(frameIndex < pAudioSource->getMaxFrameIndex());
        const SINT framesRemaining =
                pAudioSource->getMaxFrameIndex() - frameIndex;
        const SINT framesToRead =
                math_min(kAnalysisFramesPerBlock, framesRemaining);
        DEBUG_ASSERT(0 < framesToRead);

        const SINT framesRead =
                pAudioSource->readSampleFramesStereo(
                        kAnalysisFramesPerBlock,
                        &m_sampleBuffer);
        DEBUG_ASSERT(framesRead <= framesToRead);
        frameIndex += framesRead;
        DEBUG_ASSERT(pAudioSource->isValidFrameIndex(frameIndex));

        // To compare apples to apples, let's only look at blocks that are
        // the full block size.
        if (kAnalysisFramesPerBlock == framesRead) {
            // Complete analysis block of audio samples has been read.
            QListIterator<Analyser*> it(m_aq);
            while (it.hasNext()) {
                Analyser* an =  it.next();
                //qDebug() << typeid(*an).name() << ".process()";
                an->process(m_sampleBuffer.data(), m_sampleBuffer.size());
                //qDebug() << "Done " << typeid(*an).name() << ".process()";
            }
        } else {
            // Partial analysis block of audio samples has been read.
            // This should only happen at the end of an audio stream,
            // otherwise a decoding error must have occurred.
            if (frameIndex < pAudioSource->getMaxFrameIndex()) {
                // EOF not reached -> Maybe a corrupt file?
                qWarning() << "Failed to read sample data from file:"
                        << tio->getFilename()
                        << "@" << frameIndex;
                if (0 >= framesRead) {
                    // If no frames have been read then abort the analysis.
                    // Otherwise we might get stuck in this loop forever.
                    dieflag = true; // abort
                    cancelled = false; // completed, no retry
                }
            }
        }

        // emit progress updates
        // During the doAnalysis function it goes only to 100% - FINALIZE_PERCENT
        // because the finalise functions will take also some time
        //fp div here prevents insane signed overflow
        DEBUG_ASSERT(pAudioSource->isValidFrameIndex(frameIndex));
        const double frameProgress =
                double(frameIndex) / double(pAudioSource->getMaxFrameIndex());
        int progressPromille = frameProgress * (1000 - FINALIZE_PROMILLE);

        if (m_progressInfo.track_progress != progressPromille) {
            if (progressUpdateInhibitTimer.elapsed() > 60) {
                // Inhibit Updates for 60 milliseconds
                emitUpdateProgress(tio, progressPromille);
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
        if (load_atomic(m_aiCheckPriorities)) {
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
    } while (!dieflag && (frameIndex < pAudioSource->getMaxFrameIndex()));

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
            m_qm.lock();
            m_queue_size = m_tioq.size();
            m_qm.unlock();
            if (m_queue_size == 0) {
                emit(queueEmpty()); // emit asynchrony for no deadlock
            }
            continue;
        }

        Trace trace("AnalyserQueue analyzing track");

        // Get the audio
        SoundSourceProxy soundSourceProxy(nextTrack);
        Mixxx::AudioSourceConfig audioSrcCfg;
        audioSrcCfg.channelCountHint = kAnalysisChannels;
        Mixxx::AudioSourcePointer pAudioSource(soundSourceProxy.openAudioSource(audioSrcCfg));
        if (!pAudioSource) {
            qWarning() << "Failed to open file for analyzing:" << nextTrack->getLocation();
            continue;
        }

        QListIterator<Analyser*> it(m_aq);
        bool processTrack = false;
        while (it.hasNext()) {
            // Make sure not to short-circuit initialise(...)
            if (it.next()->initialise(nextTrack, pAudioSource->getFrameRate(), pAudioSource->getFrameCount() * kAnalysisChannels)) {
                processTrack = true;
            }
        }

        m_qm.lock();
        m_queue_size = m_tioq.size();
        m_qm.unlock();

        if (processTrack) {
            emitUpdateProgress(nextTrack, 0);
            bool completed = doAnalysis(nextTrack, pAudioSource);
            if (!completed) {
                //This track was cancelled
                QListIterator<Analyser*> itf(m_aq);
                while (itf.hasNext()) {
                    itf.next()->cleanup(nextTrack);
                }
                queueAnalyseTrack(nextTrack);
                emitUpdateProgress(nextTrack, 0);
            } else {
                // 100% - FINALIZE_PERCENT finished
                emitUpdateProgress(nextTrack, 1000 - FINALIZE_PROMILLE);
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
        // First tryAcqire will have always success because sema is initialized with on
        // The following tries will success if the previous signal was processed in the GUI Thread
        // This prevent the AnalysisQueue from filling up the GUI Thread event Queue
        // 100 % is emitted in any case
        if (progress < 1000 - FINALIZE_PROMILLE && progress > 0) {
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
        ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection) {
    AnalyserQueue* ret = new AnalyserQueue(pTrackCollection);

    ret->addAnalyser(new AnalyserWaveform(pConfig));
    ret->addAnalyser(new AnalyserGain(pConfig));
    VampAnalyser::initializePluginPaths();
    ret->addAnalyser(new AnalyserBeats(pConfig));
    ret->addAnalyser(new AnalyserKey(pConfig));

    ret->start(QThread::LowPriority);
    return ret;
}

// static
AnalyserQueue* AnalyserQueue::createAnalysisFeatureAnalyserQueue(
        ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection) {
    AnalyserQueue* ret = new AnalyserQueue(pTrackCollection);

    ret->addAnalyser(new AnalyserGain(pConfig));
    VampAnalyser::initializePluginPaths();
    ret->addAnalyser(new AnalyserBeats(pConfig));
    ret->addAnalyser(new AnalyserKey(pConfig));

    ret->start(QThread::LowPriority);
    return ret;
}
