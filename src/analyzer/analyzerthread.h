#pragma once

#include <vector>

#include "util/workerthread.h"

#include "analyzer/analyzerprogress.h"
#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"
#include "sources/audiosource.h"
#include "track/track.h"
#include "util/db/dbconnectionpool.h"
#include "util/performancetimer.h"
#include "util/samplebuffer.h"
#include "util/memory.h"
#include "util/mpscfifo.h"


enum AnalyzerModeFlags {
    None = 0x00,
    WithBeats = 0x01,
    WithWaveform = 0x02,
    All = WithBeats | WithWaveform,
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
    typedef std::unique_ptr<AnalyzerThread, void(*)(AnalyzerThread*)> Pointer;
    // Subclass that provides a default constructor and nothing else
    class NullPointer: public Pointer {
      public:
        NullPointer();
    };

    static Pointer createInstance(
            int id,
            mixxx::DbConnectionPoolPtr dbConnectionPool,
            UserSettingsPointer pConfig,
            AnalyzerModeFlags modeFlags);

    /*private*/ AnalyzerThread(
            int id,
            mixxx::DbConnectionPoolPtr dbConnectionPool,
            UserSettingsPointer pConfig,
            AnalyzerModeFlags modeFlags);
    ~AnalyzerThread() override = default;

    int id() const {
        return m_id;
    }

    // Submits the next track to the worker thread without
    // blocking. This is only allowed after a progress() signal
    // with state Idle has been received to avoid overwriting
    // a previously sent track that has not been received by the
    // worker thread, yet.
    bool submitNextTrack(TrackPointer nextTrack);

  signals:
    // Use a single signal for progress updates to ensure that all signals
    // are queued and received in the same order as emitted from the internal
    // worker thread. Different signals would otherwise end up in different
    // queued connections which are processed in an undefined order!
    // TODO(uklotzde): Encapsulate all signal parameters into an
    // AnalyzerThreadProgress object and register it as a new meta type.
    void progress(int threadId, AnalyzerThreadState threadState, TrackId trackId, AnalyzerProgress trackProgress);

  protected:
    void doRun() override;

    FetchWorkResult tryFetchWorkItems() override;

  private:
    /////////////////////////////////////////////////////////////////////////
    // Immutable values and pointers (objects are thread-safe)
    const int m_id;
    const mixxx::DbConnectionPoolPtr m_dbConnectionPool;
    const UserSettingsPointer m_pConfig;
    const AnalyzerModeFlags m_modeFlags;

    /////////////////////////////////////////////////////////////////////////
    // Thread-safe atomic values

    // There is only one consumer (namely the worker thread) and one producer
    // (the host thread) for this value. A single value is written and then
    // read so a lock-free FIFO with the minimum capacity is sufficient for
    // safely exchanging data between two threads.
    // NOTE(uklotzde, 2018-01-04): Ideally we would use std::atomic<TrackPointer>,
    // for this purpose, which will become available in C++20.
    MpscFifo<TrackPointer, 1> m_nextTrack;

    /////////////////////////////////////////////////////////////////////////
    // Thread local: Only used in the constructor/destructor and within
    // run() by the worker thread.

    typedef std::unique_ptr<Analyzer> AnalyzerPtr;
    std::vector<AnalyzerPtr> m_analyzers;

    mixxx::SampleBuffer m_sampleBuffer;

    TrackPointer m_currentTrack;

    AnalyzerThreadState m_emittedState;

    PerformanceTimer m_lastBusyProgressEmittedTimer;

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

    // Unconditionally emits any kind of progress() signal if not current track is present
    void emitProgress(AnalyzerThreadState state);

    // Unconditionally emits any kind of progress() signal
    void emitProgress(AnalyzerThreadState state, TrackId trackId, AnalyzerProgress trackProgress);
};
