#pragma once

#include <QThread>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "analyzer/analyzerprogress.h"
#include "analyzer/analyzer.h"
#include "control/controlvalue.h"
#include "preferences/usersettings.h"
#include "sources/audiosource.h"
#include "track/track.h"
#include "util/db/dbconnectionpool.h"
#include "util/samplebuffer.h"
#include "util/memory.h"


enum class AnalyzerMode {
    WithWaveform,
    WithoutWaveform,
    Default = WithWaveform,
};

enum class AnalyzerThreadState {
    Void,
    Idle,
    Busy,
    Done,
    Exit,
};

Q_DECLARE_TYPEINFO(AnalyzerThreadState, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(AnalyzerThreadState);

// This object lives in the creating thread of the host, i.e. does not
// run its own event loop. It does not does not use slots for communication
// with its host which would otherwise still be executed in the host's
// thread.
//
// Signals emitted from the internal worker thread use queued connections.
// Communication in the opposite direction is accomplished by using
// lock-free types to avoid locking the host thread through priority
// inversion. Lock-free types are also used for any shared state (like
// the current analyzer progress) that is read from the host thread
// after being notified about changes.
//
// The frequency of change notifications is limited to avoid flooding
// the signal queues between the internal worker thread and the host,
// which might cause unresponsiveness of the host.
class AnalyzerThread : public QThread {
    Q_OBJECT

  public:
    AnalyzerThread(
            int id,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            UserSettingsPointer pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    // The destructor must be triggered by calling deleteLater() to
    // ensure that the thread has already finished and is not running!
    ~AnalyzerThread() override;

    operator bool() const {
        return !readStopped();
    }

    int id() const {
        return m_id;
    }

    void pause();
    void resume();

    void stop();

    void sendNextTrack(const TrackPointer& nextTrack);

    AnalyzerProgress readAnalyzerProgress() const {
        return m_analyzerProgress.getValue();
    }

  signals:
    // Use a single signal for progress updates to ensure that all signals
    // are queued and received in the same order as emitted from the internal
    // worker thread. Different signals would otherwise end up in different
    // queued connections which are processed in an undefined order!
    void progress(int threadId, AnalyzerThreadState threadState, TrackId trackId);

  protected:
    void run() override;

  private:
    /////////////////////////////////////////////////////////////////////////
    // Immutable

    const int m_id;

    const mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    const UserSettingsPointer m_pConfig;
    const AnalyzerMode m_mode;

    /////////////////////////////////////////////////////////////////////////
    // Thread shared

    std::atomic<bool> m_pause;
    std::atomic<bool> m_stop;

    ControlValueAtomic<TrackPointer> m_nextTrack;

    ControlValueAtomic<AnalyzerProgress> m_analyzerProgress;

    std::mutex m_sleepMutex;
    std::condition_variable m_sleepWaitCond;

    /////////////////////////////////////////////////////////////////////////
    // Thread local

    typedef std::unique_ptr<Analyzer> AnalyzerPtr;
    std::vector<AnalyzerPtr> m_analyzers;

    mixxx::SampleBuffer m_sampleBuffer;

    AnalyzerThreadState m_emittedState;

    typedef std::chrono::steady_clock Clock;
    Clock::time_point m_lastBusyProgressEmittedAt;

    enum class AnalysisResult {
        Pending,
        Partial,
        Complete,
        Cancelled,
    };
    AnalysisResult analyzeAudioSource(
            const TrackPointer& track,
            const mixxx::AudioSourcePointer& audioSource);

    void exec();

    TrackPointer recvNextTrack(); // blocking
    void recvPaused(); // blocking

    bool readStopped() const {
        return m_stop.load();
    }

    // Conditional emitting of progress() signal
    void emitBusyProgress(const TrackPointer& track, AnalyzerProgress busyProgress);

    // Unconditional emitting of progress() signal
    void emitDoneProgress(const TrackPointer& track, AnalyzerProgress doneProgress);

    void emitProgress(AnalyzerThreadState state, const TrackPointer& track = TrackPointer());
};
