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

#include <QEvent>
#include <QtDebug>

#include "engine/enginebuffer.h"
#include "cachingreader.h"
#include "sampleutil.h"

#include "controlpushbutton.h"
#include "configobject.h"
#include "controlpotmeter.h"
#include "engine/enginebufferscalest.h"
#include "engine/enginebufferscalelinear.h"
#include "engine/enginebufferscaledummy.h"
#include "mathstuff.h"
#include "engine/engineworkerscheduler.h"
#include "engine/readaheadmanager.h"
#include "engine/enginecontrol.h"
#include "engine/loopingcontrol.h"
#include "engine/ratecontrol.h"
#include "engine/bpmcontrol.h"
#include "engine/quantizecontrol.h"

#ifdef __VINYLCONTROL__
#include "engine/vinylcontrolcontrol.h"
#endif

#include "trackinfoobject.h"

#ifdef _MSC_VER
#include <float.h>  // for _isnan() on VC++
#define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#else
#include <math.h>  // for isnan() everywhere else
#endif

const double kMaxPlayposRange = 1.14;
const double kMinPlayposRange = -0.14;

EngineBuffer::EngineBuffer(const char * _group, ConfigObject<ConfigValue> * _config) :
    m_engineLock(QMutex::Recursive),
    m_group(_group),
    m_pConfig(_config),
    m_pLoopingControl(NULL),
    m_pRateControl(NULL),
    m_pBpmControl(NULL),
    m_pReadAheadManager(NULL),
    m_pReader(NULL),
    m_filepos_play(0.),
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
    m_bScalerChanged(false),
    m_bLastBufferPaused(true),
    m_iTrackLoading(0),
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
    connect(m_playButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlPlay(double)),
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

    // Slider to show and change song position
    //these bizarre choices map conveniently to the 0-127 range of midi
    m_playposSlider = new ControlPotmeter(
        ConfigKey(m_group, "playposition"), kMinPlayposRange, kMaxPlayposRange);
    connect(m_playposSlider, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlSeek(double)),
            Qt::DirectConnection);

    // Control used to communicate ratio playpos to GUI thread
    m_visualPlaypos = new ControlPotmeter(
        ConfigKey(m_group, "visual_playposition"), kMinPlayposRange, kMaxPlayposRange);

    m_pRepeat = new ControlPushButton(ConfigKey(m_group, "repeat"));
    m_pRepeat->setButtonMode(ControlPushButton::TOGGLE);

    // Sample rate
    m_pSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));

    m_pTrackSamples = new ControlObject(ConfigKey(m_group, "track_samples"));
    m_pTrackSampleRate = new ControlObject(ConfigKey(m_group, "track_samplerate"));

    // Quantization Controller for enabling and disabling the
    // quantization (alignment) of loop in/out positions and (hot)cues with
    // beats.
    addControl(new QuantizeControl(_group, _config));

    // Create the Loop Controller
    m_pLoopingControl = new LoopingControl(_group, _config);
    addControl(m_pLoopingControl);

#ifdef __VINYLCONTROL__
    // If VinylControl is enabled, add a VinylControlControl. This must be done
    // before RateControl is created.
    addControl(new VinylControlControl(m_group, _config));
#endif

    // Create the Rate Controller
    m_pRateControl = new RateControl(_group, _config);
    addControl(m_pRateControl);

    m_fwdButton = ControlObject::getControl(ConfigKey(_group, "fwd"));
    m_backButton = ControlObject::getControl(ConfigKey(_group, "back"));

    // Create the BPM Controller
    m_pBpmControl = new BpmControl(_group, _config);
    addControl(m_pBpmControl);

    m_pReadAheadManager = new ReadAheadManager(m_pReader);
    m_pReadAheadManager->addEngineControl(m_pLoopingControl);
    m_pReadAheadManager->addEngineControl(m_pRateControl);

    // Construct scaling objects
    m_pScaleLinear = new EngineBufferScaleLinear(m_pReadAheadManager);

    m_pScaleST = new EngineBufferScaleST(m_pReadAheadManager);
    //m_pScaleST = (EngineBufferScaleST*)new EngineBufferScaleDummy(m_pReadAheadManager);
    setPitchIndpTimeStretch(false); // default to VE, let the user specify PITS in their mix

    setNewPlaypos(0.);

    m_pKeylock = new ControlPushButton(ConfigKey(m_group, "keylock"));
    m_pKeylock->setButtonMode(ControlPushButton::TOGGLE);
    m_pKeylock->set(false);

    m_pEject = new ControlPushButton(ConfigKey(m_group, "eject"));
    connect(m_pEject, SIGNAL(valueChanged(double)),
            this, SLOT(slotEjectTrack(double)),
            Qt::DirectConnection);

    //m_iRampIter = 0;
