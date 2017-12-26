#include "analyzer/analyzerthread.h"

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

#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"
#include "util/logger.h"
#include "util/timer.h"


namespace {

mixxx::Logger kLogger("AnalyzerThread");

// Analysis is done in blocks.
// We need to use a smaller block size, because on Linux the AnalyzerThread
// can starve the CPU of its resources, resulting in xruns. A block size
// of 4096 frames per block seems to do fine.
constexpr mixxx::AudioSignal::ChannelCount kAnalysisChannels = mixxx::kEngineChannelCount;
constexpr SINT kAnalysisFramesPerBlock = 4096;
const SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;

// Maximum frequency of progress updates
constexpr std::chrono::milliseconds kProgressInhibitDuration(100);

// Progress states
constexpr int kProgressStateEmpty   = 0;
constexpr int kProgressStateWriting = 1;
constexpr int kProgressStateReady   = 2;
constexpr int kProgressStateReading = 3;

std::atomic<int> s_instanceCounter(0);

} // anonymous namespace

AnalyzerThread::Progress::Progress()
    : m_state(kProgressStateEmpty) {
}

bool AnalyzerThread::Progress::tryWrite(
        TracksWithProgress* previousTracksWithProgress,
        TrackPointer currentTrack,
        int currentTrackProgress) {
    DEBUG_ASSERT(previousTracksWithProgress);
    int stateEmpty = kProgressStateEmpty;
    bool wasEmpty = m_state.compare_exchange_strong(
            stateEmpty,
            kProgressStateWriting);
    int stateReady = kProgressStateReady;
    if (wasEmpty || m_state.compare_exchange_strong(
            stateReady,
            kProgressStateWriting)) {
        DEBUG_ASSERT(m_state.load() == kProgressStateWriting);
        //kLogger.trace() << "Writing progress info";
        // Keep all track references alive until the main thread releases
        // them by moving them into the progress info exchange object!
        if (!previousTracksWithProgress->empty()) {
            if (m_tracksWithProgress.empty()) {
                m_tracksWithProgress.swap(*previousTracksWithProgress);
            } else {
                for (const auto trackProgressState: *previousTracksWithProgress) {
                    m_tracksWithProgress[trackProgressState.first] = trackProgressState.second;
                }
            }
            previousTracksWithProgress->clear();
        }
        bool currentTrackModified = m_currentTrack != currentTrack;
        if (currentTrack) {
            DEBUG_ASSERT(!m_currentTrack ||
                    (m_tracksWithProgress.find(m_currentTrack) != m_tracksWithProgress.end()));
            const auto i = m_tracksWithProgress.find(currentTrack);
            if (i == m_tracksWithProgress.end()) {
                m_tracksWithProgress[currentTrack] = currentTrackProgress;
            } else {
                i->second = currentTrackProgress;
            }
            m_currentTrack = currentTrack;
        } else {
            m_currentTrack.reset();
        }
        if (currentTrackModified || !m_tracksWithProgress.empty()) {
            // Allow the main thread to consume progress info updates
            m_state.store(kProgressStateReady);
            return true;
        } else {
            // Empty, nothing to read
            m_state.store(kProgressStateEmpty);
        }
    } else {
        // Ensure that track references are not dropped within the
        // analysis thread by accumulating progress updates until
        // the receiver is ready to consume them!
        if (currentTrack) {
            (*previousTracksWithProgress)[currentTrack] = currentTrackProgress;
        }
    }
    return false;
}

void AnalyzerThread::Progress::reset() {
    m_tracksWithProgress.clear();
    m_currentTrack.reset();
}

AnalyzerThread::Progress::ReadScope::ReadScope(Progress* progress)
    : m_progress(nullptr),
      m_empty(false) {
    DEBUG_ASSERT(progress);
    // Defer updates from the analysis thread while the
    // current progress info is consumed.
    int stateReady = kProgressStateReady;
    if (progress->m_state.compare_exchange_strong(
            stateReady,
            kProgressStateReading)) {
        m_progress = progress;
    } else {
        // It is safe to check for emptiness here, because only the
        // reader can set the state back to empty, i.e. we will not
        // get any false positives if the thread has already modified
        // m_state meanwhile.
        m_empty = progress->m_state.load() == kProgressStateEmpty;
    }
}

