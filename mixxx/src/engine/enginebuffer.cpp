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

#include "controlpushbutton.h"
#include "enginebuffer.h"

#include <qevent.h>
#include <QtDebug>
#include "configobject.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "controlbeat.h"
#include "reader.h"
#include "readerextractwave.h"
#include "enginebufferscalest.h"
#include "enginebufferscalelinear.h"
#include "enginebufferscalereal.h"
//#include "enginebufferscalesrc.h"
#include "enginebufferscaledummy.h"
#include "mathstuff.h"
#include "enginebuffercue.h"
#include "loopingcontrol.h"
#include "ratecontrol.h"


#ifdef _MSC_VER
#include <float.h>  // for _isnan() on VC++
#define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#else
#include <math.h>  // for isnan() everywhere else
#endif


EngineBuffer::EngineBuffer(const char * _group, ConfigObject<ConfigValue> * _config)
{
    group = _group;
    m_pConfig = _config;

    m_pOtherEngineBuffer = 0;

    m_dAbsPlaypos = 0.;
    m_dBufferPlaypos = 0.;
    m_dAbsStartpos = 0.;

    filepos_play = 0;
    bufferpos_play = 0;

    // Set up temporary buffer for seeking and looping
    m_pTempBuffer = new float[kiTempLength];
    m_dTempFilePos = 0.;

    // Play button
    playButton = new ControlPushButton(ConfigKey(group, "play"), true);
    connect(playButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlPlay(double)));
    playButton->set(0);

    // Reverse button
    reverseButton = new ControlPushButton(ConfigKey(group, "reverse"));
    reverseButton->set(0);
    reverseButton->setToggleButton(true);

    // Fwd button
    fwdButton = new ControlPushButton(ConfigKey(group, "fwd"));
    connect(fwdButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlFastFwd(double)));
    fwdButton->set(0);

    // Back button
    backButton = new ControlPushButton(ConfigKey(group, "back"));
    connect(backButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlFastBack(double)));
    backButton->set(0);

    // Start button
    startButton = new ControlPushButton(ConfigKey(group, "start"));
    connect(startButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlStart(double)));
    startButton->set(0);

    // End button
    endButton = new ControlPushButton(ConfigKey(group, "end"));
    connect(endButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlEnd(double)));
    endButton->set(0);

    // Playback rate slider
    rateSlider = new ControlPotmeter(ConfigKey(group, "rate"), -1.f, 1.f);

    m_pMasterRate = ControlObject::getControl(ConfigKey("[Master]", "rate"));

    // Search rate. Rate used when searching in sound. This overrules the playback rate
    m_pRateSearch = new ControlPotmeter(ConfigKey(group, "rateSearch"), -300., 300.);

    // Range of rate
    m_pRateRange = new ControlObject(ConfigKey(group, "rateRange"));

    // Actual rate (used in visuals, not for control)
    rateEngine = new ControlObject(ConfigKey(group, "rateEngine"));



    // Wheel to control playback position/speed
    wheel = new ControlTTRotary(ConfigKey(group, "wheel"));

    // Scratch controller, this is an accumulator which is useful for controllers
	// that return individiual +1 or -1s, these get added up and cleared when we read
    m_pControlScratch = new ControlTTRotary(ConfigKey(group, "scratch"));

    // BJW Wheel touch sensor (makes wheel act as scratch)
    wheelTouchSensor = new ControlPushButton(ConfigKey(group, "wheel_touch_sensor"));
    // BJW Wheel touch-sens switch (toggles ignoring touch sensor)
    wheelTouchSwitch = new ControlPushButton(ConfigKey(group, "wheel_touch_switch"));
    wheelTouchSwitch->setToggleButton(true);
    // BJW Whether to revert to PitchIndpTimeStretch after scratching
    m_bResetPitchIndpTimeStretch = false;

    m_pJog = new ControlObject(ConfigKey(group, "jog"));
    m_jogfilter = new Rotary();
    // FIXME: This should be dependent on sample rate/block size or something
    m_jogfilter->setFilterLength(5);

    // Slider to show and change song position
    playposSlider = new ControlPotmeter(ConfigKey(group, "playposition"), 0., 1.);
    connect(playposSlider, SIGNAL(valueChanged(double)), this, SLOT(slotControlSeek(double)));

    // Control used to communicate ratio playpos to GUI thread
    visualPlaypos = new ControlPotmeter(ConfigKey(group, "visual_playposition"), 0., 1.);
    
    // m_pTrackEnd is used to signal when at end of file during playback
    m_pTrackEnd = new ControlObject(ConfigKey(group, "TrackEnd"));

    // Direction of rate slider
    m_pRateDir = new ControlObject(ConfigKey(group, "rate_dir"));

    // TrackEndMode determines what to do at the end of a track
    m_pTrackEndMode = new ControlObject(ConfigKey(group,"TrackEndMode"));

    // BPM of the file
    m_pFileBpm = new ControlObject(ConfigKey(group, "file_bpm"));

    // BPM control
    bpmControl = new ControlBeat(ConfigKey(group, "bpm"), true);
    connect(bpmControl, SIGNAL(valueChanged(double)), this, SLOT(slotSetBpm(double)));

    // Beat sync (scale buffer tempo relative to tempo of other buffer)
    buttonBeatSync = new ControlPushButton(ConfigKey(group, "beatsync"));
    connect(buttonBeatSync, SIGNAL(valueChanged(double)), this, SLOT(slotControlBeatSync(double)));

