#ifndef ANALYZER_ANALYZERMANAGER_H
#define ANALYZER_ANALYZERMANAGER_H

#include <QList>
#include <QQueue>

#include <vector>

#include "analyzer/analyzerworker.h"
#include "preferences/usersettings.h"
#include "track/track.h"

class TrackCollection;

/* AnalyzerManager class
* Manages the task of analying multiple tracks. Setups a maximum amount of workers
* and provides the tracks to analyze. It also sends the signals to other parts of Mixxx.
*/
class AnalyzerManager : public QObject {
    Q_OBJECT

protected:
    AnalyzerManager(UserSettingsPointer pConfig);

public:
    //Get the only instance of the manager: 
    // TODO: maybe move it to an instance in MixxxMainWindow.
    static AnalyzerManager& getInstance(UserSettingsPointer pConfig);
    virtual ~AnalyzerManager();

    //Tell the background analysis to stop. If shutdown is true. stop also the foreground analysis.
    void stop(bool shutdown);
    //This method might need to be protected an called only via slot.
    void analyseTrackNow(TrackPointer tio);
    //Add a track to be analyzed by the background analyzer.
    void queueAnalyseTrack(TrackPointer tio);
    //Check if there is any background worker active, paused or track in queue
    //If includeForeground is true, it also checks the foreground workers.
    bool isActive(bool includeForeground);


public slots:
    // This slot is called from the decks and samplers when the track is loaded.
    void slotAnalyseTrack(TrackPointer tio);
    // This slot is called from the workers to indicate progress.
    void slotUpdateProgress(int, struct AnalyzerWorker::progress_info*);
    // This slot is called from the workers when they need a new track to process.
    void slotNextTrack(AnalyzerWorker*);
    // This slot is called from the workers to inform that they reached the paused status.
    void slotPaused(AnalyzerWorker*);
    // This slot is called from the workers to inform that they are ending themselves.
    void slotWorkerFinished(AnalyzerWorker*);
    // This slot is intended to receive textual messages. It us unused right now.
    void slotErrorString(QString);

signals:
    //This signal is emited to inform other UI elements about the analysis progress.
    //Note: the foreground analyzers don't listen to this, but instead they listen to 
    //a signal emited by the track object itself.
    void trackProgress(int worker, int progress);
    //This signal is emited to indicate that such track is done. Not really used right now.
    void trackDone(TrackPointer track);
    //This sitnal is emited to indicate that a track has finished. The size indicates how many
    //tracks remain to be scanned. It is currently used by the AnalysisFeature and DlgAnalisys
    //to show the track progression.
    void trackFinished(int size);
    //Indicates that the background analysis job has finished (I.e. all background tracks have been
    // analyzed). It is used for the UI to refresh the text and buttons.
    void queueEmpty();
private:
    //Method that creates a worker, assigns it to a new thread and the correct list, and starts
    //the thread with low priority.
    AnalyzerWorker* createNewWorker(bool batchJob);

    static AnalyzerManager* m_pAnalyzerManager;
    UserSettingsPointer m_pConfig;
    // Autoincremented ID to use as an identifier for each worker.
    int m_nextWorkerId;
    // Max number of threads to be active analyzing at a time including both, background and foreground analysis
    int m_MaxThreads;
    // TODO: We do a "contains" over these queues before adding a new track to them.
    // The more tracks that we add to the queue, the slower this check is. 
    // No UI response is shown until all tracks are queued.
    // The processing queue for batch analysis
    QQueue<TrackPointer> m_batchTrackQueue;
    // The processing queue for the player analysis.
    QQueue<TrackPointer> m_prioTrackQueue;

    //List of background workers (excluding the paused ones).
    QList<AnalyzerWorker*> m_backgroundWorkers;
    //List of foreground workers (foreground workers are not paused)
    QList<AnalyzerWorker*> m_foregroundWorkers;
    //List of background workers that are currently paused
    QList<AnalyzerWorker*> m_pausedWorkers;
    //This list is used mostly so that isActive() can return the correct value
    QList<AnalyzerWorker*> m_endingWorkers;
};

#endif /* ANALYZER_ANALYZERMANAGER_H */
