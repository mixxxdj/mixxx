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

#include "enginebuffer.h"
#include "cachingreader.h"

#include "controlpushbutton.h"
#include "controlobjectthreadmain.h"
#include "configobject.h"
#include "controlpotmeter.h"
#include "enginebufferscalest.h"
#include "enginebufferscalelinear.h"
#include "enginebufferscalereal.h"
#include "enginebufferscaledummy.h"
#include "mathstuff.h"
#include "engine/engineworkerscheduler.h"
#include "engine/readaheadmanager.h"
#include "engine/enginecontrol.h"
#include "loopingcontrol.h"
#include "ratecontrol.h"
#include "bpmcontrol.h"

#include "trackinfoobject.h"

#ifdef _MSC_VER
#include <float.h>  // for _isnan() on VC++
#define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#else
#include <math.h>  // for isnan() everywhere else
#endif


EngineBuffer::EngineBuffer(const char * _group, ConfigObject<ConfigValue> * _config) :
    m_engineLock(QMutex::Recursive),
    group(_group),
    m_pConfig(_config),
    m_pLoopingControl(NULL),
    m_pRateControl(NULL),
    m_pBpmControl(NULL),
    m_pReadAheadManager(NULL),
    m_pOtherEngineBuffer(NULL),
    m_pReader(NULL),
    filepos_play(0.),
    rate_old(0.),
    file_length_old(-1),
    file_srate_old(0),
    m_iSamplesCalculated(0),
    m_dAbsPlaypos(0.),
    m_pTrackEnd(NULL),
    m_pTrackEndMode(NULL),
    startButton(NULL),
    endButton(NULL),
    m_pScale(NULL),
    m_pScaleLinear(NULL),
    m_pScaleST(NULL),
    m_bScalerChanged(false),
    m_fLastSampleValue(0.),
    m_bLastBufferPaused(true),
    m_bResetPitchIndpTimeStretch(true) {

    // Play button
    playButton = new ControlPushButton(ConfigKey(group, "play"), true);
    connect(playButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlPlay(double)));
    playButton->set(0);
    playButtonCOT = new ControlObjectThreadMain(playButton);

    // Start button
    startButton = new ControlPushButton(ConfigKey(group, "start"));
    connect(startButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlStart(double)));
    startButton->set(0);

    // End button
    endButton = new ControlPushButton(ConfigKey(group, "end"));
    connect(endButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlEnd(double)));
    endButton->set(0);

    m_pMasterRate = ControlObject::getControl(ConfigKey("[Master]", "rate"));

    // Actual rate (used in visuals, not for control)
    rateEngine = new ControlObject(ConfigKey(group, "rateEngine"));

    // BJW Wheel touch sensor (makes wheel act as scratch)
    wheelTouchSensor = new ControlPushButton(ConfigKey(group, "wheel_touch_sensor"));
    // BJW Wheel touch-sens switch (toggles ignoring touch sensor)
    wheelTouchSwitch = new ControlPushButton(ConfigKey(group, "wheel_touch_switch"));
    wheelTouchSwitch->setToggleButton(true);
    // BJW Whether to revert to PitchIndpTimeStretch after scratching
    m_bResetPitchIndpTimeStretch = false;

    // Slider to show and change song position
    playposSlider = new ControlPotmeter(ConfigKey(group, "playposition"), 0., 1.);
    connect(playposSlider, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlSeek(double)));

    // Control used to communicate ratio playpos to GUI thread
    visualPlaypos =
        new ControlPotmeter(ConfigKey(group, "visual_playposition"), 0., 1.);

    // m_pTrackEnd is used to signal when at end of file during
    // playback. TODO(XXX) This should not even be a control object because it
    // is an internal flag used only by the EngineBuffer.
    m_pTrackEnd = new ControlObject(ConfigKey(group, "TrackEnd"));
    //A COTM for use in slots that are called by the GUI thread.
    m_pTrackEndCOT = new ControlObjectThreadMain(m_pTrackEnd);

    // TrackEndMode determines what to do at the end of a track
    m_pTrackEndMode = new ControlObject(ConfigKey(group,"TrackEndMode"));