#ifdef __VINYLCONTROL__
    // Vinyl Control status indicator
    //Disabled because it's not finished yet
    //m_pVinylControlIndicator = new ControlObject(ConfigKey(group, "VinylControlIndicator"));
#endif

    m_pEngineBufferCue = new EngineBufferCue(group, this);

    // Sample rate
    m_pSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));

    // Control file changed
//    filechanged = new ControlEngine(controlfilechanged);
//    filechanged->setNotify(this,(EngineMethod)&EngineBuffer::newtrack);

    //m_bCuePreview = false;

    m_pScale = 0;
    setNewPlaypos(0.);

    reader = new Reader(this, &pause, _config);
    read_buffer_prt = reader->getBufferWavePtr();
    file_length_old = -1;
    file_srate_old = 0;
    rate_old = 0;

    m_iBeatMarkSamplesLeft = 0;

    m_bLastBufferPaused = true;
    m_fLastSampleValue = 0;

    m_pWaveBuffer = (float *)reader->getWavePtr()->getBasePtr();

    // Construct scaling objects
    m_pScaleLinear = new EngineBufferScaleLinear(reader->getWavePtr());
    m_pScaleST = new EngineBufferScaleST(reader->getWavePtr());
    //Figure out which one to use (setPitchIndpTimeStretch does this)
    int iPitchIndpTimeStretch = _config->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt();
    this->setPitchIndpTimeStretch(iPitchIndpTimeStretch);


    // Create the Loop Controller
    m_pLoopingControl = new LoopingControl(_group, _config);

    // Create the Rate Controller
    m_pRateControl = new RateControl(_group, _config);

    oldEvent = 0.;

    // Used in update of playpos slider
    m_iSamplesCalculated = 0;

    reader->start();
}

EngineBuffer::~EngineBuffer()
{
    delete [] m_pTempBuffer;
    delete playButton;
    delete wheel;
    delete m_pControlScratch;
    delete rateSlider;
    delete m_pScaleLinear;
    delete m_pScaleST;
    delete m_pTrackEnd;
    delete m_pLoopingControl;
    delete m_pRateControl;
    delete reader;
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
    return bpmControl->get();
}

