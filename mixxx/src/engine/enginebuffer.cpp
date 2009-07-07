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
#include "controlpushbutton.h"
#include "configobject.h"
#include "controlpotmeter.h"
#include "reader.h"
#include "readerextractwave.h"
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
    reader(NULL),
    filepos_play(0.),
    filepos_start(0.),
    bufferpos_play(0.),
    rate_old(0.),
    file_length_old(-1),
    file_srate_old(0),
    m_iSamplesCalculated(0),
    m_dBufferPlaypos(0.),
    m_dAbsPlaypos(0.),
    m_dAbsStartpos(0.),
    m_pTrackEnd(NULL),
    m_pTrackEndMode(NULL),
    startButton(NULL),
    endButton(NULL),
    read_buffer_prt(NULL),
    m_pEngineBufferCue(NULL),
    m_pScale(NULL),
    m_pScaleLinear(NULL),
    m_pScaleST(NULL),
    m_fLastSampleValue(0.),
    m_bLastBufferPaused(true),
    m_pWaveBuffer(NULL),
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
    
    reader = new Reader(this, &pause, _config);
    read_buffer_prt = reader->getBufferWavePtr();
    m_pWaveBuffer = (float *)reader->getWavePtr()->getBasePtr();

    // Construct scaling objects
    m_pScaleLinear = new EngineBufferScaleLinear(reader->getWavePtr());
    m_pScaleST = new EngineBufferScaleST(reader->getWavePtr());
    //Figure out which one to use (setPitchIndpTimeStretch does this)
    int iPitchIndpTimeStretch =
        _config->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt();
    this->setPitchIndpTimeStretch(iPitchIndpTimeStretch);

    reader->start();
}

EngineBuffer::~EngineBuffer()
{
    delete m_pLoopingControl;
    delete m_pRateControl;
    delete m_pBpmControl;
    delete m_pEngineBufferCue;
    delete reader;

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
        
    delete m_pScaleLinear;
    delete m_pScaleST;
}

void EngineBuffer::lockPlayposVars()
{
    m_qPlayposMutex.lock();
}

void EngineBuffer::unlockPlayposVars()
{
    m_qPlayposMutex.unlock();
}

double EngineBuffer::getBufferPlaypos()
{
    return m_dBufferPlaypos;
}

double EngineBuffer::getAbsPlaypos()
{
    return m_dAbsPlaypos;
}

