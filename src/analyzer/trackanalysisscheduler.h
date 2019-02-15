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
    // Subclass that provides a default constructor and nothing else
    class NullPointer: public Pointer {
      public:
        NullPointer();
    };

    static Pointer createInstance(
            Library* library,
            int numWorkerThreads,
            const UserSettingsPointer& pConfig,
            AnalyzerModeFlags modeFlags);

    /*private*/ TrackAnalysisScheduler(
            Library* library,
            int numWorkerThreads,
            const UserSettingsPointer& pConfig,
            AnalyzerModeFlags modeFlags);
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
    void progress(AnalyzerProgress currentTrackProgress, int currentTrackNumber, int totalTracksCount);
    void finished();

  private slots:
    void onWorkerThreadProgress(int threadId, AnalyzerThreadState threadState, TrackId trackId, AnalyzerProgress analyzerProgress);

  private:
    // Owns an analyzer thread and buffers the most recent progress update
    // received from this thread during analysis. It does not need to be
    // thread-safe, because all functions are invoked from the host thread
    // that runs the TrackAnalysisScheduler.
    class Worker {
      public:
        explicit Worker(AnalyzerThread::Pointer thread = AnalyzerThread::NullPointer())
            : m_thread(std::move(thread)),
              m_threadIdle(false),
              m_analyzerProgress(kAnalyzerProgressUnknown) {
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
            DEBUG_ASSERT(m_thread);
            return m_threadIdle;
        }

        AnalyzerProgress analyzerProgress() const {
            return m_analyzerProgress;
        }

        bool submitNextTrack(TrackPointer track) {
            DEBUG_ASSERT(track);
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(m_threadIdle);
            if (m_thread->submitNextTrack(std::move(track))) {
                m_threadIdle = false;
                return true;
            } else {
                return false;
            }
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

        void onThreadIdle() {
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(!m_threadIdle);
            m_threadIdle = true;
            m_analyzerProgress = kAnalyzerProgressUnknown;
        }

        void onAnalyzerProgress(TrackId /*trackId*/, AnalyzerProgress analyzerProgress) {
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(!m_threadIdle);
            m_analyzerProgress = analyzerProgress;
        }

        void onThreadExit() {
            DEBUG_ASSERT(m_thread);
            m_thread.reset();
            m_threadIdle = false;
            m_analyzerProgress = kAnalyzerProgressUnknown;
        }

      private:
        AnalyzerThread::Pointer m_thread;
        bool m_threadIdle;
        AnalyzerProgress m_analyzerProgress;
    };

    bool submitNextTrack(Worker* worker);
    void emitProgressOrFinished();

    bool allTracksFinished() const {
        DEBUG_ASSERT(m_finishedTracksCount <= m_dequeuedTracksCount);
        return m_queuedTrackIds.empty() && (m_finishedTracksCount == m_dequeuedTracksCount);
    }

    Library* m_library;

    std::vector<Worker> m_workers;

    std::deque<TrackId> m_queuedTrackIds;

    AnalyzerProgress m_currentTrackProgress;

    int m_currentTrackNumber;

    int m_finishedTracksCount;

    int m_dequeuedTracksCount;

    typedef std::chrono::steady_clock Clock;
    Clock::time_point m_lastProgressEmittedAt;
};