#ifdef __SCALER_DEBUG__
    df.setFileName("mixxx-debug.csv");
    df.open(QIODevice::WriteOnly | QIODevice::Text);
    writer.setDevice(&df);
#endif
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
    delete m_visualPlaypos;
    delete m_visualBpm;

    delete m_pSlipButton;
    delete m_pSlipPosition;
    delete m_pRepeat;

    delete m_pTrackSamples;
    delete m_pTrackSampleRate;

    delete m_pScaleLinear;
    delete m_pScaleST;

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

void EngineBuffer::setPitchIndpTimeStretch(bool b)
{
    // MUST ACQUIRE THE PAUSE MUTEX BEFORE CALLING THIS METHOD

    // Change sound scale mode

    //SoundTouch's linear interpolation code doesn't sound very good.
    //Our own EngineBufferScaleLinear sounds slightly better, but it's
    //not working perfectly. Eventually we should have our own working
    //better, so scratching sounds good.

    //Update Dec 30/2007
    //If we delete the m_pScale object and recreate it, it eventually
    //causes some weird bad pointer somewhere, which will either cause
    //the waveform the roll in a weird way or fire an ASSERT from
    //visualchannel.cpp or something. Need to valgrind this or something.

    if (b == true) {
        m_pScale = m_pScaleST;
        ((EngineBufferScaleST *)m_pScaleST)->setPitchIndpTimeStretch(b);
    } else {
        m_pScale = m_pScaleLinear;
    }
    m_bScalerChanged = true;
}

double EngineBuffer::getBpm()
{
    return m_pBpmControl->getBpm();
}

double EngineBuffer::getFileBpm() {
    return m_pBpmControl->getFileBpm();
}

void EngineBuffer::setEngineMaster(EngineMaster * pEngineMaster)
{
    m_pBpmControl->setEngineMaster(pEngineMaster);
}

void EngineBuffer::setNewPlaypos(double newpos)
{
    //qDebug() << "engine new pos " << newpos;

    // Before seeking, read extra buffer for crossfading
    CSAMPLE* fadeout = m_pScale->scale(0,
                                       m_iLastBufferSize,
                                       0,
                                       0);
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

    m_playButton->set(0.0); //Stop playback
    m_pTrackSamples->set(0); // stop renderer
}

// WARNING: Always called from the EngineWorker thread pool
void EngineBuffer::slotTrackLoaded(TrackPointer pTrack,
                                   int iTrackSampleRate,
                                   int iTrackNumSamples) {
    m_pause.lock();
    m_visualPlaypos->set(-1);
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
    if (m_playButton->get() > 0)
        return;

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

    setNewPlaypos(new_playpos);
}

// WARNING: This method runs from SyncWorker and Engine Worker
void EngineBuffer::slotControlSeekAbs(double abs)
{
    slotControlSeek(abs / m_file_length_old);
}

