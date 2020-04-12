#ifndef MIXER_BASETRACKPLAYER_H
#define MIXER_BASETRACKPLAYER_H

#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "preferences/usersettings.h"
#include "engine/channels/enginechannel.h"
#include "engine/channels/enginedeck.h"
#include "mixer/baseplayer.h"
#include "track/track.h"
#include "util/memory.h"

class EngineMaster;
class ControlObject;
class ControlPotmeter;
class ControlProxy;
class EffectsManager;
class VisualsManager;

// Interface for not leaking implementation details of BaseTrackPlayer into the
// rest of Mixxx. Also makes testing a lot easier.
class BaseTrackPlayer : public BasePlayer {
    Q_OBJECT
  public:
    enum TrackLoadReset {
        RESET_NONE,
        RESET_PITCH,
        RESET_PITCH_AND_SPEED,
        RESET_SPEED
    };

    BaseTrackPlayer(QObject* pParent, const QString& group);
    virtual ~BaseTrackPlayer() {}

    virtual TrackPointer getLoadedTrack() const = 0;

  public slots:
    virtual void slotLoadTrack(TrackPointer pTrack, bool bPlay = false) = 0;
    virtual void slotCloneFromGroup(const QString& group) = 0;
    virtual void slotCloneDeck() = 0;

  signals:
    void newTrackLoaded(TrackPointer pLoadedTrack);
    void loadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void playerEmpty();
    void noPassthroughInputConfigured();
    void noVinylControlInputConfigured();
};

class BaseTrackPlayerImpl : public BaseTrackPlayer {
    Q_OBJECT
  public:
    BaseTrackPlayerImpl(QObject* pParent,
                        UserSettingsPointer pConfig,
                        EngineMaster* pMixingEngine,
                        EffectsManager* pEffectsManager,
                        VisualsManager* pVisualsManager,
                        EngineChannel::ChannelOrientation defaultOrientation,
                        const QString& group,
                        bool defaultMaster,
                        bool defaultHeadphones);
    virtual ~BaseTrackPlayerImpl();

    TrackPointer getLoadedTrack() const final;

    // TODO(XXX): Only exposed to let the passthrough AudioInput get
    // connected. Delete me when EngineMaster supports AudioInput assigning.
    EngineDeck* getEngineDeck() const;

    void setupEqControls();

    // For testing, loads a fake track.
    TrackPointer loadFakeTrack(bool bPlay, double filebpm);

  public slots:
    void slotLoadTrack(TrackPointer track, bool bPlay) final;
    void slotCloneFromGroup(const QString& group) final;
    void slotCloneDeck() final;
    void slotTrackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotLoadFailed(TrackPointer pTrack, QString reason);
    void slotSetReplayGain(mixxx::ReplayGain replayGain);
    void slotPlayToggled(double);

  private slots:
    void slotCloneChannel(EngineChannel* pChannel);
    void slotCloneFromDeck(double deck);
    void slotPassthroughEnabled(double v);
    void slotVinylControlEnabled(double v);
    void slotWaveformZoomValueChangeRequest(double pressed);
    void slotWaveformZoomUp(double pressed);
    void slotWaveformZoomDown(double pressed);
    void slotWaveformZoomSetDefault(double pressed);

  private:
    void setReplayGain(double value);

    void loadTrack(TrackPointer pTrack);
    TrackPointer unloadTrack();

    void connectLoadedTrack();
    void disconnectLoadedTrack();

    UserSettingsPointer m_pConfig;
    EngineMaster* m_pEngineMaster;
    TrackPointer m_pLoadedTrack;
    EngineDeck* m_pChannel;
    bool m_replaygainPending;
    EngineChannel* m_pChannelToCloneFrom;

    // Deck clone control
    std::unique_ptr<ControlObject> m_pCloneFromDeck;

    // Waveform display related controls
    std::unique_ptr<ControlObject> m_pWaveformZoom;
    std::unique_ptr<ControlPushButton> m_pWaveformZoomUp;
    std::unique_ptr<ControlPushButton> m_pWaveformZoomDown;
    std::unique_ptr<ControlPushButton> m_pWaveformZoomSetDefault;


    std::unique_ptr<ControlProxy> m_pLoopInPoint;
    std::unique_ptr<ControlProxy> m_pLoopOutPoint;
    std::unique_ptr<ControlObject> m_pDuration;

    // TODO() these COs are reconnected during runtime
    // This may lock the engine
    std::unique_ptr<ControlProxy> m_pFileBPM;
    std::unique_ptr<ControlProxy> m_pKey;

    std::unique_ptr<ControlProxy> m_pReplayGain;
    std::unique_ptr<ControlProxy> m_pPlay;
    std::unique_ptr<ControlProxy> m_pLowFilter;
    std::unique_ptr<ControlProxy> m_pMidFilter;
    std::unique_ptr<ControlProxy> m_pHighFilter;
    std::unique_ptr<ControlProxy> m_pLowFilterKill;
    std::unique_ptr<ControlProxy> m_pMidFilterKill;
    std::unique_ptr<ControlProxy> m_pHighFilterKill;
    std::unique_ptr<ControlProxy> m_pPreGain;
    std::unique_ptr<ControlProxy> m_pRateSlider;
    std::unique_ptr<ControlProxy> m_pPitchAdjust;
    std::unique_ptr<ControlProxy> m_pInputConfigured;
    std::unique_ptr<ControlProxy> m_pPassthroughEnabled;
    std::unique_ptr<ControlProxy> m_pVinylControlEnabled;
    std::unique_ptr<ControlProxy> m_pVinylControlStatus;
};

#endif // MIXER_BASETRACKPLAYER_H
