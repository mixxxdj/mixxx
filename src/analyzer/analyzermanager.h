#ifndef ANALYZER_ANALYZERMANAGER_H
#define ANALYZER_ANALYZERMANAGER_H

#include <QList>
#include <QQueue>

#include <vector>

#include "analyzer/analyzerworker.h"
#include "preferences/usersettings.h"
#include "track/track.h"

class TrackCollection;

class AnalyzerManager : public QObject {
    Q_OBJECT

protected:
    AnalyzerManager(UserSettingsPointer pConfig);

public:
    static AnalyzerManager& getInstance(UserSettingsPointer pConfig);
    virtual ~AnalyzerManager();

    void stop(bool shutdown);
    //This method might need to be protected an called only via slot.
    void analyseTrackNow(TrackPointer tio);
    void queueAnalyseTrack(TrackPointer tio);
    bool isActive(bool includeForeground);


public slots:
    // This slot is called from the decks and samplers when the track is loaded.
    void slotAnalyseTrack(TrackPointer tio);
    void slotUpdateProgress(int, struct AnalyzerWorker::progress_info*);
    void slotNextTrack(AnalyzerWorker*);
    void slotPaused(AnalyzerWorker*);
    void slotWorkerFinished(AnalyzerWorker*);
    void slotErrorString(QString);

signals:
    void trackProgress(int worker, int progress);
    void trackDone(TrackPointer track);
    void trackFinished(int size);
    void queueEmpty();
private:

    AnalyzerWorker* createNewWorker(bool batchJob);
    static AnalyzerManager* m_pAnalyzerManager;
    UserSettingsPointer m_pConfig;
    int m_nextWorkerId;
    int m_MaxThreads;
    // The processing queue and associated mutex
    QQueue<TrackPointer> m_batchTrackQueue;
    QQueue<TrackPointer> m_prioTrackQueue;

    QList<AnalyzerWorker*> m_backgroundWorkers;
    QList<AnalyzerWorker*> m_foregroundWorkers;
    QList<AnalyzerWorker*> m_pausedWorkers;
    //This list is used mostly so that isActive() can return the correct value
    QList<AnalyzerWorker*> m_endingWorkers;
};

#endif /* ANALYZER_ANALYZERMANAGER_H */
