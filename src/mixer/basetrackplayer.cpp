#include "mixer/basetrackplayer.h"

#include <QMessageBox>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "effects/effectsmanager.h"
#include "engine/channels/enginedeck.h"
#include "engine/controls/enginecontrol.h"
#include "engine/engine.h"
#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "engine/sync/enginesync.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "sources/soundsourceproxy.h"
#include "track/beatgrid.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/platform.h"
#include "util/sandbox.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/visualsmanager.h"

namespace {

const double kNoTrackColor = -1;
const double kShiftCuesOffsetMillis = 10;
const double kShiftCuesOffsetSmallMillis = 1;

inline double trackColorToDouble(mixxx::RgbColor::optional_t color) {
    return (color ? static_cast<double>(*color) : kNoTrackColor);
}
}

BaseTrackPlayer::BaseTrackPlayer(QObject* pParent, const QString& group)
        : BasePlayer(pParent, group) {
}

BaseTrackPlayerImpl::BaseTrackPlayerImpl(QObject* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        VisualsManager* pVisualsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const QString& group,
        bool defaultMaster,
        bool defaultHeadphones,
        bool primaryDeck)
        : BaseTrackPlayer(pParent, group),
          m_pConfig(pConfig),
          m_pEngineMaster(pMixingEngine),
          m_pLoadedTrack(),
          m_replaygainPending(false),
          m_pChannelToCloneFrom(nullptr) {
    ChannelHandleAndGroup channelGroup =
            pMixingEngine->registerChannelGroup(group);
    m_pChannel = new EngineDeck(channelGroup, pConfig, pMixingEngine, pEffectsManager, defaultOrientation, primaryDeck);

    m_pInputConfigured = make_parented<ControlProxy>(group, "input_configured", this);
#ifdef __VINYLCONTROL__
    m_pVinylControlEnabled = make_parented<ControlProxy>(group, "vinylcontrol_enabled", this);
    m_pVinylControlEnabled->connectValueChanged(this, &BaseTrackPlayerImpl::slotVinylControlEnabled);
    m_pVinylControlStatus = make_parented<ControlProxy>(group, "vinylcontrol_status", this);
#endif

    EngineBuffer* pEngineBuffer = m_pChannel->getEngineBuffer();
    pMixingEngine->addChannel(m_pChannel);

    // Set the routing option defaults for the master and headphone mixes.
    m_pChannel->setMaster(defaultMaster);
    m_pChannel->setPfl(defaultHeadphones);

    // Connect our signals and slots with the EngineBuffer's signals and
    // slots. This will let us know when the reader is done loading a track, and
    // let us request that the reader load a track.
    connect(pEngineBuffer, SIGNAL(trackLoaded(TrackPointer, TrackPointer)),
            this, SLOT(slotTrackLoaded(TrackPointer, TrackPointer)));
    connect(pEngineBuffer, SIGNAL(trackLoadFailed(TrackPointer, QString)),
            this, SLOT(slotLoadFailed(TrackPointer, QString)));

    // Get loop point control objects
    m_pLoopInPoint = make_parented<ControlProxy>(
            getGroup(), "loop_start_position", this);
    m_pLoopOutPoint = make_parented<ControlProxy>(
            getGroup(), "loop_end_position", this);

    // Duration of the current song, we create this one because nothing else does.
    m_pDuration = std::make_unique<ControlObject>(
        ConfigKey(getGroup(), "duration"));

    // Track color of the current track
    m_pTrackColor = std::make_unique<ControlObject>(
            ConfigKey(getGroup(), "track_color"));

    m_pTrackColor->set(kNoTrackColor);
    m_pTrackColor->connectValueChangeRequest(
            this, &BaseTrackPlayerImpl::slotTrackColorChangeRequest);

    // Deck cloning
    m_pCloneFromDeck = std::make_unique<ControlObject>(
            ConfigKey(getGroup(), "CloneFromDeck"),
            false);
    connect(m_pCloneFromDeck.get(),
            &ControlObject::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotCloneFromDeck);

    // Sampler cloning
    m_pCloneFromSampler = std::make_unique<ControlObject>(
            ConfigKey(getGroup(), "CloneFromSampler"),
            false);
    connect(m_pCloneFromSampler.get(),
            &ControlObject::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotCloneFromSampler);

    // Waveform controls
    // This acts somewhat like a ControlPotmeter, but the normal _up/_down methods
    // do not work properly with this CO.
    m_pWaveformZoom =
            std::make_unique<ControlObject>(ConfigKey(group, "waveform_zoom"));
    m_pWaveformZoom->connectValueChangeRequest(this,
            &BaseTrackPlayerImpl::slotWaveformZoomValueChangeRequest,
            Qt::DirectConnection);
    m_pWaveformZoom->set(1.0);
    m_pWaveformZoomUp = std::make_unique<ControlPushButton>(
            ConfigKey(group, "waveform_zoom_up"));
    connect(m_pWaveformZoomUp.get(),
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotWaveformZoomUp(double)));
    m_pWaveformZoomDown = std::make_unique<ControlPushButton>(
            ConfigKey(group, "waveform_zoom_down"));
    connect(m_pWaveformZoomDown.get(),
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotWaveformZoomDown(double)));
    m_pWaveformZoomSetDefault = std::make_unique<ControlPushButton>(
            ConfigKey(group, "waveform_zoom_set_default"));
    connect(m_pWaveformZoomSetDefault.get(),
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotWaveformZoomSetDefault(double)));

    m_pPreGain = make_parented<ControlProxy>(group, "pregain", this);

    m_pShiftCuesEarlier = std::make_unique<ControlPushButton>(
            ConfigKey(group, "shift_cues_earlier"));
    connect(m_pShiftCuesEarlier.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) { slotShiftCuesMillisButton(value, -kShiftCuesOffsetMillis); });
    m_pShiftCuesLater = std::make_unique<ControlPushButton>(
            ConfigKey(group, "shift_cues_later"));
    connect(m_pShiftCuesLater.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) { slotShiftCuesMillisButton(value, kShiftCuesOffsetMillis); });
    m_pShiftCuesEarlierSmall = std::make_unique<ControlPushButton>(
            ConfigKey(group, "shift_cues_earlier_small"));
    connect(m_pShiftCuesEarlierSmall.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) {
                slotShiftCuesMillisButton(value, -kShiftCuesOffsetSmallMillis);
            });
    m_pShiftCuesLaterSmall = std::make_unique<ControlPushButton>(
            ConfigKey(group, "shift_cues_later_small"));
    connect(m_pShiftCuesLaterSmall.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) {
                slotShiftCuesMillisButton(value, kShiftCuesOffsetSmallMillis);
            });
    m_pShiftCues = std::make_unique<ControlObject>(
            ConfigKey(group, "shift_cues"));
    connect(m_pShiftCues.get(),
            &ControlObject::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotShiftCuesMillis);

    // BPM of the current song
    m_pFileBPM = std::make_unique<ControlObject>(ConfigKey(group, "file_bpm"));
    m_pKey = make_parented<ControlProxy>(group, "file_key", this);

    m_pReplayGain = make_parented<ControlProxy>(group, "replaygain", this);
    m_pPlay = make_parented<ControlProxy>(group, "play", this);
    m_pPlay->connectValueChanged(this, &BaseTrackPlayerImpl::slotPlayToggled);

    m_pRateRatio = make_parented<ControlProxy>(group, "rate_ratio", this);
    m_pPitchAdjust = make_parented<ControlProxy>(group, "pitch_adjust", this);

    pVisualsManager->addDeck(group);
}

