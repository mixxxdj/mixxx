#include "analyzer/analyzerqueue.h"

#ifdef __VAMP__
#include "analyzer/analyzerbeats.h"
#include "analyzer/analyzerkey.h"
#endif
#include "analyzer/analyzergain.h"
#include "analyzer/analyzerebur128.h"
#include "analyzer/analyzerwaveform.h"
#include "library/dao/analysisdao.h"
#include "engine/engine.h"
#include "sources/soundsourceproxy.h"
#include "sources/audiosourcestereoproxy.h"
#include "util/compatibility.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"
#include "util/timer.h"
#include "util/trace.h"
#include "util/logger.h"


namespace {

mixxx::Logger kLogger("AnalyzerQueue");

// Analysis is done in blocks.
// We need to use a smaller block size, because on Linux the AnalyzerQueue
// can starve the CPU of its resources, resulting in xruns. A block size
// of 4096 frames per block seems to do fine.
constexpr mixxx::AudioSignal::ChannelCount kAnalysisChannels = mixxx::kEngineChannelCount;
constexpr SINT kAnalysisFramesPerBlock = 4096;
const SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;

// Maximum frequency of progress updates
constexpr int kProgressUpdateInhibitMillis = 100;

constexpr int kProgressStateEmpty   = 0;
constexpr int kProgressStateWriting = 1;
constexpr int kProgressStateReady   = 2;
constexpr int kProgressStateReading = 3;

QAtomicInt s_instanceCounter(0);

} // anonymous namespace

AnalyzerQueue::ThreadProgress::ThreadProgress()
    : m_state(kProgressStateEmpty),
      m_currentTrackProgress(kAnalysisProgressNone),
      m_queueSize(0) {
}

bool AnalyzerQueue::ThreadProgress::tryWrite(
        TracksWithProgress* previousTracksWithProgress,
        TrackPointer currentTrack,
        int currentTrackProgress,
        int queueSize) {
    DEBUG_ASSERT(previousTracksWithProgress);
    bool wasEmpty = m_state.testAndSetAcquire(
            kProgressStateEmpty, kProgressStateWriting);
    bool progressChanged = false;
    if (wasEmpty || m_state.testAndSetAcquire(
            kProgressStateReady, kProgressStateWriting)) {
        DEBUG_ASSERT(m_state == kProgressStateWriting);
        //kLogger.trace() << "Writing progress info";
        // Keep all track references alive until the main thread releases
        // them by moving them into the progress info exchange object!
        if (!previousTracksWithProgress->empty()) {
            if (m_tracksWithProgress.empty()) {
                previousTracksWithProgress->swap(m_tracksWithProgress);
            } else {
                for (const auto trackProgress: *previousTracksWithProgress) {
                    m_tracksWithProgress[trackProgress.first] = trackProgress.second;
                }
                previousTracksWithProgress->insert(
                        m_tracksWithProgress.begin(),
                        m_tracksWithProgress.end());
            }
            previousTracksWithProgress->clear();
            // Simply assume that something must have changed.
            // It is just too complicated to check for individual
            // changes.
            progressChanged = true;
        }
        if (currentTrack) {
            const auto i = m_tracksWithProgress.find(currentTrack);
            if (i == m_tracksWithProgress.end()) {
                m_tracksWithProgress[currentTrack] = currentTrackProgress;
                progressChanged = true;
            } else if (i->second != currentTrackProgress) {
                i->second = currentTrackProgress;
                progressChanged = true;
            }
            m_currentTrackProgress = currentTrackProgress;
        }
        if (m_queueSize != queueSize) {
            m_queueSize = queueSize;
            progressChanged = true;
        }
        if (wasEmpty && !progressChanged) {
            // Still empty, nothing to do
            m_state = kProgressStateEmpty;
        } else {
            // Allow the main thread to consume progress info updates
            m_state = kProgressStateReady;
        }
    } else {
        // Ensure that track references are not dropped within the
        // analysis thread by accumulating progress updates until
        // the receiver is ready to consume them!
        if (currentTrack) {
            (*previousTracksWithProgress)[currentTrack] = currentTrackProgress;
        }
    }
    return wasEmpty && progressChanged;
}

