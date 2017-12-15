#ifndef ANALYZER_ANALYZERQUEUE_H
#define ANALYZER_ANALYZERQUEUE_H

#include <QThread>
#include <QQueue>
#include <QWaitCondition>
#include <QSemaphore>

#include <vector>

#include "preferences/usersettings.h"
#include "sources/audiosource.h"
#include "track/track.h"
#include "util/db/dbconnectionpool.h"
#include "util/samplebuffer.h"
#include "util/memory.h"

class Analyzer;
class AnalysisDao;

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

    void stop();
    void enqueueTrack(TrackPointer pTrack);

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
    bool doAnalysis(TrackPointer pTrack, mixxx::AudioSourcePointer pAudioSource);
    void emitUpdateProgress(TrackPointer pTrack, int progress);
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

    struct progress_info {
        TrackPointer current_track;
        int track_progress; // in 0.1 %
        int queue_size;
        QSemaphore sema;
    };
    struct progress_info m_progressInfo;
};

#endif /* ANALYZER_ANALYZERQUEUE_H */
