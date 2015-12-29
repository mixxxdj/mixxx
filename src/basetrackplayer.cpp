#include <QMessageBox>

#include "basetrackplayer.h"
#include "playerinfo.h"

#include "controlobject.h"
#include "controlpotmeter.h"
#include "trackinfoobject.h"
#include "engine/enginebuffer.h"
#include "engine/enginedeck.h"
#include "engine/enginemaster.h"
#include "track/beatgrid.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "analyserqueue.h"
#include "util/sandbox.h"
#include "effects/effectsmanager.h"

BaseTrackPlayer::BaseTrackPlayer(QObject* pParent, const QString& group)
        : BasePlayer(pParent, group) {
}

BaseTrackPlayerImpl::BaseTrackPlayerImpl(QObject* pParent,
                                         ConfigObject<ConfigValue>* pConfig,
                                         EngineMaster* pMixingEngine,
                                         EffectsManager* pEffectsManager,
                                         EngineChannel::ChannelOrientation defaultOrientation,
                                         QString group,
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
          m_pSpeed(NULL),
          m_pPitchAdjust(NULL),
          m_replaygainPending(false) {
    ChannelHandleAndGroup channelGroup =
            pMixingEngine->registerChannelGroup(group);
    m_pChannel = new EngineDeck(channelGroup, pConfig, pMixingEngine,
                                pEffectsManager, defaultOrientation);

    EngineBuffer* pEngineBuffer = m_pChannel->getEngineBuffer();
    pMixingEngine->addChannel(m_pChannel);

    // Set the routing option defaults for the master and headphone mixes.
    m_pChannel->setMaster(defaultMaster);
    m_pChannel->setPfl(defaultHeadphones);

    // Connect our signals and slots with the EngineBuffer's signals and
    // slots. This will let us know when the reader is done loading a track, and
    // let us request that the reader load a track.
    connect(this, SIGNAL(loadTrack(TrackPointer, bool)),
            pEngineBuffer, SLOT(slotLoadTrack(TrackPointer, bool)));
    connect(pEngineBuffer, SIGNAL(trackLoaded(TrackPointer)),
            this, SLOT(slotFinishLoading(TrackPointer)));
    connect(pEngineBuffer, SIGNAL(trackLoadFailed(TrackPointer, QString)),
            this, SLOT(slotLoadFailed(TrackPointer, QString)));
    connect(pEngineBuffer, SIGNAL(trackUnloaded(TrackPointer)),
            this, SLOT(slotUnloadTrack(TrackPointer)));

    // Get loop point control objects
    m_pLoopInPoint = new ControlObjectSlave(
            getGroup(),"loop_start_position", this);
    m_pLoopOutPoint = new ControlObjectSlave(
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

    m_pPreGain = new ControlObjectSlave(group, "pregain", this);
    //BPM of the current song
    m_pBPM = new ControlObjectThread(group, "file_bpm");
    m_pKey = new ControlObjectThread(group, "file_key");

    m_pReplayGain = new ControlObjectSlave(group, "replaygain", this);
    m_pPlay = new ControlObjectSlave(group, "play", this);
    m_pPlay->connectValueChanged(SLOT(slotPlayToggled(double)));
}

BaseTrackPlayerImpl::~BaseTrackPlayerImpl() {
    if (m_pLoadedTrack) {
        emit(unloadingTrack(m_pLoadedTrack));
        disconnect(m_pLoadedTrack.data(), 0, m_pBPM, 0);
        disconnect(m_pLoadedTrack.data(), 0, this, 0);
        disconnect(m_pLoadedTrack.data(), 0, m_pKey, 0);
        m_pLoadedTrack.clear();
    }

    delete m_pDuration;
    delete m_pWaveformZoom;
    delete m_pEndOfTrack;
    delete m_pBPM;
    delete m_pKey;
}

void BaseTrackPlayerImpl::slotLoadTrack(TrackPointer track, bool bPlay) {
    // Before loading the track, ensure we have access. This uses lazy
    // evaluation to make sure track isn't NULL before we dereference it.
    if (!track.isNull() && !Sandbox::askForAccess(track->getCanonicalLocation())) {
        // We don't have access.
        return;
    }

    //Disconnect the old track's signals.
    if (m_pLoadedTrack) {
        // Save the loops that are currently set in a loop cue. If no loop cue is
        // currently on the track, then create a new one.
        int loopStart = m_pLoopInPoint->get();
        int loopEnd = m_pLoopOutPoint->get();
        if (loopStart != -1 && loopEnd != -1 &&
            even(loopStart) && even(loopEnd) && loopStart <= loopEnd) {
            Cue* pLoopCue = NULL;
            QList<Cue*> cuePoints = m_pLoadedTrack->getCuePoints();
            QListIterator<Cue*> it(cuePoints);
            while (it.hasNext()) {
                Cue* pCue = it.next();
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
        // m_pLoadedTrack->disconnect();
        disconnect(m_pLoadedTrack.data(), 0, m_pBPM, 0);
        disconnect(m_pLoadedTrack.data(), 0, this, 0);
        disconnect(m_pLoadedTrack.data(), 0, m_pKey, 0);

        m_pReplayGain->set(0);

        // Causes the track's data to be saved back to the library database.
        emit(unloadingTrack(m_pLoadedTrack));
    }

    m_pLoadedTrack = track;
    if (m_pLoadedTrack) {
        // Listen for updates to the file's BPM
        connect(m_pLoadedTrack.data(), SIGNAL(bpmUpdated(double)),
                m_pBPM, SLOT(slotSet(double)));

        connect(m_pLoadedTrack.data(), SIGNAL(keyUpdated(double)),
                m_pKey, SLOT(slotSet(double)));

        // Listen for updates to the file's Replay Gain
        connect(m_pLoadedTrack.data(), SIGNAL(ReplayGainUpdated(Mixxx::ReplayGain)),
                this, SLOT(slotSetReplayGain(Mixxx::ReplayGain)));
    }

    // Request a new track from the reader
    emit(loadTrack(track, bPlay));
}

void BaseTrackPlayerImpl::slotLoadFailed(TrackPointer track, QString reason) {
    // This slot can be delayed until a new  track is already loaded
    // We must not unload the track here
    if (track != NULL) {
        qDebug() << "Failed to load track" << track->getLocation() << reason;
        emit(loadTrackFailed(track));
    } else {
        qDebug() << "Failed to load track (NULL track object)" << reason;
    }
    // Alert user.
    QMessageBox::warning(NULL, tr("Couldn't load track."), reason);
}

void BaseTrackPlayerImpl::slotUnloadTrack(TrackPointer) {
    if (m_pLoadedTrack) {
        // WARNING: Never. Ever. call bare disconnect() on an object. Mixxx
        // relies on signals and slots to get tons of things done. Don't
        // randomly disconnect things.
        // m_pLoadedTrack->disconnect();
        disconnect(m_pLoadedTrack.data(), 0, m_pBPM, 0);
        disconnect(m_pLoadedTrack.data(), 0, this, 0);
        disconnect(m_pLoadedTrack.data(), 0, m_pKey, 0);

        // Causes the track's data to be saved back to the library database and
        // for all the widgets to unload the track and blank themselves.
        emit(unloadingTrack(m_pLoadedTrack));
    }
    m_replaygainPending = false;
    m_pDuration->set(0);
    m_pBPM->slotSet(0);
    m_pKey->slotSet(0);
    m_pReplayGain->set(0);
    m_pLoopInPoint->set(-1);
    m_pLoopOutPoint->set(-1);
    m_pLoadedTrack.clear();

    // Update the PlayerInfo class that is used in EngineShoutcast to replace
    // the metadata of a stream
    PlayerInfo::instance().setTrackInfo(getGroup(), m_pLoadedTrack);
}

void BaseTrackPlayerImpl::slotFinishLoading(TrackPointer pTrackInfoObject)
{
    m_replaygainPending = false;
    // Read the tags if required
    if (!m_pLoadedTrack->getHeaderParsed()) {
        m_pLoadedTrack->parse(false);
    }

    // m_pLoadedTrack->setPlayedAndUpdatePlaycount(true); // Actually the song is loaded but not played

    // Update the BPM and duration values that are stored in ControlObjects
    m_pDuration->set(m_pLoadedTrack->getDuration());
    m_pBPM->slotSet(m_pLoadedTrack->getBpm());
    m_pKey->slotSet(m_pLoadedTrack->getKey());
    m_pReplayGain->set(m_pLoadedTrack->getReplayGain().getRatio());

    // Update the PlayerInfo class that is used in EngineShoutcast to replace
    // the metadata of a stream
    PlayerInfo::instance().setTrackInfo(getGroup(), m_pLoadedTrack);

    // Reset the loop points.
    m_pLoopInPoint->set(-1);
    m_pLoopOutPoint->set(-1);

    const QList<Cue*> trackCues = pTrackInfoObject->getCuePoints();
    QListIterator<Cue*> it(trackCues);
    while (it.hasNext()) {
        Cue* pCue = it.next();
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
        if (m_pSpeed != NULL) {
            m_pSpeed->set(0.0);
        }
        // Fallthrough intended
      case RESET_PITCH:
        if (m_pPitchAdjust != NULL) {
            m_pPitchAdjust->set(0.0);
        }
    }
    emit(newTrackLoaded(m_pLoadedTrack));
}

TrackPointer BaseTrackPlayerImpl::getLoadedTrack() const {
    return m_pLoadedTrack;
}

void BaseTrackPlayerImpl::slotSetReplayGain(Mixxx::ReplayGain replayGain) {
    // Do not change replay gain when track is playing because
    // this may lead to an unexpected volume change
    if (m_pPlay->get() == 0.0) {
        m_pReplayGain->set(replayGain.getRatio());
    } else {
        m_replaygainPending = true;
    }
}

void BaseTrackPlayerImpl::slotPlayToggled(double v) {
    if (!v && m_replaygainPending) {
        m_pReplayGain->set(m_pLoadedTrack->getReplayGain().getRatio());
        m_replaygainPending = false;
    }
}

EngineDeck* BaseTrackPlayerImpl::getEngineDeck() const {
    return m_pChannel;
}

void BaseTrackPlayerImpl::setupEqControls() {
    const QString group = getGroup();
    m_pLowFilter = new ControlObjectSlave(group, "filterLow", this);
    m_pMidFilter = new ControlObjectSlave(group, "filterMid", this);
    m_pHighFilter = new ControlObjectSlave(group, "filterHigh", this);
    m_pLowFilterKill = new ControlObjectSlave(group, "filterLowKill", this);
    m_pMidFilterKill = new ControlObjectSlave(group, "filterMidKill", this);
    m_pHighFilterKill = new ControlObjectSlave(group, "filterHighKill", this);
    m_pSpeed = new ControlObjectSlave(group, "rate", this);
    m_pPitchAdjust = new ControlObjectSlave(group, "pitch_adjust", this);
}
