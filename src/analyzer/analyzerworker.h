#ifndef ANALYZER_ANALYZERWORKER_H
#define ANALYZER_ANALYZERWORKER_H

#include <QThread>
#include <QList>
#include <QWaitCondition>
#include <QSemaphore>

#include <vector>

#include "sources/audiosource.h"
#include "track/track.h"
#include "util/samplebuffer.h"
#include "preferences/usersettings.h"


class Analyzer;
class QThread;

class AnalyzerWorker : public QObject {
    Q_OBJECT

public:
    struct progress_info {
        int worker;
        TrackPointer current_track;
        int track_progress; // in 0.1 %
        QSemaphore sema;
    };
    //Constructor. Qthread is passed only to connect several signals. It is not stored.
    //pConfig is stored and batchJob indicates if this worker is a analysisfeature job (batch)
    //or player job.
    // Call Qthread->start() when you are ready for the worker to start.
    AnalyzerWorker(UserSettingsPointer pConfig, int workerIdx, bool batchJob);
    virtual ~AnalyzerWorker();

    //Called by the manager as a response to the waitingForNextTrack signal. and ONLY then.
    void nextTrack(TrackPointer newTrack);
    //called to pause this worker (waits on a qwaitcondition)
    void pause();
    //resumes from a previous call to pause
    void resume();
    //Tells this worker to end. It will delete itself and the Qthread.
    // An updateProgress signal with progress 0 will be emited and also the finished signal
    void endProcess();
    // Is this a batch worker?
    bool isBatch();

public slots:
    //Called automatically by the owning thread to start the process
    void slotProcess();

signals:
    //Signal that informs about the progress of the analysis.
    void updateProgress(int workerIdx, struct AnalyzerWorker::progress_info*);
    //Signal emited to the manager in order to receive a new track. The manager should call nextTrack();
    void waitingForNextTrack(AnalyzerWorker* worker);
    //Signal emited when the worker has effectively paused
    void paused(AnalyzerWorker* worker);
    //Signal emited when the worker ends and deletes itself.
    void workerFinished(AnalyzerWorker* worker);
    //Signal emited when this worker ends
    void finished();
    //Currently this signal is unused. It might be useful in the future.
    void error(QString err);

private:
    //Analyze one track.
    bool doAnalysis(TrackPointer tio, mixxx::AudioSourcePointer pAudioSource);
    //helper function to emit the updateProgress signal
    void emitUpdateProgress(int progress);
    //helper function to create the analyzers.
    void createAnalyzers();

    UserSettingsPointer m_pConfig;
    QList<Analyzer*> m_analyzelist;
    bool m_batchJob;
    int m_workerIdx;
    SampleBuffer m_sampleBuffer;
    TrackPointer m_currentTrack;

    bool m_exit;
    QAtomicInt m_pauseRequested;
    QMutex m_qm;
    QWaitCondition m_qwait;
    struct progress_info m_progressInfo;

};

inline bool AnalyzerWorker::isBatch() {
    return m_batchJob;
}

#endif /* ANALYZER_ANALYZERWORKER_H */