BaseTrackPlayerImpl::~BaseTrackPlayerImpl() {
    unloadTrack();
}

TrackPointer BaseTrackPlayerImpl::loadFakeTrack(bool bPlay, double filebpm) {
    TrackPointer pTrack(Track::newTemporary());
    pTrack->setAudioProperties(
            mixxx::kEngineChannelCount,
            mixxx::audio::SampleRate(44100),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(10));
    if (filebpm > 0) {
        pTrack->setBpm(filebpm);
    }

    TrackPointer pOldTrack = m_pLoadedTrack;
    m_pLoadedTrack = pTrack;
    if (m_pLoadedTrack) {
        // Listen for updates to the file's BPM
        connect(m_pLoadedTrack.get(),
                &Track::bpmUpdated,
                m_pFileBPM.get(),
                QOverload<double>::of(&ControlObject::set));

        connect(m_pLoadedTrack.get(),
                &Track::keyUpdated,
                m_pKey.get(),
                &ControlProxy::set);

        // Listen for updates to the file's Replay Gain
        connect(m_pLoadedTrack.get(),
                &Track::replayGainUpdated,
                this,
                &BaseTrackPlayerImpl::slotSetReplayGain);

        connect(m_pLoadedTrack.get(),
                &Track::colorUpdated,
                this,
                &BaseTrackPlayerImpl::slotSetTrackColor);
    }

    // Request a new track from EngineBuffer
    EngineBuffer* pEngineBuffer = m_pChannel->getEngineBuffer();
    pEngineBuffer->loadFakeTrack(pTrack, bPlay);

    // await slotTrackLoaded()/slotLoadFailed()
    emit loadingTrack(pTrack, pOldTrack);

    return pTrack;
}

