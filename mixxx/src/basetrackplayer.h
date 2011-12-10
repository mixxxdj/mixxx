#ifndef BASETRACKPLAYER_H
#define BASETRACKPLAYER_H

#include "configobject.h"
#include "trackinfoobject.h"
#include "baseplayer.h"
#include "analyserqueue.h"
#include "engine/enginechannel.h"

class EngineMaster;
class ControlObject;
class ControlObjectThreadMain;
class WaveformRenderer;

class BaseTrackPlayer : public BasePlayer {
    Q_OBJECT
  public:
    BaseTrackPlayer(QObject* pParent,
                    ConfigObject<ConfigValue>* pConfig,
                    EngineMaster* pMixingEngine,
                    EngineChannel::ChannelOrientation defaultOrientation,
                    AnalyserQueue* pAnalyserQueue,
                    QString group);
    virtual ~BaseTrackPlayer();

    AnalyserQueue* getAnalyserQueue() const;
    WaveformRenderer* getWaveformRenderer() const;
    TrackPointer getLoadedTrack() const;

  public slots:
    void slotLoadTrack(TrackPointer track, bool bStartFromEndPos=false);
    void slotFinishLoading(TrackPointer pTrackInfoObject);
    void slotLoadFailed(TrackPointer pTrackInfoObject, QString reason);
    void slotUnloadTrack(TrackPointer track);

  signals:
    void loadTrack(TrackPointer pTrack);
    void newTrackLoaded(TrackPointer pLoadedTrack);
    void unloadingTrack(TrackPointer pAboutToBeUnloaded);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackPointer m_pLoadedTrack;
    AnalyserQueue* m_pAnalyserQueue;

    ControlObjectThreadMain* m_pCuePoint;
    ControlObjectThreadMain* m_pLoopInPoint;
    ControlObjectThreadMain* m_pLoopOutPoint;
    ControlObjectThreadMain* m_pPlayPosition;
    ControlObject* m_pDuration;
    ControlObjectThreadMain* m_pBPM;
    ControlObjectThreadMain* m_pReplayGain;
    WaveformRenderer* m_pWaveformRenderer;
};


#endif /* BASETRACKPLAYER_H */
