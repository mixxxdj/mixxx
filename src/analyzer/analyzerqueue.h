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
    AnalyzerQueue(
            Library* library,
            int numWorkerThreads,
            const UserSettingsPointer& pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    ~AnalyzerQueue() override = default;

    int enqueueTrackId(TrackId trackId);

    int finishedCount() const {
        return m_finishedCount;
    }

    int totalCount() const {
        return m_dequeuedCount + m_queuedTrackIds.size();
    }

    // After adding tracks the analysis must be resumed once.
    void resume();

    void cancel();

  signals:
    void progress(int analyzerProgress, int finishedCount, int totalCount);
    void empty(int finishedCount);
    void done();

  public slots:
    void slotAnalyzeTrack(TrackPointer track);

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

        AnalyzerThread* thread() const {
            DEBUG_ASSERT(m_thread);
            return m_thread.get();
        }

        bool threadIdle() const {
            return m_threadIdle;
        }

        int analyzerProgress() const {
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

        void recvAnalyzerProgress(TrackId trackId) {
            DEBUG_ASSERT(m_thread);
            DEBUG_ASSERT(m_track);
            DEBUG_ASSERT(m_track->getId() == trackId);
            DEBUG_ASSERT(!m_threadIdle);
            m_analyzerProgress = m_thread->readAnalyzerProgress();
            m_track->setAnalyzerProgress(m_analyzerProgress);
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
        int m_analyzerProgress;
        bool m_threadIdle;
    };

    TrackPointer loadTrackById(TrackId trackId);
    bool resumeIdleWorker(Worker& worker);
    void emitProgress();

    Library* m_library;

    std::vector<Worker> m_workers;

    std::deque<TrackId> m_queuedTrackIds;

    int m_dequeuedCount;

    int m_finishedCount;

    typedef std::chrono::steady_clock Clock;
    Clock::time_point m_lastProgressEmittedAt;
};
