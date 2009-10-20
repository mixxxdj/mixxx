#ifndef PLAYER_H
#define PLAYER_H

#include <QtCore>

#include "configobject.h"

class EngineBuffer;
class TrackInfoObject;
class ControlObjectThreadMain;

class Player : public QObject
{
	Q_OBJECT
	public:
    Player(ConfigObject<ConfigValue> *pConfig, EngineBuffer* buffer,
           QString channel);
    ~Player();
public slots:
    void slotLoadTrack(TrackInfoObject* track, bool bStartFromEndPos=false);
    void slotFinishLoading(TrackInfoObject* pTrackInfoObject);
signals:
    void newTrackLoaded(TrackInfoObject* m_pLoadedTrack);
    void unloadingTrack(TrackInfoObject* m_pAboutToBeUnloaded);
private:
    ConfigObject<ConfigValue>* m_pConfig;
    EngineBuffer* m_pEngineBuffer;
    QString m_strChannel;

    TrackInfoObject* m_pLoadedTrack;

    ControlObjectThreadMain* m_pCuePoint;
    ControlObjectThreadMain* m_pPlayPosition;
    ControlObjectThreadMain* m_pDuration;
    ControlObjectThreadMain* m_pBPM;
};

#endif