#ifdef __VINYLCONTROL__
    // Vinyl Control status indicator
    //Disabled because it's not finished yet
    //m_pVinylControlIndicator =
    //    new ControlObject(ConfigKey(group, "VinylControlIndicator"));
#endif

    // Sample rate
    m_pSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));

    m_pTrackSamples = new ControlObject(ConfigKey(group, "track_samples"));

    // Create the Loop Controller
    m_pLoopingControl = new LoopingControl(_group, _config);
    addControl(m_pLoopingControl);

    // Create the Rate Controller
    m_pRateControl = new RateControl(_group, _config);
    addControl(m_pRateControl);

    fwdButton = ControlObject::getControl(ConfigKey(_group, "fwd"));
    backButton = ControlObject::getControl(ConfigKey(_group, "back"));

    // Create the BPM Controller
    m_pBpmControl = new BpmControl(_group, _config);
    addControl(m_pBpmControl);

    m_pReader = new CachingReader(_group, _config);
    connect(m_pReader, SIGNAL(trackLoaded(TrackPointer, int, int)),
            this, SLOT(slotTrackLoaded(TrackPointer, int, int)));
    connect(m_pReader, SIGNAL(trackLoadFailed(TrackPointer, QString)),
            this, SLOT(slotTrackLoadFailed(TrackPointer, QString)));

    m_pReadAheadManager = new ReadAheadManager(m_pReader);
    m_pReadAheadManager->addEngineControl(m_pLoopingControl);

    // Construct scaling objects
    m_pScaleLinear = new EngineBufferScaleLinear(m_pReadAheadManager);

    m_pScaleST = new EngineBufferScaleST(m_pReadAheadManager);
    //m_pScaleST = (EngineBufferScaleST*)new EngineBufferScaleDummy(m_pReadAheadManager);
    //Figure out which one to use (setPitchIndpTimeStretch does this)
    int iPitchIndpTimeStretch =
        _config->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt();
    this->setPitchIndpTimeStretch(iPitchIndpTimeStretch);

    setNewPlaypos(0.);
}

EngineBuffer::~EngineBuffer()
{
    delete m_pLoopingControl;
    delete m_pRateControl;
    delete m_pBpmControl;
    delete m_pReadAheadManager;
    delete m_pReader;

    delete playButton;
    delete startButton;
    delete endButton;
    delete rateEngine;
    delete wheelTouchSensor;
    delete wheelTouchSwitch;
    delete playposSlider;
    delete visualPlaypos;

    delete m_pTrackEnd;
    delete m_pTrackEndMode;

    delete m_pTrackSamples;

    delete m_pScaleLinear;
    delete m_pScaleST;

}

void EngineBuffer::setPitchIndpTimeStretch(bool b)
{
    pause.lock(); //Just to be safe - Albert

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
    pause.unlock();
}

double EngineBuffer::getBpm()
{
    return m_pBpmControl->getBpm();
}

void EngineBuffer::setOtherEngineBuffer(EngineBuffer * pOtherEngineBuffer)
{
    if (!m_pOtherEngineBuffer) {
        m_pOtherEngineBuffer = pOtherEngineBuffer;
        m_pBpmControl->setOtherEngineBuffer(pOtherEngineBuffer);
    } else
        qCritical("EngineBuffer: Other engine buffer already set!");
}