void BaseTrackPlayerImpl::loadTrack(TrackPointer pTrack) {
    DEBUG_ASSERT(!m_pLoadedTrack);

    m_pLoadedTrack = std::move(pTrack);
    if (!m_pLoadedTrack) {
        // nothing to
        return;
    }

    // Clear loop
    // It seems that the trick is to first clear the loop out point, and then
    // the loop in point. If we first clear the loop in point, the loop out point
    // does not get cleared.
    m_pLoopOutPoint->set(kNoTrigger);
    m_pLoopInPoint->set(kNoTrigger);

    // The loop in and out points must be set here and not in slotTrackLoaded
    // so LoopingControl::trackLoaded can access them.
    if (!m_pChannelToCloneFrom) {
        const QList<CuePointer> trackCues(m_pLoadedTrack->getCuePoints());
        QListIterator<CuePointer> it(trackCues);
        CuePointer pLoopCue;
        // Restore loop from the first loop cue with minimum hotcue number.
        // For the volatile "most recent loop" the hotcue number will be -1.
        // If no such loop exists, restore a saved loop cue.
        while (it.hasNext()) {
            CuePointer pCue(it.next());
            if (pCue->getType() != mixxx::CueType::Loop) {
                continue;
            }

            if (pLoopCue && pLoopCue->getHotCue() <= pCue->getHotCue()) {
                continue;
            }

            pLoopCue = pCue;
        }

        if (pLoopCue) {
            double loopStart = pLoopCue->getPosition();
            double loopEnd = loopStart + pLoopCue->getLength();
            if (loopStart != kNoTrigger && loopEnd != kNoTrigger && loopStart <= loopEnd) {
                m_pLoopInPoint->set(loopStart);
                m_pLoopOutPoint->set(loopEnd);
            }
        }
    } else {
        // copy loop in and out points from other deck because any new loops
        // won't be saved yet
        m_pLoopInPoint->set(ControlObject::get(
                ConfigKey(m_pChannelToCloneFrom->getGroup(), "loop_start_position")));
        m_pLoopOutPoint->set(ControlObject::get(
                ConfigKey(m_pChannelToCloneFrom->getGroup(), "loop_end_position")));
    }

    connectLoadedTrack();
}

