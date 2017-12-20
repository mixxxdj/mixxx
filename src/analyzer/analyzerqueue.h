#pragma once

#include <QThread>
#include <QQueue>
#include <QWaitCondition>
#include <QAtomicInt>

#include <map>
#include <set>
#include <vector>

#include "preferences/usersettings.h"
#include "sources/audiosource.h"
#include "track/track.h"
#include "analyzer/analyzer.h"
#include "util/db/dbconnectionpool.h"
#include "util/samplebuffer.h"
#include "util/memory.h"


class AnalysisDao;

// Measured in 0.1%, i.e. promille
constexpr int kAnalysisProgressUnknown = -1;
constexpr int kAnalysisProgressNone = 0; // 0.0 %
constexpr int kAnalysisProgressFinalizing = 950; // 95.0 %
constexpr int kAnalysisProgressDone = 1000; // 100.0%

class AnalyzerQueue : public QThread {
    Q_OBJECT

  public:
    enum class Mode {
        Default,
        WithoutWaveform,
    };

    AnalyzerQueue(
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            const UserSettingsPointer& pConfig,
            Mode mode = Mode::Default);
    ~AnalyzerQueue() override;

    bool enqueueTrack(TrackPointer pTrack);

    // After adding tracks the analysis must be resumed.
    // This function returns the number of tracks that
    // are currently queued for analysis.
    int resume();

    bool isTrackPending(const TrackPointer& pTrack) const;

    void stop();

  public slots:
    void slotAnalyseTrack(TrackPointer pTrack);
    void slotThreadProgress();

  signals:
    void trackProgress(int progress);
    void trackFinished(int queueSize);

    // Queued signals from the worker thread
    void threadProgress();
    void threadIdle();

  protected:
    void run();

  private:
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    // This DAO belongs to the analysis thread and must not
    // be accessed from any other thread!
    std::unique_ptr<AnalysisDao> m_pAnalysisDao;

    // The analyzers belong to the analysis thread and must not
    // be accessed from any other thread!
    typedef std::unique_ptr<Analyzer> AnalyzerPtr;
    std::vector<AnalyzerPtr> m_pAnalyzers;

    void execThread();

    void dequeueNextTrackBlocking();
    void finishCurrentTrack(int finishedProgress = kAnalysisProgressUnknown);

    enum class AnalysisResult {
        Pending,
        Partial,
        Complete,
        Cancelled,
    };
    AnalysisResult analyzeCurrentTrack(
            mixxx::AudioSourcePointer pAudioSource);

    void emitThreadProgress(int currentTrackProgress = kAnalysisProgressUnknown);

    QAtomicInt m_cancelCurrentTrack;
    QAtomicInt m_exitThread;

    QAtomicInt m_threadIdle;
    void emitThreadIdle();

    // The processing queue and associated mutex
    mutable QMutex m_qm;
    QWaitCondition m_qwait;
    // pendingTracks = queuedTracks + currentTrack
    std::set<TrackPointer> m_pendingTracks;
    QQueue<TrackPointer> m_queuedTracks;
    TrackPointer m_currentTrack;

    // The following members are only accessed by the worker thread

    mixxx::SampleBuffer m_sampleBuffer;

    typedef std::map<TrackPointer, int> TracksWithProgress;

    class ThreadProgress {
      public:
        ThreadProgress();

        // Returns true if this was the first successful write
        // operation while idling. If false is returned the
        // write operation has either been rejected because
        // a read operation is in progress or the write operation
        // has succeeded and updated the results of a previous
        // write operation that has not been read yet. Only a
        // result of true requires further action.
        bool tryWrite(
                TracksWithProgress* previousTracksWithProgress,
                TrackPointer /*nullable*/ currentTrack,
                int currentTrackProgress,
                int queueSize);

        friend class ReadScope;
        class ReadScope final {
          public:
            ReadScope(const ReadScope&) = delete;
            ReadScope(ReadScope&& that)
                : m_pThreadProgress(that.m_pThreadProgress) {
                that.m_pThreadProgress = nullptr;
            }
            ~ReadScope();

            ReadScope& operator=(const ReadScope&) = delete;
            ReadScope& operator=(const ReadScope&&) = delete;

            operator const ThreadProgress*() const {
                return m_pThreadProgress;
            }

            const TracksWithProgress& tracksWithProgress() const {
                DEBUG_ASSERT(m_pThreadProgress);
                return m_pThreadProgress->m_tracksWithProgress;
            }

            int currentTrackProgress() const {
                DEBUG_ASSERT(m_pThreadProgress);
                return m_pThreadProgress->m_currentTrackProgress;
            }

            int queueSize() const {
                DEBUG_ASSERT(m_pThreadProgress);
                return m_pThreadProgress->m_queueSize;
            }

          private:
            friend class ThreadProgress;
            ReadScope(ThreadProgress* pThreadProgress);
            ThreadProgress* m_pThreadProgress;
        };

        ReadScope read() {
            return ReadScope(this);
        }


      private:
        friend class AnalyzerQueue;
        QAtomicInt m_state;
        TracksWithProgress m_tracksWithProgress;
        int m_currentTrackProgress;
        int m_queueSize;
    };
    ThreadProgress m_threadProgress;

    // These members are only accessed by the analysis thread
    // for collecting tracks until the next update.
    TracksWithProgress m_previousTracksWithProgress;
};
