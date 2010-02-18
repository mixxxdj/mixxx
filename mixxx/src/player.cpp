#include <QtCore>
#include <QMessageBox>

#include "player.h"

#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"
#include "engine/enginebuffer.h"
#include "playerinfo.h"
#include "soundsourceproxy.h"
#include "engine/cuecontrol.h"

Player::Player(ConfigObject<ConfigValue> *pConfig,
               EngineBuffer* buffer,
               QString channel)
    : m_pConfig(pConfig),
      m_pEngineBuffer(buffer),
      m_strChannel(channel),
      m_pLoadedTrack(NULL) {

    CueControl* pCueControl = new CueControl(channel, pConfig);
    connect(this, SIGNAL(newTrackLoaded(TrackInfoObject*)),
            pCueControl, SLOT(loadTrack(TrackInfoObject*)));
    connect(this, SIGNAL(unloadingTrack(TrackInfoObject*)),
            pCueControl, SLOT(unloadTrack(TrackInfoObject*)));
    m_pEngineBuffer->addControl(pCueControl);

    //Tell the reader to notify us when it's done loading a track so we can
    //finish doing stuff.
    connect(m_pEngineBuffer, SIGNAL(trackLoaded(TrackInfoObject*)),
            this, SLOT(slotFinishLoading(TrackInfoObject*)));
    connect(m_pEngineBuffer, SIGNAL(trackLoadFailed(TrackInfoObject*, QString)),
            this, SLOT(slotLoadFailed(TrackInfoObject*, QString)));

    //Get cue point control object
    m_pCuePoint = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_strChannel,"cue_point")));
    // Get loop point control objects
    m_pLoopInPoint = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_strChannel,"loop_start_position")));
    m_pLoopOutPoint = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_strChannel,"loop_end_position")));
    //Playback position within the currently loaded track (in this player).
    m_pPlayPosition = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_strChannel, "playposition")));
    //Duration of the current song
    m_pDuration = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_strChannel, "duration")));
    //BPM of the current song
    m_pBPM = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_strChannel, "file_bpm")));
}

Player::~Player()
{
    emit(unloadingTrack(m_pLoadedTrack));
    // TODO(XXX) Really? Delete the TIO? Seems unsafe until we figure out the
    // lifetime of TIOs issue.
    delete m_pLoadedTrack;
    delete m_pCuePoint;
    delete m_pLoopInPoint;
    delete m_pLoopOutPoint;
    delete m_pPlayPosition;
    delete m_pDuration;
    delete m_pBPM;
}

void Player::slotLoadTrack(TrackInfoObject* track, bool bStartFromEndPos)
{
    //Disconnect the old track's signals.
    if (m_pLoadedTrack) {
        // TODO(XXX) This could be a help or a hurt. This should disconnect
        // every signal connected to the track. Other parts of Mixxx might be
        // relying on this -- but if it's being unloaded maybe that's a good
        // thing.
        m_pLoadedTrack->disconnect();
        // Causes the track's data to be saved back to the library database.
        emit(unloadingTrack(m_pLoadedTrack));
    }

    //TODO: Free m_pLoadedTrack, but make sure nobody else still has a pointer to it...
    //			(ie. I think we should use auto-pointers for TrackInfoObjects...)

    m_pLoadedTrack = track;

    // Listen for updates to the file's BPM
    connect(m_pLoadedTrack, SIGNAL(bpmUpdated(double)),
            m_pBPM, SLOT(slotSet(double)));

    //Request a new track from the reader
    m_pEngineBuffer->loadTrack(track);
}

void Player::slotLoadFailed(TrackInfoObject* track, QString reason) {
    qDebug() << "Failed to load track" << track->getLocation() << reason;
    // Alert user.
    QMessageBox::warning(NULL, tr("Couldn't load track."), reason);
    if (m_pLoadedTrack) {
        // TODO(XXX) This could be a help or a hurt. This should disconnect
        // every signal connected to the track. Other parts of Mixxx might be
        // relying on this -- but if it's being unloaded maybe that's a good
        // thing.
        m_pLoadedTrack->disconnect();
        // Causes the track's data to be saved back to the library database and
        // for all the widgets to unload the track and blank themselves.
        emit(unloadingTrack(m_pLoadedTrack));
    }
    m_pDuration->slotSet(0);
    m_pBPM->slotSet(0);
    m_pLoopInPoint->slotSet(-1);
    m_pLoopOutPoint->slotSet(-1);
    m_pLoadedTrack = NULL;
}

void Player::slotFinishLoading(TrackInfoObject* pTrackInfoObject)
{
    // Read the tags if required
    if(!m_pLoadedTrack->getHeaderParsed())
        SoundSourceProxy::ParseHeader(m_pLoadedTrack);

    // Generate waveform summary
    //TODO: Consider reworking this visual resample stuff... need to ask rryan about this -- Albert.
    // TODO(rryan) : fix this crap -- the waveform renderers should be owned by
    // Player so they can just set this directly or something.
    ControlObjectThreadMain* pVisualResampleCO = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(m_strChannel,"VisualResample")));
    m_pLoadedTrack->setVisualResampleRate(pVisualResampleCO->get());
    delete pVisualResampleCO;

    //Update the BPM and duration values that are stored in ControlObjects
    m_pDuration->slotSet(m_pLoadedTrack->getDuration());
    m_pBPM->slotSet(m_pLoadedTrack->getBpm());

    // Update TrackInfoObject of the helper class //FIXME
    //PlayerInfo::Instance().setTrackInfo(1, m_pLoadedTrack);

    // Reset the loop points. TODO(XXX) once loops are stored in the DB, replace
    // this with the default load loop.
    m_pLoopInPoint->slotSet(-1);
    m_pLoopOutPoint->slotSet(-1);

    emit(newTrackLoaded(m_pLoadedTrack));
}