TrackPointer BaseTrackPlayerImpl::unloadTrack() {
    if (!m_pLoadedTrack) {
        // nothing to do
        return TrackPointer();
    }

    // Save the loops that are currently set in a loop cue. If no loop cue is
    // currently on the track, then create a new one.
    double loopStart = m_pLoopInPoint->get();
    double loopEnd = m_pLoopOutPoint->get();
    if (loopStart != kNoTrigger && loopEnd != kNoTrigger && loopStart <= loopEnd) {
        CuePointer pLoopCue;
        QList<CuePointer> cuePoints(m_pLoadedTrack->getCuePoints());
        QListIterator<CuePointer> it(cuePoints);
        while (it.hasNext()) {
            CuePointer pCue(it.next());
            if (pCue->getType() == mixxx::CueType::Loop && pCue->getHotCue() == Cue::kNoHotCue) {
                pLoopCue = pCue;
                break;
            }
        }
        if (!pLoopCue) {
            pLoopCue = m_pLoadedTrack->createAndAddCue();
            pLoopCue->setType(mixxx::CueType::Loop);
        }
        pLoopCue->setStartPosition(loopStart);
        pLoopCue->setEndPosition(loopEnd);
    }

    disconnectLoadedTrack();

    // Do not reset m_pReplayGain here, because the track might be still
    // playing and the last buffer will be processed.

    m_pPlay->set(0.0);

    TrackPointer pUnloadedTrack(std::move(m_pLoadedTrack));
    DEBUG_ASSERT(!m_pLoadedTrack);
    return pUnloadedTrack;
}

void BaseTrackPlayerImpl::connectLoadedTrack() {
    connect(m_pLoadedTrack.get(),
            &Track::bpmUpdated,
            m_pFileBPM.get(),
            QOverload<double>::of(&ControlObject::set));
    connect(m_pLoadedTrack.get(),
            &Track::keyUpdated,
            m_pKey.get(),
            &ControlProxy::set);
    connect(m_pLoadedTrack.get(),
            &Track::replayGainUpdated,
            this,
            &BaseTrackPlayerImpl::slotSetReplayGain);
    connect(m_pLoadedTrack.get(),
            &Track::colorUpdated,
            this,
            &BaseTrackPlayerImpl::slotSetTrackColor);
}

void BaseTrackPlayerImpl::disconnectLoadedTrack() {
    // WARNING: Never. Ever. call bare disconnect() on an object. Mixxx
    // relies on signals and slots to get tons of things done. Don't
    // randomly disconnect things.
    disconnect(m_pLoadedTrack.get(), 0, m_pFileBPM.get(), 0);
    disconnect(m_pLoadedTrack.get(), 0, this, 0);
    disconnect(m_pLoadedTrack.get(), 0, m_pKey.get(), 0);
}

void BaseTrackPlayerImpl::slotLoadTrack(TrackPointer pNewTrack, bool bPlay) {
    qDebug() << "BaseTrackPlayerImpl::slotLoadTrack" << getGroup();
    // Before loading the track, ensure we have access. This uses lazy
    // evaluation to make sure track isn't NULL before we dereference it.
    if (pNewTrack && !Sandbox::askForAccess(pNewTrack->getCanonicalLocation())) {
        // We don't have access.
        return;
    }

    auto pOldTrack = unloadTrack();

    loadTrack(pNewTrack);

    // Request a new track from EngineBuffer
    EngineBuffer* pEngineBuffer = m_pChannel->getEngineBuffer();
    pEngineBuffer->loadTrack(pNewTrack, bPlay);

    // await slotTrackLoaded()/slotLoadFailed()
    emit loadingTrack(pNewTrack, pOldTrack);
}