void EngineBuffer::setOtherEngineBuffer(EngineBuffer * pOtherEngineBuffer)
{
    if (!m_pOtherEngineBuffer)
        m_pOtherEngineBuffer = pOtherEngineBuffer;
    else
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

    m_iBeatMarkSamplesLeft = 0;

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
    return rateSlider->get()*m_pRateRange->get()*m_pRateDir->get();
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

void EngineBuffer::slotSetBpm(double bpm)
{
    double filebpm = m_pFileBpm->get(); //reader->getBpm();

    if (filebpm!=0.)
        rateSlider->set(bpm/filebpm-1.);
}


void EngineBuffer::slotControlBeatSync(double)
{
    double fOtherBpm = m_pOtherEngineBuffer->getBpm();
    double fThisBpm  = bpmControl->get();
    double fRateScale;

    if (fOtherBpm>0. && fThisBpm>0.)
    {
        // Test if this buffers bpm is the double of the other one, and find rate scale:
        if (fabs(fThisBpm*2.-fOtherBpm) < fabs(fThisBpm-fOtherBpm))
            fRateScale = fOtherBpm/(2*fThisBpm) * (1.+m_pOtherEngineBuffer->getRate());
        else if ( fabs(fThisBpm-2.*fOtherBpm) < fabs(fThisBpm-fOtherBpm))
            fRateScale = 2.*fOtherBpm/fThisBpm * (1.+m_pOtherEngineBuffer->getRate());
        else
            fRateScale = (fOtherBpm*(1.+m_pOtherEngineBuffer->getRate()))/fThisBpm;

        // Ensure the rate is within resonable boundaries
        if (fRateScale<2. && fRateScale>0.5)
        {
            // Adjust the rate:
            fRateScale = (fRateScale-1.)/m_pRateRange->get();
            rateSlider->set(fRateScale * m_pRateDir->get());

            // Adjust the phase:
            // (removed, see older version for this info)
        }
    }

}

void EngineBuffer::slotControlFastFwd(double v)
{
    qDebug() << "slotControlFastFwd(" << v << ")";
    if (v==0.)
        m_pRateSearch->set(0.);
    else
        m_pRateSearch->set(4.);
}

void EngineBuffer::slotControlFastBack(double v)
{
    qDebug() << "slotControlFastBack(" << v << ")";
    if (v==0.)
        m_pRateSearch->set(0.);
    else
        m_pRateSearch->set(-4.);
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
     

    
    CSAMPLE * pOutput = (CSAMPLE *)pOut;

    bool bCurBufferPaused = false;

    if(!m_pTrackEnd->get() && pause.tryLock()) {
        bool readerinfo = false;
        long int filepos_start = 0;
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

        double filebpm = m_pFileBpm->get();

        double baserate = ((double)file_srate_old/m_pSampleRate->get());

        double rate;

        // BJW: Touch sensitive wheels: If enabled via the Switch, while the top of the wheel is touched it acts
        // as a "vinyl-like" scratch controller. Playback stops, pitch-independent time stretch is disabled, and
        // the wheel's motion is amplified to produce a scrub effect. Also note that playback speed factors like
        // rateSlider and reverseButton are ignored so that the feel of the wheel is always consistent.
        // TODO: Configurable vinyl stop effect.
        if (wheelTouchSwitch->get() && wheelTouchSensor->get()) {
            // Act as scratch controller
            if (m_pConfig->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt()) {
                // Use vinyl-style pitch bending
                // qDebug() << "Disabling Pitch-Independent Time Stretch for scratching";
                m_bResetPitchIndpTimeStretch = true;
                setPitchIndpTimeStretch(false);
            }
            // Experimental formula
            rate = wheel->get() * 40. * baserate;
        } else if(playButton->get()) {

            // BJW: Reset timestretch mode if required. NB it's intentional that this should not
            // get reset until playing; this enables released spinbacks in stop mode.
            if (m_bResetPitchIndpTimeStretch) {
                setPitchIndpTimeStretch(true);
                m_bResetPitchIndpTimeStretch = false;
                // qDebug() << "Re-enabling Pitch-Independent Time Stretch";
                }
            // BJW: Split up initial rate setting from any wheel influence
            // rate=wheel->get()+(1.+rateSlider->get()*m_pRateRange->get()*m_pRateDir->get())*baserate;
            rate = (1. + rateSlider->get() * m_pRateRange->get() * m_pRateDir->get()) * baserate;
            // Apply jog wheel
            rate += wheel->get(); // / 40.;
            // rate = wheel->get() / 40 + (1. + rateSlider->get() * m_pRateRange->get() * m_pRateDir->get()) * baserate;

            // qDebug() << "wheel " << wheel->get() << ", slider " << rateSlider->get() << ", range " << m_pRateRange->get() << ", dir " << m_pRateDir->get();

            // Apply scratch
            double scratch = m_pControlScratch->get();
            if(!isnan(scratch)) {
                if (scratch < 0.) {
                    rate = rate * (scratch-1.);                
                } else if (scratch > 0.) {
                    rate = rate * (scratch+1.);
                }
            }

            // Apply jog
            // FIXME: Sensitivity should be configurable separately?
            const double fact = m_pRateRange->get();
            double jogVal = m_pJog->get();
            double val = m_jogfilter->filter(jogVal);
            rate += val * fact;
            if(jogVal != 0.)
                m_pJog->set(0.);

            // BJW: Apply reverse button (moved from above)
            if (reverseButton->get()) {
                rate = -rate;
            }

        } else {
            // Stopped. Wheel, jog and scratch controller all scrub through audio.
            double jogVal = m_pJog->get();

            // Don't trust values from m_pControlScratch
            double scratch = m_pControlScratch->get();
            if(isnan(scratch)) {
                scratch = 0.0;
            }
            
            rate=(wheel->get()*40.+scratch+m_jogfilter->filter(jogVal))*baserate; //*10.;
            if(jogVal != 0.)
                m_pJog->set(0.);
        }

        // If searching in progress...
        if (m_pRateSearch->get()!=0.)
        {
            rate = m_pRateSearch->get();
//            m_pScale->setFastMode(true);
        }
//         else
//             m_pScale->setFastMode(false);
        
        // If the rate has changed, set it in the scale object
        if (rate != rate_old)
        {
            // The rate returned by the scale object can be different from the wanted rate!
            rate_old = rate;
            rate = baserate*m_pScale->setTempo(rate/baserate);
            m_pScale->setBaseRate(baserate);
            rate_old = rate;
        }

        bool at_start = false;
        bool at_end = false;
        bool backwards = false;

        if(rate < 0)
            backwards = true;
        
        if((rate == 0) || (filepos_play == 0 && backwards) ||
           (filepos_play == file_length_old && !backwards)) {
            rampOut(pOut, iBufferSize);
            bCurBufferPaused = true;
        } else {

            // Check if we are at the boundaries of the file
            if ((filepos_play<0. && backwards) || (filepos_play>file_length_old && !backwards))
            {
                //qDebug() << "buffer out of range, filepos_play " << filepos_play << ", length " << file_length_old << "i";

                if (!m_bLastBufferPaused)
                    rampOut(pOut, iBufferSize);
                bCurBufferPaused = true;
            } else {

                CSAMPLE *output;
                double idx;

                // Perform scaling of Reader buffer into buffer.
                output = m_pScale->scale(bufferpos_play, iBufferSize);
                idx = m_pScale->getNewPlaypos();
                
                // qDebug() << "idx " << idx << ", buffer pos " << bufferpos_play << ", play " << filepos_play;

                for(int i=0; i<iBufferSize; i++) {
                    pOutput[i] = output[i];
                }

                // Adjust filepos_play
                filepos_play += (idx-bufferpos_play);

                if(filepos_play > file_length_old && readerinfo && file_length_old > 0) {
                    idx -= filepos_play-file_length_old;
                    filepos_play = file_length_old;
                    at_end = true;
                } else if(filepos_play < 0) {
                    idx -= filepos_play;
                    filepos_play = 0;
                    at_start = true;
                }

                // Ensure valid range of idx
                while (idx>READBUFFERSIZE)
                    idx -= (double)READBUFFERSIZE;
                while (idx<0)
                    idx += (double)READBUFFERSIZE;

                // Write buffer playpos
                bufferpos_play = idx;
            }
        }

        // Let RateControl do its logic. This is a temporary hack until this
        // step is just processing a list of EngineControls
        m_pRateControl->process(filepos_play, file_length_old);
        
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

        //
        // Check if end or start of file, and playmode, write new rate, playpos and do wakeall
        // if playmode is next file: set next in playlistcontrol
        //

        // Update playpos slider and bpm display if necessary
        m_iSamplesCalculated += iBufferSize;
        if (m_iSamplesCalculated > (m_pSampleRate->get()/UPDATE_RATE)) {
            if (file_length_old!=0.) {
                double f = math_max(0.,math_min(filepos_play,file_length_old));
                playposSlider->set(f/file_length_old);

                //qDebug() << "f " << f << ", len " << file_length_old << "i, " << f/file_length_old;
            } else {
                playposSlider->set(0.);
            }
            
            if(filebpm != bpmControl->get())
                bpmControl->set(filebpm);
            if(rate != rateEngine->get())
                rateEngine->set(rate);
            m_iSamplesCalculated = 0;
        }

        // Update buffer and abs position. These variables are not in the ControlObject
        // framework because they need very frequent updates.
        if (m_qPlayposMutex.tryLock()) {
            m_dBufferPlaypos = bufferpos_play;
            m_dAbsPlaypos = filepos_play;
            m_dAbsStartpos = filepos_start;
            m_qPlayposMutex.unlock();
        }

        // Update visual control object, this needs to be done more often than the bpm display and playpos slider
        if(file_length_old != 0.) {
            double f = math_max(0.,math_min(filepos_play, file_length_old));
            visualPlaypos->set(f/file_length_old);
        } else {
            visualPlaypos->set(0.);
        }

        // HANDLE END-OF-TRACK MODE

        // If playbutton is pressed, check if we are at start or end of track
        if ((playButton->get() || (fwdButton->get() || backButton->get())) &&
            !m_pTrackEnd->get() && readerinfo &&
            ((filepos_play<=0. && backwards) ||
             ((int)filepos_play>=file_length_old && !backwards)))
        {
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
                //default:
                //qDebug() << "Invalid track end mode: " << m;
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
