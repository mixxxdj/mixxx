#ifndef MIXER_BASETRACKPLAYER_H
#define MIXER_BASETRACKPLAYER_H

#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "preferences/usersettings.h"
#include "engine/enginechannel.h"
#include "engine/enginedeck.h"
#include "mixer/baseplayer.h"
#include "trackinfoobject.h"

class EngineMaster;
class ControlObject;
class ControlPotmeter;
class ControlObjectSlave;
class EffectsManager;

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
    virtual void slotLoadTrack(TrackPointer pTrack, bool bPlay=false) = 0;

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
                        EngineChannel::ChannelOrientation defaultOrientation,
                        const QString& group,
                        bool defaultMaster,
                        bool defaultHeadphones);
    virtual ~BaseTrackPlayerImpl();

    TrackPointer getLoadedTrack() const;

    // TODO(XXX): Only exposed to let the passthrough AudioInput get
    // connected. Delete me when EngineMaster supports AudioInput assigning.
    EngineDeck* getEngineDeck() const;

    void setupEqControls();

  public slots:
    void slotLoadTrack(TrackPointer track, bool bPlay) override;
    void slotTrackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotLoadFailed(TrackPointer pTrackInfoObject, QString reason);
    void slotSetReplayGain(Mixxx::ReplayGain replayGain);
    void slotPlayToggled(double);

  private slots:
    void slotPassthroughEnabled(double v);
    void slotVinylControlEnabled(double v);

  private:
    void setReplayGain(double value);

    UserSettingsPointer m_pConfig;
    TrackPointer m_pLoadedTrack;

    // Waveform display related controls
    ControlPotmeter* m_pWaveformZoom;
    ControlObject* m_pEndOfTrack;

    ControlObjectSlave* m_pLoopInPoint;
    ControlObjectSlave* m_pLoopOutPoint;
    ControlObject* m_pDuration;

    // TODO() these COs are reconnected during runtime
    // This may lock the engine
    ControlObjectSlave* m_pBPM;
    ControlObjectSlave* m_pKey;

    ControlObjectSlave* m_pReplayGain;
    ControlObjectSlave* m_pPlay;
    ControlObjectSlave* m_pLowFilter;
    ControlObjectSlave* m_pMidFilter;
    ControlObjectSlave* m_pHighFilter;
    ControlObjectSlave* m_pLowFilterKill;
    ControlObjectSlave* m_pMidFilterKill;
    ControlObjectSlave* m_pHighFilterKill;
    ControlObjectSlave* m_pPreGain;
    ControlObjectSlave* m_pRateSlider;
    ControlObjectSlave* m_pPitchAdjust;
    QScopedPointer<ControlObjectSlave> m_pInputConfigured;
    QScopedPointer<ControlObjectSlave> m_pPassthroughEnabled;
    QScopedPointer<ControlObjectSlave> m_pVinylControlEnabled;
    QScopedPointer<ControlObjectSlave> m_pVinylControlStatus;
    EngineDeck* m_pChannel;

    bool m_replaygainPending;
};

#endif // MIXER_BASETRACKPLAYER_H