void BaseTrackPlayerImpl::slotLoadFailed(TrackPointer pTrack, QString reason) {
    // Note: This slot can be a load failure from the current track or a
    // a delayed signal from a previous load.
    // We have probably received a slotTrackLoaded signal, of an old track that
    // was loaded before. Here we must unload the
    // We must unload the track m_pLoadedTrack as well
    if (pTrack == m_pLoadedTrack) {
        qDebug() << "Failed to load track" << pTrack->getFileInfo() << reason;
        slotTrackLoaded(TrackPointer(), pTrack);
    } else if (pTrack) {
        qDebug() << "Stray failed to load track" << pTrack->getFileInfo() << reason;
    } else {
        qDebug() << "Failed to load track (NULL track object)" << reason;
    }
    m_pChannelToCloneFrom = nullptr;
    // Alert user.
    QMessageBox::warning(nullptr, tr("Couldn't load track."), reason);
}

void BaseTrackPlayerImpl::slotTrackLoaded(TrackPointer pNewTrack,
                                          TrackPointer pOldTrack) {
    //qDebug() << "BaseTrackPlayerImpl::slotTrackLoaded";
    if (!pNewTrack &&
            pOldTrack &&
            pOldTrack == m_pLoadedTrack) {
        // eject Track
        unloadTrack();

        // Causes the track's data to be saved back to the library database and
        // for all the widgets to change the track and update themselves.
        emit loadingTrack(pNewTrack, pOldTrack);
        m_pDuration->set(0);
        m_pFileBPM->set(0);
        m_pKey->set(0);
        setReplayGain(0);
        slotSetTrackColor(std::nullopt);
        m_pLoopInPoint->set(kNoTrigger);
        m_pLoopOutPoint->set(kNoTrigger);
        m_pLoadedTrack.reset();
        emit playerEmpty();
    } else if (pNewTrack && pNewTrack == m_pLoadedTrack) {
        // NOTE(uklotzde): In a previous version track metadata was reloaded
        // from the source file at this point again. This is no longer necessary
        // since track objects will always be created in a controlled manner
        // and populated from the database and their source file as required
        // before handing them out to application code.
        // TODO(XXX): Don't hesitate to delete the preceding NOTE if you think
        // that it is not needed anymore.

        // Update the BPM and duration values that are stored in ControlObjects
        m_pDuration->set(m_pLoadedTrack->getDuration());
        m_pFileBPM->set(m_pLoadedTrack->getBpm());
        m_pKey->set(m_pLoadedTrack->getKey());
        setReplayGain(m_pLoadedTrack->getReplayGain().getRatio());
        slotSetTrackColor(m_pLoadedTrack->getColor());

        if(m_pConfig->getValue(
                ConfigKey("[Mixer Profile]", "EqAutoReset"), false)) {
            if (m_pLowFilter) {
                m_pLowFilter->set(1.0);
            }
            if (m_pMidFilter) {
                m_pMidFilter->set(1.0);
            }
            if (m_pHighFilter) {
                m_pHighFilter->set(1.0);
            }
            if (m_pLowFilterKill) {
                m_pLowFilterKill->set(0.0);
            }
            if (m_pMidFilterKill) {
                m_pMidFilterKill->set(0.0);
            }
            if (m_pHighFilterKill) {
                m_pHighFilterKill->set(0.0);
            }
        }
        if (m_pConfig->getValue(
                ConfigKey("[Mixer Profile]", "GainAutoReset"), false)) {
            m_pPreGain->set(1.0);
        }

        if (!m_pChannelToCloneFrom) {
            int reset = m_pConfig->getValue<int>(
                    ConfigKey("[Controls]", "SpeedAutoReset"), RESET_PITCH);
            if (reset == RESET_SPEED || reset == RESET_PITCH_AND_SPEED) {
                // Avoid resetting speed if master sync is enabled and other decks with sync enabled
                // are playing, as this would change the speed of already playing decks.
                if (!m_pEngineMaster->getEngineSync()->otherSyncedPlaying(getGroup())) {
                    m_pRateRatio->set(1.0);
                }
            }
            if (reset == RESET_PITCH || reset == RESET_PITCH_AND_SPEED) {
                m_pPitchAdjust->set(0.0);
            }
        } else {
            // perform a clone of the given channel

            // copy rate
            m_pRateRatio->set(ControlObject::get(ConfigKey(
                    m_pChannelToCloneFrom->getGroup(), "rate_ratio")));

            // copy pitch
            m_pPitchAdjust->set(ControlObject::get(ConfigKey(
                    m_pChannelToCloneFrom->getGroup(), "pitch_adjust")));

            // copy play state
            ControlObject::set(ConfigKey(getGroup(), "play"),
                    ControlObject::get(ConfigKey(m_pChannelToCloneFrom->getGroup(), "play")));

            // copy the play position
            m_pChannel->getEngineBuffer()->requestClonePosition(m_pChannelToCloneFrom);

            // copy the loop state
            if (ControlObject::get(ConfigKey(m_pChannelToCloneFrom->getGroup(), "loop_enabled")) == 1.0) {
                ControlObject::set(ConfigKey(getGroup(), "reloop_toggle"), 1.0);
            }
        }

        emit newTrackLoaded(m_pLoadedTrack);
    } else {
        // this is the result from an outdated load or unload signal
        // A new load is already pending
        // Ignore this signal and wait for the new one
        qDebug() << "stray BaseTrackPlayerImpl::slotTrackLoaded()";
    }

    m_pChannelToCloneFrom = nullptr;

    // Update the PlayerInfo class that is used in EngineBroadcast to replace
    // the metadata of a stream
    PlayerInfo::instance().setTrackInfo(getGroup(), m_pLoadedTrack);
}

