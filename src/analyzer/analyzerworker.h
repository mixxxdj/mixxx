#ifndef ANALYZER_ANALYZERWORKER_H
#define ANALYZER_ANALYZERWORKER_H

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

/* Worker class. 
* It represents a job that runs on a thread, analyzing tracks until no more tracks need to be analyzed.
*/
class AnalyzerWorker : public QObject {
    Q_OBJECT

public:
    //Information of the analysis used with the updateProgress signal.
    struct progress_info {
        //Worker identifier. This is used to differentiate between updateProgress signals, and it is
        //what lets the DlgAnalysis class to differentiate and show the different percentages.
        int worker;
        //Track being analyzed. This is currently used in the AnalyzerManager.
        TrackPointer current_track;
        //track progress in steps of 0.1 %
        int track_progress;
        //Semaphore to avoid exccesive signaling.
        QSemaphore sema;
    };

    // Constructor. If it is a priorized job, the analyzers are configured differently.
    // Call Qthread->start() when you are ready for the worker to start.
    AnalyzerWorker(UserSettingsPointer pConfig, int workerIdx, bool priorized);
    virtual ~AnalyzerWorker();

    //Called by the manager as a response to the waitingForNextTrack signal. and ONLY then.
    void nextTrack(TrackPointer newTrack);
    //called to pause this worker (The call is not blocking. The worker will wait on a qwaitcondition)
    void pause();
    //resumes from a previous call to pause (The call is not blocking)
    void resume();
    //Tells this worker to end. (the call is not blocking. Sets a variable for the worker to end)
    // An updateProgress signal with progress 0 will be emited and also the finished signal.
    // The AnalyzerManager connects it so that it will delete itself and the Qthread.
    void endProcess();
    // Is this a priorized worker?
    bool isPriorized();

public slots:
    //starts the analysis job.
    //Called automatically by the owning thread to start the process (Configured to do so by AnalyzerManager)
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
    bool m_priorizedJob;
    int m_workerIdx;
    SampleBuffer m_sampleBuffer;
    TrackPointer m_currentTrack;

    QAtomicInt m_exit;
    QAtomicInt m_pauseRequested;
    QMutex m_qm;
    QWaitCondition m_qwait;
    struct progress_info m_progressInfo;

};

inline bool AnalyzerWorker::isPriorized() {
    return m_priorizedJob;
}

#endif /* ANALYZER_ANALYZERWORKER_H */
