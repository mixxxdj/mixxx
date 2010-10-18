#include <QtCore>
#include <QMessageBox>

#include "player.h"
#include "playerinfo.h"

#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"
#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "playerinfo.h"
#include "soundsourceproxy.h"
#include "engine/cuecontrol.h"
#include "mathstuff.h"

Player::Player(ConfigObject<ConfigValue> *pConfig,
               EngineMaster* pMixingEngine,
               int playerNumber, QString group)
    : m_pConfig(pConfig),
      m_iPlayerNumber(playerNumber),
      m_strChannel(group),
      m_pLoadedTrack() {

    EngineChannel::ChannelOrientation orientation;
    if (playerNumber % 2 == 1)
        orientation = EngineChannel::LEFT;
    else
        orientation = EngineChannel::RIGHT;

    // Need to strdup the string because EngineChannel will save the pointer,
    // but we might get deleted before the EngineChannel. TODO(XXX)
    // pSafeGroupName is leaked. It's like 5 bytes so whatever.
    const char* pSafeGroupName = strdup(m_strChannel.toAscii().constData());

    EngineChannel* pChannel = new EngineChannel(pSafeGroupName,
                                                pConfig, orientation);
    EngineBuffer* pEngineBuffer = pChannel->getEngineBuffer();
    pMixingEngine->addChannel(pChannel);

    CueControl* pCueControl = new CueControl(pSafeGroupName, pConfig);

    connect(this, SIGNAL(newTrackLoaded(TrackPointer)),
            pCueControl, SLOT(loadTrack(TrackPointer)));
    connect(this, SIGNAL(unloadingTrack(TrackPointer)),
            pCueControl, SLOT(unloadTrack(TrackPointer)));
    pEngineBuffer->addControl(pCueControl);

    // Connect our signals and slots with the EngineBuffer's signals and
    // slots. This will let us know when the reader is done loading a track, and
    // let us request that the reader load a track.
    connect(this, SIGNAL(loadTrack(TrackPointer)),
            pEngineBuffer, SLOT(slotLoadTrack(TrackPointer)));
    connect(pEngineBuffer, SIGNAL(trackLoaded(TrackPointer)),
            this, SLOT(slotFinishLoading(TrackPointer)));
    connect(pEngineBuffer, SIGNAL(trackLoadFailed(TrackPointer, QString)),
            this, SLOT(slotLoadFailed(TrackPointer, QString)));

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

    //Duration of the current song, we create this one because nothing else does.
    m_pDuration = new ControlObject(ConfigKey(m_strChannel, "duration"));

    //BPM of the current song
    m_pBPM = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_strChannel, "file_bpm")));

    // Setup state of End of track controls from config database
    QString config_key = QString("TrackEndModeCh%1").arg(m_iPlayerNumber);
    ControlObject::getControl(ConfigKey(m_strChannel,"TrackEndMode"))->queueFromThread(
        m_pConfig->getValueString(ConfigKey("[Controls]",config_key)).toDouble());
}

Player::~Player()
{
    // Save state of End of track controls in config database
    int config_value = (int)ControlObject::getControl(ConfigKey(m_strChannel, "TrackEndMode"))->get();
    QString config_key = QString("TrackEndModeCh%1").arg(m_iPlayerNumber);
    m_pConfig->set(ConfigKey("[Controls]",config_key), ConfigValue(config_value));

    if (m_pLoadedTrack) {
        emit(unloadingTrack(m_pLoadedTrack));
        m_pLoadedTrack.clear();
    }

    delete m_pCuePoint;
    delete m_pLoopInPoint;
    delete m_pLoopOutPoint;
    delete m_pPlayPosition;
    delete m_pBPM;
}

void Player::slotLoadTrack(TrackPointer track, bool bStartFromEndPos)
{
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

        // TODO(XXX) This could be a help or a hurt. This should disconnect
        // every signal connected to the track. Other parts of Mixxx might be
        // relying on this -- but if it's being unloaded maybe that's a good
        // thing.
        m_pLoadedTrack->disconnect();
        // Causes the track's data to be saved back to the library database.
        emit(unloadingTrack(m_pLoadedTrack));
    }

    m_pLoadedTrack = track;

    // Listen for updates to the file's BPM
    connect(m_pLoadedTrack.data(), SIGNAL(bpmUpdated(double)),
            m_pBPM, SLOT(slotSet(double)));

    //Request a new track from the reader
    emit(loadTrack(track));
}

void Player::slotLoadFailed(TrackPointer track, QString reason) {
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
    m_pDuration->set(0);
    m_pBPM->slotSet(0);
    m_pLoopInPoint->slotSet(-1);
    m_pLoopOutPoint->slotSet(-1);
    m_pLoadedTrack.clear();

    // Update the PlayerInfo class that is used in EngineShoutcast to replace
    // the metadata of a stream
    PlayerInfo::Instance().setTrackInfo(m_strChannel.mid(8,1).toInt(), m_pLoadedTrack);
}

void Player::slotFinishLoading(TrackPointer pTrackInfoObject)
{
    // Read the tags if required
    if(!m_pLoadedTrack->getHeaderParsed())
        SoundSourceProxy::ParseHeader(m_pLoadedTrack.data());

    m_pLoadedTrack->incTimesPlayed();

    // Generate waveform summary
    //TODO: Consider reworking this visual resample stuff... need to ask rryan about this -- Albert.
    // TODO(rryan) : fix this crap -- the waveform renderers should be owned by
    // Player so they can just set this directly or something.
    ControlObjectThreadMain* pVisualResampleCO = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(m_strChannel,"VisualResample")));
    m_pLoadedTrack->setVisualResampleRate(pVisualResampleCO->get());
    delete pVisualResampleCO;

    //Update the BPM and duration values that are stored in ControlObjects
    m_pDuration->set(m_pLoadedTrack->getDuration());
    m_pBPM->slotSet(m_pLoadedTrack->getBpm());

    // Update the PlayerInfo class that is used in EngineShoutcast to replace
    // the metadata of a stream
    PlayerInfo::Instance().setTrackInfo(m_strChannel.mid(8,1).toInt(), m_pLoadedTrack);

    // Reset the loop points.
    m_pLoopInPoint->slotSet(-1);
    m_pLoopOutPoint->slotSet(-1);

    const QList<Cue*> trackCues = pTrackInfoObject->getCuePoints();
    QListIterator<Cue*> it(trackCues);
    while (it.hasNext()) {
        Cue* pCue = it.next();
        if (pCue->getType() == Cue::LOOP) {
            int loopStart = pCue->getPosition();
            int loopEnd = loopStart + pCue->getLength();
            if (loopStart != -1 && loopEnd != -1 && even(loopStart) && even(loopEnd)) {
                m_pLoopInPoint->slotSet(loopStart);
                m_pLoopOutPoint->slotSet(loopEnd);
                break;
            }
        }
    }

    emit(newTrackLoaded(m_pLoadedTrack));
}

QString Player::getGroup() {
    return m_strChannel;
}
