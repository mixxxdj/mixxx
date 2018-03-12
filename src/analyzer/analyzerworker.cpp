#include "analyzer/analyzerworker.h"

#include <typeinfo>

#include <QtDebug>
#include <QThread>

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
#include "util/event.h"
#include "util/timer.h"
#include "util/trace.h"
#include "util/logger.h"

// Measured in 0.1%,
// 0 for no progress during finalize
// 1 to display the text "finalizing"
// 100 for 10% step after finalize
// NOTE: If this is changed, change woverview.cpp slotAnalyzerProgress().
#define FINALIZE_PROMILLE 1.0

namespace {
    
mixxx::Logger kLogger("AnalyzerWorker");
// Analysis is done in blocks.
// We need to use a smaller block size, because on Linux the AnalyzerWorker
// can starve the CPU of its resources, resulting in xruns. A block size
// of 4096 frames per block seems to do fine.
const mixxx::AudioSignal::ChannelCount kAnalysisChannels(mixxx::kEngineChannelCount);
const SINT kAnalysisFramesPerBlock = 4096;
const SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;

inline
AnalyzerWaveform::Mode getAnalyzerWorkerMode(
        const UserSettingsPointer& pConfig) {
    if (pConfig->getValue<bool>(ConfigKey("[Library]", "EnableWaveformGenerationWithAnalysis"), true)) {
        return AnalyzerWaveform::Mode::Default;
    } else {
        return AnalyzerWaveform::Mode::WithoutWaveform;
    }
}

} // anonymous namespace



// --- CONSTRUCTOR ---
AnalyzerWorker::AnalyzerWorker(UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool, int workerIdx, bool priorized) :
    m_pConfig(pConfig),
    m_pAnalyzers(),
    m_priorizedJob(priorized),
    m_workerIdx(workerIdx),
    m_sampleBuffer(kAnalysisSamplesPerBlock),
    m_exit(false),
    m_pauseRequested(false),
    m_qm(),
    m_qwait(),
    m_pDbConnectionPool(std::move(pDbConnectionPool)) {

    m_pAnalysisDao = std::make_unique<AnalysisDao>(m_pConfig);
    createAnalyzers();

}

// --- DESTRUCTOR ---
AnalyzerWorker::~AnalyzerWorker() {
    kLogger.debug() << "Ending AnalyzerWorker";
    // free resources
    m_progressInfo.sema.release();
}

void AnalyzerWorker::nextTrack(TrackPointer newTrack) {
    QMutexLocker locker(&m_qm);
    m_currentTrack = newTrack;
    m_qwait.wakeAll();
}
void AnalyzerWorker::pause() {
    m_pauseRequested = true;
}
void AnalyzerWorker::resume() {
    QMutexLocker locker(&m_qm);
    m_qwait.wakeAll();
}

void AnalyzerWorker::endProcess() {
    QMutexLocker locker(&m_qm);
    m_exit = true;
    m_qwait.wakeAll();
}

// This is called from the AnalyzerWorker thread
bool AnalyzerWorker::doAnalysis(TrackPointer pTrack, mixxx::AudioSourcePointer pAudioSource) {

    QTime progressUpdateInhibitTimer;
    progressUpdateInhibitTimer.start(); // Inhibit Updates for 60 milliseconds

    mixxx::AudioSourceStereoProxy audioSourceProxy(
            pAudioSource,
            kAnalysisFramesPerBlock);
    DEBUG_ASSERT(audioSourceProxy.channelCount() == kAnalysisChannels);

    mixxx::IndexRange remainingFrames = pAudioSource->frameIndexRange();
    bool dieflag = false;
    bool cancelled = false;

    kLogger.debug() << "Analyzing" << pTrack->getTitle() << pTrack->getLocation();
    while (!dieflag && !remainingFrames.empty()) {
        ScopedTimer t("AnalyzerWorker::doAnalysis block");

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
            for (auto const& pAnalyzer: m_pAnalyzers) {
                pAnalyzer->process(
                        readableSampleFrames.readableData(),
                        readableSampleFrames.readableLength());
            }
        } else {
            // Partial analysis block of audio samples has been read.
            // This should only happen at the end of an audio stream,
            // otherwise a decoding error must have occurred.
            if (!remainingFrames.empty()) {
                // EOF not reached -> Maybe a corrupt file?
                kLogger.warning()
                        << "Aborting analysis after failed to read sample data from "
                        << pTrack->getLocation()
                        << ": expected frames =" << inputFrameIndexRange
                        << ", actual frames =" << readableSampleFrames.frameIndexRange();
                dieflag = true; // abort
                cancelled = false; // completed, no retry
            }
        }

        // emit progress updates
        // During the doAnalysis function it goes only to 100% - FINALIZE_PERCENT
        // because the finalize functions will take also some time
        //fp div here prevents insane signed overflow
        const double frameProgress =
                double(pAudioSource->frameLength() - remainingFrames.length()) /
                double(pAudioSource->frameLength());
        int progressPromille = frameProgress * (1000.0 - FINALIZE_PROMILLE);

        if (m_progressInfo.track_progress != progressPromille &&
                progressUpdateInhibitTimer.elapsed() > 60) {
            // Inhibit Updates for 60 milliseconds
            emitUpdateProgress(progressPromille);
            progressUpdateInhibitTimer.start();
        }

        // When a priority analysis comes in, we pause this working thread until one prioritized
        // worker finishes. Once it finishes, this worker will get resumed.
        if (m_pauseRequested.fetchAndStoreAcquire(false)) {
            QMutexLocker locker(&m_qm);
            emit(paused(this));
            m_qwait.wait(&m_qm);
        }

        if (m_exit) {
            dieflag = true;
            cancelled = true;
        }

        // Ignore blocks in which we decided to bail for stats purposes.
        if (dieflag || cancelled) {
            t.cancel();
        }
    }

    return !cancelled; //don't return !dieflag or we might reanalyze over and over
}

