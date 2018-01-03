#pragma once

#include <deque>
#include <vector>

#include "analyzer/analyzerthread.h"

#include "util/memory.h"


// forward declaration(s)
class Library;

class AnalyzerQueue : public QObject {
    Q_OBJECT

  public:
    // Don't use this constructor which is publicly visible only for technical
    // reasons! AnalyzerQueue objects should always be allocated dynamically
    // through AnalyzerQueuePointer (see below) to ensure that all worker threads
    // have finished running before the corresponding queue is destroyed.
    AnalyzerQueue(
            Library* library,
            int numWorkerThreads,
            const UserSettingsPointer& pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    ~AnalyzerQueue() override = default;

    // finishedCount() <= dequeuedCount() <= totalCount()
    int finishedCount() const {
        return m_finishedCount;
    }
    int dequeuedCount() const {
        return m_dequeuedCount;
    }
    int totalCount() const {
        return m_dequeuedCount + m_queuedTrackIds.size();
    }

    // Enqueue tracks one by one. After all tracks have been enqueued
    // the caller must call resume() once.
    void enqueueTrackId(TrackId trackId);

    // After enqueuing tracks the analysis must be resumed once.
    // Resume must also be called after pausing the analysis.
    void resume();

    void pause();

    // Cancels a running analysis and discards all enqueued tracks.
    void cancel();

  signals:
    // Progress for individual tracks is passed-through from the workers
    void trackProgress(TrackId trackId, AnalyzerProgress analyzerProgress);
    void progress(AnalyzerProgress analyzerProgress, int currentCount, int totalCount);
    void empty(int finishedCount);
    void done();

  private slots:
    void slotWorkerThreadProgress(int threadId, AnalyzerThreadState threadState, TrackId trackId);

  private:
    class Worker {
      public:
        explicit Worker(std::unique_ptr<AnalyzerThread> thread = std::unique_ptr<AnalyzerThread>())
            : m_thread(std::move(thread)),
              m_analyzerProgress(kAnalyzerProgressUnknown),
              m_threadIdle(false) {
        }
        Worker(const Worker&) = delete;
        Worker(Worker&&) = default;

        operator bool() const {
            return static_cast<bool>(m_thread);
        }

        AnalyzerThread* thread() const {
            DEBUG_ASSERT(m_thread);
            return m_thread.get();
        }

        bool threadIdle() const {
            return m_threadIdle;
        }

        AnalyzerProgress analyzerProgress() const {
            return m_analyzerProgress;
        }

        void sendNextTrack(TrackPointer track) {
            DEBUG_ASSERT(track);
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(m_threadIdle);
            m_track = std::move(track);
            m_threadIdle = false;
            m_thread->sendNextTrack(m_track);
        }

        void pauseThread() {
            if (m_thread) {
                m_thread->pause();
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

        void recvThreadIdle() {
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(!m_threadIdle);
            m_track.reset();
            m_analyzerProgress = kAnalyzerProgressUnknown;
            m_threadIdle = true;
        }

        AnalyzerProgress recvAnalyzerProgress(TrackId trackId) {
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(m_track);
            DEBUG_ASSERT(m_track->getId() == trackId);
            DEBUG_ASSERT(!m_threadIdle);
            // Read and store the atomic control value...
            m_analyzerProgress = m_thread->readAnalyzerProgress();
            // ...and return the result for further processing
            return m_analyzerProgress;
        }

        void recvThreadExit() {
            DEBUG_ASSERT(m_thread);
            m_thread->deleteLater();
            m_thread.release();
            DEBUG_ASSERT(!m_thread);
            m_analyzerProgress = kAnalyzerProgressUnknown;
            m_threadIdle = false;
        }

      private:
        std::unique_ptr<AnalyzerThread> m_thread;
        TrackPointer m_track;
        AnalyzerProgress m_analyzerProgress;
        bool m_threadIdle;
    };

    TrackPointer loadTrackById(TrackId trackId);
    bool resumeIdleWorker(Worker* worker);
    void emitProgress();

    bool hasUnfinishedTracks() const {
        return !m_queuedTrackIds.empty() || (m_finishedCount < m_dequeuedCount);
    }

    Library* m_library;

    std::vector<Worker> m_workers;

    std::deque<TrackId> m_queuedTrackIds;

    int m_dequeuedCount;

    int m_finishedCount;

    typedef std::chrono::steady_clock Clock;
    Clock::time_point m_lastProgressEmittedAt;
};

// A movable pointer for the dynamic allocation AnalyzerQueue instances
// that accounts for the special destruction semantics.
class AnalyzerQueuePointer final {
  public:
    AnalyzerQueuePointer() = default;
    AnalyzerQueuePointer(
            Library* library,
            int numWorkerThreads,
            const UserSettingsPointer& pConfig,
            AnalyzerMode mode = AnalyzerMode::Default)
          : m_impl(std::make_unique<AnalyzerQueue>(library, numWorkerThreads, pConfig, mode)) {
    }
    AnalyzerQueuePointer(AnalyzerQueuePointer&&) = default;
    AnalyzerQueuePointer(const AnalyzerQueuePointer&) = delete;
    ~AnalyzerQueuePointer() {
        reset();
    }

    AnalyzerQueuePointer& operator=(AnalyzerQueuePointer&&) = default;
    AnalyzerQueuePointer& operator=(const AnalyzerQueuePointer&) = delete;

    void reset();

    operator bool() const {
        return static_cast<bool>(m_impl);
    }
    bool operator!() const {
        return !m_impl;
    }
    operator AnalyzerQueue*() const {
        DEBUG_ASSERT(m_impl.get());
        return m_impl.get();
    }
    AnalyzerQueue* operator->() const {
        DEBUG_ASSERT(m_impl.get());
        return m_impl.get();
    }

  private:
    std::unique_ptr<AnalyzerQueue> m_impl;
};