TrackPointer BaseTrackPlayerImpl::getLoadedTrack() const {
    return m_pLoadedTrack;
}

void BaseTrackPlayerImpl::slotCloneDeck() {
    Syncable* syncable = m_pEngineMaster->getEngineSync()->pickNonSyncSyncTarget(m_pChannel);
    if (syncable) {
        slotCloneChannel(syncable->getChannel());
    }
}

void BaseTrackPlayerImpl::slotCloneFromGroup(const QString& group) {
    EngineChannel* pChannel = m_pEngineMaster->getChannel(group);
    if (!pChannel) {
        return;
    }

    slotCloneChannel(pChannel);
}

void BaseTrackPlayerImpl::slotCloneFromDeck(double d) {
    int deck = static_cast<int>(d);
    if (deck < 1) {
        slotCloneDeck();
    } else {
        slotCloneFromGroup(PlayerManager::groupForDeck(deck - 1));
    }
}

void BaseTrackPlayerImpl::slotCloneFromSampler(double d) {
    int sampler = static_cast<int>(d);
    if (sampler >= 1) {
        slotCloneFromGroup(PlayerManager::groupForSampler(sampler - 1));
    }
}

void BaseTrackPlayerImpl::slotCloneChannel(EngineChannel* pChannel) {
    // don't clone from ourselves
    if (pChannel == m_pChannel) {
        return;
    }

    m_pChannelToCloneFrom = pChannel;
    if (!m_pChannelToCloneFrom) {
        return;
    }

    TrackPointer pTrack = m_pChannelToCloneFrom->getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        m_pChannelToCloneFrom = nullptr;
        return;
    }

    slotLoadTrack(pTrack, false);
}

void BaseTrackPlayerImpl::slotSetReplayGain(mixxx::ReplayGain replayGain) {
    // Do not change replay gain when track is playing because
    // this may lead to an unexpected volume change
    if (m_pPlay->get() == 0.0) {
        setReplayGain(replayGain.getRatio());
    } else {
        m_replaygainPending = true;
    }
}

void BaseTrackPlayerImpl::slotSetTrackColor(mixxx::RgbColor::optional_t color) {
    m_pTrackColor->forceSet(trackColorToDouble(color));
}