void EngineBuffer::slotControlPlay(double v)
{
    // If no track is currently loaded, turn play off. If a track is loading
    // allow the set since it might apply to a track we are loading due to the
    // asynchrony.
    if (v > 0.0 && !m_pCurrentTrack && m_iTrackLoading == 0) {
        m_playButton->set(0.0f);
    }
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
        m_pSlipPosition->set(fractionalPlayposFromAbsolute(m_dSlipPosition));
        m_dSlipRate = m_rate_old;
    } else {
        // TODO(owen) assuming that looping will get canceled properly
        slotControlSeekAbs(m_dSlipPosition);
        m_dSlipPosition = 0;
        m_pSlipPosition->set(0);
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

    bool bTrackLoading = m_iTrackLoading != 0;
    if (!bTrackLoading && m_pause.tryLock()) {
        float sr = m_pSampleRate->get();

        double baserate = 0.0f;
        if (sr > 0)
            baserate = ((double)m_file_srate_old / sr);

        bool paused = m_playButton->get() != 0.0f ? false : true;

        bool is_scratching = false;
        rate = m_pRateControl->calculateRate(baserate, paused, iBufferSize,
                                             &is_scratching);

        //qDebug() << "rate" << rate << " paused" << paused;

        // Update the slipped position
        if (m_bSlipEnabled) {
            m_dSlipPosition += static_cast<double>(iBufferSize) * m_dSlipRate;
            m_pSlipPosition->set(fractionalPlayposFromAbsolute(m_dSlipPosition));
        }

        // Scratching always disables keylock because keylock sounds terrible
        // when not going at a constant rate.
        if (is_scratching && m_pScale != m_pScaleLinear) {
            setPitchIndpTimeStretch(false);
        } else if (!is_scratching) {
            if (m_pKeylock->get() && m_pScale != m_pScaleST) {
                setPitchIndpTimeStretch(true);
            } else if (!m_pKeylock->get() && m_pScale == m_pScaleST) {
                setPitchIndpTimeStretch(false);
            }
        }

        // If the rate has changed, set it in the scale object
        if (rate != m_rate_old || m_bScalerChanged) {
            // The rate returned by the scale object can be different from the wanted rate!
            // Make sure new scaler has proper position
            if (m_bScalerChanged) {
                setNewPlaypos(m_filepos_play);
            } else if (m_pScale != m_pScaleLinear) { //linear scaler does this part for us now
                //XXX: Trying to force RAMAN to read from correct
                //     playpos when rate changes direction - Albert
                if ((m_rate_old <= 0 && rate > 0) ||
                    (m_rate_old >= 0 && rate < 0)) {
                    setNewPlaypos(m_filepos_play);
                }
            }

            if (baserate > 0) //Prevent division by 0
                rate = baserate*m_pScale->setTempo(rate/baserate);
            m_pScale->setBaseRate(baserate);
            m_rate_old = rate;
            // Scaler is up to date now.
            m_bScalerChanged = false;
        }

        bool at_start = m_filepos_play <= 0;
        bool at_end = m_filepos_play >= m_file_length_old;
        bool backwards = rate < 0;

        // If we're playing past the end, playing before the start, or standing
        // still then by definition the buffer is paused.
        bCurBufferPaused = rate == 0 ||
            //(at_start && backwards) ||
            (at_end && !backwards);

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
            CSAMPLE* output = m_pScale->scale(0,
                                     iBufferSize,
                                     0,
                                     0);
            double samplesRead = m_pScale->getNewPlaypos();

            // qDebug() << "sourceSamples used " << iSourceSamples
            //          <<" samplesRead " << samplesRead
            //          << ", buffer pos " << iBufferStartSample
            //          << ", play " << filepos_play
            //          << " bufferlen " << iBufferSize;

            // Copy scaled audio into pOutput
            memcpy(pOutput, output, sizeof(pOutput[0]) * iBufferSize);

            // Adjust filepos_play by the amount we processed. TODO(XXX) what
            // happens if samplesRead is a fraction?
            m_filepos_play =
                    m_pReadAheadManager->getEffectiveVirtualPlaypositionFromLog(
                        static_cast<int>(m_filepos_play), samplesRead);
        } // else (bCurBufferPaused)

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
            double control_seek = pControl->process(rate, m_filepos_play,
                                                    m_file_length_old, iBufferSize);

            if (control_seek != kNoTrigger) {
                // If we have not processed loops by this point then we have a
                // bug. RAMAN should be in charge of taking loops now. This
                // final step is more to notify all the EngineControls of the
                // happenings of the engine. TODO(rryan) log condition to a
                // stats-pipe once we have them.

                m_filepos_play = control_seek;
                double filepos_play_rounded = round(m_filepos_play);
                if (filepos_play_rounded != m_filepos_play) {
                    qWarning() << __FILE__ << __LINE__ << "ERROR: filepos_play is not round:" << m_filepos_play;
                    m_filepos_play = filepos_play_rounded;
                }

                // Fix filepos_play so that it is not out of bounds.
                if (m_file_length_old > 0) {
                    if (m_filepos_play > m_file_length_old) {
                        // TODO(XXX) limit to kMaxPlayposRange instead of file_length_old
                        m_filepos_play = m_file_length_old;
                    } else if(m_filepos_play < m_file_length_old * kMinPlayposRange) {
                        m_filepos_play = kMinPlayposRange * m_file_length_old;
                    }
                }

                // Safety check that the EngineControl didn't pass us a bogus
                // value
                if (!even(m_filepos_play))
                    m_filepos_play--;

                // TODO(XXX) need to re-evaluate this later. If we
                // setNewPlaypos, that clear()'s soundtouch, which might screw
                // up the audio. This sort of jump is a normal event. Also, the
                // EngineControl which caused this jump will get a notifySeek
                // for the same jump which might be confusing. For 1.8.0
                // purposes this works fine. If we do not notifySeek the RAMAN,
                // the engine and RAMAN can get out of sync.

                //setNewPlaypos(filepos_play);
                m_pReadAheadManager->notifySeek(m_filepos_play);
                // Notify seek the rate control since it needs to track things
                // like looping. Hacky, I know, but this helps prevent things
                // like the scratch controller from flipping out.
                m_pRateControl->notifySeek(m_filepos_play);
            }
        }
        m_engineLock.unlock();


        // Update all the indicators that EngineBuffer publishes to allow
        // external parts of Mixxx to observe its status.
        updateIndicators(rate, iBufferSize);

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
    }

    // Give the Reader hints as to which chunks of the current song we
    // really care about. It will try very hard to keep these in memory
    hintReader(rate);

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
        ramp_inc = m_iRampState * 300 / m_pSampleRate->get();

        for (int i=0; i<iBufferSize; i+=2) {
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
            }
            if (m_fRampValue <= 0.0) {
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

void EngineBuffer::updateIndicators(double rate, int iBufferSize) {

    // Increase samplesCalculated by the buffer size
    m_iSamplesCalculated += iBufferSize;

    double fFractionalPlaypos = fractionalPlayposFromAbsolute(m_filepos_play);

    // Update indicators that are only updated after every
    // sampleRate/kiUpdateRate samples processed.  (e.g. playposSlider,
    // rateEngine)
    if (m_iSamplesCalculated > (m_pSampleRate->get()/kiUpdateRate)) {
        m_playposSlider->set(fFractionalPlaypos);

        if(rate != m_rateEngine->get())
            m_rateEngine->set(rate);

        //Update the BPM even more slowly
        m_iUiSlowTick = (m_iUiSlowTick + 1) % kiBpmUpdateRate;
        if (m_iUiSlowTick == 0) {
            m_visualBpm->set(m_pBpmControl->getBpm());
        }

        // Reset sample counter
        m_iSamplesCalculated = 0;
    }

    // Update visual control object, this needs to be done more often than the
    // rateEngine and playpos slider
    m_visualPlaypos->set(fFractionalPlaypos);
}

void EngineBuffer::hintReader(const double dRate) {
    m_engineLock.lock();

    m_hintList.clear();
    m_pReadAheadManager->hintReader(dRate, m_hintList);

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
        pControl->hintReader(m_hintList);
    }
    m_pReader->hintAndMaybeWake(m_hintList);

    m_engineLock.unlock();
}