//Called automatically by the owning thread to start the process (Configured to do so by AnalyzerManager)
void AnalyzerWorker::slotProcess() {
    QThread::currentThread()->setObjectName(QString("AnalyzerWorker %1").arg(m_workerIdx));


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

    m_progressInfo.sema.release();

    while (!m_exit) {
        //We emit waitingForNextTrack to inform that we're done and we need a new track.
        {
            QMutexLocker locker(&m_qm);
            emit(waitingForNextTrack(this));
            m_qwait.wait(&m_qm);
        }
        // We recheck m_exit, since it's also the way that the manager indicates that there are no
        // more tracks to process.
        if (m_exit) {
            break;
        }
        Event::start(QString("AnalyzerWorker %1 process").arg(m_workerIdx));
        Trace trace("AnalyzerWorker analyzing track");

        // Get the audio
        mixxx::AudioSource::OpenParams openParams;
        openParams.setChannelCount(kAnalysisChannels);
        auto pAudioSource = SoundSourceProxy(m_currentTrack).openAudioSource(openParams);
        if (!pAudioSource) {
            kLogger.warning() 
                    << "Failed to open file for analyzing: " 
                    << m_currentTrack->getLocation()
                    << " " << *pAudioSource;
            //TODO: maybe emit error("Failed to bblablalba");
            continue;
        }

        bool processTrack = false;
        for (auto const& pAnalyzer: m_pAnalyzers) {
            // Make sure not to short-circuit initialize(...)
            if (pAnalyzer->initialize(m_currentTrack, 
                    pAudioSource->sampleRate(),
                    pAudioSource->frameLength() * kAnalysisChannels)) {
                processTrack = true;
            }
        }

        if (processTrack) {
            emitUpdateProgress(0);
            bool completed = doAnalysis(m_currentTrack, pAudioSource);
            // We can end doAnalysis because of two causes: The analysis has finished
            // or the analysis has been interrupted.
            if (!completed) {
                // This track was cancelled
                for (auto const& pAnalyzer: m_pAnalyzers) {
                    pAnalyzer->cleanup(m_currentTrack);
                }
                emitUpdateProgress(0);
            } else {
                // 100% - FINALIZE_PERCENT finished
                emitUpdateProgress(1000 - FINALIZE_PROMILLE);
                for (auto const& pAnalyzer: m_pAnalyzers) {
                    pAnalyzer->finalize(m_currentTrack);
                }
                emitUpdateProgress(1000); // 100%
            }
        } else {
            emitUpdateProgress(1000); // 100%
            kLogger.debug() << "Skipping track analysis because no analyzer initialized.";
        }
        Event::end(QString("AnalyzerWorker %1 process").arg(m_workerIdx));
    }
    
    if (m_pAnalysisDao) {
        // Invalidate reference to the thread-local database connection
        // that will be closed soon. Not necessary, just in case ;)
        m_pAnalysisDao->initialize(QSqlDatabase());
    }
    
    emit(workerFinished(this));
    emit(finished());
}

// This is called from the AnalyzerWorker thread
void AnalyzerWorker::emitUpdateProgress(int progress) {
    if (!m_exit) {
        // TODO: Since we are using a timer of 60 milliseconds in doAnalysis, we probably don't
        // need the semaphore. On the other hand, the semaphore would be useful if it was the UI
        // the one that requested us about updates. Then, the semaphore would prevent updates
        // until the UI has rendered the last update. As it is, it only prevents sending another
        // update if the analyzermanager slot hasn't read the update. (not the UI slot).
        // ---------
        // First tryAcquire will have always success because sema is initialized with on
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
        m_progressInfo.current_track = m_currentTrack;
        m_progressInfo.track_progress = progress;
        emit(updateProgress(m_workerIdx, &m_progressInfo));
    }
}

void AnalyzerWorker::createAnalyzers() {
    AnalyzerWaveform::Mode mode;
    if (m_priorizedJob) {
        mode = AnalyzerWaveform::Mode::Default;
    } else {
        mode = getAnalyzerWorkerMode(m_pConfig);
    }

    m_pAnalyzers.push_back(std::make_unique<AnalyzerWaveform>(m_pAnalysisDao.get(), mode));
    m_pAnalyzers.push_back(std::make_unique<AnalyzerGain>(m_pConfig));
    m_pAnalyzers.push_back(std::make_unique<AnalyzerEbur128>(m_pConfig));
#ifdef __VAMP__
    m_pAnalyzers.push_back(std::make_unique<AnalyzerBeats>(m_pConfig, !m_priorizedJob));
    m_pAnalyzers.push_back(std::make_unique<AnalyzerKey>(m_pConfig));
#endif
}
