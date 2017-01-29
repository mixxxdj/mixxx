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

#include "sources/soundsourceproxy.h"
#include "util/compatibility.h"
#include "util/event.h"
#include "util/timer.h"
#include "util/trace.h"

namespace {
    // Analysis is done in blocks.
    // We need to use a smaller block size, because on Linux the AnalyzerWorker
    // can starve the CPU of its resources, resulting in xruns. A block size
    // of 4096 frames per block seems to do fine.
    const SINT kAnalysisChannels = mixxx::AudioSource::kChannelCountStereo;
    const SINT kAnalysisFramesPerBlock = 4096;
    const SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;
} // anonymous namespace

// Measured in 0.1%,
// 0 for no progress during finalize
// 1 to display the text "finalizing"
// 100 for 10% step after finalize
// NOTE: If this is changed, change woverview.cpp slotAnalyzerProgress().
#define FINALIZE_PROMILLE 1.0

// --- CONSTRUCTOR ---
AnalyzerWorker::AnalyzerWorker(UserSettingsPointer pConfig, int workerIdx, bool priorized) :
    m_pConfig(pConfig),
    m_analyzelist(),
    m_priorizedJob(priorized),
    m_workerIdx(workerIdx),
    m_sampleBuffer(kAnalysisSamplesPerBlock),
    m_exit(false),
    m_pauseRequested(false),
    m_qm(),
    m_qwait() {
}

// --- DESTRUCTOR ---
AnalyzerWorker::~AnalyzerWorker() {
    qDebug() << "Ending AnalyzerWorker";
    // free resources
    m_progressInfo.sema.release();
 
    QListIterator<Analyzer*> it(m_analyzelist);
    while (it.hasNext()) {
        Analyzer* an = it.next();
        delete an;
    }
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
bool AnalyzerWorker::doAnalysis(TrackPointer tio, mixxx::AudioSourcePointer pAudioSource) {

    QTime progressUpdateInhibitTimer;
    progressUpdateInhibitTimer.start(); // Inhibit Updates for 60 milliseconds

    SINT frameIndex = pAudioSource->getMinFrameIndex();
    bool dieflag = false;
    bool cancelled = false;

    qDebug() << "Analyzing" << tio->getTitle() << tio->getLocation();
    do {
        ScopedTimer t("AnalyzerWorker::doAnalysis block");

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
            QListIterator<Analyzer*> it(m_analyzelist);
            while (it.hasNext()) {
                Analyzer* an =  it.next();
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
        int progressPromille = frameProgress * (1000.0 - FINALIZE_PROMILLE);

        if (m_progressInfo.track_progress != progressPromille &&
                progressUpdateInhibitTimer.elapsed() > 60) {
            // Inhibit Updates for 60 milliseconds
            emitUpdateProgress(progressPromille);
            progressUpdateInhibitTimer.start();
        }

        // This has proven to not be necessary, and if used, it should be done with care so as to 
        // not make the analysis slower than what it should. Also note that the user has the option
        // to reduce the number of threads that run the analysis.

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
    } while (!dieflag && (frameIndex < pAudioSource->getMaxFrameIndex()));

    return !cancelled; //don't return !dieflag or we might reanalyze over and over
}

//Called automatically by the owning thread to start the process (Configured to do so by AnalyzerManager)
void AnalyzerWorker::slotProcess() {
    QThread::currentThread()->setObjectName(QString("AnalyzerWorker %1").arg(m_workerIdx));

    //The instantiation of the initializers needs to be done on the worker thread. 
    //cannot be done on constructor.
    createAnalyzers();
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
        SoundSourceProxy soundSourceProxy(m_currentTrack);
        mixxx::AudioSourceConfig audioSrcCfg;
        audioSrcCfg.setChannelCount(kAnalysisChannels);
        mixxx::AudioSourcePointer pAudioSource(soundSourceProxy.openAudioSource(audioSrcCfg));
        if (!pAudioSource) {
            qWarning() << "Failed to open file for analyzing:" << m_currentTrack->getLocation();
            //TODO: maybe emit error("Failed to bblablalba");
            continue;
        }

        QListIterator<Analyzer*> it(m_analyzelist);
        bool processTrack = false;
        while (it.hasNext()) {
            // Make sure not to short-circuit initialize(...)
            if (it.next()->initialize(m_currentTrack, pAudioSource->getSamplingRate(), pAudioSource->getFrameCount() * kAnalysisChannels)) {
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
                QListIterator<Analyzer*> itf(m_analyzelist);
                while (itf.hasNext()) {
                    itf.next()->cleanup(m_currentTrack);
                }
                emitUpdateProgress(0);
            } else {
                // 100% - FINALIZE_PERCENT finished
                emitUpdateProgress(1000 - FINALIZE_PROMILLE);
                QListIterator<Analyzer*> itf(m_analyzelist);
                while (itf.hasNext()) {
                    itf.next()->finalize(m_currentTrack);
                }
                emitUpdateProgress(1000); // 100%
            }
        } else {
            emitUpdateProgress(1000); // 100%
            qDebug() << "Skipping track analysis because no analyzer initialized.";
        }
        Event::end(QString("AnalyzerWorker %1 process").arg(m_workerIdx));
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
    m_analyzelist.append(new AnalyzerWaveform(m_pConfig, m_priorizedJob));
    m_analyzelist.append(new AnalyzerGain(m_pConfig));
    m_analyzelist.append(new AnalyzerEbur128(m_pConfig));
#ifdef __VAMP__
    m_analyzelist.append(new AnalyzerBeats(m_pConfig, !m_priorizedJob));
    m_analyzelist.append(new AnalyzerKey(m_pConfig));
#endif
}
