#include "analyzer/analyzerqueue.h"

#ifdef __VAMP__
#include "analyzer/analyzerbeats.h"
#include "analyzer/analyzerkey.h"
#endif
#include "analyzer/analyzergain.h"
#include "analyzer/analyzerebur128.h"
#include "analyzer/analyzerwaveform.h"
#include "library/dao/analysisdao.h"
#include "mixer/playerinfo.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"
#include "util/event.h"
#include "util/timer.h"
#include "util/trace.h"
#include "util/logger.h"

// Measured in 0.1%,
// 0 for no progress during finalize
// 1 to display the text "finalizing"
// 100 for 10% step after finalize
#define FINALIZE_PROMILLE 1

namespace {

mixxx::Logger kLogger("AnalyzerQueue");

// Analysis is done in blocks.
// We need to use a smaller block size, because on Linux the AnalyzerQueue
// can starve the CPU of its resources, resulting in xruns. A block size
// of 4096 frames per block seems to do fine.
const SINT kAnalysisChannels = mixxx::AudioSource::kChannelCountStereo;
const SINT kAnalysisFramesPerBlock = 4096;
const SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;

QAtomicInt s_instanceCounter(0);

} // anonymous namespace

AnalyzerQueue::AnalyzerQueue(
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        const UserSettingsPointer& pConfig,
        Mode mode)
        : m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_exit(false),
          m_aiCheckPriorities(false),
          m_sampleBuffer(kAnalysisSamplesPerBlock),
          m_queue_size(0) {

    if (mode != Mode::WithoutWaveform) {
        m_pAnalysisDao = std::make_unique<AnalysisDao>(pConfig);
        m_pAnalyzers.push_back(std::make_unique<AnalyzerWaveform>(m_pAnalysisDao.get()));
    }
    m_pAnalyzers.push_back(std::make_unique<AnalyzerGain>(pConfig));
    m_pAnalyzers.push_back(std::make_unique<AnalyzerEbur128>(pConfig));
#ifdef __VAMP__
    m_pAnalyzers.push_back(std::make_unique<AnalyzerBeats>(pConfig));
    m_pAnalyzers.push_back(std::make_unique<AnalyzerKey>(pConfig));
#endif

    connect(this, SIGNAL(updateProgress()),
            this, SLOT(slotUpdateProgress()));

    start(QThread::LowPriority);
}

AnalyzerQueue::~AnalyzerQueue() {
    stop();
    m_progressInfo.sema.release();
    wait(); //Wait until thread has actually stopped before proceeding.
}

// This is called from the AnalyzerQueue thread
bool AnalyzerQueue::isLoadedTrackWaiting(TrackPointer analysingTrack) {
    const PlayerInfo& info = PlayerInfo::instance();
    TrackPointer pTrack;
    bool trackWaiting = false;
    QList<TrackPointer> progress100List;
    QList<TrackPointer> progress0List;

    m_qm.lock();
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
        int progress = pTrack->getAnalyzerProgress();
        if (progress < 0) {
            // Load stored analysis
            bool processTrack = false;
            for (auto const& pAnalyzer: m_pAnalyzers) {
                if (!pAnalyzer->isDisabledOrLoadStoredSuccess(pTrack)) {
                    processTrack = true;
                }
            }
            if (!processTrack) {
                progress100List.append(pTrack);
                it.remove(); // since pTrack is a reference it is invalid now.
            } else {
                progress0List.append(pTrack);
            }
        } else if (progress == 1000) {
            it.remove();
        }
    }

    m_qm.unlock();

    // update progress after unlock to avoid a deadlock
    foreach (TrackPointer pTrack, progress100List) {
        emitUpdateProgress(pTrack, 1000);
    }
    foreach (TrackPointer pTrack, progress0List) {
        emitUpdateProgress(pTrack, 0);
    }

    if (info.isTrackLoaded(analysingTrack)) {
        return false;
    }
    return trackWaiting;
}

// This is called from the AnalyzerQueue thread
TrackPointer AnalyzerQueue::dequeueNextBlocking() {
    m_qm.lock();
    if (m_tioq.isEmpty()) {
        Event::end("AnalyzerQueue process");
        m_qwait.wait(&m_qm);
        Event::start("AnalyzerQueue process");

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
            kLogger.debug() << "Prioritizing" << pTrack->getTitle() << pTrack->getLocation();
            pLoadTrack = pTrack;
            it.remove();
            break;
        }
    }

    if (!pLoadTrack && !m_tioq.isEmpty()) {
        // no prioritized track found, use head track
        pLoadTrack = m_tioq.dequeue();
    }

    m_qm.unlock();

    if (pLoadTrack) {
        kLogger.debug() << "Analyzing" << pLoadTrack->getTitle() << pLoadTrack->getLocation();
    }
    // pTrack might be NULL, up to the caller to check.
    return pLoadTrack;
}

