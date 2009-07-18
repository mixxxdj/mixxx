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
#include "configobject.h"
#include "controlpotmeter.h"
#include "enginebufferscalest.h"
#include "enginebufferscalelinear.h"
#include "enginebufferscalereal.h"
#include "enginebufferscaledummy.h"
#include "mathstuff.h"

#include "engine/enginecontrol.h"
#include "enginebuffercue.h"
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
    group(_group),
    m_pConfig(_config),
    m_pLoopingControl(NULL),
    m_pRateControl(NULL),
    m_pBpmControl(NULL),
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
    m_pEngineBufferCue(NULL),
    m_pScale(NULL),
    m_pScaleLinear(NULL),
    m_pScaleST(NULL),
    m_fLastSampleValue(0.),
    m_bLastBufferPaused(true),
    m_bResetPitchIndpTimeStretch(true) {
    
    // Play button
    playButton = new ControlPushButton(ConfigKey(group, "play"), true);
    connect(playButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlPlay(double)));
    playButton->set(0);

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
    
    // m_pTrackEnd is used to signal when at end of file during playback
    m_pTrackEnd = new ControlObject(ConfigKey(group, "TrackEnd"));

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

    
    setNewPlaypos(0.);

    
    m_pTrackSamples = new ControlObject(ConfigKey(group, "track_samples"));

    // Create the Cue Controller TODO(rryan) : this has to happen before Reader
    // is constructed. Fix that.
    m_pEngineBufferCue = new EngineBufferCue(_group, _config, this);

    // Create the Loop Controller
    m_pLoopingControl = new LoopingControl(_group, _config);

    // Create the Rate Controller
    m_pRateControl = new RateControl(_group, _config);
    fwdButton = ControlObject::getControl(ConfigKey(_group, "fwd"));
    backButton = ControlObject::getControl(ConfigKey(_group, "back"));

    // Create the BPM Controller
    m_pBpmControl = new BpmControl(_group, _config);
    
    m_pReader = new CachingReader(_group, _config);
    connect(m_pReader, SIGNAL(trackLoaded(TrackInfoObject*, int, int)),
            this, SLOT(slotTrackLoaded(TrackInfoObject*, int, int)));
    

    // Construct scaling objects
    m_pScaleLinear = new EngineBufferScaleLinear();
    m_pScaleST = new EngineBufferScaleST();
    //Figure out which one to use (setPitchIndpTimeStretch does this)
    int iPitchIndpTimeStretch =
        _config->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt();
    this->setPitchIndpTimeStretch(iPitchIndpTimeStretch);

    m_pReader->start();

    m_pBuffer = new CSAMPLE[MAX_BUFFER_LEN];
    m_iBufferRead = 0;
    m_iBufferReadSample = 0;
    m_iBufferWrite = 0;
    m_iBufferSize = MAX_BUFFER_LEN;
}

EngineBuffer::~EngineBuffer()
{
    delete m_pLoopingControl;
    delete m_pRateControl;
    delete m_pBpmControl;
    delete m_pEngineBufferCue;
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

    delete m_pBuffer;
}

void EngineBuffer::lockPlayposVars()
{
    m_qPlayposMutex.lock();
}

void EngineBuffer::unlockPlayposVars()
{
    m_qPlayposMutex.unlock();
}

double EngineBuffer::getAbsPlaypos()
{
    return m_dAbsPlaypos;
}

void EngineBuffer::setPitchIndpTimeStretch(bool b)
{
    m_qPlayposMutex.lock(); //Just to be safe - Albert

    // Change sound scale mode

    //SoundTouch's linear interpolation code doesn't sound very good.
    //Our own EngineBufferScaleLinear sounds slightly better, but it's
    //not working perfectly. Eventually we should have our own working
    //better, so scratching sounds good.

    //Update Dec 30/2007
    //If we delete the m_pScale object and recreate it, it eventually
    //causes some weird bad pointer somewhere, which will either cause
    //the waveform the roll in a weird way or fire an ASSERT from
    //visualchannel.cpp or something. Need to valgrind this or something,
    //but for now, I've just changed it back to using EngineBufferScaleST
    //exclusively. - Albert

    if (b == true)
    {
        m_pScale = m_pScaleST;
        ((EngineBufferScaleST *)m_pScale)->setPitchIndpTimeStretch(b);
        //m_pScale = m_pScaleLinear;
    }
    else
    {
        m_pScale = m_pScaleLinear;
    }
    m_qPlayposMutex.unlock();

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

    // Update bufferposSlider
    if (m_qPlayposMutex.tryLock())
    {
        m_dAbsPlaypos = filepos_play;
        m_qPlayposMutex.unlock();
    }

    // Ensures that the playpos slider gets updated in next process call
    m_iSamplesCalculated = 1000000;

    // The right place to do this?
    if (m_pScale)
        m_pScale->clear();
}

const char * EngineBuffer::getGroup()
{
    return group;
}