AnalyzerThread::Progress::ReadScope::~ReadScope() {
    if (m_progress) {
        DEBUG_ASSERT(m_progress->m_state.load() == kProgressStateReading);
        // Releasing all track reference here in the main thread might
        // trigger save actions. This is necessary to avoid that the
        // last reference is dropped within the analysis thread!
        m_progress->reset();
        // Finally allow the analysis thread to write progress info
        // updates again
        m_progress->m_state.store(kProgressStateEmpty);
    }
}

AnalyzerThread::AnalyzerThread(
        int id,
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        UserSettingsPointer pConfig,
        AnalyzerMode mode)
        : m_id(id),
          m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_pConfig(std::move(pConfig)),
          m_mode(mode),
          m_currentTrackProgress(kAnalyzerProgressUnknown),
          m_run(true),
          m_sampleBuffer(kAnalysisSamplesPerBlock),
          m_idling(false),
          // The first signal should always be emitted
          m_lastProgressEmittedAt(Clock::now() - kProgressInhibitDuration) {
}

AnalyzerThread::~AnalyzerThread() {
    stop();
    // Wait until thread has actually stopped before proceeding
    wait();
}

void AnalyzerThread::run() {
    if (!m_run.load()) {
        return;
    }

    const int instanceId = s_instanceCounter.fetch_add(1) + 1;
    QThread::currentThread()->setObjectName(QString("AnalyzerThread %1").arg(instanceId));

    kLogger.debug() << "Running" << m_id;

    exec();

    kLogger.debug() << "Exiting" << m_id;

    m_run.store(false);
    emit(exit(m_id));
}

void AnalyzerThread::exec() {
    // The thread-local database connection for waveform analysis
    // must not be closed before returning from this function.
    // Therefore the DbConnectionPooler is defined here independent
    // of whether a database connection will be opened or not.
    mixxx::DbConnectionPooler dbConnectionPooler;

    std::unique_ptr<AnalysisDao> pAnalysisDao;
    if (m_mode != AnalyzerMode::WithoutWaveform) {
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
    // Currently only the waveform analyzer makes use of it.
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

    mixxx::AudioSource::OpenParams openParams;
    openParams.setChannelCount(kAnalysisChannels);

    while (m_run.load()) {
        emitProgress();
        waitForCurrentTrack();
        if (!m_currentTrack) {
            break;
        }

        kLogger.debug() << "Analyzing" << m_currentTrack->getLocation();

        // Get the audio
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
            emitProgress(kAnalyzerProgressNone);
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
                emitProgress(kAnalyzerProgressFinalizing);
                // This takes around 3 sec on a Atom Netbook
                for (auto const& pAnalyzer: m_analyzers) {
                    pAnalyzer->finalize(m_currentTrack);
                }
                finishCurrentTrack(kAnalyzerProgressDone);
            } else {
                for (auto const& pAnalyzer: m_analyzers) {
                    pAnalyzer->cleanup(m_currentTrack);
                }
                finishCurrentTrack();
            }
        } else {
            kLogger.debug() << "Skipping track analysis because no analyzer initialized.";
            finishCurrentTrack(kAnalyzerProgressDone);
        }
        // All references to the track object within the analysis thread
        // should have been released to avoid exporting metadata or updating
        // the database within the low-prio analysis thread!
        DEBUG_ASSERT(!m_currentTrack);
    }
    DEBUG_ASSERT(!m_run.load());

    m_analyzers.clear();
}

bool AnalyzerThread::wake(const TrackPointer& nextTrack) {
    std::lock_guard<std::mutex> locked(m_nextTrackMutex);
    if (!m_nextTrack) {
        m_nextTrack = nextTrack;
    }
    if (!nextTrack || (m_nextTrack == nextTrack)) {
        m_nextTrackWaitCond.notify_one();
        return true;
    } else {
        return false;
    }
}

