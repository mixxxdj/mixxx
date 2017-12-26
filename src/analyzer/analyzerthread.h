#pragma once

#include <QThread>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "analyzer/analyzerprogress.h"
#include "preferences/usersettings.h"
#include "sources/audiosource.h"
#include "track/track.h"
#include "analyzer/analyzer.h"
#include "util/db/dbconnectionpool.h"
#include "util/samplebuffer.h"
#include "util/memory.h"


enum class AnalyzerMode {
    WithWaveform,
    WithoutWaveform,
    Default = WithWaveform,
};

class AnalyzerThread : public QThread {
    Q_OBJECT

  public:
    AnalyzerThread(
            int id,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            UserSettingsPointer pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    ~AnalyzerThread() override;

    operator bool() const {
        return m_run.load();
    }

    bool wake(const TrackPointer& nextTrack = TrackPointer());

    // Temporary storage of AnalyzerQueue, not accessed by the worker thread!
    // TODO(XXX): Get rid of this member if a simpler and cleaner solution
    // exists.
    void setCurrentTrackProgress(int currentTrackProgress) {
        m_currentTrackProgress = currentTrackProgress;
    }
    bool hasCurrentTrackProgress() const {
        return analyzerProgressValid(m_currentTrackProgress);
    }
    int getCurrentTrackProgress() const {
        return m_currentTrackProgress;
    }

    void stop();

    typedef std::map<TrackPointer, int> TracksWithProgress;

    class Progress {
      public:
        Progress();

        friend class ReadScope;
        class ReadScope final {
          public:
            ReadScope(const ReadScope&) = delete;
            ReadScope(ReadScope&& that)
                : m_progress(that.m_progress),
                  m_empty(that.m_empty) {
                that.m_progress = nullptr;
            }
            ~ReadScope();

            ReadScope& operator=(const ReadScope&) = delete;
            ReadScope& operator=(const ReadScope&&) = delete;

            operator const Progress*() const {
                return m_progress;
            }

            // Returns false if the shared progress update is currently
            // unreadable and we cannot determine if it actually is empty,
            // False positives must strictly be avoided to avoid race
            // conditions. A result of true indicates that an idle
            // analyzer thread can be stopped and destroyed safely
            // without losing the final progress update(s).
            bool empty() const {
                return m_empty;
            }

            const TracksWithProgress& tracksWithProgress() const {
                DEBUG_ASSERT(m_progress);
                return m_progress->m_tracksWithProgress;
            }

            const TrackPointer& currentTrack() const {
                DEBUG_ASSERT(m_progress);
                return m_progress->m_currentTrack;
            }

            int currentTrackProgress() const {
                DEBUG_ASSERT(m_progress);
                return m_progress->currentTrackProgress();
            }

            int finishedCount() const {
                DEBUG_ASSERT(m_progress);
                return m_progress->m_finishedCount;
            }

          private:
            friend class Progress;
            explicit ReadScope(Progress* progress);
            Progress* m_progress;
            bool m_empty;
        };

        ReadScope read() {
            return ReadScope(this);
        }

      private:
        void reset();

        bool tryWrite(
                TracksWithProgress* previousTracksWithProgress,
                int* finishedCount,
                TrackPointer /*nullable*/ currentTrack,
                int currenTrackProgress);

        int currentTrackProgress() const {
            DEBUG_ASSERT(m_currentTrack);
            DEBUG_ASSERT(m_tracksWithProgress.find(m_currentTrack)
                    != m_tracksWithProgress.end());
            return m_tracksWithProgress.find(m_currentTrack)->second;
        }

        friend class AnalyzerThread;
        std::atomic<int> m_state;
        TracksWithProgress m_tracksWithProgress;
        int m_finishedCount;
        TrackPointer m_currentTrack;
    };

    Progress::ReadScope readProgress() {
        return m_progress.read();
    }

  signals:
    // Ask for next track
    void idle(int id);
    // Report progress (shared memory)
    void progress(int id);
    // Notify about imminent death just before exiting (last signal)
    void exit(int id);

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
    // Temporary storage of AnalyzerQueue, not accessed by the worker thread!
    // TODO(XXX): Get rid of this member if a simpler and cleaner solution
    // exists.

    int m_currentTrackProgress;

    /////////////////////////////////////////////////////////////////////////
    // Thread shared

    std::atomic<bool> m_run;

    std::mutex m_nextTrackMutex;
    std::condition_variable m_nextTrackWaitCond;

    TrackPointer m_nextTrack;

    Progress m_progress;

    /////////////////////////////////////////////////////////////////////////
    // Thread local

    typedef std::unique_ptr<Analyzer> AnalyzerPtr;
    std::vector<AnalyzerPtr> m_analyzers;

    mixxx::SampleBuffer m_sampleBuffer;

    TracksWithProgress m_previousTracksWithProgress;

    int m_finishedCount;

    TrackPointer m_currentTrack;

    typedef std::chrono::steady_clock Clock;
    Clock::time_point m_lastProgressEmittedAt;

    enum class AnalysisResult {
        Pending,
        Partial,
        Complete,
        Cancelled,
    };
    AnalysisResult analyzeCurrentTrack(
            mixxx::AudioSourcePointer pAudioSource);

    void exec();

    void waitForCurrentTrack();

    void finishCurrentTrack(int currentTrackProgress = kAnalyzerProgressUnknown);

    void emitProgress(bool finished, int currentTrackProgress = kAnalyzerProgressUnknown);
};
