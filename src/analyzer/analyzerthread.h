#pragma once

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <atomic>
#include <vector>

#include "analyzer/analysisprogress.h"
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
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            UserSettingsPointer pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    ~AnalyzerThread() override;

    bool wake(const TrackPointer& nextTrack = TrackPointer());

    void stop();

    typedef std::map<TrackPointer, int> TracksWithProgress;

    class ProgressUpdate {
      public:
        ProgressUpdate();

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
                int currenTrackProgress);

        friend class ReadScope;
        class ReadScope final {
          public:
            ReadScope(const ReadScope&) = delete;
            ReadScope(ReadScope&& that)
                : m_progressUpdate(that.m_progressUpdate) {
                that.m_progressUpdate = nullptr;
            }
            ~ReadScope();

            ReadScope& operator=(const ReadScope&) = delete;
            ReadScope& operator=(const ReadScope&&) = delete;

            operator const ProgressUpdate*() const {
                return m_progressUpdate;
            }

            const TracksWithProgress& tracksWithProgress() const {
                DEBUG_ASSERT(m_progressUpdate);
                return m_progressUpdate->m_tracksWithProgress;
            }

            const TrackPointer& currentTrack() const {
                DEBUG_ASSERT(m_progressUpdate);
                return m_progressUpdate->m_currentTrack;
            }

            int currentTrackProgress() const {
                DEBUG_ASSERT(m_progressUpdate);
                return m_progressUpdate->currentTrackProgress();
            }

          private:
            friend class ProgressUpdate;
            explicit ReadScope(ProgressUpdate* progressUpdate);
            ProgressUpdate* m_progressUpdate;
        };

        ReadScope read() {
            return ReadScope(this);
        }

      private:
        void reset();

        int currentTrackProgress() const {
            if (m_currentTrack) {
                DEBUG_ASSERT(m_tracksWithProgress.find(m_currentTrack)
                        != m_tracksWithProgress.end());
                return m_tracksWithProgress.find(m_currentTrack)->second;
            } else {
                return kAnalysisProgressUnknown;
            }
        }

        friend class AnalyzerThread;
        std::atomic<int> m_state;
        TracksWithProgress m_tracksWithProgress;
        TrackPointer m_currentTrack;
    };

    ProgressUpdate::ReadScope readProgressUpdate() {
        return m_progressUpdate.read();
    }

  signals:
    void progressUpdate();
    void idle();

  protected:
    void run() override;

  private:
    /////////////////////////////////////////////////////////////////////////
    // Immutable

    const mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    const UserSettingsPointer m_pConfig;
    const AnalyzerMode m_mode;

    /////////////////////////////////////////////////////////////////////////
    // Thread shared

    std::atomic<bool> m_stop;

    mutable QMutex m_nextTrackMutex;
    QWaitCondition m_nextTrackWaitCond;

    TrackPointer m_nextTrack;

    ProgressUpdate m_progressUpdate;

    /////////////////////////////////////////////////////////////////////////
    // Thread local

    typedef std::unique_ptr<Analyzer> AnalyzerPtr;
    std::vector<AnalyzerPtr> m_analyzers;

    mixxx::SampleBuffer m_sampleBuffer;

    TracksWithProgress m_previousTracksWithProgress;

    TrackPointer m_currentTrack;

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

    void finishCurrentTrack(int currentTrackProgress = kAnalysisProgressUnknown);

    void emitProgressUpdate(int currentTrackProgress = kAnalysisProgressUnknown);
};
