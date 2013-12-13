/***************************************************************************
                          enginebuffer.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>

#include "engine/enginebuffer.h"
#include "cachingreader.h"
#include "sampleutil.h"

#include "controlpushbutton.h"
#include "configobject.h"
#include "controlpotmeter.h"
#include "controllinpotmeter.h"
#include "engine/enginechannel.h"
#include "engine/enginebufferscalest.h"
#include "engine/enginebufferscalerubberband.h"
#include "engine/enginebufferscalelinear.h"
#include "engine/enginebufferscaledummy.h"
#include "mathstuff.h"
#include "engine/sync/enginesync.h"
#include "engine/engineworkerscheduler.h"
#include "engine/readaheadmanager.h"
#include "engine/enginecontrol.h"
#include "engine/loopingcontrol.h"
#include "engine/ratecontrol.h"
#include "engine/bpmcontrol.h"
#include "engine/keycontrol.h"
#include "engine/sync/synccontrol.h"
#include "engine/quantizecontrol.h"
#include "visualplayposition.h"
#include "engine/cuecontrol.h"
#include "engine/clockcontrol.h"
#include "engine/enginemaster.h"
#include "util/timer.h"
#include "track/keyutils.h"
#include "controlobjectslave.h"
#include "util/compatibility.h"

#ifdef __VINYLCONTROL__
#include "engine/vinylcontrolcontrol.h"
#endif

#include "trackinfoobject.h"

const double kMaxPlayposRange = 1.14;
const double kMinPlayposRange = -0.14;

EngineBuffer::EngineBuffer(const char* _group, ConfigObject<ConfigValue>* _config,
                           EngineChannel* pChannel, EngineMaster* pMixingEngine) :
    m_engineLock(QMutex::Recursive),
    m_group(_group),
    m_pConfig(_config),
    m_pLoopingControl(NULL),
    m_pSyncControl(NULL),
    m_pRateControl(NULL),
    m_pBpmControl(NULL),
    m_pKeyControl(NULL),
    m_pReadAheadManager(NULL),
    m_pReader(NULL),
    m_filepos_play(0.),
    m_speed_old(0),
    m_pitch_old(0),
    m_baserate_old(0),
    m_rate_old(0.),
    m_file_length_old(-1),
    m_file_srate_old(0),
    m_iSamplesCalculated(0),
    m_iUiSlowTick(0),
    m_pRepeat(NULL),
    m_startButton(NULL),
    m_endButton(NULL),
    m_pScale(NULL),
    m_pScaleLinear(NULL),
    m_pScaleST(NULL),
    m_pScaleRB(NULL),
    m_bScalerChanged(false),
    m_bScalerOverride(false),
    m_bSeekQueued(0),
    m_dQueuedPosition(0),
    m_bLastBufferPaused(true),
    m_iTrackLoading(0),
    m_bPlayAfterLoading(false),
    m_fRampValue(0.0),
    m_iRampState(ENGINE_RAMP_NONE),
    m_pDitherBuffer(new CSAMPLE[MAX_BUFFER_LEN]),
    m_iDitherBufferReadIndex(0),
    m_pCrossFadeBuffer(new CSAMPLE[MAX_BUFFER_LEN]),
    m_iCrossFadeSamples(0),
    m_iLastBufferSize(0) {

    // Generate dither values
    for (int i = 0; i < MAX_BUFFER_LEN; ++i) {
        m_pDitherBuffer[i] = static_cast<float>(rand() % 32768) / 32768.0 - 0.5;
    }

    //zero out crossfade buffer
    SampleUtil::applyGain(m_pCrossFadeBuffer, 0.0, MAX_BUFFER_LEN);

    m_fLastSampleValue[0] = 0;
    m_fLastSampleValue[1] = 0;

    m_pReader = new CachingReader(_group, _config);
    connect(m_pReader, SIGNAL(trackLoading()),
            this, SLOT(slotTrackLoading()),
            Qt::DirectConnection);
    connect(m_pReader, SIGNAL(trackLoaded(TrackPointer, int, int)),
            this, SLOT(slotTrackLoaded(TrackPointer, int, int)),
            Qt::DirectConnection);
    connect(m_pReader, SIGNAL(trackLoadFailed(TrackPointer, QString)),
            this, SLOT(slotTrackLoadFailed(TrackPointer, QString)),
            Qt::DirectConnection);

    // Play button
    m_playButton = new ControlPushButton(ConfigKey(m_group, "play"));
    m_playButton->setButtonMode(ControlPushButton::TOGGLE);
    m_playButton->connectValueChangeRequest(
            this, SLOT(slotControlPlayRequest(double)),
            Qt::DirectConnection);

    //Play from Start Button (for sampler)
    m_playStartButton = new ControlPushButton(ConfigKey(m_group, "start_play"));
    connect(m_playStartButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlPlayFromStart(double)),
            Qt::DirectConnection);

    // Jump to start and stop button
    m_stopStartButton = new ControlPushButton(ConfigKey(m_group, "start_stop"));
    connect(m_stopStartButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlJumpToStartAndStop(double)),
            Qt::DirectConnection);

    //Stop playback (for sampler)
    m_stopButton = new ControlPushButton(ConfigKey(m_group, "stop"));
    connect(m_stopButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlStop(double)),
            Qt::DirectConnection);

    // Start button
    m_startButton = new ControlPushButton(ConfigKey(m_group, "start"));
    connect(m_startButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlStart(double)),
            Qt::DirectConnection);

    // End button
    m_endButton = new ControlPushButton(ConfigKey(m_group, "end"));
    connect(m_endButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlEnd(double)),
            Qt::DirectConnection);

    m_pSlipButton = new ControlPushButton(ConfigKey(m_group, "slip_enabled"));
    m_pSlipButton->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pSlipButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlSlip(double)),
            Qt::DirectConnection);
    connect(m_pSlipButton, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotControlSlip(double)),
            Qt::DirectConnection);

    // Actual rate (used in visuals, not for control)
    m_rateEngine = new ControlObject(ConfigKey(m_group, "rateEngine"));

    // BPM to display in the UI (updated more slowly than the actual bpm)
    m_visualBpm = new ControlObject(ConfigKey(m_group, "visual_bpm"));
    m_visualKey = new ControlObject(ConfigKey(m_group, "visual_key"));

    // Slider to show and change song position
    //these bizarre choices map conveniently to the 0-127 range of midi
    m_playposSlider = new ControlLinPotmeter(
        ConfigKey(m_group, "playposition"), kMinPlayposRange, kMaxPlayposRange);
    connect(m_playposSlider, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlSeek(double)),
            Qt::DirectConnection);

    // Control used to communicate ratio playpos to GUI thread
    m_visualPlayPos = VisualPlayPosition::getVisualPlayPosition(m_group);

    m_pRepeat = new ControlPushButton(ConfigKey(m_group, "repeat"));
    m_pRepeat->setButtonMode(ControlPushButton::TOGGLE);

    // Sample rate
    m_pSampleRate = new ControlObjectSlave("[Master]", "samplerate", this);

    m_pTrackSamples = new ControlObject(ConfigKey(m_group, "track_samples"));
    m_pTrackSampleRate = new ControlObject(ConfigKey(m_group, "track_samplerate"));

    m_pKeylock = new ControlPushButton(ConfigKey(m_group, "keylock"));
    m_pKeylock->setButtonMode(ControlPushButton::TOGGLE);
    m_pKeylock->set(false);

    m_pEject = new ControlPushButton(ConfigKey(m_group, "eject"));
    connect(m_pEject, SIGNAL(valueChanged(double)),
            this, SLOT(slotEjectTrack(double)),
            Qt::DirectConnection);

    // Quantization Controller for enabling and disabling the
    // quantization (alignment) of loop in/out positions and (hot)cues with
    // beats.
    addControl(new QuantizeControl(_group, _config));
    m_pQuantize = ControlObject::getControl(ConfigKey(_group, "quantize"));

    // Create the Loop Controller
    m_pLoopingControl = new LoopingControl(_group, _config);
    addControl(m_pLoopingControl);

    m_pSyncControl = new SyncControl(_group, _config, pChannel,
                                     pMixingEngine->getEngineSync());
    addControl(m_pSyncControl);

    m_pRateControl = new RateControl(_group, _config);
    // Add the Rate Controller
    addControl(m_pRateControl);
#ifdef __VINYLCONTROL__
    VinylControlControl *vcc = new VinylControlControl(_group, _config);
    addControl(vcc);
    m_pRateControl->setVinylControlControl(vcc);
#endif

    // Create the BPM Controller
    m_pBpmControl = new BpmControl(_group, _config);
    addControl(m_pBpmControl);

    // TODO(rryan) remove this dependence?
    m_pRateControl->setBpmControl(m_pBpmControl);
    m_pSyncControl->setEngineControls(m_pRateControl, m_pBpmControl);
    pMixingEngine->getEngineSync()->addSyncableDeck(m_pSyncControl);

    m_fwdButton = ControlObject::getControl(ConfigKey(_group, "fwd"));
    m_backButton = ControlObject::getControl(ConfigKey(_group, "back"));

    m_pKeyControl = new KeyControl(_group, _config);
    addControl(m_pKeyControl);

    // Create the clock controller
    m_pClockControl = new ClockControl(_group, _config);
    addControl(m_pClockControl);

    // Create the cue controller
    m_pCueControl = new CueControl(_group, _config);
    addControl(m_pCueControl);

    m_pReadAheadManager = new ReadAheadManager(m_pReader);
    m_pReadAheadManager->addEngineControl(m_pLoopingControl);
    m_pReadAheadManager->addEngineControl(m_pRateControl);

    // Construct scaling objects
    m_pScaleLinear = new EngineBufferScaleLinear(m_pReadAheadManager);
    m_pScaleST = new EngineBufferScaleST(m_pReadAheadManager);
    m_pScaleDummy = new EngineBufferScaleDummy(m_pReadAheadManager);
    m_pScaleRB = new EngineBufferScaleRubberBand(m_pReadAheadManager);
    enablePitchAndTimeScaling(false);

    //m_iRampIter = 0;
#ifdef __SCALER_DEBUG__
    df.setFileName("mixxx-debug.csv");
    df.open(QIODevice::WriteOnly | QIODevice::Text);
    writer.setDevice(&df);
#endif


    m_hintList.reserve(256); // Avoid reallocation
}

EngineBuffer::~EngineBuffer()
{
#ifdef __SCALER_DEBUG__
    //close the writer
    df.close();
#endif
    delete m_pReadAheadManager;
    delete m_pReader;

    delete m_playButton;
    delete m_playStartButton;
    delete m_stopStartButton;

    delete m_startButton;
    delete m_endButton;
    delete m_stopButton;
    delete m_rateEngine;
    delete m_playposSlider;
    delete m_visualBpm;
    delete m_visualKey;

    delete m_pSlipButton;
    delete m_pRepeat;

    delete m_pTrackSamples;
    delete m_pTrackSampleRate;

    delete m_pScaleLinear;
    delete m_pScaleDummy;
    delete m_pScaleST;
    delete m_pScaleRB;

    delete m_pKeylock;
    delete m_pEject;

    delete [] m_pDitherBuffer;
    delete [] m_pCrossFadeBuffer;

    while (m_engineControls.size() > 0) {
        EngineControl* pControl = m_engineControls.takeLast();
        delete pControl;
    }
}

double EngineBuffer::fractionalPlayposFromAbsolute(double absolutePlaypos) {
    double fFractionalPlaypos = 0.0;
    if (m_file_length_old != 0.) {
        fFractionalPlaypos = math_min(absolutePlaypos, m_file_length_old);
        fFractionalPlaypos /= m_file_length_old;
    } else {
        fFractionalPlaypos = 0.;
    }
    return fFractionalPlaypos;
}

void EngineBuffer::enablePitchAndTimeScaling(bool bEnable) {
    // MUST ACQUIRE THE PAUSE MUTEX BEFORE CALLING THIS METHOD

    // When no time-stretching or pitch-shifting is needed we use our own linear
    // interpolation code (EngineBufferScaleLinear). It is faster and sounds
    // much better for scratching.

    if (bEnable && m_pScale != m_pScaleRB) {
        m_pScale = m_pScaleRB;
        m_bScalerChanged = true;
    } else if (!bEnable && m_pScale != m_pScaleLinear) {
        m_pScale = m_pScaleLinear;
        m_bScalerChanged = true;
    }
}

double EngineBuffer::getBpm()
{
    return m_pBpmControl->getBpm();
}

double EngineBuffer::getFileBpm() {
    return m_pBpmControl->getFileBpm();
}

void EngineBuffer::setEngineMaster(EngineMaster * pEngineMaster) {
    m_engineLock.lock();
    foreach (EngineControl* pControl, m_engineControls) {
        pControl->setEngineMaster(pEngineMaster);
    }
    m_engineLock.unlock();
}

void EngineBuffer::queueNewPlaypos(double newpos) {
    // Temp Workaround: All seeks need to be done in the Engine thread so queue
    // it up.
    m_dQueuedPosition = newpos;
    m_bSeekQueued.fetchAndStoreRelease(1);
}

// WARNING: This method is not thread safe and must not be called from outside
// the engine callback!
void EngineBuffer::setNewPlaypos(double newpos) {
    //qDebug() << "engine new pos " << newpos;

    // Before seeking, read extra buffer for crossfading
    CSAMPLE* fadeout = m_pScale->getScaled(m_iLastBufferSize);
    m_iCrossFadeSamples = m_iLastBufferSize;
    SampleUtil::copyWithGain(m_pCrossFadeBuffer, fadeout, 1.0, m_iLastBufferSize);

    m_filepos_play = newpos;

    // Ensures that the playpos slider gets updated in next process call
    m_iSamplesCalculated = 1000000;

    // The right place to do this?
    if (m_pScale)
        m_pScale->clear();
    m_pReadAheadManager->notifySeek(m_filepos_play);

    // Must hold the engineLock while using m_engineControls
    m_engineLock.lock();
    for (QList<EngineControl*>::iterator it = m_engineControls.begin();
         it != m_engineControls.end(); it++) {
        EngineControl *pControl = *it;
        pControl->notifySeek(m_filepos_play);
    }
    m_engineLock.unlock();
}

const char * EngineBuffer::getGroup()
{
    return m_group;
}

double EngineBuffer::getRate()
{
    return m_rate_old;
}

// WARNING: Always called from the EngineWorker thread pool
void EngineBuffer::slotTrackLoading() {
    // Pause EngineBuffer from processing frames
    m_pause.lock();
    // Setting m_iTrackLoading inside a m_pause.lock ensures that
    // track buffer is not processed when starting to load a new one
    m_iTrackLoading = 1;
    m_pause.unlock();

    // Set play here, to signal the user that the play command is adopted
    m_playButton->set((double)m_bPlayAfterLoading);
    m_pTrackSamples->set(0); // Stop renderer
}

void EngineBuffer::loadFakeTrack() {
    TrackPointer pTrack(new TrackInfoObject(), &QObject::deleteLater);
    slotTrackLoaded(pTrack, 44100, 44100 * 10);
}

// WARNING: Always called from the EngineWorker thread pool
void EngineBuffer::slotTrackLoaded(TrackPointer pTrack,
                                   int iTrackSampleRate,
                                   int iTrackNumSamples) {
    m_pause.lock();
    m_visualPlayPos->setInvalid();
    m_pCurrentTrack = pTrack;
    m_file_srate_old = iTrackSampleRate;
    m_file_length_old = iTrackNumSamples;
    m_pTrackSamples->set(iTrackNumSamples);
    m_pTrackSampleRate->set(iTrackSampleRate);
    m_pause.unlock();

    // All EngingeControls are connected directly
    emit(trackLoaded(pTrack));
    // Start buffer processing after all EngineContols are up to date
    // with the current track e.g track is seeked to Cue
    m_iTrackLoading = 0;
}

// WARNING: Always called from the EngineWorker thread pool
void EngineBuffer::slotTrackLoadFailed(TrackPointer pTrack,
                                       QString reason) {
    m_iTrackLoading = 0;
    m_playButton->set(0.0f);
    ejectTrack();
    emit(trackLoadFailed(pTrack, reason));
}

TrackPointer EngineBuffer::getLoadedTrack() const {
    return m_pCurrentTrack;
}

void EngineBuffer::ejectTrack() {
    // Don't allow ejections while playing a track. We don't need to lock to
    // call ControlObject::get() so this is fine.
    if (m_playButton->get() > 0 || !m_pCurrentTrack) {
        return;
    }

    m_pause.lock();
    m_iTrackLoading = 0;
    m_pTrackSamples->set(0);
    m_pTrackSampleRate->set(0);
    TrackPointer pTrack = m_pCurrentTrack;
    m_pCurrentTrack.clear();
    m_file_srate_old = 0;
    m_file_length_old = 0;
    m_playButton->set(0.0);
    m_visualBpm->set(0.0);
    m_visualKey->set(0.0);
    slotControlSeek(0.);
    m_pause.unlock();

    emit(trackUnloaded(pTrack));
}

// WARNING: This method runs in both the GUI thread and the Engine Thread
void EngineBuffer::slotControlSeek(double change)
{
    if(isnan(change) || change > kMaxPlayposRange || change < kMinPlayposRange) {
        // This seek is ridiculous.
        return;
    }

    // Find new playpos, restrict to valid ranges.
    double new_playpos = round(change * m_file_length_old);

    // TODO(XXX) currently not limiting seeks file_length_old instead of
    // kMaxPlayposRange.
    if (new_playpos > m_file_length_old)
        new_playpos = m_file_length_old;

    // Ensure that the file position is even (remember, stereo channel files...)
    if (!even((int)new_playpos))
        new_playpos--;

    if (m_pQuantize->get() > 0.0) {
        int offset = static_cast<int>(m_pBpmControl->getPhaseOffset(new_playpos));
        if (!even(offset)) {
            offset--;
        }
        new_playpos += offset;
    }

    queueNewPlaypos(new_playpos);
}

// WARNING: This method runs from SyncWorker and Engine Worker
void EngineBuffer::slotControlSeekAbs(double abs)
{
    slotControlSeek(abs / m_file_length_old);
}

void EngineBuffer::slotControlPlayRequest(double v)
{
    if (v == 0.0 && m_pCueControl->isCuePreviewing()) {
        v = 1.0;
    }

    // If no track is currently loaded, turn play off. If a track is loading
    // allow the set since it might apply to a track we are loading due to the
    // asynchrony.

    if (v > 0.0 && !m_pCurrentTrack && deref(m_iTrackLoading) == 0) {
        v = 0.0;
    }

    // set and confirm must be called in any case to update the widget toggle state
    m_playButton->setAndConfirm(v);
}

void EngineBuffer::slotControlStart(double v)
{
    if (v > 0.0) {
        slotControlSeek(0.);
    }
}

void EngineBuffer::slotControlEnd(double v)
{
    if (v > 0.0) {
        slotControlSeek(1.);
    }
}

void EngineBuffer::slotControlPlayFromStart(double v)
{
    if (v > 0.0) {
        slotControlSeek(0.);
        m_playButton->set(1);
    }
}

void EngineBuffer::slotControlJumpToStartAndStop(double v)
{
    if (v > 0.0) {
        slotControlSeek(0.);
        m_playButton->set(0);
    }
}

void EngineBuffer::slotControlStop(double v)
{
    if (v > 0.0) {
        m_playButton->set(0);
    }
}

void EngineBuffer::slotControlSlip(double v)
{
    bool enabled = v > 0.0;
    if (enabled == m_bSlipEnabled) {
        return;
    }

    m_bSlipEnabled = enabled;

    if (enabled) {
        // TODO(rryan): Should this filepos instead be the RAMAN current
        // position? filepos_play could be out of date.
        m_dSlipPosition = m_filepos_play;
        m_dSlipRate = m_rate_old;
    } else {
        // TODO(owen) assuming that looping will get canceled properly
        slotControlSeekAbs(m_dSlipPosition);
        m_dSlipPosition = 0;
    }
}


void EngineBuffer::process(const CSAMPLE *, const CSAMPLE * pOut, const int iBufferSize)
{
    Q_ASSERT(even(iBufferSize));
    m_pReader->process();
    // Steps:
    // - Lookup new reader information
    // - Calculate current rate
    // - Scale the audio with m_pScale, copy the resulting samples into the
    //   output buffer
    // - Give EngineControl's a chance to do work / request seeks, etc
    // - Process repeat mode if we're at the end or beginning of a track
    // - Set last sample value (m_fLastSampleValue) so that rampOut works? Other
    //   miscellaneous upkeep issues.

    CSAMPLE * pOutput = (CSAMPLE *)pOut; // strip const attribute TODO(XXX): avoid this hack
    bool bCurBufferPaused = false;
    double rate = 0;

    bool bTrackLoading = deref(m_iTrackLoading) != 0;
    if (!bTrackLoading && m_pause.tryLock()) {
        ScopedTimer t("EngineBuffer::process_pauselock");
        float sr = m_pSampleRate->get();

        double baserate = 0.0f;
        if (sr > 0) {
            baserate = ((double)m_file_srate_old / sr);
        }

        bool paused = m_playButton->get() != 0.0f ? false : true;
        bool is_scratching = false;
        bool keylock_enabled = m_pKeylock->get() > 0;

        // speed is the percentage change in player speed. Depending on whether
        // keylock is enabled, this is applied to either the rate or the tempo.
        double speed = m_pRateControl->calculateRate(
            baserate, paused, iBufferSize, &is_scratching);
        double pitch = m_pKeyControl->getPitchAdjustOctaves();

        // Update the slipped position
        if (m_bSlipEnabled) {
            m_dSlipPosition += static_cast<double>(iBufferSize) * m_dSlipRate;
        }

        // If either keylock is enabled or the pitch slider is non-zero then we
        // need to use pitch and time scaling. Scratching always disables
        // keylock because keylock sounds terrible when not going at a constant
        // rate.
        bool use_pitch_and_time_scaling = !is_scratching && (keylock_enabled || pitch != 0);
        enablePitchAndTimeScaling(use_pitch_and_time_scaling);

        if (m_bSeekQueued.testAndSetAcquire(1, 0)) {
            setNewPlaypos(m_dQueuedPosition);
        }

        // If the baserate, rate, or pitch has changed, we need to update the
        // scaler. Also, if we have changed scalers then we need to update the
        // scaler.
        if (baserate != m_baserate_old || speed != m_speed_old ||
            pitch != m_pitch_old || m_bScalerChanged) {
            // The rate returned by the scale object can be different from the
            // wanted rate!  Make sure new scaler has proper position. This also
            // crossfades between the old scaler and new scaler to prevent
            // clicks.
            if (m_bScalerChanged) {
                setNewPlaypos(m_filepos_play);
            } else if (m_pScale != m_pScaleLinear) { // linear scaler does this part for us now
                //XXX: Trying to force RAMAN to read from correct
                //     playpos when rate changes direction - Albert
                if ((m_speed_old <= 0 && speed > 0) ||
                    (m_speed_old >= 0 && speed < 0)) {
                    setNewPlaypos(m_filepos_play);
                }
            }

            // At this point, rate is baserate multiplied by the speed
            // adjustment, or the speed above normal that the engine should play
            // the track. Baserate accounts for re-sampling the source audio to
            // match our output sample rate (file_samplerate /
            // master_samplerate).
            //
            // The rate adjustment in percentage of rate (1.0 being normal
            // rate).
            double rate_adjust = speed * baserate;
            // The tempo adjustment in percentage of tempo (1.0 being normal
            // tempo).
            double tempo_adjust = 1.0;
            // The pitch adjustment in percentage of octaves (0.0 being normal
            // pitch. 1.0 is a full octave shift up).
            double pitch_adjust = pitch;

            if (keylock_enabled && !is_scratching) {
                // If keylock is enabled, then we need to take the speed
                // adjustment that is currently built into the rate and instead
                // control the tempo by that amount.
                rate_adjust = baserate;
                // Protect against division by 0.
                tempo_adjust = speed;
            }

            m_pScale->setScaleParameters(m_pSampleRate->get(),
                                         &rate_adjust,
                                         &tempo_adjust,
                                         &pitch_adjust);

            m_baserate_old = baserate;
            m_speed_old = speed;
            m_pitch_old = pitch;

            // The way we treat rate inside of EngineBuffer is actually a
            // description of "sample consumption rate" or percentage of samples
            // consumed relative to playing back the track at its native sample
            // rate and normal speed. pitch_adjust does not change the playback
            // rate.
            m_rate_old = rate = rate_adjust * tempo_adjust;

            // Scaler is up to date now.
            m_bScalerChanged = false;
        } else {
            // Scaler did not need updating. By definition this means we are at
            // our old rate.
            rate = m_rate_old;
        }

        bool at_start = m_filepos_play <= 0;
        bool at_end = m_filepos_play >= m_file_length_old;
        bool backwards = rate < 0;

        // If we're playing past the end, playing before the start, or standing
        // still then by definition the buffer is paused.
        bCurBufferPaused = (rate == 0 || (at_end && !backwards));

        // If the buffer is not paused, then scale the audio.
        if (!bCurBufferPaused) {
            // The fileposition should be: (why is this thing a double anyway!?
            // Integer valued.
            double filepos_play_rounded = round(m_filepos_play);
            if (filepos_play_rounded != m_filepos_play) {
                qWarning() << __FILE__ << __LINE__ << "ERROR: filepos_play is not round:" << m_filepos_play;
                m_filepos_play = filepos_play_rounded;
            }

            // Even.
            if (!even(m_filepos_play)) {
                qWarning() << "ERROR: filepos_play is not even:" << m_filepos_play;
                m_filepos_play--;
            }

            // Perform scaling of Reader buffer into buffer.
            CSAMPLE* output = m_pScale->getScaled(iBufferSize);
            double samplesRead = m_pScale->getSamplesRead();

            // qDebug() << "sourceSamples used " << iSourceSamples
            //          <<" samplesRead " << samplesRead
            //          << ", buffer pos " << iBufferStartSample
            //          << ", play " << filepos_play
            //          << " bufferlen " << iBufferSize;

            // Copy scaled audio into pOutput
            memcpy(pOutput, output, sizeof(pOutput[0]) * iBufferSize);

            if (m_bScalerOverride) {
                // If testing, we don't have a real log so we fake the position.
                m_filepos_play += samplesRead;
            } else {
                // Adjust filepos_play by the amount we processed. TODO(XXX) what
                // happens if samplesRead is a fraction?
                m_filepos_play =
                        m_pReadAheadManager->getEffectiveVirtualPlaypositionFromLog(
                                static_cast<int>(m_filepos_play), samplesRead);
            }
        }

        //Crossfade if we just did a seek
        if (m_iCrossFadeSamples > 0) {
            int i = 0;
            double cross_len = 0;
            if (m_iCrossFadeSamples >= iBufferSize) {
                i = m_iCrossFadeSamples - iBufferSize;
                cross_len = static_cast<double>(iBufferSize) / 2.0;
            } else {
                cross_len = static_cast<double>(m_iCrossFadeSamples) / 2.0;
            }

            double cross_mix = 0.0;
            double cross_inc = 1.0 / cross_len;
            // Do crossfade from old fadeout buffer to this new data
            for (int j = 0; j + 1 < iBufferSize && i + 1 < m_iCrossFadeSamples; i += 2, j += 2) {
                pOutput[j] = pOutput[j] * cross_mix + m_pCrossFadeBuffer[i] * (1.0 - cross_mix);
                pOutput[j+1] = pOutput[j+1] * cross_mix + m_pCrossFadeBuffer[i+1] * (1.0 - cross_mix);
                cross_mix += cross_inc;
            }
            m_iCrossFadeSamples = 0;
        }

        m_engineLock.lock();
        QListIterator<EngineControl*> it(m_engineControls);
        while (it.hasNext()) {
            EngineControl* pControl = it.next();
            pControl->setCurrentSample(m_filepos_play, m_file_length_old);
            pControl->process(rate, m_filepos_play, m_file_length_old, iBufferSize);
        }
        m_engineLock.unlock();

        // Report our speed to SyncControl. If we are the master then it will
        // broadcast this update to followers.
        m_pSyncControl->reportPlayerSpeed(speed, is_scratching);

        // Update all the indicators that EngineBuffer publishes to allow
        // external parts of Mixxx to observe its status.
        updateIndicators(speed, iBufferSize);

        // Handle repeat mode
        at_start = m_filepos_play <= 0;
        at_end = m_filepos_play >= m_file_length_old;

        bool repeat_enabled = m_pRepeat->get() != 0.0f;

        bool end_of_track = //(at_start && backwards) ||
            (at_end && !backwards);

        // If playbutton is pressed, check if we are at start or end of track
        if ((m_playButton->get() || (m_fwdButton->get() || m_backButton->get()))
                && end_of_track) {
            if (repeat_enabled) {
                double seekPosition = at_start ? m_file_length_old : 0;
                slotControlSeek(seekPosition);
            } else {
                m_playButton->set(0.);
            }
        }

        // release the pauselock
        m_pause.unlock();
    } else { // if (!bTrackLoading && m_pause.tryLock()) {
        // If we can't get the pause lock then this buffer will be silence.
        bCurBufferPaused = true;

        // We are stopped. Report a speed of 0 to SyncControl.
        m_pSyncControl->reportPlayerSpeed(0.0, false);
    }

    if (!bTrackLoading) {
        // Give the Reader hints as to which chunks of the current song we
        // really care about. It will try very hard to keep these in memory
        hintReader(rate);
    }

    const double kSmallRate = 0.005;
    if (m_bLastBufferPaused && !bCurBufferPaused) {
        if (fabs(rate) > kSmallRate) { //at very slow forward rates, don't ramp up
            m_iRampState = ENGINE_RAMP_UP;
        }
    } else if (!m_bLastBufferPaused && bCurBufferPaused) {
        m_iRampState = ENGINE_RAMP_DOWN;
    } else { //we are not changing state
        // Make sure we aren't accidentally ramping down. This is how we make
        // sure that ramp value will become 1.0 eventually.
        //
        // 9/2012 rryan -- As I understand it this code intends to prevent us
        // from getting stuck ramped down. If there is a meaningfully large rate
        // and we aren't ramped up completely then it makes us ramp up. This
        // causes crazy feedback if you scratch at the non-silent end of a
        // track. See Bug #1006111. I added a !bCurBufferPaused term here because
        // if rate > 0 and bCurBufferPaused then basically you are at the end of
        // the track and trying to jog forward so this uniquely blocks that
        // situation.
        if (fabs(rate) > kSmallRate && !bCurBufferPaused &&
            m_iRampState != ENGINE_RAMP_UP && m_fRampValue < 1.0) {
            m_iRampState = ENGINE_RAMP_UP;
        }
    }

    //let's try holding the last sample value constant, and pull it
    //towards zero
    float ramp_inc = 0;
    if (m_iRampState == ENGINE_RAMP_UP ||
        m_iRampState == ENGINE_RAMP_DOWN) {
        // Ramp of 3.33 ms
        ramp_inc = m_iRampState * 300 / m_pSampleRate->get();

        for (int i=0; i < iBufferSize; i += 2) {
            if (bCurBufferPaused) {
                float dither = m_pDitherBuffer[m_iDitherBufferReadIndex];
                m_iDitherBufferReadIndex = (m_iDitherBufferReadIndex + 1) % MAX_BUFFER_LEN;
                pOutput[i] = m_fLastSampleValue[0] * m_fRampValue + dither;
                pOutput[i+1] = m_fLastSampleValue[1] * m_fRampValue + dither;
            } else {
                pOutput[i] = pOutput[i] * m_fRampValue;
                pOutput[i+1] = pOutput[i+1] * m_fRampValue;
            }

            m_fRampValue += ramp_inc;
            if (m_fRampValue >= 1.0) {
                m_iRampState = ENGINE_RAMP_NONE;
                m_fRampValue = 1.0;
            } else if (m_fRampValue <= 0.0) {
                m_iRampState = ENGINE_RAMP_NONE;
                m_fRampValue = 0.0;
            }
        }
    } else if (m_fRampValue == 0.0) {
        SampleUtil::applyGain(pOutput, 0.0, iBufferSize);
    }

    if ((!bCurBufferPaused && m_iRampState == ENGINE_RAMP_NONE) ||
        (bCurBufferPaused && m_fRampValue == 0.0)) {
        m_fLastSampleValue[0] = pOutput[iBufferSize-2];
        m_fLastSampleValue[1] = pOutput[iBufferSize-1];
    }

#ifdef __SCALER_DEBUG__
    for (int i=0; i<iBufferSize; i+=2) {
        writer << pOutput[i] <<  "\n";
    }
#endif

    m_bLastBufferPaused = bCurBufferPaused;
    m_iLastBufferSize = iBufferSize;
}

void EngineBuffer::updateIndicators(double speed, int iBufferSize) {

    // Increase samplesCalculated by the buffer size
    m_iSamplesCalculated += iBufferSize;

    double fFractionalPlaypos = fractionalPlayposFromAbsolute(m_filepos_play);
    if(speed > 0 && fFractionalPlaypos == 1.0) {
        speed = 0;
    }

    // Report fractional playpos to SyncControl.
    // TODO(rryan) It's kind of hacky that this is in updateIndicators but it
    // prevents us from computing fFractionalPlaypos multiple times per
    // EngineBuffer::process().
    m_pSyncControl->reportTrackPosition(fFractionalPlaypos);

    // Update indicators that are only updated after every
    // sampleRate/kiUpdateRate samples processed.
    if (m_iSamplesCalculated > (m_pSampleRate->get()/kiUpdateRate)) {
        m_playposSlider->set(fFractionalPlaypos);

        if (speed != m_rateEngine->get()) {
            m_rateEngine->set(speed);
        }

        //Update the BPM even more slowly
        m_iUiSlowTick = (m_iUiSlowTick + 1) % kiBpmUpdateRate;
        if (m_iUiSlowTick == 0) {
            m_visualBpm->set(m_pBpmControl->getBpm());
        }
        m_visualKey->set(m_pKeyControl->getKey());

        // Reset sample counter
        m_iSamplesCalculated = 0;
    }

    // Update visual control object, this needs to be done more often than the
    // rateEngine and playpos slider
    m_visualPlayPos->set(fFractionalPlaypos, speed,
            (double)iBufferSize/m_file_length_old,
            fractionalPlayposFromAbsolute(m_dSlipPosition));
}

void EngineBuffer::hintReader(const double dRate) {
    m_engineLock.lock();

    m_hintList.clear();
    m_pReadAheadManager->hintReader(dRate, &m_hintList);

    //if slipping, hint about virtual position so we're ready for it
    if (m_bSlipEnabled) {
        Hint hint;
        hint.length = 2048; //default length please
        hint.sample = m_dSlipRate >= 0 ? m_dSlipPosition : m_dSlipPosition - 2048;
        hint.priority = 1;
        m_hintList.append(hint);
    }

    QListIterator<EngineControl*> it(m_engineControls);
    while (it.hasNext()) {
        EngineControl* pControl = it.next();
        pControl->hintReader(&m_hintList);
    }
    m_pReader->hintAndMaybeWake(m_hintList);

    m_engineLock.unlock();
}

// WARNING: This method runs in the GUI thread
void EngineBuffer::slotLoadTrack(TrackPointer pTrack, bool play) {
    // Signal to the reader to load the track. The reader will respond with
    // trackLoading and then either with trackLoaded or trackLoadFailed signals.
    m_bPlayAfterLoading = play;
    m_pReader->newTrack(pTrack);
}

void EngineBuffer::addControl(EngineControl* pControl) {
    // Connect to signals from EngineControl here...
    m_engineLock.lock();
    m_engineControls.push_back(pControl);
    m_engineLock.unlock();
    pControl->setEngineBuffer(this);
    connect(this, SIGNAL(trackLoaded(TrackPointer)),
            pControl, SLOT(trackLoaded(TrackPointer)),
            Qt::DirectConnection);
    connect(this, SIGNAL(trackUnloaded(TrackPointer)),
            pControl, SLOT(trackUnloaded(TrackPointer)),
            Qt::DirectConnection);
}

void EngineBuffer::bindWorkers(EngineWorkerScheduler* pWorkerScheduler) {
    m_pReader->setScheduler(pWorkerScheduler);
}

bool EngineBuffer::isTrackLoaded() {
    if (m_pCurrentTrack) {
        return true;
    }
    return false;
}

void EngineBuffer::slotEjectTrack(double v) {
    if (v > 0) {
        ejectTrack();
    }
}

double EngineBuffer::getVisualPlayPos() {
    return m_visualPlayPos->getEnginePlayPos();
}

double EngineBuffer::getTrackSamples() {
    return m_pTrackSamples->get();
}

/*
void EngineBuffer::setReader(CachingReader* pReader) {
    disconnect(m_pReader, 0, this, 0);
    delete m_pReader;
    m_pReader = pReader;
    m_pReadAheadManager->setReader(pReader);
    connect(m_pReader, SIGNAL(trackLoading()),
            this, SLOT(slotTrackLoading()),
            Qt::DirectConnection);
    connect(m_pReader, SIGNAL(trackLoaded(TrackPointer, int, int)),
            this, SLOT(slotTrackLoaded(TrackPointer, int, int)),
            Qt::DirectConnection);
    connect(m_pReader, SIGNAL(trackLoadFailed(TrackPointer, QString)),
            this, SLOT(slotTrackLoadFailed(TrackPointer, QString)),
            Qt::DirectConnection);
}
*/

void EngineBuffer::setScalerForTest(EngineBufferScale* pScale) {
    m_pScale = pScale;
    // This bool is permanently set and can't be undone.
    m_bScalerOverride = true;
}
