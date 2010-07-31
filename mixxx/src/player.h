#ifndef PLAYER_H
#define PLAYER_H

#include <QtCore>

#include "configobject.h"
#include "trackinfoobject.h"

class EngineBuffer;
class ControlObjectThreadMain;

class Player : public QObject
{
	Q_OBJECT
	public:
    Player(ConfigObject<ConfigValue> *pConfig, EngineBuffer* buffer,
           QString channel);
    ~Player();
public slots:
    void slotLoadTrack(TrackPointer track, bool bStartFromEndPos=false);
    void slotFinishLoading(TrackPointer pTrackInfoObject);
    void slotLoadFailed(TrackPointer pTrackInfoObject, QString reason);
signals:
    void newTrackLoaded(TrackPointer m_pLoadedTrack);
    void unloadingTrack(TrackPointer m_pAboutToBeUnloaded);
private:
    ConfigObject<ConfigValue>* m_pConfig;
    EngineBuffer* m_pEngineBuffer;
    QString m_strChannel;

    TrackPointer m_pLoadedTrack;

    ControlObjectThreadMain* m_pCuePoint;
    ControlObjectThreadMain* m_pLoopInPoint;
    ControlObjectThreadMain* m_pLoopOutPoint;
    ControlObjectThreadMain* m_pPlayPosition;
    ControlObjectThreadMain* m_pDuration;
    ControlObjectThreadMain* m_pBPM;
};

#endif
