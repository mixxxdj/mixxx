#pragma once

#include <gsl/pointers>
#include <memory>

#ifdef __STEM__
#include "engine/engine.h"
#endif
#include "engine/channels/enginechannel.h"
#include "mixer/baseplayer.h"
#include "preferences/colorpalettesettings.h"
#include "preferences/usersettings.h"
#include "track/replaygain.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/color/rgbcolor.h"
#include "util/parented_ptr.h"
#include "util/performancetimer.h"

class EngineMixer;
class ControlObject;
class ControlProxy;
class ControlEncoder;
class EffectsManager;
class QString;
class EngineDeck;

constexpr int kUnreplaceDelay = 500;

/// Interface for not leaking implementation details of BaseTrackPlayer into the
/// rest of Mixxx. Also makes testing a lot easier.
class BaseTrackPlayer : public BasePlayer {
    Q_OBJECT
  public:
    enum TrackLoadReset {
        RESET_NONE,
        RESET_PITCH,
        RESET_PITCH_AND_SPEED,
        RESET_SPEED
    };

    BaseTrackPlayer(PlayerManager* pParent, const QString& group);
    ~BaseTrackPlayer() override = default;

    virtual TrackPointer getLoadedTrack() const = 0;
    virtual void setupEqControls() = 0;
    virtual bool isTrackMenuControlAvailable() {
        return false;
    };

  public slots:
#ifdef __STEM__
    virtual void slotLoadTrack(TrackPointer pTrack,
            mixxx::StemChannelSelection stemMask,
            bool bPlay = false) = 0;
#else
    virtual void slotLoadTrack(TrackPointer pTrack,
            bool bPlay = false) = 0;
#endif
    virtual void slotCloneFromGroup(const QString& group) = 0;
    virtual void slotCloneDeck() = 0;
    virtual void slotEjectTrack(double) = 0;
    virtual void slotSetAndConfirmTrackMenuControl(bool){};
    virtual void slotTrackRatingChangeRequest(int){};

  signals:
    void newTrackLoaded(TrackPointer pLoadedTrack);
    void trackUnloaded(TrackPointer pUnloadedTrack);
    void loadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
#ifdef __STEM__
    void selectedStems(mixxx::StemChannelSelection stemMask);
#endif
    void playerEmpty();
    void noVinylControlInputConfigured();
    void trackRatingChanged(int rating);
    void trackMenuChangeRequest(bool show);
};

class BaseTrackPlayerImpl : public BaseTrackPlayer {
    Q_OBJECT
  public:
    BaseTrackPlayerImpl(PlayerManager* pParent,
            UserSettingsPointer pConfig,
            EngineMixer* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const ChannelHandleAndGroup& handleGroup,
            bool defaultMainMix,
            bool defaultHeadphones,
            bool primaryDeck);
    ~BaseTrackPlayerImpl() override;

    TrackPointer getLoadedTrack() const final;

    /// TODO(XXX): Only exposed to let the passthrough AudioInput get
    /// connected. Delete me when EngineMixer supports AudioInput assigning.
    EngineDeck* getEngineDeck() const;

    void setupEqControls() final;

    /// Returns true if PushButton has been created and no slot is currently
    /// connected to trackMenuChangeRequest().
    /// PushButtons persist skin reload, connected widgets don't, i.e. the
    /// connection is removed on skin reload and available again afterwards.
    bool isTrackMenuControlAvailable() final;
    /// For testing, loads a fake track.
    TrackPointer loadFakeTrack(bool bPlay, double filebpm);

  public slots:
#ifdef __STEM__
    void slotLoadTrack(TrackPointer track,
            mixxx::StemChannelSelection stemMask,
            bool bPlay) final;
#else
    void slotLoadTrack(TrackPointer track,
            bool bPlay) final;
#endif
    void slotEjectTrack(double) final;
    void slotCloneFromGroup(const QString& group) final;
    void slotCloneDeck() final;
    void slotTrackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotLoadFailed(TrackPointer pTrack, const QString& reason);
    void slotSetReplayGain(mixxx::ReplayGain replayGain);
    /// When the replaygain is adjusted, we modify the track pregain
    /// to compensate so there is no audible change in volume.
    void slotAdjustReplayGain(mixxx::ReplayGain replayGain, const QString& requestingPlayerGroup);
    void slotSetTrackColor(const mixxx::RgbColor::optional_t& color);
    void slotTrackColorSelector(int steps);

