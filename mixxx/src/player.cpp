#include <QtCore>

#include "player.h"

#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"
#include "reader.h"
#include "engine/enginebuffer.h"
#include "playerinfo.h"
#include "soundsourceproxy.h"

Player::Player(ConfigObject<ConfigValue> *pConfig,
               EngineBuffer* buffer,
               QString channel)
    : m_pConfig(pConfig),
      m_pEngineBuffer(buffer),
      m_strChannel(channel),
      m_pLoadedTrack(NULL) {
    
	//Tell the reader to notify us when it's done loading a track so we can finish doing stuff.
	connect(m_pEngineBuffer->getReader(), SIGNAL(finishedLoading(TrackInfoObject*, bool)),
            this, SLOT(slotFinishLoading(TrackInfoObject*, bool)));
	
 	//Get cue point control objects
    m_pCuePoint = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(m_strChannel,"cue_point")));
	//Playback position within the currently loaded track (in this player).
	m_pPlayPosition = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(m_strChannel, "playposition")));
	//Duration of the current song
    m_pDuration = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(m_strChannel, "duration")));
    //BPM of the current song
    m_pBPM = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(m_strChannel, "file_bpm")));

}

Player::~Player()
{
	emit(unloadingTrack(m_pLoadedTrack));
	delete m_pLoadedTrack;
	delete m_pCuePoint;
	delete m_pPlayPosition;
    delete m_pDuration;
    delete m_pBPM;
}

void Player::slotLoadTrack(TrackInfoObject* track, bool bStartFromEndPos)
{
	//Disconnect the old track's signals.
	if (m_pLoadedTrack) {
		m_pLoadedTrack->disconnect(); 
		emit(unloadingTrack(m_pLoadedTrack)); //Causes the track's data to be saved back to the library database.
	}
	
	//TODO: Free m_pLoadedTrack, but make sure nobody else still has a pointer to it...
	//			(ie. I think we should use auto-pointers for TrackInfoObjects...)
	
	m_pLoadedTrack = track;
    
    // Listen for updates to the file's BPM
    connect(m_pLoadedTrack, SIGNAL(bpmUpdated(double)),
            m_pBPM, SLOT(slotSet(double)));
	
	//Request a new track from the reader
	m_pEngineBuffer->getReader()->requestNewTrack(track, bStartFromEndPos);
 
}

void Player::slotFinishLoading(TrackInfoObject* pTrackInfoObject, bool bStartFromEndPos)
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

    //Set the cue point, if it was saved.
    m_pCuePoint->slotSet(m_pLoadedTrack->getCuePoint());

    //Seek to cue position if we're not starting at end of song and cue recall is on
    if (!bStartFromEndPos) {
        int cueRecall = m_pConfig->getValueString(ConfigKey("[Controls]","CueRecall")).toInt();
        if (cueRecall == 0) { //If cue recall is ON in the prefs, then we're supposed to seek to the cue point on song load.
            //Note that cueRecall == 0 corresponds to "ON", not OFF.
            float cue_point = m_pLoadedTrack->getCuePoint();
            long numSamplesInSong = m_pEngineBuffer->getReader()->getFileLength();
            cue_point = cue_point / (numSamplesInSong);
            m_pPlayPosition->slotSet(cue_point);
        }
    }

    emit(newTrackLoaded(m_pLoadedTrack));
}
