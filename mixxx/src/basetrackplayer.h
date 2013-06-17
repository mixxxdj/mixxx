#ifndef BASETRACKPLAYER_H
#define BASETRACKPLAYER_H

#include "configobject.h"
#include "trackinfoobject.h"
#include "baseplayer.h"
#include "engine/enginechannel.h"
#include "engine/enginedeck.h"

class EngineMaster;
class ControlObject;
class ControlPotmeter;
class ControlObjectThreadMain;
class AnalyserQueue;

class BaseTrackPlayer : public BasePlayer {
    Q_OBJECT
  public:
    BaseTrackPlayer(QObject* pParent,
                    ConfigObject<ConfigValue>* pConfig,
                    EngineMaster* pMixingEngine,
                    EngineChannel::ChannelOrientation defaultOrientation,
                    QString group,
                    bool defaultMaster,
                    bool defaultHeadphones);
    virtual ~BaseTrackPlayer();

    TrackPointer getLoadedTrack() const;

    // TODO(XXX): Only exposed to let the passthrough AudioInput get
    // connected. Delete me when EngineMaster supports AudioInput assigning.
    EngineDeck* getEngineDeck() const;

  public slots:
    void slotLoadTrack(TrackPointer track, bool bPlay=false);
    void slotFinishLoading(TrackPointer pTrackInfoObject);
    void slotLoadFailed(TrackPointer pTrackInfoObject, QString reason);
    void slotUnloadTrack(TrackPointer track);
    void slotSetReplayGain(double replayGain);

  signals:
    void loadTrack(TrackPointer pTrack, bool bPlay=false);
    void loadTrackFailed(TrackPointer pTrack);
    void newTrackLoaded(TrackPointer pLoadedTrack);
    void unloadingTrack(TrackPointer pAboutToBeUnloaded);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackPointer m_pLoadedTrack;

    // Waveform display related controls
    ControlPotmeter* m_pWaveformZoom;
    ControlObject* m_pEndOfTrack;

    ControlObjectThreadMain* m_pCuePoint;
    ControlObjectThreadMain* m_pLoopInPoint;
    ControlObjectThreadMain* m_pLoopOutPoint;
    ControlObjectThreadMain* m_pPlayPosition;
    ControlObject* m_pDuration;
    ControlObjectThreadMain* m_pBPM;
    ControlObjectThreadMain* m_pReplayGain;
    ControlObjectThreadMain* m_pPlay;
    EngineDeck* m_pChannel;
};


#endif // BASETRACKPLAYER_H