AnalyzerQueue::ThreadProgress::ReadScope::ReadScope(ThreadProgress* pThreadProgress)
    : m_pThreadProgress(nullptr) {
    DEBUG_ASSERT(pThreadProgress);
    // Defer updates from the analysis thread while the
    // current progress info is consumed.
    if (pThreadProgress->m_state.testAndSetAcquire(
            kProgressStateReady, kProgressStateReading)) {
        m_pThreadProgress = pThreadProgress;
    }
}

AnalyzerQueue::ThreadProgress::ReadScope::~ReadScope() {
    if (m_pThreadProgress) {
        DEBUG_ASSERT(m_pThreadProgress->m_state == kProgressStateReading);
        // Releasing all track reference here in the main thread might
        // trigger save actions. This is necessary to avoid that the
        // last reference is dropped within the analysis thread!
        m_pThreadProgress->m_tracksWithProgress.clear();
        // Finally allow the analysis thread to write progress info
        // updates again
        m_pThreadProgress->m_state = kProgressStateEmpty;
    }
}

AnalyzerQueue::AnalyzerQueue(
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        UserSettingsPointer pConfig,
        Mode mode)
        : m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_pConfig(std::move(pConfig)),
          m_mode(mode),
          m_exitThread(false),
          m_sampleBuffer(kAnalysisSamplesPerBlock) {

    connect(this, SIGNAL(threadProgress()),
            this, SLOT(slotThreadProgress()));

    start(QThread::LowPriority);
}

AnalyzerQueue::~AnalyzerQueue() {
    stopThread();
    // Wait until thread has actually stopped before proceeding
    wait();
}

// This is called from the AnalyzerQueue thread
// The returned track might be null if the analysis has been cancelled
void AnalyzerQueue::dequeueNextTrackBlocking() {
    QMutexLocker locked(&m_qm);
    DEBUG_ASSERT(!m_currentTrack);

    Event::end("AnalyzerQueue process");
    while (m_queuedTracks.isEmpty()) {
        DEBUG_ASSERT(m_pendingTracks.empty());
        kLogger.debug() << "Suspending thread";
        m_qwait.wait(&m_qm);
        kLogger.debug() << "Resuming thread";

        if (m_exitThread) {
            return;
        }
    }
    Event::start("AnalyzerQueue process");

    DEBUG_ASSERT(!m_queuedTracks.isEmpty());
    DEBUG_ASSERT(m_queuedTracks.size() == int(m_pendingTracks.size()));
    m_currentTrack = m_queuedTracks.dequeue();
    DEBUG_ASSERT(m_currentTrack);
}

void AnalyzerQueue::finishCurrentTrack(int currentTrackProgress) {
    bool queueEmpty = false;
    {
        QMutexLocker locked(&m_qm);
        if (m_currentTrack) {
            m_pendingTracks.erase(m_currentTrack);
        }
        DEBUG_ASSERT(m_queuedTracks.size() == int(m_pendingTracks.size()));
        if (m_queuedTracks.empty()) {
            queueEmpty = true;
        }
    }
    // Emit signals after unlocking to avoid deadlocks!
    emitThreadProgress(currentTrackProgress);
    // Reset the current thread AFTER emitting the final progress update
    m_currentTrack.reset();
    // Finally indicate if all scheduled tasks has been finished
    if (queueEmpty) {
        emit(threadIdle());
    }
}