void EngineBuffer::setNewPlaypos(double newpos)
{
    //qDebug() << "engine new pos " << newpos;

    filepos_play = newpos;

    // Ensures that the playpos slider gets updated in next process call
    m_iSamplesCalculated = 1000000;

    // The right place to do this?
    if (m_pScale)
        m_pScale->clear();
    m_pReadAheadManager->notifySeek(filepos_play);

    // Must hold the engineLock while using m_engineControls
    m_engineLock.lock();
    for (QList<EngineControl*>::iterator it = m_engineControls.begin();
         it != m_engineControls.end(); it++) {
        EngineControl *pControl = *it;
        pControl->notifySeek(filepos_play);
    }
    m_engineLock.unlock();
}

const char * EngineBuffer::getGroup()
{
    return group;
}

double EngineBuffer::getRate()
{
    return m_pRateControl->getRawRate();
}

void EngineBuffer::slotTrackLoaded(TrackPointer pTrack,
                                   int iTrackSampleRate,
                                   int iTrackNumSamples) {
    pause.lock();
    file_srate_old = iTrackSampleRate;
    file_length_old = iTrackNumSamples;
    m_pTrackSamples->set(iTrackNumSamples);
    slotControlSeek(0.);

    // Let the engine know that a track is loaded now.
    m_pTrackEndCOT->slotSet(0.0f); //XXX: Not sure if to use the COT or CO here

    pause.unlock();

    emit(trackLoaded(pTrack));
}

void EngineBuffer::slotTrackLoadFailed(TrackPointer pTrack,
                                       QString reason) {
    pause.lock();
    file_srate_old = 0;
    file_length_old = 0;
    playButton->set(0.0);
    slotControlSeek(0.);
    m_pTrackSamples->set(0);
    pause.unlock();
    emit(trackLoadFailed(pTrack, reason));
}


void EngineBuffer::slotControlSeek(double change)
{
    if(isnan(change) || change > 1.0 || change < 0.0) {
        // This seek is ridiculous.
        return;
    }

    // Find new playpos, restrict to valid ranges.
    double new_playpos = round(change*file_length_old);
    if (new_playpos > file_length_old)
        new_playpos = file_length_old;
    if (new_playpos < 0.)
        new_playpos = 0.;

    // Ensure that the file position is even (remember, stereo channel files...)
    if (!even((int)new_playpos))
        new_playpos--;

    // Give EngineControl's a chance to veto or correct the seek target.

    // Seek reader
    Hint seek_hint;
    seek_hint.sample = new_playpos;
    seek_hint.length = 0;
    seek_hint.priority = 1;
    QList<Hint> hint_list;
    hint_list.append(seek_hint);
    m_pReader->hintAndMaybeWake(hint_list);
    setNewPlaypos(new_playpos);
}

void EngineBuffer::slotControlSeekAbs(double abs)
{
    slotControlSeek(abs/file_length_old);
}

void EngineBuffer::slotControlPlay(double)
{
}

void EngineBuffer::slotControlStart(double)
{
    slotControlSeek(0.);
}

void EngineBuffer::slotControlEnd(double)
{
    slotControlSeek(1.);
}

