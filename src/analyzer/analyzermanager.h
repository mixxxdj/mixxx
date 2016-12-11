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
enum class WorkerType {
    defaultWorker,
    priorityWorker
};

public:
    //There should exist only one AnalyzerManager in order to control the amount of threads executing.
    AnalyzerManager(UserSettingsPointer pConfig);
    virtual ~AnalyzerManager();

    //Tell the analyzers of the default queue to stop. If shutdown is true. stop also the priority analyzers.
    void stop(bool shutdown);
    //Add a track to be analyzed with a priority worker. (Like those required by loading a track into a player).
    //This method might need to be protected an called only via the slotAnalyzeTrack slot.
    void analyseTrackNow(TrackPointer tio);
    //Add a track to be analyzed by the default queue.
    void queueAnalyseTrack(TrackPointer tio);
    //Check if there is any default worker or priority worker active, paused or a track in queue
    bool isActive();
    //Check if there is any default worker active, paused or track in queue
    bool isDefaultQueueActive();


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
    // This slot is called from the preferences dialog to update the max value. It will stop extra threads if running.
    void slotMaxThreadsChanged(int threads);

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
    //Indicates that the default analysis job has finished (I.e. all tracks queued on the default queue have been
    // analyzed). It is used for the UI to refresh the text and buttons.
    void queueEmpty();
private:
    //Method that creates a worker, assigns it to a new thread and the correct list, and starts
    //the thread with low priority.
    AnalyzerWorker* createNewWorker(WorkerType wtype);

    UserSettingsPointer m_pConfig;
    // Autoincremented ID to use as an identifier for each worker.
    int m_nextWorkerId;
    // Max number of threads to be active analyzing at a time including both, default and priority analysis
    int m_MaxThreads;
    // TODO: We do a "contains" over these queues before adding a new track to them.
    // The more tracks that we add to the queue, the slower this check is. 
    // No UI response is shown until all tracks are queued.
    // The processing queue for the analysis feature of the library.
    QQueue<TrackPointer> m_defaultTrackQueue;
    // The processing queue for the analysis of tracks loaded into players.
    QQueue<TrackPointer> m_prioTrackQueue;

    //List of default workers (excluding the paused ones).
    QList<AnalyzerWorker*> m_defaultWorkers;
    //List of priority workers (excluding the paused ones).
    QList<AnalyzerWorker*> m_priorityWorkers;
    //List of workers that are currently paused (priority workers are only paused if it was required from reducing the maxThreads)
    QList<AnalyzerWorker*> m_pausedWorkers;
    //This list is used mostly so that isActive() can return the correct value
    QList<AnalyzerWorker*> m_endingWorkers;
};

#endif /* ANALYZER_ANALYZERMANAGER_H */
