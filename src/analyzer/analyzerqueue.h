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
    void queueAnalyseTrack(TrackPointer tio);

  public slots:
    void slotAnalyseTrack(TrackPointer tio);
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
    struct progress_info {
        TrackPointer current_track;
        int track_progress; // in 0.1 %
        int queue_size;
        QSemaphore sema;
    };

    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    std::unique_ptr<AnalysisDao> m_pAnalysisDao;

    typedef std::unique_ptr<Analyzer> AnalyzerPtr;
    std::vector<AnalyzerPtr> m_pAnalyzers;

    void execThread();

    bool isLoadedTrackWaiting(TrackPointer analysingTrack);
    TrackPointer dequeueNextBlocking();
    bool doAnalysis(TrackPointer tio, mixxx::AudioSourcePointer pAudioSource);
    void emitUpdateProgress(TrackPointer tio, int progress);
    void emptyCheck();

    bool m_exit;
    QAtomicInt m_aiCheckPriorities;

    SampleBuffer m_sampleBuffer;

    // The processing queue and associated mutex
    QQueue<TrackPointer> m_tioq;
    QMutex m_qm;
    QWaitCondition m_qwait;
    struct progress_info m_progressInfo;
    int m_queue_size;
};

#endif /* ANALYZER_ANALYZERQUEUE_H */
