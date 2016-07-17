#include <QMessageBox>

#include "mixer/basetrackplayer.h"
#include "mixer/playerinfo.h"

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "track/track.h"
#include "sources/soundsourceproxy.h"
#include "engine/enginebuffer.h"
#include "engine/enginedeck.h"
#include "engine/enginemaster.h"
#include "track/beatgrid.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "analyzer/analyzerqueue.h"
#include "util/platform.h"
#include "util/sandbox.h"
#include "effects/effectsmanager.h"
#include "vinylcontrol/defs_vinylcontrol.h"

BaseTrackPlayer::BaseTrackPlayer(QObject* pParent, const QString& group)
        : BasePlayer(pParent, group) {
}

BaseTrackPlayerImpl::BaseTrackPlayerImpl(QObject* pParent,
                                         UserSettingsPointer pConfig,
                                         EngineMaster* pMixingEngine,
                                         EffectsManager* pEffectsManager,
                                         EngineChannel::ChannelOrientation defaultOrientation,
                                         const QString& group,
                                         bool defaultMaster,
                                         bool defaultHeadphones)
        : BaseTrackPlayer(pParent, group),
          m_pConfig(pConfig),
          m_pLoadedTrack(),
          m_pLowFilter(NULL),
          m_pMidFilter(NULL),
          m_pHighFilter(NULL),
          m_pLowFilterKill(NULL),
          m_pMidFilterKill(NULL),
          m_pHighFilterKill(NULL),
          m_pRateSlider(NULL),
          m_pPitchAdjust(NULL),
          m_replaygainPending(false) {
    ChannelHandleAndGroup channelGroup =
            pMixingEngine->registerChannelGroup(group);
    m_pChannel = new EngineDeck(channelGroup, pConfig, pMixingEngine,
                                pEffectsManager, defaultOrientation);

    m_pInputConfigured.reset(new ControlProxy(group, "input_configured", this));
    m_pPassthroughEnabled.reset(new ControlProxy(group, "passthrough", this));
    m_pPassthroughEnabled->connectValueChanged(SLOT(slotPassthroughEnabled(double)));
#ifdef __VINYLCONTROL__
    m_pVinylControlEnabled.reset(new ControlProxy(group, "vinylcontrol_enabled", this));
    m_pVinylControlEnabled->connectValueChanged(SLOT(slotVinylControlEnabled(double)));
    m_pVinylControlStatus.reset(new ControlProxy(group, "vinylcontrol_status", this));
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
    m_pLoopInPoint = new ControlProxy(
            getGroup(),"loop_start_position", this);
    m_pLoopOutPoint = new ControlProxy(
            getGroup(),"loop_end_position", this);

    // Duration of the current song, we create this one because nothing else does.
    m_pDuration = new ControlObject(ConfigKey(getGroup(), "duration"));

    // Waveform controls
    m_pWaveformZoom = new ControlPotmeter(ConfigKey(group, "waveform_zoom"),
                                          WaveformWidgetRenderer::s_waveformMinZoom,
                                          WaveformWidgetRenderer::s_waveformMaxZoom);
    m_pWaveformZoom->set(1.0);
    m_pWaveformZoom->setStepCount(WaveformWidgetRenderer::s_waveformMaxZoom -
            WaveformWidgetRenderer::s_waveformMinZoom);
    m_pWaveformZoom->setSmallStepCount(WaveformWidgetRenderer::s_waveformMaxZoom -
            WaveformWidgetRenderer::s_waveformMinZoom);

    m_pEndOfTrack = new ControlObject(ConfigKey(group, "end_of_track"));
    m_pEndOfTrack->set(0.);

    m_pPreGain = new ControlProxy(group, "pregain", this);
    //BPM of the current song
    m_pBPM = new ControlProxy(group, "file_bpm", this);
    m_pKey = new ControlProxy(group, "file_key", this);

    m_pReplayGain = new ControlProxy(group, "replaygain", this);
    m_pPlay = new ControlProxy(group, "play", this);
    m_pPlay->connectValueChanged(SLOT(slotPlayToggled(double)));
}

BaseTrackPlayerImpl::~BaseTrackPlayerImpl() {
    if (m_pLoadedTrack) {
        emit(loadingTrack(TrackPointer(), m_pLoadedTrack));
        disconnect(m_pLoadedTrack.data(), 0, m_pBPM, 0);
        disconnect(m_pLoadedTrack.data(), 0, this, 0);
        disconnect(m_pLoadedTrack.data(), 0, m_pKey, 0);
        m_pLoadedTrack.clear();
    }

    delete m_pDuration;
    delete m_pWaveformZoom;
    delete m_pEndOfTrack;
}

