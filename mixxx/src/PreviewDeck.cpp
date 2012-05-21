#include "PreviewDeck.h"
#include "engine/enginebuffer.h"
#include "engine/enginePreviewDeck.h"
#include "engine/enginemaster.h"
#include "engine/clockcontrol.h"
#include "engine/cuecontrol.h"
#include "waveform/waveformrenderer.h"

PreviewDeck::PreviewDeck(QObject* pParent,
                 ConfigObject<ConfigValue>* pConfig,
                 EngineMaster* pMixingEngine,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 QString group)
        : BaseTrackPlayer(pParent, group) {

    const char* pSafeGroupName = strdup(getGroup().toAscii().constData());

    EnginePreviewDeck* pChannel = new EnginePreviewDeck(pSafeGroupName,
                                        pConfig, defaultOrientation);
    EngineBuffer* pEngineBuffer = pChannel->getEngineBuffer();
    pMixingEngine->addChannel(pChannel);
    
    ClockControl* pClockControl = new ClockControl(pSafeGroupName, pConfig);
    pEngineBuffer->addControl(pClockControl);

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
    connect(pEngineBuffer, SIGNAL(trackUnloaded(TrackPointer)),
            this, SLOT(slotUnloadTrack(TrackPointer)));

    //Get cue point control object
    m_pCuePoint = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(getGroup(),"cue_point")));
    // Get loop point control objects
    m_pLoopInPoint = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(getGroup(),"loop_start_position")));
    m_pLoopOutPoint = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(getGroup(),"loop_end_position")));
    //Playback position within the currently loaded track (in this player).
    m_pPlayPosition = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(getGroup(), "playposition")));

    // Duration of the current song, we create this one because nothing else does.
    m_pDuration = new ControlObject(ConfigKey(getGroup(), "duration"));

    //BPM of the current song
    m_pBPM = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(getGroup(), "file_bpm")));

    m_pReplayGain = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(getGroup(), "replaygain")));

    // Create WaveformRenderer last, because it relies on controls created above
    // (e.g. EngineBuffer)

    m_pWaveformRenderer = new WaveformRenderer(pSafeGroupName);
    connect(this, SIGNAL(newTrackLoaded(TrackPointer)),
            m_pWaveformRenderer, SLOT(slotNewTrack(TrackPointer)));
    connect(this, SIGNAL(unloadingTrack(TrackPointer)),
            m_pWaveformRenderer, SLOT(slotUnloadTrack(TrackPointer)));
}

PreviewDeck::~PreviewDeck()
{
}
