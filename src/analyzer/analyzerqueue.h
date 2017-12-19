#pragma once

#include <QThread>
#include <QQueue>
#include <QWaitCondition>
#include <QAtomicInt>

#include <map>
#include <vector>

#include "preferences/usersettings.h"
#include "sources/audiosource.h"
#include "track/track.h"
#include "util/db/dbconnectionpool.h"
#include "util/samplebuffer.h"
#include "util/memory.h"


class Analyzer;
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

    void enqueueTrack(TrackPointer pTrack);

    // After adding tracks the analysis must be resumed.
    // This function returns the number of tracks that
    // are currently queued for analysis.
    int resume();

    void stop();

  public slots:
    void slotAnalyseTrack(TrackPointer pTrack);
    void slotUpdateProgress();

  signals:
    void trackProgress(int progress);
    void trackDone(TrackPointer track);
    void trackFinished(int size);
    // Signals from AnalyzerQueue Thread:
    void queueEmpty();
    void updateProgress();

  protected:
    void run();

  private:

    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    std::unique_ptr<AnalysisDao> m_pAnalysisDao;

    typedef std::unique_ptr<Analyzer> AnalyzerPtr;
    std::vector<AnalyzerPtr> m_pAnalyzers;

    void execThread();

    bool isLoadedTrackWaiting(TrackPointer pAnalyzingTrack);
    TrackPointer dequeueNextBlocking();
    enum class AnalysisResult {
        Pending,
        Partial,
        Complete,
        Cancelled,
    };
    AnalysisResult doAnalysis(TrackPointer pTrack, mixxx::AudioSourcePointer pAudioSource);
    void emitUpdateProgress(TrackPointer pTrack, int progress);
    void emitUpdateProgress() {
        // No current track
        emitUpdateProgress(TrackPointer(), kAnalysisProgressNone);
    }
    void emptyCheck();
    void updateSize();

    QAtomicInt m_queueSize;
    QAtomicInt m_queueModifiedFlag;
    QAtomicInt m_exitPendingFlag;

    // The processing queue and associated mutex
    QMutex m_qm;
    QWaitCondition m_qwait;
    QQueue<TrackPointer> m_queuedTracks;

    // The following members are only accessed by the worker thread

    mixxx::SampleBuffer m_sampleBuffer;

    typedef std::map<TrackPointer, int> TracksWithProgress;

    class ProgressInfo {
      public:
        ProgressInfo();

        // Returns true if this was the first successful write
        // operation while idling. If false is returned the
        // write operation has either been rejected because
        // a read operation is in progress or the write operation
        // has succeeded and updated the results of a previous
        // write operation that has not been read yet. Only a
        // result of true requires further action.
        bool tryWrite(
                TracksWithProgress* pTracksWithProgress,
                TrackPointer /*nullable*/ pCurrentTrack,
                int currentTrackProgress,
                int queueSize);

        friend class ReadScope;
        class ReadScope final {
          public:
            ReadScope(const ReadScope&) = delete;
            ReadScope(ReadScope&& that)
                : m_pProgressInfo(that.m_pProgressInfo) {
                that.m_pProgressInfo = nullptr;
            }
            ~ReadScope();

            ReadScope& operator=(const ReadScope&) = delete;
            ReadScope& operator=(const ReadScope&&) = delete;

            operator const ProgressInfo*() const {
                return m_pProgressInfo;
            }

            const TracksWithProgress& tracksWithProgress() const {
                DEBUG_ASSERT(m_pProgressInfo);
                return m_pProgressInfo->m_tracksWithProgress;
            }

            int currentTrackProgress() const {
                DEBUG_ASSERT(m_pProgressInfo);
                return m_pProgressInfo->m_currentTrackProgress;
            }

            int queueSize() const {
                DEBUG_ASSERT(m_pProgressInfo);
                return m_pProgressInfo->m_queueSize;
            }

          private:
            friend class ProgressInfo;
            ReadScope(ProgressInfo* pProgressInfo);
            ProgressInfo* m_pProgressInfo;
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
    ProgressInfo m_progressInfo;

    // These members are only accessed by the analysis thread
    // for collecting tracks until the next update.
    TracksWithProgress m_tracksWithProgress;
};