void AnalyzerThread::stop() {
    m_run.store(false);
    m_nextTrackWaitCond.notify_one();
}

void AnalyzerThread::waitForCurrentTrack() {
    DEBUG_ASSERT(!m_currentTrack);

    std::unique_lock<std::mutex> locked(m_nextTrackMutex);
    while (!(m_currentTrack = std::move(m_nextTrack))) {
        DEBUG_ASSERT(!m_currentTrack);
        DEBUG_ASSERT(!m_nextTrack);
        if (!m_run.load()) {
            return;
        }
        kLogger.debug() << "Suspending" << m_id;
        if (!m_idling) {
            emit(idle(m_id));
            m_idling = true;
        }
        m_nextTrackWaitCond.wait(locked);
        kLogger.debug() << "Resuming" << m_id;
    }
    DEBUG_ASSERT(m_currentTrack);
    DEBUG_ASSERT(!m_nextTrack);
    m_idling = false;
}

AnalyzerThread::AnalysisResult AnalyzerThread::analyzeCurrentTrack(
        mixxx::AudioSourcePointer pAudioSource) {

    mixxx::AudioSourceStereoProxy audioSourceProxy(
            pAudioSource,
            kAnalysisFramesPerBlock);
    DEBUG_ASSERT(audioSourceProxy.channelCount() == kAnalysisChannels);

    emitProgress(kAnalyzerProgressNone);

    mixxx::IndexRange remainingFrames = pAudioSource->frameIndexRange();
    auto result = remainingFrames.empty() ? AnalysisResult::Complete : AnalysisResult::Pending;
    while (result == AnalysisResult::Pending) {
        if (!m_run.load()) {
            return AnalysisResult::Cancelled;
        }

        // 1st step: Decode next chunk of audio data
        const auto inputFrameIndexRange =
                remainingFrames.splitAndShrinkFront(
                        math_min(kAnalysisFramesPerBlock, remainingFrames.length()));
        DEBUG_ASSERT(!inputFrameIndexRange.empty());
        const auto readableSampleFrames =
                audioSourceProxy.readSampleFrames(
                        mixxx::WritableSampleFrames(
                                inputFrameIndexRange,
                                mixxx::SampleBuffer::WritableSlice(m_sampleBuffer)));

        if (!m_run.load()) {
            return AnalysisResult::Cancelled;
        }

        // 2nd: step: Analyze chunk of decoded audio data
        if (readableSampleFrames.frameLength() == kAnalysisFramesPerBlock) {
            // Complete chunk of audio samples has been read for analysis
            for (auto const& pAnalyzer: m_analyzers) {
                pAnalyzer->process(
                        readableSampleFrames.readableData(),
                        readableSampleFrames.readableLength());
            }
            if (remainingFrames.empty()) {
                result = AnalysisResult::Complete;
            }
        } else {
            // Partial chunk of audio samples has been read.
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

        if (!m_run.load()) {
            return AnalysisResult::Cancelled;
        }

        // 3rd step: Update progress
        const double frameProgress =
                double(pAudioSource->frameLength() - remainingFrames.length()) /
                double(pAudioSource->frameLength());
        const int trackProgress = frameProgress * kAnalyzerProgressFinalizing;
        emitProgress(trackProgress);
    }

    return result;
}

// This is called from the AnalyzerThread thread
void AnalyzerThread::emitProgress(int currentTrackProgress) {
    if (m_progress.tryWrite(
            &m_previousTracksWithProgress,
            m_currentTrack,
            currentTrackProgress)) {
        const auto now = Clock::now();
        if (now < (m_lastProgressEmittedAt + kProgressInhibitDuration)) {
            // Don't emit progress update signal
            return;
        }
        m_lastProgressEmittedAt = now;
        emit(progress(m_id));
    }
}

void AnalyzerThread::finishCurrentTrack(int currentTrackProgress) {
    emitProgress(currentTrackProgress);
    // Reset the current thread AFTER emitting the final progress update
    m_currentTrack.reset();
}