    /// Called via signal from WTrackProperty. Just set and confirm as requested.
    void slotSetAndConfirmTrackMenuControl(bool visible) final;
    /// Slot for change signals from WStarRating (absolute values)
    void slotTrackRatingChangeRequest(int rating) final;
    void slotPlayToggled(double);

  private slots:
    void slotCloneChannel(EngineChannel* pChannel);
    void slotCloneFromDeck(double deck);
    void slotCloneFromSampler(double sampler);
    void loadTrackFromGroup(const QString& group);
    void slotLoadTrackFromDeck(double deck);
    void slotLoadTrackFromSampler(double sampler);
    void slotLoadTrackFromPreviewDeck(double deck);
    void slotTrackColorChangeRequest(double value);
    /// Slot for change signals from up/down controls (relative values)
    void slotTrackRatingChangeRequestRelative(int change);
    void slotVinylControlEnabled(double v);
    void slotWaveformZoomValueChangeRequest(double pressed);
    void slotWaveformZoomUp(double pressed);
    void slotWaveformZoomDown(double pressed);
    void slotWaveformZoomSetDefault(double pressed);
    void slotShiftCuesMillis(double milliseconds);
    void slotShiftCuesMillisButton(double value, double milliseconds);
    void slotUpdateReplayGainFromPregain(double pressed);

  private:
    void setReplayGain(double value);

    void loadTrack(TrackPointer pTrack);
    TrackPointer unloadTrack();

    void connectLoadedTrack();
    void disconnectLoadedTrack();

    UserSettingsPointer m_pConfig;
    EngineMixer* m_pEngineMixer;
    TrackPointer m_pLoadedTrack;
    TrackId m_pPrevFailedTrackId;
    // non-owning reference. Owned by pMixingEngine.
    EngineDeck* m_pChannel;
    bool m_replaygainPending;
    EngineChannel* m_pChannelToCloneFrom;

    PerformanceTimer m_ejectTimer;

    std::unique_ptr<ControlPushButton> m_pEject;

    // Deck clone control
    std::unique_ptr<ControlObject> m_pCloneFromDeck;
    std::unique_ptr<ControlObject> m_pCloneFromSampler;

    // Load track from other deck/sampler
    std::unique_ptr<ControlObject> m_pLoadTrackFromDeck;
    std::unique_ptr<ControlObject> m_pLoadTrackFromSampler;
    std::unique_ptr<ControlObject> m_pLoadTrackFromPreviewDeck;

    // Track color control
    std::unique_ptr<ControlObject> m_pTrackColor;
    std::unique_ptr<ControlPushButton> m_pTrackColorPrev;
    std::unique_ptr<ControlPushButton> m_pTrackColorNext;
    std::unique_ptr<ControlEncoder> m_pTrackColorSelect;

#ifdef __STEM__
    // Stems color
    std::vector<std::unique_ptr<ControlObject>> m_pStemColors;
#endif

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
    std::unique_ptr<ControlObject> m_pVisualBpm;
    parented_ptr<ControlProxy> m_pKey;
    std::unique_ptr<ControlObject> m_pVisualKey;

    std::unique_ptr<ControlObject> m_pTimeElapsed;
    std::unique_ptr<ControlObject> m_pTimeRemaining;
    std::unique_ptr<ControlObject> m_pEndOfTrack;

    std::unique_ptr<ControlPushButton> m_pShiftCuesEarlier;
    std::unique_ptr<ControlPushButton> m_pShiftCuesEarlierSmall;
    std::unique_ptr<ControlPushButton> m_pShiftCuesLater;
    std::unique_ptr<ControlPushButton> m_pShiftCuesLaterSmall;
    std::unique_ptr<ControlObject> m_pShiftCues;

    std::unique_ptr<ControlPushButton> m_pShowTrackMenuControl;

    std::unique_ptr<ControlPushButton> m_pStarsUp;
    std::unique_ptr<ControlPushButton> m_pStarsDown;

    std::unique_ptr<ControlObject> m_pUpdateReplayGainFromPregain;

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
