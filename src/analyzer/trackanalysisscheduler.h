#pragma once

#include <deque>
#include <vector>

#include "analyzer/analyzerthread.h"

#include "util/memory.h"


// forward declaration(s)
class Library;
class TrackAnalysisSchedulerPointer;

class TrackAnalysisScheduler : public QObject {
    Q_OBJECT

  public:
    // Don't use this constructor which is publicly visible only for technical
    // reasons! TrackAnalysisScheduler objects should always be allocated dynamically
    // through TrackAnalysisSchedulerPointer (see below) to ensure that all worker threads
    // have finished running before the corresponding queue is destroyed.
    TrackAnalysisScheduler(
            Library* library,
            int numWorkerThreads,
            const UserSettingsPointer& pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    ~TrackAnalysisScheduler() override;

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
            thread.release()->deleteAfterFinished();
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
            m_track = std::move(track);
            m_threadIdle = false;
            m_thread->submitNextTrack(m_track);
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
            m_track.reset();
            m_analyzerProgress = kAnalyzerProgressUnknown;
            m_threadIdle = true;
        }

        void receiveAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress) {
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(m_track);
            DEBUG_ASSERT(m_track->getId() == trackId);
            DEBUG_ASSERT(!m_threadIdle);
            m_analyzerProgress = analyzerProgress;
        }

        void receiveThreadExit() {
            DEBUG_ASSERT(m_thread);
            m_thread = nullptr;
            m_track.reset();
            m_analyzerProgress = kAnalyzerProgressUnknown;
            m_threadIdle = false;
        }

      private:
        AnalyzerThread* m_thread;
        TrackPointer m_track;
        AnalyzerProgress m_analyzerProgress;
        bool m_threadIdle;
    };

    TrackPointer loadTrackById(TrackId trackId);
    bool submitNextTrack(Worker* worker);
    void emitProgressOrFinished();

    bool isFinished() const {
        DEBUG_ASSERT(m_finishedCount <= m_dequeuedCount);
        return m_queuedTrackIds.empty() && (m_finishedCount == m_dequeuedCount);
    }

    // Stops a running analysis and discards all enqueued tracks.
    friend class TrackAnalysisSchedulerPointer;
    void stop();

    Library* m_library;

    std::vector<Worker> m_workers;

    std::deque<TrackId> m_queuedTrackIds;

    int m_dequeuedCount;

    int m_finishedCount;

    typedef std::chrono::steady_clock Clock;
    Clock::time_point m_lastProgressEmittedAt;
};

// A movable pointer for the dynamic allocation TrackAnalysisScheduler instances
// that accounts for the special destruction semantics.
class TrackAnalysisSchedulerPointer final {
  public:
    TrackAnalysisSchedulerPointer() = default;
    TrackAnalysisSchedulerPointer(
            Library* library,
            int numWorkerThreads,
            const UserSettingsPointer& pConfig,
            AnalyzerMode mode = AnalyzerMode::Default)
          : m_impl(std::make_unique<TrackAnalysisScheduler>(library, numWorkerThreads, pConfig, mode)) {
    }
    TrackAnalysisSchedulerPointer(TrackAnalysisSchedulerPointer&&) = default;
    TrackAnalysisSchedulerPointer(const TrackAnalysisSchedulerPointer&) = delete;
    ~TrackAnalysisSchedulerPointer() {
        reset();
    }

    TrackAnalysisSchedulerPointer& operator=(TrackAnalysisSchedulerPointer&&) = default;
    TrackAnalysisSchedulerPointer& operator=(const TrackAnalysisSchedulerPointer&) = delete;

    void reset();

    operator bool() const {
        return static_cast<bool>(m_impl);
    }
    bool operator!() const {
        return !m_impl;
    }
    operator TrackAnalysisScheduler*() const {
        DEBUG_ASSERT(m_impl.get());
        return m_impl.get();
    }
    TrackAnalysisScheduler* operator->() const {
        DEBUG_ASSERT(m_impl.get());
        return m_impl.get();
    }

  private:
    std::unique_ptr<TrackAnalysisScheduler> m_impl;
};