double EngineBuffer::getAbsStartpos()
{
    return m_dAbsStartpos;
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

Reader * EngineBuffer::getReader()
{
    return reader;
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
    bufferpos_play = 0.;

    // Update bufferposSlider
    if (m_qPlayposMutex.tryLock())
    {
        m_dBufferPlaypos = bufferpos_play;
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
    reader->requestSeek(new_playpos);
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
    
    // - If playing, copy samples from read buffer to write buffer, process as
    //   necessary.
    
    // - Query LoopingControl for seek necessary
    
    // - Query BPMControl for

    // - Process EOT Mode

    // - Set last sample value (m_fLastSampleValue) so that rampOut works?
     

    
    CSAMPLE * pOutput = (CSAMPLE *)pOut;

    bool bCurBufferPaused = false;

    if(!m_pTrackEnd->get() && pause.tryLock()) {
        bool readerinfo = false;
        long int filepos_end = 0;
        if(reader->tryLock()) {
            file_length_old = reader->getFileLength();
            file_srate_old = reader->getFileSrate();
            filepos_start = reader->getFileposStart();
            filepos_end = reader->getFileposEnd();
            reader->setFileposPlay((long int)filepos_play);
            reader->setRate(rate_old);

            reader->unlock();
            readerinfo = true;
        }

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
        if (rate != rate_old)
        {
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
        bool bCurBufferPaused = rate == 0 ||
            (at_start && backwards) ||
            (at_end && !backwards);
        
        // If paused, then ramp out.
        if (bCurBufferPaused) {
            // If this is the first process() since being paused, then ramp out.
            if (!m_bLastBufferPaused) {
                rampOut(pOut, iBufferSize);
                m_bLastBufferPaused = true;
            }
        // Otherwise, scale the audio.
        } else { // if (bCurBufferPaused)            
            CSAMPLE *output;
            double idx;

            m_bLastBufferPaused = false;

            // Perform scaling of Reader buffer into buffer.
            output = m_pScale->scale(bufferpos_play, iBufferSize);
            idx = m_pScale->getNewPlaypos();
                
            // qDebug() << "idx " << idx << ", buffer pos " << bufferpos_play << ", play " << filepos_play;

            // Copy scaled audio into pOutput
            // TODO(XXX) could this be done safely/faster with a memcpy?
            for(int i=0; i<iBufferSize; i++) {
                pOutput[i] = output[i];
            }

            // Adjust filepos_play by the amount we processed.
            filepos_play += (idx-bufferpos_play);

            if (file_length_old > 0 && readerinfo) {
                if(filepos_play > file_length_old) {
                    idx -= filepos_play-file_length_old;
                    filepos_play = file_length_old;
                    at_end = true;
                } else if(filepos_play < 0) {
                    idx -= filepos_play;
                    filepos_play = 0;
                    at_start = true;
                }
            }

            // Ensure valid range of idx
            while (idx>READBUFFERSIZE)
                idx -= (double)READBUFFERSIZE;
            while (idx<0)
                idx += (double)READBUFFERSIZE;

            // Write buffer playpos
            bufferpos_play = idx;
        } // else (bCurBufferPaused)

        // Let RateControl do its logic. This is a temporary hack until this
        // step is just processing a list of EngineControls
        m_pRateControl->process(filepos_play, file_length_old, 
                            ((double)m_pSampleRate->get() / iBufferSize));
        
        // See if the loop controller wants us to loop back.
        double new_filepos_play = m_pLoopingControl->process(filepos_play,
                                                             file_length_old);
        if(new_filepos_play != filepos_play) {
            // We have no better way of solving this problem than 
            reader->requestSeek(new_filepos_play);
            setNewPlaypos(new_filepos_play);
            rampOut(pOut, iBufferSize);
        }

        //
        // Check if more samples are needed from reader, and wake it up if necessary.
        //
        if(readerinfo && filepos_end > 0) {

            if(filepos_play > filepos_end || filepos_play < filepos_start) {
                reader->wake();
            } else if((filepos_end - filepos_play < READCHUNKSIZE*(READCHUNK_NO/2-1))) {
                reader->wake();
            } else if(fabs(filepos_play-filepos_start) < (float)(READCHUNKSIZE*(READCHUNK_NO/2-1))) {
                reader->wake();
            }
        }

        // Update all the indicators that EngineBuffer publishes to allow
        // external parts of Mixxx to observe its status.
        updateIndicators(rate, iBufferSize, filepos_start);

        // Handle End-Of-Track mode
        at_start = filepos_play <= 0;
        at_end = filepos_play >= file_length_old;

        // If playbutton is pressed, check if we are at start or end of track
        if ((playButton->get() || (fwdButton->get() || backButton->get())) &&
            !m_pTrackEnd->get() && readerinfo &&
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
    }
}


void EngineBuffer::rampOut(const CSAMPLE * pOut, int iBufferSize)
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


void EngineBuffer::updateIndicators(double rate, int iBufferSize, double filepos_start) {

    // Increase samplesCalculated by the buffer size
    m_iSamplesCalculated += iBufferSize;

    double fFractionalPlaypos = 0.0;
    if (file_length_old!=0.) {
        fFractionalPlaypos = math_max(0.,math_min(filepos_play,file_length_old));
        fFractionalPlaypos /= file_length_old;
        //qDebug() << "f " << f << ", len " << file_length_old << "i, " << f/file_length_old;
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
        m_dBufferPlaypos = bufferpos_play;
        m_dAbsPlaypos = filepos_play;
        m_dAbsStartpos = filepos_start;
        m_qPlayposMutex.unlock();
    }
}