void EngineBuffer::process(const CSAMPLE *, const CSAMPLE * pOut, const int iBufferSize)
{

    // Steps:
    // - Lookup new reader information
    // - Calculate current rate
    // - Scale the audio with m_pScale, copy the resulting samples into the
    //   output buffer
    // - Give EngineControl's a chance to do work / request seeks, etc
    // - Process EndOfTrack mode if we're at the end of a track
    // - Set last sample value (m_fLastSampleValue) so that rampOut works? Other
    //   miscellaneous upkeep issues.

    CSAMPLE * pOutput = (CSAMPLE *)pOut;

    bool bCurBufferPaused = false;

    if (!m_pTrackEnd->get() && pause.tryLock()) {
        float sr = m_pSampleRate->get();
        double baserate = 0.0f;
        if (sr > 0)
            baserate = ((double)file_srate_old/sr);

        // Is a touch sensitive wheel being touched?
        bool wheelTouchSensorEnabled = wheelTouchSwitch->get() && wheelTouchSensor->get();

        bool paused = false;

        if (!playButton->get())
            paused = true;

        if (wheelTouchSensorEnabled) {
            paused = true;
        }

        // TODO(rryan) : review this touch sensitive business with the Mixxx
        // team to see if this is what we actually want.

        // BJW: Touch sensitive wheels: If enabled via the Switch, while the top of the wheel is touched it acts
        // as a "vinyl-like" scratch controller. Playback stops, pitch-independent time stretch is disabled, and
        // the wheel's motion is amplified to produce a scrub effect. Also note that playback speed factors like
        // rateSlider and reverseButton are ignored so that the feel of the wheel is always consistent.
        // TODO: Configurable vinyl stop effect.
        if (wheelTouchSensorEnabled) {
            // Act as scratch controller
            if (m_pConfig->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt()) {
                // Use vinyl-style pitch bending
                // qDebug() << "Disabling Pitch-Independent Time Stretch for scratching";
                m_bResetPitchIndpTimeStretch = true;
                setPitchIndpTimeStretch(false);
            }
        } else if (!paused) {
            // BJW: Reset timestretch mode if required. NB it's intentional that this should not
            // get reset until playing; this enables released spinbacks in stop mode.
            if (m_bResetPitchIndpTimeStretch) {
                setPitchIndpTimeStretch(true);
                m_bResetPitchIndpTimeStretch = false;
                // qDebug() << "Re-enabling Pitch-Independent Time Stretch";
            }
        }


        double rate = m_pRateControl->calculateRate(baserate, paused);
        //qDebug() << "rate" << rate << " paused" << paused;

        // If the rate has changed, set it in the scale object
        if (rate != rate_old || m_bScalerChanged) {
            // The rate returned by the scale object can be different from the wanted rate!

            //XXX: Trying to force RAMAN to read from correct
            //     playpos when rate changes direction - Albert
            if ((rate_old <= 0 && rate > 0) ||
                (rate_old >= 0 && rate < 0))
            {
                setNewPlaypos(filepos_play);
            }

            rate_old = rate;
            if (baserate > 0) //Prevent division by 0
                rate = baserate*m_pScale->setTempo(rate/baserate);
            m_pScale->setBaseRate(baserate);
            rate_old = rate;
            // Scaler is up to date now.
            m_bScalerChanged = false;
        }

        bool at_start = filepos_play <= 0;
        bool at_end = filepos_play >= file_length_old;
        bool backwards = rate < 0;

        // If we're playing past the end, playing before the start, or standing
        // still then by definition the buffer is paused.
        bCurBufferPaused = rate == 0 ||
            (at_start && backwards) ||
            (at_end && !backwards);

        // If paused, then ramp out.
        if (bCurBufferPaused) {
            // If this is the first process() since being paused, then ramp out.
            if (!m_bLastBufferPaused) {
                rampOut(pOut, iBufferSize);
            }
        // Otherwise, scale the audio.
        } else { // if (bCurBufferPaused)
            CSAMPLE *output;
            double idx;

            Q_ASSERT(even(iBufferSize));

            // The fileposition should be: (why is this thing a double anyway!?
            // Integer valued.
            Q_ASSERT(round(filepos_play) == filepos_play);
            // Even.
            Q_ASSERT(even(filepos_play));

            // Perform scaling of Reader buffer into buffer.
            output = m_pScale->scale(0,
                                     iBufferSize,
                                     0,
                                     0);
            idx = m_pScale->getNewPlaypos();

            // qDebug() << "sourceSamples used " << iSourceSamples
            //          <<" idx " << idx
            //          << ", buffer pos " << iBufferStartSample
            //          << ", play " << filepos_play
            //          << " bufferlen " << iBufferSize;

            // Copy scaled audio into pOutput
            memcpy(pOutput, output, sizeof(CSAMPLE) * iBufferSize);

            // for(int i=0; i<iBufferSize; i++) {
            //     pOutput[i] = output[i];
            // }

            // Adjust filepos_play by the amount we processed.
            filepos_play += idx;
            filepos_play = math_max(0, filepos_play); 
            // We need the above protection against negative playpositions
            // in case SoundTouch/EngineBufferSoundTouch gives us too many samples. 

            // Get rid of annoying decimals that the scaler sometimes produces
            filepos_play = round(filepos_play);

            if (!even(filepos_play))
                filepos_play--;

        } // else (bCurBufferPaused)

        m_engineLock.lock();
        QListIterator<EngineControl*> it(m_engineControls);
        while (it.hasNext()) {
            EngineControl* pControl = it.next();
            pControl->setCurrentSample(filepos_play);
            double control_seek = pControl->process(rate, filepos_play,
                                                    file_length_old, iBufferSize);

            if (control_seek != kNoTrigger) {
                filepos_play = control_seek;
                Q_ASSERT(round(filepos_play) == filepos_play);

                // Safety check that the EngineControl didn't pass us a bogus
                // value
                if (!even(filepos_play))
                    filepos_play--;

                // Fix filepos_play so that it is not out of bounds.
                if (file_length_old > 0) {
                    if(filepos_play > file_length_old) {
                        filepos_play = file_length_old;
                        at_end = true;
                    } else if(filepos_play < 0) {
                        filepos_play = 0;
                        at_start = true;
                    }
                }
                // TODO(XXX) need to re-evaluate this later. If we
                // setNewPlaypos, that clear()'s soundtouch, which might screw
                // up the audio. This sort of jump is a normal event. Also, the
                // EngineControl which caused this jump will get a notifySeek
                // for the same jump which might be confusing. For 1.8.0
                // purposes this works fine. If we do not notifySeek the RAMAN,
                // the engine and RAMAN can get out of sync.

                //setNewPlaypos(filepos_play);
                m_pReadAheadManager->notifySeek(filepos_play);
            }
        }
        m_engineLock.unlock();


        // Give the Reader hints as to which chunks of the current song we
        // really care about. It will try very hard to keep these in memory
        hintReader(rate, iBufferSize);

        // Update all the indicators that EngineBuffer publishes to allow
        // external parts of Mixxx to observe its status.
        updateIndicators(rate, iBufferSize);

        // Handle End-Of-Track mode
        at_start = filepos_play <= 0;
        at_end = filepos_play >= file_length_old;

        bool end_of_track = (at_start && backwards) ||
            (at_end && !backwards);

        // If playbutton is pressed, check if we are at start or end of track
        if ((playButton->get() || (fwdButton->get() || backButton->get())) &&
            !m_pTrackEnd->get() &&
            ((at_start && backwards) ||
             (at_end && !backwards))) {

            // If end of track mode is set to next, signal EndOfTrack to TrackList,
            // otherwise start looping, pingpong or stop the track
            int m = (int)m_pTrackEndMode->get();
            //qDebug() << "end mode " << m;
            switch (m)
            {
            case TRACK_END_MODE_STOP:
                //qDebug() << "stop";
                playButton->set(0.);
                break;
            case TRACK_END_MODE_NEXT:
                //m_pTrackEnd->set(1.);
                playButton->set(0.);
                emit(loadNextTrack());
                break;

            case TRACK_END_MODE_LOOP:
                //qDebug() << "loop";
                if(filepos_play <= 0)
                    slotControlSeek(file_length_old);
                else
                    slotControlSeek(0.);
                break;
/*
            case TRACK_END_MODE_PING:
                qDebug() << "Ping not implemented yet";

                if (reverseButton->get())
                reverseButton->set(0.);
                else
                reverseButton->set(1.);

                break;
 */
            default:
                qDebug() << "Invalid track end mode: " << m;
            }
        }

        // release the pauselock
        pause.unlock();
    } else { // if (!m_pTrackEnd->get() && pause.tryLock()) {
        if (!m_bLastBufferPaused)
            rampOut(pOut, iBufferSize);
        bCurBufferPaused = true;
    }

    // Force ramp in if this is the first buffer during a play
    if (m_bLastBufferPaused && !bCurBufferPaused) {
        // Ramp from zero
        int iLen = math_min(iBufferSize, kiRampLength);
        float fStep = pOutput[iLen-1]/(float)iLen;
        for (int i=0; i<iLen; ++i)
            pOutput[i] = fStep*i;
    }

    m_bLastBufferPaused = bCurBufferPaused;
    m_fLastSampleValue = pOutput[iBufferSize-1];
}


