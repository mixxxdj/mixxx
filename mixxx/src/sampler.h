#ifndef SAMPLER_H
#define SAMPLER_H

#include <QtCore>

#include <configobject.h>

class EngineBuffer;
class TrackInfoObject;
class ControlObjectThreadMain;

class Sampler : public QObject
{
    Q_OBJECT
    public:
    Sampler(ConfigObject<ConfigValue> *pConfig, EngineBuffer* buffer,
           QString channel);
    ~Sampler();
public slots:
    void slotLoadTrack(TrackInfoObject* track, bool bStartFromEndPos=false);
    void slotFinishLoading(TrackInfoObject* pTrackInfoObject);
    void slotLoadFailed(TrackInfoObject* pTrackInfoObject, QString reason);
signals:
    void newTrackLoaded(TrackInfoObject* m_pLoadedTrack);
    void unloadingTrack(TrackInfoObject* m_pAboutToBeUnloaded);
private:
    ConfigObject<ConfigValue>* m_pConfig;
    EngineBuffer* m_pEngineBuffer;
    QString m_strChannel;

    TrackInfoObject* m_pLoadedTrack;

    ControlObjectThreadMain * m_pCuePoint;
    ControlObjectThreadMain * m_pLoopInPoint;
    ControlObjectThreadMain* m_pLoopOutPoint;
    ControlObjectThreadMain* m_pPlayPosition;
    ControlObjectThreadMain* m_pDuration;
    ControlObjectThreadMain* m_pBPM;
};

#endif // SAMPLER_H