void BaseTrackPlayerImpl::slotLoadTrack(TrackPointer pNewTrack, bool bPlay) {
    qDebug() << "BaseTrackPlayerImpl::slotLoadTrack";
    // Before loading the track, ensure we have access. This uses lazy
    // evaluation to make sure track isn't NULL before we dereference it.
    if (!pNewTrack.isNull() && !Sandbox::askForAccess(pNewTrack->getCanonicalLocation())) {
        // We don't have access.
        return;
    }

    TrackPointer pOldTrack = m_pLoadedTrack;

    // Disconnect the old track's signals.
    if (m_pLoadedTrack) {
        // Save the loops that are currently set in a loop cue. If no loop cue is
        // currently on the track, then create a new one.
        int loopStart = m_pLoopInPoint->get();
        int loopEnd = m_pLoopOutPoint->get();
        if (loopStart != -1 && loopEnd != -1 &&
            even(loopStart) && even(loopEnd) && loopStart <= loopEnd) {
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
                pLoopCue = m_pLoadedTrack->addCue();
                pLoopCue->setType(Cue::LOOP);
            }
            pLoopCue->setPosition(loopStart);
            pLoopCue->setLength(loopEnd - loopStart);
        }

        // WARNING: Never. Ever. call bare disconnect() on an object. Mixxx
        // relies on signals and slots to get tons of things done. Don't
        // randomly disconnect things.
        disconnect(m_pLoadedTrack.data(), 0, m_pBPM, 0);
        disconnect(m_pLoadedTrack.data(), 0, this, 0);
        disconnect(m_pLoadedTrack.data(), 0, m_pKey, 0);

        // Do not reset m_pReplayGain here, because the track might be still
        // playing and the last buffer will be processed.

        m_pPlay->set(0.0);
    }

    m_pLoadedTrack = pNewTrack;
    if (m_pLoadedTrack) {
        // Listen for updates to the file's BPM
        connect(m_pLoadedTrack.data(), SIGNAL(bpmUpdated(double)),
                m_pBPM, SLOT(set(double)));

        connect(m_pLoadedTrack.data(), SIGNAL(keyUpdated(double)),
                m_pKey, SLOT(set(double)));

        // Listen for updates to the file's Replay Gain
        connect(m_pLoadedTrack.data(), SIGNAL(ReplayGainUpdated(mixxx::ReplayGain)),
                this, SLOT(slotSetReplayGain(mixxx::ReplayGain)));
    }

    // Request a new track from EngineBuffer and wait for slotTrackLoaded()
    // call.
    EngineBuffer* pEngineBuffer = m_pChannel->getEngineBuffer();
    pEngineBuffer->loadTrack(pNewTrack, bPlay);
    // Causes the track's data to be saved back to the library database and
    // for all the widgets to change the track and update themselves.
    emit(loadingTrack(pNewTrack, pOldTrack));
}

void BaseTrackPlayerImpl::slotLoadFailed(TrackPointer track, QString reason) {
    // Note: This slot can be a load failure from the current track or a
    // a delayed signal from a previous load.
    // We have probably received a slotTrackLoaded signal, of an old track that
    // was loaded before. Here we must unload the
    // We must unload the track m_pLoadedTrack as well
    if (track == m_pLoadedTrack) {
        qDebug() << "Failed to load track" << track->getLocation() << reason;
        slotTrackLoaded(TrackPointer(), track);
    } else if (!track.isNull()) {
        qDebug() << "Stray failed to load track" << track->getLocation() << reason;
    } else {
        qDebug() << "Failed to load track (NULL track object)" << reason;
    }
    // Alert user.
    QMessageBox::warning(NULL, tr("Couldn't load track."), reason);
}