// WARNING: This method runs in the GUI thread
void EngineBuffer::slotLoadTrack(TrackPointer pTrack) {
    // Signal to the reader to load the track. The reader will respond with
    // trackLoading and then either with trackLoaded or trackLoadFailed signals.
    m_pReader->newTrack(pTrack);
    m_pReader->wake();
}

void EngineBuffer::addControl(EngineControl* pControl) {
    // Connect to signals from EngineControl here...
    m_engineLock.lock();
    m_engineControls.push_back(pControl);
    m_engineLock.unlock();
    connect(pControl, SIGNAL(seek(double)),
            this, SLOT(slotControlSeek(double)),
            Qt::DirectConnection);
    connect(pControl, SIGNAL(seekAbs(double)),
            this, SLOT(slotControlSeekAbs(double)),
            Qt::DirectConnection);
    connect(this, SIGNAL(trackLoaded(TrackPointer)),
            pControl, SLOT(trackLoaded(TrackPointer)),
            Qt::DirectConnection);
    connect(this, SIGNAL(trackUnloaded(TrackPointer)),
            pControl, SLOT(trackUnloaded(TrackPointer)),
            Qt::DirectConnection);
}

void EngineBuffer::bindWorkers(EngineWorkerScheduler* pWorkerScheduler) {
    pWorkerScheduler->bindWorker(m_pReader);
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
