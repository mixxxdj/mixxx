#ifndef PLAYER_H
#define PLAYER_H

#include <QtCore>

#include "configobject.h"

class EngineMaster;
class TrackInfoObject;
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
    void slotLoadTrack(TrackInfoObject* track, bool bStartFromEndPos=false);
    void slotFinishLoading(TrackInfoObject* pTrackInfoObject);
    void slotLoadFailed(TrackInfoObject* pTrackInfoObject, QString reason);
signals:
    void loadTrack(TrackInfoObject* pTrack);
    void newTrackLoaded(TrackInfoObject* pLoadedTrack);
    void unloadingTrack(TrackInfoObject* pAboutToBeUnloaded);
private:
    ConfigObject<ConfigValue>* m_pConfig;
    int m_iPlayerNumber;
    QString m_strChannel;

    TrackInfoObject* m_pLoadedTrack;

    ControlObjectThreadMain* m_pCuePoint;
    ControlObjectThreadMain* m_pLoopInPoint;
    ControlObjectThreadMain* m_pLoopOutPoint;
    ControlObjectThreadMain* m_pPlayPosition;
    ControlObject* m_pDuration;
    ControlObjectThreadMain* m_pBPM;
};

#endif