void BaseTrackPlayerImpl::slotTrackLoaded(TrackPointer pNewTrack,
                                          TrackPointer pOldTrack) {
    qDebug() << "BaseTrackPlayerImpl::slotTrackLoaded";
    if (pNewTrack.isNull() &&
            !pOldTrack.isNull() &&
            pOldTrack == m_pLoadedTrack) {
        // eject Track
        // WARNING: Never. Ever. call bare disconnect() on an object. Mixxx
        // relies on signals and slots to get tons of things done. Don't
        // randomly disconnect things.
        // m_pLoadedTrack->disconnect();
        disconnect(m_pLoadedTrack.data(), 0, m_pBPM, 0);
        disconnect(m_pLoadedTrack.data(), 0, this, 0);
        disconnect(m_pLoadedTrack.data(), 0, m_pKey, 0);

        // Causes the track's data to be saved back to the library database and
        // for all the widgets to change the track and update themselves.
        emit(loadingTrack(pNewTrack, pOldTrack));
        m_pDuration->set(0);
        m_pBPM->set(0);
        m_pKey->set(0);
        setReplayGain(0);
        m_pLoopInPoint->set(-1);
        m_pLoopOutPoint->set(-1);
        m_pLoadedTrack.clear();
        emit(playerEmpty());
    } else if (!pNewTrack.isNull() && pNewTrack == m_pLoadedTrack) {
        // Successful loaded a new track
        // Reload metadata from file, but only if required
        SoundSourceProxy(m_pLoadedTrack).loadTrackMetadata();

        // Update the BPM and duration values that are stored in ControlObjects
        m_pDuration->set(m_pLoadedTrack->getDuration());
        m_pBPM->set(m_pLoadedTrack->getBpm());
        m_pKey->set(m_pLoadedTrack->getKey());
        setReplayGain(m_pLoadedTrack->getReplayGain().getRatio());

        // Clear loop
        // It seems that the trick is to first clear the loop out point, and then
        // the loop in point. If we first clear the loop in point, the loop out point
        // does not get cleared.
        m_pLoopOutPoint->set(-1);
        m_pLoopInPoint->set(-1);

        const QList<CuePointer> trackCues(pNewTrack->getCuePoints());
        QListIterator<CuePointer> it(trackCues);
        while (it.hasNext()) {
            CuePointer pCue(it.next());
            if (pCue->getType() == Cue::LOOP) {
                int loopStart = pCue->getPosition();
                int loopEnd = loopStart + pCue->getLength();
                if (loopStart != -1 && loopEnd != -1 && even(loopStart) && even(loopEnd)) {
                    m_pLoopInPoint->set(loopStart);
                    m_pLoopOutPoint->set(loopEnd);
                    break;
                }
            }
        }
        if(m_pConfig->getValueString(ConfigKey("[Mixer Profile]", "EqAutoReset"), 0).toInt()) {
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
            m_pPreGain->set(1.0);
        }
        int reset = m_pConfig->getValueString(ConfigKey(
                "[Controls]", "SpeedAutoReset"),
                QString("%1").arg(RESET_PITCH)).toInt();
        switch (reset) {
          case RESET_PITCH_AND_SPEED:
            // Note: speed may affect pitch
            if (m_pRateSlider != NULL) {
                m_pRateSlider->set(0.0);
            }
            M_FALLTHROUGH_INTENDED;
          case RESET_PITCH:
            if (m_pPitchAdjust != NULL) {
                m_pPitchAdjust->set(0.0);
            }
            break;
          case RESET_SPEED:
            // Note: speed may affect pitch
            if (m_pRateSlider != NULL) {
                m_pRateSlider->set(0.0);
            }
            break;
        }
        emit(newTrackLoaded(m_pLoadedTrack));
    } else {
        // this is the result from an outdated load or unload signal
        // A new load is already pending
        // Ignore this signal and wait for the new one
        qDebug() << "stray BaseTrackPlayerImpl::slotTrackLoaded()";
    }

    // Update the PlayerInfo class that is used in EngineBroadcast to replace
    // the metadata of a stream
    PlayerInfo::instance().setTrackInfo(getGroup(), m_pLoadedTrack);
}

TrackPointer BaseTrackPlayerImpl::getLoadedTrack() const {
    return m_pLoadedTrack;
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
    m_pLowFilter = new ControlProxy(group, "filterLow", this);
    m_pMidFilter = new ControlProxy(group, "filterMid", this);
    m_pHighFilter = new ControlProxy(group, "filterHigh", this);
    m_pLowFilterKill = new ControlProxy(group, "filterLowKill", this);
    m_pMidFilterKill = new ControlProxy(group, "filterMidKill", this);
    m_pHighFilterKill = new ControlProxy(group, "filterHighKill", this);
    m_pRateSlider = new ControlProxy(group, "rate", this);
    m_pPitchAdjust = new ControlProxy(group, "pitch_adjust", this);
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

void BaseTrackPlayerImpl::setReplayGain(double value) {
    m_pReplayGain->set(value);
    m_replaygainPending = false;
}
