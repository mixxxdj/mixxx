#include <QMessageBox>

#include "mixer/basetrackplayer.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "track/track.h"
#include "sources/soundsourceproxy.h"
#include "engine/enginebuffer.h"
#include "engine/controls/enginecontrol.h"
#include "engine/channels/enginedeck.h"
#include "engine/enginemaster.h"
#include "track/beatgrid.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/visualsmanager.h"
#include "util/platform.h"
#include "util/sandbox.h"
#include "effects/effectsmanager.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "engine/sync/enginesync.h"

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
                                         bool defaultHeadphones)
        : BaseTrackPlayer(pParent, group),
          m_pConfig(pConfig),
          m_pEngineMaster(pMixingEngine),
          m_pLoadedTrack(),
          m_replaygainPending(false),
          m_pChannelToCloneFrom(nullptr) {
    ChannelHandleAndGroup channelGroup =
            pMixingEngine->registerChannelGroup(group);
    m_pChannel = new EngineDeck(channelGroup, pConfig, pMixingEngine,
                                pEffectsManager, defaultOrientation);

    m_pInputConfigured = std::make_unique<ControlProxy>(group, "input_configured", this);
    m_pPassthroughEnabled = std::make_unique<ControlProxy>(group, "passthrough", this);
    m_pPassthroughEnabled->connectValueChanged(this, &BaseTrackPlayerImpl::slotPassthroughEnabled);
#ifdef __VINYLCONTROL__
    m_pVinylControlEnabled = std::make_unique<ControlProxy>(group, "vinylcontrol_enabled", this);
    m_pVinylControlEnabled->connectValueChanged(this, &BaseTrackPlayerImpl::slotVinylControlEnabled);
    m_pVinylControlStatus = std::make_unique<ControlProxy>(group, "vinylcontrol_status", this);
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
    m_pLoopInPoint = std::make_unique<ControlProxy>(
            getGroup(), "loop_start_position", this);
    m_pLoopOutPoint = std::make_unique<ControlProxy>(
            getGroup(), "loop_end_position", this);

    // Duration of the current song, we create this one because nothing else does.
    m_pDuration = std::make_unique<ControlObject>(
        ConfigKey(getGroup(), "duration"));

    // Deck cloning
    m_pCloneFromDeck = std::make_unique<ControlObject>(
        ConfigKey(getGroup(), "CloneFromDeck"),
        false);
    connect(m_pCloneFromDeck.get(), &ControlObject::valueChanged,
        this, &BaseTrackPlayerImpl::slotCloneFromDeck);

    // Waveform controls
    // This acts somewhat like a ControlPotmeter, but the normal _up/_down methods
    // do not work properly with this CO.
    m_pWaveformZoom = std::make_unique<ControlObject>(
        ConfigKey(group, "waveform_zoom"));
    m_pWaveformZoom->connectValueChangeRequest(
        this, &BaseTrackPlayerImpl::slotWaveformZoomValueChangeRequest,
        Qt::DirectConnection);
    m_pWaveformZoom->set(1.0);
    m_pWaveformZoomUp = std::make_unique<ControlPushButton>(
        ConfigKey(group, "waveform_zoom_up"));
    connect(m_pWaveformZoomUp.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotWaveformZoomUp(double)));
    m_pWaveformZoomDown = std::make_unique<ControlPushButton>(
        ConfigKey(group, "waveform_zoom_down"));
    connect(m_pWaveformZoomDown.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotWaveformZoomDown(double)));
    m_pWaveformZoomSetDefault = std::make_unique<ControlPushButton>(
        ConfigKey(group, "waveform_zoom_set_default"));
    connect(m_pWaveformZoomSetDefault.get(), SIGNAL(valueChanged(double)),
            this, SLOT(slotWaveformZoomSetDefault(double)));

    m_pPreGain = std::make_unique<ControlProxy>(group, "pregain", this);
    // BPM of the current song
    m_pFileBPM = std::make_unique<ControlProxy>(group, "file_bpm", this);
    m_pKey = std::make_unique<ControlProxy>(group, "file_key", this);

    m_pReplayGain = std::make_unique<ControlProxy>(group, "replaygain", this);
    m_pPlay = std::make_unique<ControlProxy>(group, "play", this);
    m_pPlay->connectValueChanged(this, &BaseTrackPlayerImpl::slotPlayToggled);

    pVisualsManager->addDeck(group);
}