// This is called from the AnalyzerQueue thread
AnalyzerQueue::AnalysisResult AnalyzerQueue::analyzeCurrentTrack(
        mixxx::AudioSourcePointer pAudioSource) {

    QTime progressUpdateInhibitTimer;
    progressUpdateInhibitTimer.start();

    mixxx::AudioSourceStereoProxy audioSourceProxy(
            pAudioSource,
            kAnalysisFramesPerBlock);
    DEBUG_ASSERT(audioSourceProxy.channelCount() == kAnalysisChannels);

    mixxx::IndexRange remainingFrames = pAudioSource->frameIndexRange();
    auto result = remainingFrames.empty() ? AnalysisResult::Complete : AnalysisResult::Pending;
    while (result == AnalysisResult::Pending) {
        ScopedTimer t("AnalyzerQueue::doAnalysis block");

        const auto inputFrameIndexRange =
                remainingFrames.splitAndShrinkFront(
                        math_min(kAnalysisFramesPerBlock, remainingFrames.length()));
        DEBUG_ASSERT(!inputFrameIndexRange.empty());
        const auto readableSampleFrames =
                audioSourceProxy.readSampleFrames(
                        mixxx::WritableSampleFrames(
                                inputFrameIndexRange,
                                mixxx::SampleBuffer::WritableSlice(m_sampleBuffer)));
        // To compare apples to apples, let's only look at blocks that are
        // the full block size.
        if (readableSampleFrames.frameLength() == kAnalysisFramesPerBlock) {
            // Complete analysis block of audio samples has been read.
            for (auto const& pAnalyzer: m_analyzers) {
                pAnalyzer->process(
                        readableSampleFrames.readableData(),
                        readableSampleFrames.readableLength());
            }
            if (remainingFrames.empty()) {
                result = AnalysisResult::Complete;
            }
        } else {
            // Partial analysis block of audio samples has been read.
            // This should only happen at the end of an audio stream,
            // otherwise a decoding error must have occurred.
            if (remainingFrames.empty()) {
                result = AnalysisResult::Complete;
            } else {
                // EOF not reached -> Maybe a corrupt file?
                kLogger.warning()
                        << "Aborting analysis after failed to read sample data from"
                        << m_currentTrack->getLocation()
                        << ": expected frames =" << inputFrameIndexRange
                        << ", actual frames =" << readableSampleFrames.frameIndexRange();
                result = AnalysisResult::Partial;
            }
        }

        // emit progress updates
        // During the doAnalysis function it goes only to kAnalysisProgressFinalizing
        // because the finalize functions will take also some time
        // fp div here prevents insane signed overflow
        const double frameProgress =
                double(pAudioSource->frameLength() - remainingFrames.length()) /
                double(pAudioSource->frameLength());
        int progressPromille = frameProgress * kAnalysisProgressFinalizing;

        if ((m_threadProgress.m_currentTrackProgress != progressPromille) ||
                !m_previousTracksWithProgress.empty()) {
            if (progressUpdateInhibitTimer.elapsed() > kProgressUpdateInhibitMillis) {
                emitThreadProgress(progressPromille);
                progressUpdateInhibitTimer.start();
            }
        }

        if (m_exitThread) {
            result = AnalysisResult::Cancelled;
        }

        // Ignore blocks in which we decided to bail for stats purposes.
        if ((result != AnalysisResult::Pending) || (result != AnalysisResult::Complete)) {
            t.cancel();
        }
    }

    return result;
}

int AnalyzerQueue::resumeThread() {
    QMutexLocker locked(&m_qm);
    m_qwait.wakeAll();
    return m_queuedTracks.size();
}

void AnalyzerQueue::stopThread() {
    m_exitThread = true;
    resumeThread();
}

void AnalyzerQueue::run() {
    const int instanceId = s_instanceCounter.fetchAndAddAcquire(1) + 1;
    QThread::currentThread()->setObjectName(QString("AnalyzerQueue %1").arg(instanceId));

    kLogger.debug() << "Entering thread";

    execThread();

    kLogger.debug() << "Exiting thread";
}

