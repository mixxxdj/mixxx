#pragma once

#include <deque>
#include <vector>

#include "analyzer/analyzerthread.h"

#include "util/memory.h"


// forward declaration(s)
class Library;

class TrackAnalysisScheduler : public QObject {
    Q_OBJECT

  public:
    typedef std::unique_ptr<TrackAnalysisScheduler, void(*)(TrackAnalysisScheduler*)> Pointer;
    static Pointer nullPointer();

    static Pointer createInstance(
            Library* library,
            int numWorkerThreads,
            const UserSettingsPointer& pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);

    /*private*/ TrackAnalysisScheduler(
            Library* library,
            int numWorkerThreads,
            const UserSettingsPointer& pConfig,
            AnalyzerMode mode);
    ~TrackAnalysisScheduler() override;

    // Stops a running analysis and discards all enqueued tracks.
    void stop();

  public slots:
    // Schedule tracks one by one. After all tracks have been scheduled
    // the caller must invoke resume() once.
    void scheduleTrackById(TrackId trackId);

    void suspend();

    // After scheduling tracks the analysis must be resumed once.
    // Resume must also be called after suspending the analysis.
    void resume();

  signals:
    // Progress for individual tracks is passed-through from the workers
    void trackProgress(TrackId trackId, AnalyzerProgress analyzerProgress);
    // Current average progress for all scheduled tracks and from all workers
    void progress(AnalyzerProgress currentProgress, int currentCount, int totalCount);
    void finished();

  private slots:
    void onWorkerThreadProgress(int threadId, AnalyzerThreadState threadState, TrackId trackId, AnalyzerProgress analyzerProgress);

  private:
    class Worker {
      public:
        explicit Worker(std::unique_ptr<AnalyzerThread> thread = std::unique_ptr<AnalyzerThread>())
            : m_thread(thread.get()),
              m_analyzerProgress(kAnalyzerProgressUnknown),
              m_threadIdle(false) {
            if (thread) {
                thread.release()->deleteAfterFinished();
            }
        }
        Worker(const Worker&) = delete;
        Worker(Worker&&) = default;

        operator bool() const {
            return m_thread != nullptr;
        }

        AnalyzerThread* thread() const {
            DEBUG_ASSERT(m_thread);
            return m_thread;
        }

        bool threadIdle() const {
            DEBUG_ASSERT(m_thread);
            return m_threadIdle;
        }

        AnalyzerProgress analyzerProgress() const {
            return m_analyzerProgress;
        }

        void submitNextTrack(TrackPointer track) {
            DEBUG_ASSERT(track);
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(m_threadIdle);
            m_thread->submitNextTrack(std::move(m_track));
            m_threadIdle = false;
        }

        void wakeThread() {
            if (m_thread) {
                m_thread->wake();
            }
        }

        void suspendThread() {
            if (m_thread) {
                m_thread->suspend();
            }
        }

        void resumeThread() {
            if (m_thread) {
                m_thread->resume();
            }
        }

        void stopThread() {
            if (m_thread) {
                m_thread->stop();
            }
        }

        void receiveThreadIdle() {
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(!m_threadIdle);
            m_threadIdle = true;
            m_analyzerProgress = kAnalyzerProgressUnknown;
        }

        void receiveAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress) {
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(!m_threadIdle);
            m_analyzerProgress = analyzerProgress;
        }

        void receiveThreadExit() {
            DEBUG_ASSERT(m_thread);
            m_thread = nullptr;
            m_threadIdle = false;
            m_analyzerProgress = kAnalyzerProgressUnknown;
        }

      private:
        AnalyzerThread* m_thread;
        bool m_threadIdle;
        AnalyzerProgress m_analyzerProgress;
    };

    TrackPointer loadTrackById(TrackId trackId);
    bool submitNextTrack(Worker* worker);
    void emitProgressOrFinished();

    bool isFinished() const {
        DEBUG_ASSERT(m_finishedCount <= m_dequeuedCount);
        return m_queuedTrackIds.empty() && (m_finishedCount == m_dequeuedCount);
    }

    Library* m_library;

    std::vector<Worker> m_workers;

    std::deque<TrackId> m_queuedTrackIds;

    AnalyzerProgress m_currentProgress;

    int m_currentCount;

    int m_finishedCount;

    int m_dequeuedCount;

    typedef std::chrono::steady_clock Clock;
    Clock::time_point m_lastProgressEmittedAt;
};