BaseTrackPlayerImpl::~BaseTrackPlayerImpl() {
    unloadTrack();
}

TrackPointer BaseTrackPlayerImpl::loadFakeTrack(bool bPlay, double filebpm) {
    TrackPointer pTrack(Track::newTemporary());
    pTrack->setSampleRate(44100);
    // 10 seconds
    pTrack->setDuration(10);
    if (filebpm > 0) {
        pTrack->setBpm(filebpm);
    }

    TrackPointer pOldTrack = m_pLoadedTrack;
    m_pLoadedTrack = pTrack;
    if (m_pLoadedTrack) {
        // Listen for updates to the file's BPM
        connect(m_pLoadedTrack.get(), SIGNAL(bpmUpdated(double)),
                m_pFileBPM.get(), SLOT(set(double)));

        connect(m_pLoadedTrack.get(), SIGNAL(keyUpdated(double)),
                m_pKey.get(), SLOT(set(double)));

        // Listen for updates to the file's Replay Gain
        connect(m_pLoadedTrack.get(), SIGNAL(ReplayGainUpdated(mixxx::ReplayGain)),
                this, SLOT(slotSetReplayGain(mixxx::ReplayGain)));
    }

    // Request a new track from EngineBuffer
    EngineBuffer* pEngineBuffer = m_pChannel->getEngineBuffer();
    pEngineBuffer->loadFakeTrack(pTrack, bPlay);

    // await slotTrackLoaded()/slotLoadFailed()
    emit(loadingTrack(pTrack, pOldTrack));

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
        while (it.hasNext()) {
            CuePointer pCue(it.next());
            if (pCue->getType() == Cue::LOOP) {
                double loopStart = pCue->getPosition();
                double loopEnd = loopStart + pCue->getLength();
                if (loopStart != kNoTrigger && loopEnd != kNoTrigger && loopStart <= loopEnd) {
                    m_pLoopInPoint->set(loopStart);
                    m_pLoopOutPoint->set(loopEnd);
                    break;
                }
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
            if (pCue->getType() == Cue::LOOP) {
                pLoopCue = pCue;
            }
        }
        if (!pLoopCue) {
            pLoopCue = m_pLoadedTrack->createAndAddCue();
            pLoopCue->setSource(Cue::MANUAL);
            pLoopCue->setType(Cue::LOOP);
        }
        pLoopCue->setPosition(loopStart);
        pLoopCue->setLength(loopEnd - loopStart);
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
    connect(m_pLoadedTrack.get(), SIGNAL(bpmUpdated(double)),
            m_pFileBPM.get(), SLOT(set(double)));
    connect(m_pLoadedTrack.get(), SIGNAL(keyUpdated(double)),
            m_pKey.get(), SLOT(set(double)));
    connect(m_pLoadedTrack.get(), SIGNAL(ReplayGainUpdated(mixxx::ReplayGain)),
            this, SLOT(slotSetReplayGain(mixxx::ReplayGain)));
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
    emit(loadingTrack(pNewTrack, pOldTrack));
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
    QMessageBox::warning(NULL, tr("Couldn't load track."), reason);
}

void BaseTrackPlayerImpl::slotTrackLoaded(TrackPointer pNewTrack,
                                          TrackPointer pOldTrack) {
    qDebug() << "BaseTrackPlayerImpl::slotTrackLoaded";
    if (!pNewTrack &&
            pOldTrack &&
            pOldTrack == m_pLoadedTrack) {
        // eject Track
        unloadTrack();

        // Causes the track's data to be saved back to the library database and
        // for all the widgets to change the track and update themselves.
        emit(loadingTrack(pNewTrack, pOldTrack));
        m_pDuration->set(0);
        m_pFileBPM->set(0);
        m_pKey->set(0);
        setReplayGain(0);
        m_pLoopInPoint->set(kNoTrigger);
        m_pLoopOutPoint->set(kNoTrigger);
        m_pLoadedTrack.reset();
        emit(playerEmpty());
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

        if(m_pConfig->getValue(
                ConfigKey("[Mixer Profile]", "EqAutoReset"), false)) {
            if (m_pLowFilter != NULL) {
                m_pLowFilter->set(1.0);
            }
            if (m_pMidFilter != NULL) {
                m_pMidFilter->set(1.0);
            }
            if (m_pHighFilter != NULL) {
                m_pHighFilter->set(1.0);
            }
            if (m_pLowFilterKill != NULL) {
                m_pLowFilterKill->set(0.0);
            }
            if (m_pMidFilterKill != NULL) {
                m_pMidFilterKill->set(0.0);
            }
            if (m_pHighFilterKill != NULL) {
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
                    if (m_pRateSlider != NULL) {
                        m_pRateSlider->set(0.0);
                    }
                }
            }
            if (reset == RESET_PITCH || reset == RESET_PITCH_AND_SPEED) {
                if (m_pPitchAdjust != NULL) {
                    m_pPitchAdjust->set(0.0);
                }
            }
        } else {
            // perform a clone of the given channel

            // copy rate
            if (m_pRateSlider != nullptr) {
                m_pRateSlider->set(ControlObject::get(ConfigKey(m_pChannelToCloneFrom->getGroup(), "rate")));
            }

            // copy pitch
            if (m_pPitchAdjust != nullptr) {
                m_pPitchAdjust->set(ControlObject::get(ConfigKey(m_pChannelToCloneFrom->getGroup(), "pitch_adjust")));
            }

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

        emit(newTrackLoaded(m_pLoadedTrack));
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
    slotCloneChannel(m_pEngineMaster->getEngineSync()->pickNonSyncSyncTarget(m_pChannel));
}

void BaseTrackPlayerImpl::slotCloneFromGroup(const QString& group) {
    EngineChannel* pChannel = m_pEngineMaster->getChannel(group);
    if (!pChannel) {
        return;
    }

    slotCloneChannel(pChannel);
}

void BaseTrackPlayerImpl::slotCloneFromDeck(double d) {
    int deck = std::lround(d);
    if (deck < 1) {
        slotCloneDeck();
    } else {
        slotCloneFromGroup(PlayerManager::groupForDeck(deck-1));
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

void BaseTrackPlayerImpl::slotPlayToggled(double v) {
    if (!v && m_replaygainPending) {
        setReplayGain(m_pLoadedTrack->getReplayGain().getRatio());
    }
}

EngineDeck* BaseTrackPlayerImpl::getEngineDeck() const {
    return m_pChannel;
}

void BaseTrackPlayerImpl::setupEqControls() {
    const QString group = getGroup();
    m_pLowFilter = std::make_unique<ControlProxy>(group, "filterLow", this);
    m_pMidFilter = std::make_unique<ControlProxy>(group, "filterMid", this);
    m_pHighFilter = std::make_unique<ControlProxy>(group, "filterHigh", this);
    m_pLowFilterKill = std::make_unique<ControlProxy>(group, "filterLowKill", this);
    m_pMidFilterKill = std::make_unique<ControlProxy>(group, "filterMidKill", this);
    m_pHighFilterKill = std::make_unique<ControlProxy>(group, "filterHighKill", this);
    m_pRateSlider = std::make_unique<ControlProxy>(group, "rate", this);
    m_pPitchAdjust = std::make_unique<ControlProxy>(group, "pitch_adjust", this);
}

void BaseTrackPlayerImpl::slotPassthroughEnabled(double v) {
    bool configured = m_pInputConfigured->toBool();
    bool passthrough = v > 0.0;

    // Warn the user if they try to enable passthrough on a player with no
    // configured input.
    if (!configured && passthrough) {
        m_pPassthroughEnabled->set(0.0);
        emit(noPassthroughInputConfigured());
    }
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
        emit(noVinylControlInputConfigured());
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

void BaseTrackPlayerImpl::setReplayGain(double value) {
    m_pReplayGain->set(value);
    m_replaygainPending = false;
}
