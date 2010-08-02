#ifndef PLAYER_H
#define PLAYER_H

#include <QtCore>

#include "configobject.h"
#include "trackinfoobject.h"

class EngineMaster;
class ControlObjectThreadMain;
class ControlObject;

class Player : public QObject
{
	Q_OBJECT
	public:
    Player(ConfigObject<ConfigValue> *pConfig, EngineMaster* pMixingEngine,
           int playerNumber, const char* pGroup);
    ~Player();
    QString getGroup();
public slots:
    void slotLoadTrack(TrackPointer track, bool bStartFromEndPos=false);
    void slotFinishLoading(TrackPointer pTrackInfoObject);
    void slotLoadFailed(TrackPointer pTrackInfoObject, QString reason);
signals:
    void loadTrack(TrackPointer pTrack);
    void newTrackLoaded(TrackPointer pLoadedTrack);
    void unloadingTrack(TrackPointer pAboutToBeUnloaded);
private:
    ConfigObject<ConfigValue>* m_pConfig;
    int m_iPlayerNumber;
    QString m_strChannel;

    TrackPointer m_pLoadedTrack;

    ControlObjectThreadMain* m_pCuePoint;
    ControlObjectThreadMain* m_pLoopInPoint;
    ControlObjectThreadMain* m_pLoopOutPoint;
    ControlObjectThreadMain* m_pPlayPosition;
    ControlObject* m_pDuration;
    ControlObjectThreadMain* m_pBPM;
};

#endif