void AnalyzerQueue::execThread() {
    // The thread-local database connection for waveform analysis
    // must not be closed before returning from this function.
    // Therefore the DbConnectionPooler is defined here independent
    // of whether a database connection will be opened or not.
    mixxx::DbConnectionPooler dbConnectionPooler;

    std::unique_ptr<AnalysisDao> pAnalysisDao;
    if (m_mode != Mode::WithoutWaveform) {
        pAnalysisDao = std::make_unique<AnalysisDao>(m_pConfig);
        m_analyzers.push_back(std::make_unique<AnalyzerWaveform>(pAnalysisDao.get()));
    }
    m_analyzers.push_back(std::make_unique<AnalyzerGain>(m_pConfig));
    m_analyzers.push_back(std::make_unique<AnalyzerEbur128>(m_pConfig));
#ifdef __VAMP__
    m_analyzers.push_back(std::make_unique<AnalyzerBeats>(m_pConfig));
    m_analyzers.push_back(std::make_unique<AnalyzerKey>(m_pConfig));
#endif
    // If there are no analyzers, don't waste time running.
    if (m_analyzers.empty()) {
        kLogger.warning() << "No analyzers activated";
        return;
    } else {
        kLogger.info() << "Activated" << m_analyzers.size() << "analyzers";
    }

    // pAnalysisDao remains null if no analyzer needs database access.
    // Currently only waveform analyses makes use of it.
    if (pAnalysisDao) {
        dbConnectionPooler = mixxx::DbConnectionPooler(m_pDbConnectionPool); // move assignment
        if (!dbConnectionPooler.isPooling()) {
            kLogger.warning()
                    << "Failed to obtain database connection for analyzer queue thread";
            return;
        }
        // Obtain and use the newly created database connection within this thread
        QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pDbConnectionPool);
        DEBUG_ASSERT(dbConnection.isOpen());
        pAnalysisDao->initialize(dbConnection);
    }

    while (!m_exitThread) {
        DEBUG_ASSERT(!m_currentTrack);
        emitThreadProgress();
        dequeueNextTrackBlocking();
        if (!m_currentTrack) {
            break;
        }

        kLogger.debug() << "Analyzing" << m_currentTrack->getTitle() << m_currentTrack->getLocation();
        Trace trace("AnalyzerQueue analyzing track");

        // Get the audio
        mixxx::AudioSource::OpenParams openParams;
        openParams.setChannelCount(kAnalysisChannels);
        auto pAudioSource = SoundSourceProxy(m_currentTrack).openAudioSource(openParams);
        if (!pAudioSource) {
            kLogger.warning()
                    << "Failed to open file for analyzing:"
                    << m_currentTrack->getLocation();
            finishCurrentTrack();
            continue;
        }

        bool processTrack = false;
        for (auto const& pAnalyzer: m_analyzers) {
            // Make sure not to short-circuit initialize(...)
            if (pAnalyzer->initialize(
                    m_currentTrack,
                    pAudioSource->sampleRate(),
                    pAudioSource->frameLength() * kAnalysisChannels)) {
                processTrack = true;
            }
        }

        if (processTrack) {
            emitThreadProgress(kAnalysisProgressNone);
            const auto analysisResult = analyzeCurrentTrack(pAudioSource);
            DEBUG_ASSERT(analysisResult != AnalysisResult::Pending);
            if ((analysisResult == AnalysisResult::Complete) ||
                    (analysisResult == AnalysisResult::Partial)) {
                // The analysis has been finished, and is either complete without
                // any errors or partial if it has been aborted due to a corrupt
                // audio file. In both cases don't reanalyze tracks during this
                // session. A partial analysis would otherwise be repeated again
                // and again, because it is very unlikely that the error vanishes
                // suddenly.
                emitThreadProgress(kAnalysisProgressFinalizing);
                // This takes around 3 sec on a Atom Netbook
                for (auto const& pAnalyzer: m_analyzers) {
                    pAnalyzer->finalize(m_currentTrack);
                }
                finishCurrentTrack(kAnalysisProgressDone);
            } else {
                for (auto const& pAnalyzer: m_analyzers) {
                    pAnalyzer->cleanup(m_currentTrack);
                }
                TrackPointer deferredTrack = m_currentTrack;
                finishCurrentTrack(kAnalysisProgressNone);
                // Re-enqueue AFTER finishing the current track
                enqueueTrack(std::move(deferredTrack));
            }
        } else {
            kLogger.debug() << "Skipping track analysis because no analyzer initialized.";
            finishCurrentTrack(kAnalysisProgressDone);
        }
        // All references to the track object within the analysis thread
        // should have been released to avoid exporting metadata or updating
        // the database within the low-prio analysis thread!
        DEBUG_ASSERT(!m_currentTrack);
    }
    DEBUG_ASSERT(m_exitThread);

    {
        QMutexLocker locked(&m_qm);
        DEBUG_ASSERT(!m_currentTrack);
        m_pendingTracks.clear();
        m_queuedTracks.clear();
    }

    emitThreadProgress();
    emit(threadIdle());

    m_analyzers.clear();
}

