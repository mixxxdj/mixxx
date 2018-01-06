#pragma once

#include <chrono>
#include <vector>

#include "util/workerthread.h"

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
    Default,
    WithBeats,
    WithBeatsWithoutWaveform,
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

// Atomic control values are used for transferring data between the
// host and the worker thread, e.g. the next track to be analyzed or
// the current analyzer progress that can be read independent of any
// progress signal
//
// The frequency of progress signal is limited to avoid flooding the
// signal queued connection between the internal worker thread and
// the host, which might otherwise cause unresponsiveness of the host.
class AnalyzerThread : public WorkerThread {
    Q_OBJECT

  public:
    AnalyzerThread(
            int id,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            UserSettingsPointer pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    ~AnalyzerThread() override = default;

    int id() const {
        return m_id;
    }

    // Transmits the next track to the worker thread without
    // blocking. This is only allowed after a progress() signal
    // with state Idle has been received to avoid overwriting
    // a previously sent track that has not been received by the
    // worker thread, yet.
    void sendNextTrack(const TrackPointer& nextTrack);

    // Non-blocking atomic read of the current analyzer progress
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
    void exec() override;

    FetchWorkResult fetchWork() override;

  private:
    /////////////////////////////////////////////////////////////////////////
    // Immutable values and pointers (objects are thread-safe)

    const int m_id;

    const mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    const UserSettingsPointer m_pConfig;
    const AnalyzerMode m_mode;

    /////////////////////////////////////////////////////////////////////////
    // Thread-safe atomic values

    ControlValueAtomic<TrackPointer> m_nextTrack;

    ControlValueAtomic<AnalyzerProgress> m_analyzerProgress;

    /////////////////////////////////////////////////////////////////////////
    // Thread local: Only used in the constructor/destructor and within
    // run() by the worker thread.

    typedef std::unique_ptr<Analyzer> AnalyzerPtr;
    std::vector<AnalyzerPtr> m_analyzers;

    mixxx::SampleBuffer m_sampleBuffer;

    TrackPointer m_currentTrack;

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
            const mixxx::AudioSourcePointer& audioSource);

    // Blocks the worker thread until a next track becomes available
    TrackPointer receiveNextTrack();

    // Conditionally emit a progress() signal while busy (frequency is limited)
    void emitBusyProgress(AnalyzerProgress busyProgress);

    // Unconditionally emits a progress() signal when done
    void emitDoneProgress(AnalyzerProgress doneProgress);

    // Unconditionally emits any kind of progress() signal
    void emitProgress(AnalyzerThreadState state, TrackId trackId = TrackId());
};
