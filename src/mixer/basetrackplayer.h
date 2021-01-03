#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "engine/channels/enginechannel.h"
#include "engine/channels/enginedeck.h"
#include "mixer/baseplayer.h"
#include "preferences/usersettings.h"
#include "track/replaygain.h"
#include "track/track_decl.h"
#include "util/color/rgbcolor.h"
#include "util/memory.h"
#include "util/parented_ptr.h"

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
    ~BaseTrackPlayer() override = default;

    virtual TrackPointer getLoadedTrack() const = 0;

  public slots:
    virtual void slotLoadTrack(TrackPointer pTrack, bool bPlay = false) = 0;
    virtual void slotCloneFromGroup(const QString& group) = 0;
    virtual void slotCloneDeck() = 0;

  signals:
    void newTrackLoaded(TrackPointer pLoadedTrack);
    void loadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void playerEmpty();
    void noVinylControlInputConfigured();
};

class BaseTrackPlayerImpl : public BaseTrackPlayer {
    Q_OBJECT
  public:
    BaseTrackPlayerImpl(QObject* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const ChannelHandleAndGroup& handleGroup,
            bool defaultMaster,
            bool defaultHeadphones,
            bool primaryDeck);
    ~BaseTrackPlayerImpl() override;

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
    void slotLoadFailed(TrackPointer pTrack, const QString& reason);
    void slotSetReplayGain(mixxx::ReplayGain replayGain);
    void slotSetTrackColor(const mixxx::RgbColor::optional_t& color);
    void slotPlayToggled(double);

  private slots:
    void slotCloneChannel(EngineChannel* pChannel);
    void slotCloneFromDeck(double deck);
    void slotCloneFromSampler(double sampler);
    void slotTrackColorChangeRequest(double value);
    void slotVinylControlEnabled(double v);
    void slotWaveformZoomValueChangeRequest(double pressed);
    void slotWaveformZoomUp(double pressed);
    void slotWaveformZoomDown(double pressed);
    void slotWaveformZoomSetDefault(double pressed);
    void slotShiftCuesMillis(double milliseconds);
    void slotShiftCuesMillisButton(double value, double milliseconds);

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
    std::unique_ptr<ControlObject> m_pCloneFromSampler;

    // Track color control
    std::unique_ptr<ControlObject> m_pTrackColor;

    // Waveform display related controls
    std::unique_ptr<ControlObject> m_pWaveformZoom;
    std::unique_ptr<ControlPushButton> m_pWaveformZoomUp;
    std::unique_ptr<ControlPushButton> m_pWaveformZoomDown;
    std::unique_ptr<ControlPushButton> m_pWaveformZoomSetDefault;

    parented_ptr<ControlProxy> m_pLoopInPoint;
    parented_ptr<ControlProxy> m_pLoopOutPoint;
    std::unique_ptr<ControlObject> m_pDuration;

    // TODO() these COs are reconnected during runtime
    // This may lock the engine
    std::unique_ptr<ControlObject> m_pFileBPM;
    parented_ptr<ControlProxy> m_pKey;

    std::unique_ptr<ControlPushButton> m_pShiftCuesEarlier;
    std::unique_ptr<ControlPushButton> m_pShiftCuesEarlierSmall;
    std::unique_ptr<ControlPushButton> m_pShiftCuesLater;
    std::unique_ptr<ControlPushButton> m_pShiftCuesLaterSmall;
    std::unique_ptr<ControlObject> m_pShiftCues;

    parented_ptr<ControlProxy> m_pReplayGain;
    parented_ptr<ControlProxy> m_pPlay;
    parented_ptr<ControlProxy> m_pLowFilter;
    parented_ptr<ControlProxy> m_pMidFilter;
    parented_ptr<ControlProxy> m_pHighFilter;
    parented_ptr<ControlProxy> m_pLowFilterKill;
    parented_ptr<ControlProxy> m_pMidFilterKill;
    parented_ptr<ControlProxy> m_pHighFilterKill;
    parented_ptr<ControlProxy> m_pPreGain;
    parented_ptr<ControlProxy> m_pRateRatio;
    parented_ptr<ControlProxy> m_pPitchAdjust;
    parented_ptr<ControlProxy> m_pInputConfigured;
    parented_ptr<ControlProxy> m_pVinylControlEnabled;
    parented_ptr<ControlProxy> m_pVinylControlStatus;
};