// This is called from the AnalyzerQueue thread
void AnalyzerQueue::emitThreadProgress(int currentTrackProgress) {
    if (!m_exitThread) {
        int queueSize;
        {
            QMutexLocker locked(&m_qm);
            queueSize = m_queuedTracks.size();
            if (m_currentTrack) {
                ++queueSize;
            }
        }
        if (m_threadProgress.tryWrite(
                &m_previousTracksWithProgress,
                m_currentTrack,
                currentTrackProgress,
                queueSize)) {
            emit(threadProgress());
        }
    }
}

void AnalyzerQueue::slotThreadProgress() {
    const auto readScope = m_threadProgress.read();
    if (readScope) {
        //kLogger.trace() << "Reading progress info";
        // TODO(XXX): Bulk progress updates of many track objects
        // that have been skipped during analysis can still limit
        // the responsiveness of the UI.
        bool oneOrMoreTracksFinished = false;
        for (const auto trackWithProgress: readScope.tracksWithProgress()) {
            if (trackWithProgress.second != kAnalysisProgressUnknown) {
                trackWithProgress.first->setAnalysisProgress(trackWithProgress.second);
                if (trackWithProgress.second == kAnalysisProgressDone) {
                    oneOrMoreTracksFinished = true;
                }
            }
        }
        DEBUG_ASSERT(oneOrMoreTracksFinished ||
                (readScope.currentTrackProgress() != kAnalysisProgressDone));
        if (readScope.currentTrackProgress() != kAnalysisProgressUnknown) {
            int trackProgressPercent = readScope.currentTrackProgress() / 10;
            emit(trackProgress(trackProgressPercent));
        }
        if (oneOrMoreTracksFinished) {
            emit(trackFinished(readScope.queueSize()));
        }
    }
}

void AnalyzerQueue::slotAnalyseTrack(TrackPointer pTrack) {
    // This slot is called from the decks and and samplers when the track was loaded.
    if (enqueueTrack(pTrack)) {
        resumeThread();
    }
}

// This is called from the GUI and from the AnalyzerQueue thread
bool AnalyzerQueue::enqueueTrack(TrackPointer pTrack) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return false;
    }

    QMutexLocker locked(&m_qm);
    if (m_pendingTracks.insert(pTrack).second) {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Enqueuing track"
                    << pTrack->getLocation();
        }
        m_queuedTracks.enqueue(pTrack);
        // Don't wake up the paused thread now to avoid race conditions
        // if multiple threads are added in a row. The caller is
        // responsible to finish the enqueuing of tracks with resumeThread().
        return true;
    } else {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Analysis of track is already pending"
                    << pTrack->getLocation();
        }
        return false;
    }
}