// This is called from the AnalyzerQueue thread
bool AnalyzerQueue::doAnalysis(TrackPointer tio, mixxx::AudioSourcePointer pAudioSource) {

    QTime progressUpdateInhibitTimer;
    progressUpdateInhibitTimer.start(); // Inhibit Updates for 60 milliseconds

    SINT frameIndex = pAudioSource->getMinFrameIndex();
    bool dieflag = false;
    bool cancelled = false;
    do {
        ScopedTimer t("AnalyzerQueue::doAnalysis block");

        DEBUG_ASSERT(frameIndex < pAudioSource->getMaxFrameIndex());
        const SINT framesRemaining =
                pAudioSource->getMaxFrameIndex() - frameIndex;
        const SINT framesToRead =
                math_min(kAnalysisFramesPerBlock, framesRemaining);
        DEBUG_ASSERT(0 < framesToRead);

        const SINT framesRead =
                pAudioSource->readSampleFramesStereo(
                        framesToRead,
                        &m_sampleBuffer);
        DEBUG_ASSERT(framesRead <= framesToRead);
        frameIndex += framesRead;
        DEBUG_ASSERT(pAudioSource->isValidFrameIndex(frameIndex));

        // To compare apples to apples, let's only look at blocks that are
        // the full block size.
        if (kAnalysisFramesPerBlock == framesRead) {
            // Complete analysis block of audio samples has been read.
            for (auto const& pAnalyzer: m_pAnalyzers) {
                pAnalyzer->process(m_sampleBuffer.data(), m_sampleBuffer.size());
            }
        } else {
            // Partial analysis block of audio samples has been read.
            // This should only happen at the end of an audio stream,
            // otherwise a decoding error must have occurred.
            if (frameIndex < pAudioSource->getMaxFrameIndex()) {
                // EOF not reached -> Maybe a corrupt file?
                kLogger.warning() << "Failed to read sample data from file:"
                        << tio->getLocation()
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
        // because the finalize functions will take also some time
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
        // analyzer queue is CPU intensive so we want to get out of the way of
        // the audio callback thread.
        //QThread::yieldCurrentThread();
        //QThread::usleep(10);

        // has something new entered the queue?
        if (m_aiCheckPriorities.fetchAndStoreAcquire(false)) {
            if (isLoadedTrackWaiting(tio)) {
                kLogger.debug() << "Interrupting analysis to give preference to a loaded track.";
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

void AnalyzerQueue::stop() {
    m_exit = true;
    m_qm.lock();
    m_qwait.wakeAll();
    m_qm.unlock();
}

void AnalyzerQueue::run() {
    // If there are no analyzers, don't waste time running.
    if (m_pAnalyzers.empty()) {
        return;
    }

    const int instanceId = s_instanceCounter.fetchAndAddAcquire(1) + 1;
    QThread::currentThread()->setObjectName(QString("AnalyzerQueue %1").arg(instanceId));

    kLogger.debug() << "Entering thread";

    execThread();

    kLogger.debug() << "Exiting thread";
}

void AnalyzerQueue::execThread() {
    // The thread-local database connection for waveform analysis must not
    // be closed before returning from this function. Therefore the
    // DbConnectionPooler is defined at this outer function scope,
    // independent of whether a database connection will be opened
    // or not.
    mixxx::DbConnectionPooler dbConnectionPooler;
    // m_pAnalysisDao remains null if no analyzer needs database access.
    // Currently only waveform analyses makes use of it.
    if (m_pAnalysisDao) {
        dbConnectionPooler = mixxx::DbConnectionPooler(m_pDbConnectionPool); // move assignment
        if (!dbConnectionPooler.isPooling()) {
            kLogger.warning()
                    << "Failed to obtain database connection for analyzer queue thread";
            return;
        }
        // Obtain and use the newly created database connection within this thread
        QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pDbConnectionPool);
        DEBUG_ASSERT(dbConnection.isOpen());
        m_pAnalysisDao->initialize(dbConnection);
    }

    m_progressInfo.current_track.reset();
    m_progressInfo.track_progress = 0;
    m_progressInfo.queue_size = 0;
    m_progressInfo.sema.release(); // Initialize with one

    while (!m_exit) {
        TrackPointer nextTrack = dequeueNextBlocking();

        // It's important to check for m_exit here in case we decided to exit
        // while blocking for a new track.
        if (m_exit) {
            break;
        }

        // If the track is NULL, try to get the next one.
        // Could happen if the track was queued but then deleted.
        // Or if dequeueNextBlocking is unblocked by exit == true
        if (!nextTrack) {
            emptyCheck();
            continue;
        }

        Trace trace("AnalyzerQueue analyzing track");

        // Get the audio
        SoundSourceProxy soundSourceProxy(nextTrack);
        mixxx::AudioSourceConfig audioSrcCfg;
        audioSrcCfg.setChannelCount(kAnalysisChannels);
        mixxx::AudioSourcePointer pAudioSource(soundSourceProxy.openAudioSource(audioSrcCfg));
        if (!pAudioSource) {
            kLogger.warning() << "Failed to open file for analyzing:" << nextTrack->getLocation();
            emptyCheck();
            continue;
        }

        bool processTrack = false;
        for (auto const& pAnalyzer: m_pAnalyzers) {
            // Make sure not to short-circuit initialize(...)
            if (pAnalyzer->initialize(nextTrack, pAudioSource->getSamplingRate(), pAudioSource->getFrameCount() * kAnalysisChannels)) {
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
                // This track was cancelled
                for (auto const& pAnalyzer: m_pAnalyzers) {
                    pAnalyzer->cleanup(nextTrack);
                }
                queueAnalyseTrack(nextTrack);
                emitUpdateProgress(nextTrack, 0);
            } else {
                // 100% - FINALIZE_PERCENT finished
                emitUpdateProgress(nextTrack, 1000 - FINALIZE_PROMILLE);
                // This takes around 3 sec on a Atom Netbook
                for (auto const& pAnalyzer: m_pAnalyzers) {
                    pAnalyzer->finalize(nextTrack);
                }
                emit(trackDone(nextTrack));
                emitUpdateProgress(nextTrack, 1000); // 100%
            }
        } else {
            emitUpdateProgress(nextTrack, 1000); // 100%
            kLogger.debug() << "Skipping track analysis because no analyzer initialized.";
        }
        emptyCheck();
    }

    if (m_pAnalysisDao) {
        // Invalidate reference to the thread-local database connection
        // that will be closed soon. Not necessary, just in case ;)
        m_pAnalysisDao->initialize(QSqlDatabase());
    }

    emit(queueEmpty()); // emit in case of exit;
}

void AnalyzerQueue::emptyCheck() {
    m_qm.lock();
    m_queue_size = m_tioq.size();
    m_qm.unlock();
    if (m_queue_size == 0) {
        emit(queueEmpty()); // emit asynchrony for no deadlock
    }
}

// This is called from the AnalyzerQueue thread
void AnalyzerQueue::emitUpdateProgress(TrackPointer track, int progress) {
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
        m_progressInfo.current_track = track;
        m_progressInfo.track_progress = progress;
        m_progressInfo.queue_size = m_queue_size;
        emit(updateProgress());
    }
}

//slot
void AnalyzerQueue::slotUpdateProgress() {
    if (m_progressInfo.current_track) {
        m_progressInfo.current_track->setAnalyzerProgress(m_progressInfo.track_progress);
        m_progressInfo.current_track.reset();
    }
    emit(trackProgress(m_progressInfo.track_progress / 10));
    if (m_progressInfo.track_progress == 1000) {
        emit(trackFinished(m_progressInfo.queue_size));
    }
    m_progressInfo.sema.release();
}

void AnalyzerQueue::slotAnalyseTrack(TrackPointer tio) {
    // This slot is called from the decks and and samplers when the track was loaded.
    queueAnalyseTrack(tio);
    m_aiCheckPriorities = true;
}

// This is called from the GUI and from the AnalyzerQueue thread
void AnalyzerQueue::queueAnalyseTrack(TrackPointer tio) {
    m_qm.lock();
    if (!m_tioq.contains(tio)) {
        m_tioq.enqueue(tio);
        m_qwait.wakeAll();
    }
    m_qm.unlock();
}
