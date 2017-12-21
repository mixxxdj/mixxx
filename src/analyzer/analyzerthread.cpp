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
constexpr int kProgressUpdateInhibitMillis = 100;

// ProgressUpdate states
constexpr int kProgressUpdateEmpty   = 0;
constexpr int kProgressUpdateWriting = 1;
constexpr int kProgressUpdateReady   = 2;
constexpr int kProgressUpdateReading = 3;

std::atomic<int> s_instanceCounter(0);

} // anonymous namespace

AnalyzerThread::ProgressUpdate::ProgressUpdate()
    : m_state(kProgressUpdateEmpty) {
}

bool AnalyzerThread::ProgressUpdate::tryWrite(
        TracksWithProgress* previousTracksWithProgress,
        TrackPointer currentTrack,
        int currentTrackProgress) {
    DEBUG_ASSERT(previousTracksWithProgress);
    bool progressChanged = false;
    int stateEmpty = kProgressUpdateEmpty;
    bool wasEmpty = m_state.compare_exchange_strong(
            stateEmpty,
            kProgressUpdateWriting);
    int stateReady = kProgressUpdateReady;
    if (wasEmpty || m_state.compare_exchange_strong(
            stateReady,
            kProgressUpdateWriting)) {
        DEBUG_ASSERT(m_state.load() == kProgressUpdateWriting);
        //kLogger.trace() << "Writing progress info";
        // Keep all track references alive until the main thread releases
        // them by moving them into the progress info exchange object!
        if (!previousTracksWithProgress->empty()) {
            if (m_tracksWithProgress.empty()) {
                m_tracksWithProgress.swap(*previousTracksWithProgress);
            } else {
                for (const auto trackProgress: *previousTracksWithProgress) {
                    m_tracksWithProgress[trackProgress.first] = trackProgress.second;
                }
            }
            previousTracksWithProgress->clear();
            // Simply assume that something must have changed.
            // It is just too tedious to check for individual
            // changes.
            progressChanged = true;
        }
        if (currentTrack) {
            DEBUG_ASSERT(!m_currentTrack ||
                    (m_tracksWithProgress.find(m_currentTrack) != m_tracksWithProgress.end()));
            const auto i = m_tracksWithProgress.find(currentTrack);
            if (i == m_tracksWithProgress.end()) {
                m_tracksWithProgress[currentTrack] = currentTrackProgress;
                progressChanged = true;
            } else if (i->second != currentTrackProgress) {
                i->second = currentTrackProgress;
                progressChanged = true;
            }
            m_currentTrack = currentTrack;
        } else {
            if (m_currentTrack) {
                m_currentTrack.reset();
                progressChanged = true;
            }
        }
        if (wasEmpty && !progressChanged) {
            // Still empty, nothing to do
            m_state.store(kProgressUpdateEmpty);
        } else {
            // Allow the main thread to consume progress info updates
            m_state.store(kProgressUpdateReady);
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

void AnalyzerThread::ProgressUpdate::reset() {
    m_tracksWithProgress.clear();
    m_currentTrack.reset();
}

AnalyzerThread::ProgressUpdate::ReadScope::ReadScope(ProgressUpdate* progressUpdate)
    : m_progressUpdate(nullptr) {
    DEBUG_ASSERT(progressUpdate);
    // Defer updates from the analysis thread while the
    // current progress info is consumed.
    int stateReady = kProgressUpdateReady;
    if (progressUpdate->m_state.compare_exchange_strong(
            stateReady,
            kProgressUpdateReading)) {
        m_progressUpdate = progressUpdate;
    }
}

AnalyzerThread::ProgressUpdate::ReadScope::~ReadScope() {
    if (m_progressUpdate) {
        DEBUG_ASSERT(m_progressUpdate->m_state.load() == kProgressUpdateReading);
        // Releasing all track reference here in the main thread might
        // trigger save actions. This is necessary to avoid that the
        // last reference is dropped within the analysis thread!
        m_progressUpdate->reset();
        // Finally allow the analysis thread to write progress info
        // updates again
        m_progressUpdate->m_state.store(kProgressUpdateEmpty);
    }
}

AnalyzerThread::AnalyzerThread(
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        UserSettingsPointer pConfig,
        AnalyzerMode mode)
        : m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_pConfig(std::move(pConfig)),
          m_mode(mode),
          m_stop(false),
          m_sampleBuffer(kAnalysisSamplesPerBlock) {
}

AnalyzerThread::~AnalyzerThread() {
    stop();
    // Wait until thread has actually stopped before proceeding
    wait();
}

void AnalyzerThread::run() {
    const int instanceId = s_instanceCounter.fetch_add(1) + 1;
    QThread::currentThread()->setObjectName(QString("AnalyzerThread %1").arg(instanceId));

    kLogger.debug() << "Running";

    exec();

    kLogger.debug() << "Exiting";
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

    mixxx::AudioSource::OpenParams openParams;
    openParams.setChannelCount(kAnalysisChannels);

    while (!m_stop) {
        emitProgressUpdate();
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
            emitProgressUpdate(kAnalysisProgressNone);
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
                emitProgressUpdate(kAnalysisProgressFinalizing);
                // This takes around 3 sec on a Atom Netbook
                for (auto const& pAnalyzer: m_analyzers) {
                    pAnalyzer->finalize(m_currentTrack);
                }
                finishCurrentTrack(kAnalysisProgressDone);
            } else {
                for (auto const& pAnalyzer: m_analyzers) {
                    pAnalyzer->cleanup(m_currentTrack);
                }
                finishCurrentTrack(kAnalysisProgressNone);
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
    DEBUG_ASSERT(m_stop);

    emitProgressUpdate();
    emit(idle());

    m_analyzers.clear();
}

bool AnalyzerThread::wake(const TrackPointer& nextTrack) {
    QMutexLocker locked(&m_nextTrackMutex);
    if (!m_nextTrack) {
        m_nextTrack = nextTrack;
    }
    if (!nextTrack || (m_nextTrack == nextTrack)) {
        m_nextTrackWaitCond.wakeOne();
        return true;
    } else {
        return false;
    }
}

void AnalyzerThread::stop() {
    QMutexLocker locked(&m_nextTrackMutex);
    m_stop.store(true);
    m_nextTrackWaitCond.wakeOne();
}

void AnalyzerThread::waitForCurrentTrack() {
    DEBUG_ASSERT(!m_currentTrack);

    QMutexLocker locked(&m_nextTrackMutex);
    while (!(m_currentTrack = std::move(m_nextTrack))) {
        if (m_stop.load()) {
            return;
        }
        kLogger.debug() << "Suspending";
        m_nextTrackWaitCond.wait(&m_nextTrackMutex);
        kLogger.debug() << "Resuming";
    }
    DEBUG_ASSERT(m_currentTrack);
    DEBUG_ASSERT(!m_nextTrack);
}

AnalyzerThread::AnalysisResult AnalyzerThread::analyzeCurrentTrack(
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
        ScopedTimer timer("AnalyzerThread::analyzeCurrentTrack() chunk");

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

        if ((m_progressUpdate.currentTrackProgress() != progressPromille) ||
                !m_previousTracksWithProgress.empty()) {
            if (progressUpdateInhibitTimer.elapsed() > kProgressUpdateInhibitMillis) {
                emitProgressUpdate(progressPromille);
                progressUpdateInhibitTimer.start();
            }
        }

        if (m_stop) {
            result = AnalysisResult::Cancelled;
        }

        // Ignore blocks in which we decided to bail for stats purposes.
        if ((result != AnalysisResult::Pending) || (result != AnalysisResult::Complete)) {
            timer.cancel();
        }
    }

    return result;
}

// This is called from the AnalyzerThread thread
void AnalyzerThread::emitProgressUpdate(int currentTrackProgress) {
    if (m_stop) {
        return;
    }
    if (m_progressUpdate.tryWrite(
            &m_previousTracksWithProgress,
            m_currentTrack,
            currentTrackProgress)) {
        emit(progressUpdate());
    }
}

void AnalyzerThread::finishCurrentTrack(int currentTrackProgress) {
    emitProgressUpdate(currentTrackProgress);
    // Reset the current thread AFTER emitting the final progress update
    m_currentTrack.reset();
    emit(idle());
}