void BaseTrackPlayerImpl::slotTrackColorChangeRequest(double v) {
    if (!m_pLoadedTrack) {
        return;
    }

    mixxx::RgbColor::optional_t color = std::nullopt;
    if (v != kNoTrackColor) {
        auto colorCode = static_cast<mixxx::RgbColor::code_t>(v);
        if (!mixxx::RgbColor::isValidCode(colorCode)) {
            return;
        }
        color = mixxx::RgbColor::optional(colorCode);
    }
    m_pTrackColor->setAndConfirm(trackColorToDouble(color));
    m_pLoadedTrack->setColor(color);
}

void BaseTrackPlayerImpl::slotPlayToggled(double value) {
    if (value == 0 && m_replaygainPending) {
        setReplayGain(m_pLoadedTrack->getReplayGain().getRatio());
    }
}

EngineDeck* BaseTrackPlayerImpl::getEngineDeck() const {
    return m_pChannel;
}

void BaseTrackPlayerImpl::setupEqControls() {
    const QString group = getGroup();
    m_pLowFilter = make_parented<ControlProxy>(group, "filterLow", this);
    m_pMidFilter = make_parented<ControlProxy>(group, "filterMid", this);
    m_pHighFilter = make_parented<ControlProxy>(group, "filterHigh", this);
    m_pLowFilterKill = make_parented<ControlProxy>(group, "filterLowKill", this);
    m_pMidFilterKill = make_parented<ControlProxy>(group, "filterMidKill", this);
    m_pHighFilterKill = make_parented<ControlProxy>(group, "filterHighKill", this);
}

void BaseTrackPlayerImpl::slotVinylControlEnabled(double v) {
#ifdef __VINYLCONTROL__
    bool configured = m_pInputConfigured->toBool();
    bool vinylcontrol_enabled = v > 0.0;

    // Warn the user if they try to enable vinyl control on a player with no
    // configured input.
    if (!configured && vinylcontrol_enabled) {
        m_pVinylControlEnabled->set(0.0);
        m_pVinylControlStatus->set(VINYL_STATUS_DISABLED);
        emit noVinylControlInputConfigured();
    }
#endif
}

void BaseTrackPlayerImpl::slotWaveformZoomValueChangeRequest(double v) {
    if (v <= WaveformWidgetRenderer::s_waveformMaxZoom
            && v >= WaveformWidgetRenderer::s_waveformMinZoom) {
        m_pWaveformZoom->setAndConfirm(v);
    }
}

void BaseTrackPlayerImpl::slotWaveformZoomUp(double pressed) {
    if (pressed <= 0.0) {
        return;
    }

    m_pWaveformZoom->set(m_pWaveformZoom->get() + 1.0);
}

void BaseTrackPlayerImpl::slotWaveformZoomDown(double pressed) {
    if (pressed <= 0.0) {
        return;
    }

    m_pWaveformZoom->set(m_pWaveformZoom->get() - 1.0);
}

void BaseTrackPlayerImpl::slotWaveformZoomSetDefault(double pressed) {
    if (pressed <= 0.0) {
        return;
    }

    double defaultZoom = m_pConfig->getValue(ConfigKey("[Waveform]","DefaultZoom"),
        WaveformWidgetRenderer::s_waveformDefaultZoom);
    m_pWaveformZoom->set(defaultZoom);
}

void BaseTrackPlayerImpl::slotShiftCuesMillis(double milliseconds) {
    if (!m_pLoadedTrack) {
        return;
    }
    m_pLoadedTrack->shiftCuePositionsMillis(milliseconds);
}

void BaseTrackPlayerImpl::slotShiftCuesMillisButton(double value, double milliseconds) {
    if (value <= 0) {
        return;
    }
    slotShiftCuesMillis(milliseconds);
}

void BaseTrackPlayerImpl::setReplayGain(double value) {
    m_pReplayGain->set(value);
    m_replaygainPending = false;
}