void EngineBuffer::rampOut(const CSAMPLE* pOut, int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;

    //qDebug() << "ramp out";

    // Ramp to zero
    int i=0;
    if (m_fLastSampleValue!=0.)
    {
        int iLen = math_min(iBufferSize, kiRampLength);
        float fStep = m_fLastSampleValue/(float)iLen;
        while (i<iLen)
        {
            pOutput[i] = fStep*(iLen-(i+1));
            ++i;
        }
    }

    // Reset rest of buffer
    while (i<iBufferSize)
    {
        pOutput[i]=0.;
        ++i;
    }
}

void EngineBuffer::updateIndicators(double rate, int iBufferSize) {

    // Increase samplesCalculated by the buffer size
    m_iSamplesCalculated += iBufferSize;

    double fFractionalPlaypos = 0.0;
    if (file_length_old!=0.) {
        fFractionalPlaypos = math_max(0.,math_min(filepos_play,file_length_old));
        fFractionalPlaypos /= file_length_old;
    } else {
        fFractionalPlaypos = 0.;
    }

    // Update indicators that are only updated after every
    // sampleRate/UPDATE_RATE samples processed.  (e.g. playposSlider,
    // rateEngine)
    if (m_iSamplesCalculated > (m_pSampleRate->get()/UPDATE_RATE)) {
        playposSlider->set(fFractionalPlaypos);

        if(rate != rateEngine->get())
            rateEngine->set(rate);

        // Reset sample counter
        m_iSamplesCalculated = 0;
    }

    // Update visual control object, this needs to be done more often than the
    // rateEngine and playpos slider
    visualPlaypos->set(fFractionalPlaypos);
}

