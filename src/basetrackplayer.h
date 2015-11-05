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
class ControlObjectThread;
class ControlObjectSlave;
class AnalyserQueue;
class EffectsManager;

class BaseTrackPlayer : public BasePlayer {
    Q_OBJECT
  public:
    BaseTrackPlayer(QObject* pParent,
                    ConfigObject<ConfigValue>* pConfig,
                    EngineMaster* pMixingEngine,
                    EffectsManager* pEffectsManager,
                    EngineChannel::ChannelOrientation defaultOrientation,
                    QString group,
                    bool defaultMaster,
                    bool defaultHeadphones);
    virtual ~BaseTrackPlayer();

    TrackPointer getLoadedTrack() const;

    // TODO(XXX): Only exposed to let the passthrough AudioInput get
    // connected. Delete me when EngineMaster supports AudioInput assigning.
    EngineDeck* getEngineDeck() const;

    void setupEqControls();

  public slots:
    void slotLoadTrack(TrackPointer track, bool bPlay=false);
    void slotFinishLoading(TrackPointer pTrackInfoObject);
    void slotLoadFailed(TrackPointer pTrackInfoObject, QString reason);
    void slotUnloadTrack(TrackPointer track);
    void slotSetReplayGain(double replayGain);
    void slotPlayToggled(double);

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

    ControlObjectThread* m_pLoopInPoint;
    ControlObjectThread* m_pLoopOutPoint;
    ControlObject* m_pDuration;
    ControlObjectThread* m_pBPM;
    ControlObjectThread* m_pKey;
    ControlObjectThread* m_pReplayGain;
    ControlObjectThread* m_pPlay;
    ControlObjectSlave* m_pLowFilter;
    ControlObjectSlave* m_pMidFilter;
    ControlObjectSlave* m_pHighFilter;
    ControlObjectSlave* m_pLowFilterKill;
    ControlObjectSlave* m_pMidFilterKill;
    ControlObjectSlave* m_pHighFilterKill;
    ControlObjectSlave* m_pPreGain;
    EngineDeck* m_pChannel;

    bool m_replaygainPending;
};


#endif // BASETRACKPLAYER_H