double EngineBuffer::getRate()
{
    return m_pRateControl->getRawRate();
}

void EngineBuffer::slotTrackLoaded(TrackInfoObject *pTrack,
                                   int iTrackSampleRate,
                                   int iTrackNumSamples) {
    pause.lock();
    file_srate_old = iTrackSampleRate;
    file_length_old = iTrackNumSamples;
    pause.unlock();
    
    m_pTrackSamples->set(iTrackNumSamples);

    emit(trackLoaded(pTrack));
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

    // Seek reader
    m_pReader->hint(new_playpos, 1000, 0);
    m_pReader->wake();
    setNewPlaypos(new_playpos);
}

void EngineBuffer::slotControlSeekAbs(double abs)
{
    slotControlSeek(abs/file_length_old);
}

void EngineBuffer::slotControlPlay(double)
{
    m_pEngineBufferCue->slotControlCueSet();
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
    // - Prepare an intermediate source sample buffer with loops taken into account.
    // - Scale the audio with m_pScale, copy the resulting samples into the
    //   output buffer
    // - Give EngineControl's a chance to do work / request seeks, etc
    // - Process EndOfTrack mode if we're at the end of a track
    // - Set last sample value (m_fLastSampleValue) so that rampOut works? Other
    //   miscellaneous upkeep issues.
    
    CSAMPLE * pOutput = (CSAMPLE *)pOut;

    bool bCurBufferPaused = false;

    if (!m_pTrackEnd->get() && pause.tryLock()) {
        double baserate = ((double)file_srate_old/m_pSampleRate->get());
        
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
        if (rate != rate_old) {
            // The rate returned by the scale object can be different from the wanted rate!
            rate_old = rate;
            rate = baserate*m_pScale->setTempo(rate/baserate);
            m_pScale->setBaseRate(baserate);
            rate_old = rate;
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

            // We need to figure out number of source samples required. rate is
            // the rate speedup from the base rate.  iBufferSize is the number
            // of samples to consume. If rate == 2, then for every 1 sample the
            // original song consumed in a given amount of buffer space, the
            // twice sped up song will consume 2. The number of `sourceSamples'
            // needed to fill a buffer of `iBufferSize' if the song is going at
            // a `rate' speedup,
            //
            //    sourceSamples = iBufferSize * rate
            //
            int iSourceSamples = abs(double(iBufferSize) * rate);

            Q_ASSERT(even(iBufferSize));
            
            //Q_ASSERT(even(iSourceSamples));
            if (!even(iSourceSamples))
                iSourceSamples++;

            // The fileposition should be: (why is this thing a double anyway!?
            // Integer valued.
            Q_ASSERT(round(filepos_play) == filepos_play);
            // Even.
            Q_ASSERT(even(filepos_play));
            
            // Read the raw source data into m_pBuffer
            prepareSampleBuffer(iSourceSamples, rate, iBufferSize);

            // This is because of some oddness with EngineBufferScalers. We need
            // to revisit the EngineBufferScale design now that we have more
            // control
            int iBufferStartSample = (backwards ? iSourceSamples-1 : 0);
 
            // Perform scaling of Reader buffer into buffer.
            output = m_pScale->scale(iBufferStartSample,
                                     iBufferSize,
                                     m_pBuffer,
                                     iSourceSamples);
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
            filepos_play += (idx-iBufferStartSample);

            // Get rid of annoying decimals that the scaler sometimes produces
            filepos_play = round(filepos_play);

            if (!even(filepos_play))
                filepos_play--;

            // Adjust filepos_play in case we took any loops during this buffer
            filepos_play = m_pLoopingControl->process(rate,
                                                      filepos_play,
                                                      file_length_old,
                                                      iBufferSize);

            Q_ASSERT(round(filepos_play) == filepos_play);

            // Safety check that LoopingControl didn't pass us a bogus value
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

        } // else (bCurBufferPaused)

        // Let RateControl do its logic. This is a temporary hack until this
        // step is just processing a list of EngineControls
        m_pRateControl->process(rate, filepos_play,
                                file_length_old, iBufferSize);
        

        // Give the Reader hints as to which chunks of the current song we
        // really care about. It will try very hard to keep these in memory
        hintReader(rate, iBufferSize);

        // And wake it up so that it processes our hints (hopefully) before the
        // next callback.
        m_pReader->wake();

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
                m_pTrackEnd->set(1.);
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

int EngineBuffer::prepareSampleBuffer(int iSourceSamples,
                                      const double dRate,
                                      const int iBufferSize) {
    
    if (!even(iSourceSamples))
        iSourceSamples--;

    // We have to ensure that the buffer we prepare has enough source samples to
    // fill an entire iBufferSize buffer with scaled audio, whether we are
    // progressing forward or in reverse.

    bool in_reverse = (dRate < 0);

    // First read samples from the current position of the audio.  If we read
    // from a given audio position p, then we need to read at least
    // iSourceSamples forward and backward in order for the EngineBufferScale we
    // are using to have enough data to do its work.
    
    int samples_needed = iSourceSamples;
    int actual_samples = 0;
    int samples_to_read = samples_needed;

    double next_loop = m_pLoopingControl->nextTrigger(dRate,
                                                      filepos_play,
                                                      file_length_old,
                                                      iBufferSize);

    if (next_loop != kNoTrigger) {
        samples_to_read = math_min(fabs(next_loop-filepos_play),
                                   samples_needed);
    }

    // qDebug() << "Reading " << samples_to_read << " from CachingReader. "
    //          << "next_loop is " << next_loop
    //          << "samples_needed:" << samples_needed
    //          << "filepos_play" << filepos_play
    //          << "diff" << (next_loop - filepos_play);
    // qDebug("%f, %f, %f", next_loop, filepos_play, next_loop-filepos_play);

    Q_ASSERT(even(samples_to_read));

    // The logic below will fill these values out, and well pass them as
    // arguments to m_pReader
    int start_sample = filepos_play;
    int num_samples = samples_to_read;
    CSAMPLE* baseBuffer = m_pBuffer;

    if (samples_to_read == samples_needed) {
        // The loop does not matter, we will not hit it in this buffer.
        if (in_reverse) {
            // Read samples_to_read samples into baseBuffer from
            // filepos-samples_to_read, since we're in reverse.
            start_sample = filepos_play - samples_needed;
        } else {
            // Read samples_to_read bytes into baseBuffer from filepos
        }
    } else {
        // The loop will affect us in this buffer
        if (in_reverse) {
            // Funky. We need to read samples_to_read bytes so that the section
            // of data read fills up to the very last element of the buffer.
            start_sample = filepos_play-samples_to_read;
            int read_offset = iSourceSamples - samples_to_read;
            baseBuffer += read_offset;
        } else {
            // Read samples_to_read bytes into baseBuffer from filepos
        }        
    }

    // Ensure we don't have a wacky start sample.
    if (start_sample < 0) {
        // Clear the samples we're requesting before the track.
        memset(baseBuffer, 0, sizeof(CSAMPLE) * (-start_sample));
        // Decrease the num_samples we will read, and increase baseBuffer by
        // that number too (start_sample is negative)
        num_samples += start_sample;
        baseBuffer -= start_sample;
        start_sample = 0;
    }
    
    // Do the actual read
    actual_samples = m_pReader->read(start_sample,
                                     num_samples,
                                     baseBuffer);

    if (actual_samples != samples_to_read) {
        // For some reason there was a reader error. This should not happen.
        qDebug() << "While reading got fewer samples than asked for:"
                 << actual_samples << " / " << samples_to_read;
        // Zero the samples we didn't get
        for (int i = actual_samples; i < samples_to_read; i++) {
            baseBuffer[i] = 0.0f;
        }
    }

    baseBuffer += actual_samples;
    samples_needed -= actual_samples;

    // TODO(rryan) -- loops of size less than the buffer size will not be
    // properly handled here.

    if (samples_needed > 0 && actual_samples == samples_to_read) {
        // No read error occured, and we still need more samples, so that means
        // we need to read from the loop target.
        samples_to_read = samples_needed;

        // Whether we are going reverse or forward, this returns the sample to
        // which we should jump once we pass the next_loop sample.
        double loop_trigger = m_pLoopingControl->getTrigger(dRate,
                                                            filepos_play,
                                                            file_length_old,
                                                            iBufferSize);
        
        if (in_reverse) {
            // Special case -- since we're reading backwards, the loop samples
            // go at the start of the buffer.
            baseBuffer = m_pBuffer;
            actual_samples = m_pReader->read(loop_trigger,
                                             samples_to_read,
                                             baseBuffer);
        } else {
            actual_samples = m_pReader->read(loop_trigger,
                                             samples_to_read,
                                             baseBuffer);
        }

        if (actual_samples != samples_to_read) {
            // For some reason there was a reader error. This should not happen.
            qDebug() << "While reading loop got fewer samples than asked for:"
                     << actual_samples << " / " << samples_to_read;
            // Zero the samples we didn't get
            for (int i = actual_samples; i < samples_to_read; i++) {
                baseBuffer[i] = 0.0f;
            }
        }
    }

    return 0;
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

    // Update buffer and abs position. These variables are not in the ControlObject
    // framework because they need very frequent updates.
    if (m_qPlayposMutex.tryLock()) {
        m_dAbsPlaypos = filepos_play;
        m_qPlayposMutex.unlock();
    }
}

void EngineBuffer::hintReader(const double dRate,
                              const int iSourceSamples) {
    // TODO(rryan) ... hint the reader!
}

void EngineBuffer::loadTrack(TrackInfoObject *pTrack) {
    m_pReader->newTrack(pTrack);
}