void EngineBuffer::hintReader(const double dRate,
                              const int iSourceSamples) {
    m_engineLock.lock();

    m_hintList.clear();
    m_pReadAheadManager->hintReader(m_hintList, iSourceSamples);

    QListIterator<EngineControl*> it(m_engineControls);
    while (it.hasNext()) {
        EngineControl* pControl = it.next();
        pControl->hintReader(m_hintList);
    }
    m_pReader->hintAndMaybeWake(m_hintList);

    m_engineLock.unlock();
}

void EngineBuffer::loadTrack(TrackPointer pTrack) {
    // Raise the track end flag so the EngineBuffer stops processing frames
    m_pTrackEndCOT->slotSet(1.0);

    //Stop playback
    playButtonCOT->slotSet(0.0);

    // Signal to the reader to load the track. The reader will respond with
    // either trackLoaded or trackLoadFailed signals.
    m_pReader->newTrack(pTrack);
    m_pReader->wake();
}

void EngineBuffer::addControl(EngineControl* pControl) {
    // Connect to signals from EngineControl here...
    m_engineLock.lock();
    m_engineControls.push_back(pControl);
    m_engineLock.unlock();
    connect(pControl, SIGNAL(seek(double)),
            this, SLOT(slotControlSeek(double)));
    connect(pControl, SIGNAL(seekAbs(double)),
            this, SLOT(slotControlSeekAbs(double)));
}

void EngineBuffer::bindWorkers(EngineWorkerScheduler* pWorkerScheduler) {
    pWorkerScheduler->bindWorker(m_pReader);
}
