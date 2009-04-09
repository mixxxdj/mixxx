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
//#include "readerextractbeat.h"
#include "readerextractwave.h"
#include "enginebufferscalest.h"
#include "enginebufferscalelinear.h"
#include "enginebufferscalereal.h"
//#include "enginebufferscalesrc.h"
#include "enginebufferscaledummy.h"
#include "mathstuff.h"
#include "enginebuffercue.h"

// Static default values for rate buttons
double EngineBuffer::m_dTemp = 0.01;
double EngineBuffer::m_dTempSmall = 0.001;
double EngineBuffer::m_dPerm = 0.01;
double EngineBuffer::m_dPermSmall = 0.001;

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

    m_bTempPress = false;

    m_dAbsPlaypos = 0.;
    m_dBufferPlaypos = 0.;
    m_dAbsStartpos = 0.;

    filepos_play = 0;
    bufferpos_play = 0;

    // Set up temporary buffer for seeking and looping
    m_pTempBuffer = new float[kiTempLength];
    m_dTempFilePos = 0.;
    m_dSeekFilePos = 0.;
    m_bSeekBeat = false;

    m_dBeatFirst = -1;
    m_dBeatInterval = 0.;

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

    // Permanent rate-change buttons
    buttonRatePermDown = new ControlPushButton(ConfigKey(group,"rate_perm_down"));
    connect(buttonRatePermDown, SIGNAL(valueChanged(double)), this, SLOT(slotControlRatePermDown(double)));
    buttonRatePermDownSmall = new ControlPushButton(ConfigKey(group,"rate_perm_down_small"));
    connect(buttonRatePermDownSmall, SIGNAL(valueChanged(double)), this, SLOT(slotControlRatePermDownSmall(double)));
    buttonRatePermUp = new ControlPushButton(ConfigKey(group,"rate_perm_up"));
    connect(buttonRatePermUp, SIGNAL(valueChanged(double)), this, SLOT(slotControlRatePermUp(double)));
    buttonRatePermUpSmall = new ControlPushButton(ConfigKey(group,"rate_perm_up_small"));
    connect(buttonRatePermUpSmall, SIGNAL(valueChanged(double)), this, SLOT(slotControlRatePermUpSmall(double)));

    // Temporary rate-change buttons
    buttonRateTempDown = new ControlPushButton(ConfigKey(group,"rate_temp_down"));
    connect(buttonRateTempDown, SIGNAL(valueChanged(double)), this, SLOT(slotControlRateTempDown(double)));
    buttonRateTempDownSmall = new ControlPushButton(ConfigKey(group,"rate_temp_down_small"));
    connect(buttonRateTempDownSmall, SIGNAL(valueChanged(double)), this, SLOT(slotControlRateTempDownSmall(double)));
    buttonRateTempUp = new ControlPushButton(ConfigKey(group,"rate_temp_up"));
    connect(buttonRateTempUp, SIGNAL(valueChanged(double)), this, SLOT(slotControlRateTempUp(double)));
    buttonRateTempUpSmall = new ControlPushButton(ConfigKey(group,"rate_temp_up_small"));
    connect(buttonRateTempUpSmall, SIGNAL(valueChanged(double)), this, SLOT(slotControlRateTempUpSmall(double)));

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
    
    // Control used to communicate absolute playpos to GUI thread
    //ControlObject *controlabsplaypos = new ControlObject(ConfigKey(group, "absplayposition"));
    //absPlaypos = new ControlEngine(controlabsplaypos);

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

    // Beat event control
    beatEventControl = new ControlPotmeter(ConfigKey(group, "beatevent"));

    // Beat sync (scale buffer tempo relative to tempo of other buffer)
    buttonBeatSync = new ControlPushButton(ConfigKey(group, "beatsync"));
    connect(buttonBeatSync, SIGNAL(valueChanged(double)), this, SLOT(slotControlBeatSync(double)));

    // Audio beat mark toggle
    audioBeatMark = new ControlPushButton(ConfigKey(group, "audiobeatmarks"));

    // Loop button
    buttonLoop = new ControlPushButton(ConfigKey(group, "loop"), true);
    m_pControlObjectBeatLoop = new ControlObject(ConfigKey(group, "beatloop"));

    m_bLoopActive = false;
    m_bLoopMeasureTime = false;
    connect(buttonLoop, SIGNAL(valueChanged(double)), this, SLOT(slotControlLoop(double)));


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
    delete m_pControlObjectBeatLoop;
    delete rateSlider;
    delete m_pScaleLinear;
    delete m_pScaleST;
    delete m_pTrackEnd;
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

float EngineBuffer::getDistanceNextBeatMark()
{
    return 0.;
    /*
       float *p = (float *)reader->getBeatPtr()->getBasePtr();
       int size = reader->getBeatPtr()->getBufferSize();

       int pos = (int)(bufferpos_play*size/READBUFFERSIZE);
       //int pos = (int)(bufferpos_play/size);

       //qDebug() << "s1 " << size << " s2 " << READBUFFERSIZE;

       int i;
       bool found = false;

       for (i=0; i>(pos-size) && !found; --i)
       {
        //qDebug() << "p[" << (pos+i)%size << "] = " << p[(pos+i)%size] << " ";
        if (p[(pos-i)%size] > 0.0)
        {
            found = true;
            //qDebug() << "p[" << (pos+i)%size << "] = " << p[(pos+i)%size] << " ";
        }
       }

       if (found)
       {
        //qDebug() << "found";

        return ((float)i)*(float)READBUFFERSIZE/(float)size;
        //return ((float)i)*(float)size;
       }
       else
        return 0.f;
     */
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

    // Drop any ongoing seeks with crossfading
    m_bSeekBeat = false;
}

const char * EngineBuffer::getGroup()
{
    return group;
}

double EngineBuffer::getRate()
{
    return rateSlider->get()*m_pRateRange->get()*m_pRateDir->get();
}

void EngineBuffer::setTemp(double v)
{
    m_dTemp = v;
}

void EngineBuffer::setTempSmall(double v)
{
    m_dTempSmall = v;
}

void EngineBuffer::setPerm(double v)
{
    m_dPerm = v;
}

void EngineBuffer::setPermSmall(double v)
{
    m_dPermSmall = v;
}

void EngineBuffer::slotControlSeek(double change, bool bBeatSync)
{
    //qDebug() << "seeking... " << change;

    if(isnan(change) || change > 1.0 || change < 0.0) {
        // This seek is ridiculous.
        return;
    }

    // If seeking, and in loop mode, disable loop
    if (buttonLoop->get())
    {
        buttonLoop->set(0.);
        slotControlLoop(0.);
    }

    // Find new playpos
    double new_playpos = round(change*file_length_old);
    if (new_playpos > file_length_old)
        new_playpos = file_length_old;
    if (new_playpos < 0.)
        new_playpos = 0.;

    // Seek using crossfade (and possible beat syncronized?)
    if (bBeatSync && m_pControlObjectBeatLoop->get() && playButton->get())
    {
        // Copy current buffer to temporary playback buffer
        // Store file index for start position of temporary buffer
        int pos = (int)bufferpos_play;
        for (int i=0; i<kiTempLength; ++i)
        {
            m_pTempBuffer[i] = m_pWaveBuffer[pos++];
            pos = pos%READBUFFERSIZE;
        }
        m_dTempFilePos = filepos_play;

        // Check if we have reliable beat information. If that is the case, perform beat syncronized seek
        if (m_dBeatFirst>=0. && m_dBeatInterval>0.)
        {
            //qDebug() << "sync, beat first " << m_dBeatFirst;
            //qDebug() << "interval " << m_dBeatInterval;

            // Distance to next beat from current playpos
            double dBeatDistance = (m_dBeatFirst + m_dBeatInterval*4.*ceil((filepos_play-m_dBeatFirst)/(m_dBeatInterval*4.)))-filepos_play;

            //qDebug() << "playpos " << filepos_play << ", new play pos " << new_playpos << ", beat dist " << dBeatDistance;

            // Adjust new seek position to beat syncronious with current play position
            new_playpos = (m_dBeatFirst - dBeatDistance + m_dBeatInterval*4.*ceil((new_playpos-m_dBeatFirst)/(m_dBeatInterval*4.)));

            //qDebug() << "adj new play pos " << new_playpos;
        }

        // Ensure new playpos is in valid range
        if (new_playpos<0.)
            new_playpos = 0.;
        else if (new_playpos>=file_length_old)
            new_playpos = file_length_old-1;
    }

    // Ensure that the file position is even (remember, stereo channel files...)
    if (!even((int)new_playpos))
        new_playpos--;

    // Seek reader
    //qDebug() << "seek " << new_playpos;
    reader->requestSeek(new_playpos);

    m_dSeekFilePos = new_playpos;

    setNewPlaypos(new_playpos);

    if (bBeatSync && m_pControlObjectBeatLoop->get())
    {
        m_bSeekBeat = true;
        m_bSeekCrossfade = false;
    }
}

void EngineBuffer::slotControlSeekAbs(double abs, bool bBeatSync)
{
    slotControlSeek(abs/file_length_old, bBeatSync);
}

void EngineBuffer::slotControlLoop(double v)
{
    if (m_pControlObjectBeatLoop->get())
    {
        // Automatic beat loop

        if (!m_bLoopActive && v && m_dBeatInterval>0.)
        {
            // Loop length include one beat before the actual loop and a lot of samples after
            m_dLoopLength = (double)(int)(m_dBeatInterval*4.);
            if (!even((long)m_dLoopLength))
                m_dLoopLength++;

            if (m_dLoopLength>(double)kiTempLength)
                m_dLoopLength = (double)kiTempLength;

            // Loop starting position
            m_dTempFilePos = filepos_play;

            m_bLoopActive = false;
        }
        else if (v==0. && m_bLoopActive)
        {
            m_pScale->clear();
            m_bLoopActive = false;
        }
    }
    else
    {
        // Manual loop
        if (!m_bLoopActive && v)
        {
            if (!m_bLoopMeasureTime)
            {
                // Loop starting position
                m_dTempFilePos = filepos_play;
                m_bLoopMeasureTime = true;
            }
            else
            {
                m_dLoopLength = round(filepos_play-m_dTempFilePos);
                if (!even((long)m_dLoopLength))
                    m_dLoopLength++;
                if (m_dLoopLength>(double)kiTempLength)
                    m_dLoopLength = (double)kiTempLength;
                m_bLoopMeasureTime = false;
                //qDebug() << "start " << m_dTempFilePos << ", length " << m_dLoopLength;
            }
        }
        else if (v==0. && m_bLoopActive)
        {
            m_pScale->clear();
            m_bLoopActive = false;
            m_bLoopMeasureTime = false;
        }
    }
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

void EngineBuffer::slotControlRatePermDown(double)
{
    // Adjusts temp rate down if button pressed
    if (buttonRatePermDown->get())
        rateSlider->sub(m_pRateDir->get() * m_dPerm / (100. * m_pRateRange->get()));
}

void EngineBuffer::slotControlRatePermDownSmall(double)
{
    // Adjusts temp rate down if button pressed
    if (buttonRatePermDownSmall->get())
        rateSlider->sub(m_pRateDir->get() * m_dPermSmall / (100. * m_pRateRange->get()));
}

void EngineBuffer::slotControlRatePermUp(double)
{
    // Adjusts temp rate up if button pressed
    if (buttonRatePermUp->get())
        rateSlider->add(m_pRateDir->get() * m_dPerm / (100. * m_pRateRange->get()));
}

void EngineBuffer::slotControlRatePermUpSmall(double)
{
    // Adjusts temp rate up if button pressed
    if (buttonRatePermUpSmall->get())
        rateSlider->add(m_pRateDir->get() * m_dPermSmall / (100. * m_pRateRange->get()));
}

void EngineBuffer::slotControlRateTempDown(double)
{
    // Adjusts temp rate down if button pressed, otherwise set to 0.
    if (buttonRateTempDown->get() && !m_bTempPress)
    {
        m_bTempPress = true;
        m_dOldRate = rateSlider->get();
        rateSlider->sub(m_pRateDir->get() * m_dTemp / (100. * m_pRateRange->get()));
    }
    else if (!buttonRateTempDown->get())
    {
        m_bTempPress = false;
        rateSlider->set(m_dOldRate);
    }
}

void EngineBuffer::slotControlRateTempDownSmall(double)
{
    // Adjusts temp rate down if button pressed, otherwise set to 0.
    if (buttonRateTempDownSmall->get() && !m_bTempPress)
    {
        m_bTempPress = true;
        m_dOldRate = rateSlider->get();
        rateSlider->sub(m_pRateDir->get() * m_dTempSmall / (100. * m_pRateRange->get()));
    }
    else if (!buttonRateTempDownSmall->get())
    {
        m_bTempPress = false;
        rateSlider->set(m_dOldRate);
    }
}

void EngineBuffer::slotControlRateTempUp(double)
{
    // Adjusts temp rate up if button pressed, otherwise set to 0.
    if (buttonRateTempUp->get() && !m_bTempPress)
    {
        m_bTempPress = true;
        m_dOldRate = rateSlider->get();
        rateSlider->add(m_pRateDir->get() * m_dTemp / (100. * m_pRateRange->get()));
    }
    else if (!buttonRateTempUp->get())
    {
        m_bTempPress = false;
        rateSlider->set(m_dOldRate);
    }
}

void EngineBuffer::slotControlRateTempUpSmall(double)
{
    // Adjusts temp rate up if button pressed, otherwise set to 0.
    if (buttonRateTempUpSmall->get() && !m_bTempPress)
    {
        m_bTempPress = true;
        m_dOldRate = rateSlider->get();
        rateSlider->add(m_pRateDir->get() * m_dTempSmall / (100. * m_pRateRange->get()));
    }
    else if (!buttonRateTempUpSmall->get())
    {
        m_bTempPress = false;
        rateSlider->set(m_dOldRate);
    }
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

            // Adjust phase
            adjustPhase();
        }
    }

}

void EngineBuffer::adjustPhase()
{
    // Search for distance from playpos to beat mark of both buffers
    float fThisDistance = getDistanceNextBeatMark();
    float fOtherDistance = m_pOtherEngineBuffer->getDistanceNextBeatMark();

    //qDebug() << "this " << fThisDistance << ", other " << fOtherDistance << " diff " << fOtherDistance-fThisDistance;

    //filepos_play += fOtherDistance-fThisDistance;
    bufferpos_play = bufferpos_play+fOtherDistance-fThisDistance;
    while (bufferpos_play<0.)
        bufferpos_play += (double)READBUFFERSIZE;
    while (bufferpos_play>(double)READBUFFERSIZE)
        bufferpos_play -= (double)READBUFFERSIZE;

    //qDebug() << "buffer pos " << bufferpos_play << ", file pos " << filepos_play;
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
    CSAMPLE * pOutput = (CSAMPLE *)pOut;

    bool bCurBufferPaused = false;

    //Q_ASSERT( scale->getNewPlaypos() == 0);
    // pause can be locked if the reader is currently loading a new track.
    if (!m_pTrackEnd->get() && pause.tryLock())
    {
        // Try to fetch info from the reader
        bool readerinfo = false;
        long int filepos_start = 0;
        long int filepos_end = 0;
        if (reader->tryLock())
        {
            file_length_old = reader->getFileLength();
            file_srate_old = reader->getFileSrate();
            filepos_start = reader->getFileposStart();
            filepos_end = reader->getFileposEnd();
            reader->setFileposPlay((long int)filepos_play);
            reader->setRate(rate_old);

            reader->unlock();

            m_dBeatFirst = 0.;
            m_dBeatInterval = (m_pFileBpm->get()/60.)*file_srate_old*2.;

            readerinfo = true;
        }
        //else
        //     qDebug() << "Did not read!!!";

        //qDebug() << "filepos_play " << filepos_play << ",\tstart " << filepos_start << ",\tend " << filepos_end << "\t info " << readerinfo << ", len " << file_length_old;


        //Clamp the wheel value (workaround for rotary bug that crops up with the Hercules controllers)
        //The downside to this is that you can't use the jogwheels at their "lowest" possible speed...
        // BJW: Commented out -- should no longer be required as ControlTTRotary now regards 63-65 as zero.
        // If this is insufficient for Hercules, suggest modifying the HERC_JOG value calculation in configobject.cpp?
        // if (fabs(wheel->get()) <= 0.001250)
        //	wheel->set(0.0f);


        //
        // Calculate rate
        //

        // Find BPM adjustment factor
        // BJW: Unused?
        // double bpmrate = 1.;

        double filebpm = m_pFileBpm->get();

        // Determine direction of playback from reverse button
        // BJW: Moved so it only takes effect during playback, not scratching
        // double dir = 1.;
        // if (reverseButton->get()==1.)
        //    dir = -1.;

        double baserate = ((double)file_srate_old/m_pSampleRate->get());
        //qDebug() << "baserate " << baserate;

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
        }
        // BJW: Enabled fwd and back buttons, and made independent of playback
        // BJW: Except that this should be handled by m_pRateSearch etc
        // else if (fwdButton->get())
        // {
        //     rate = baserate * 5.;
        // }
        // else if (backButton->get())
        // {
        //     rate = baserate * -5.;
        // }
        else if (playButton->get())
        {
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
            if(jogVal != 0.0) 
                m_pJog->set(0.);

            // BJW: Apply reverse button (moved from above)
            if (reverseButton->get()) {
                rate = -rate;
            }
        }
        else
        {
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

        // BJW: Disabled this. bpmrate was always 1. [Master],rate doesn't appear to be useable.
        // And we don't want the reverse button to influence direction in non-playback modes.
        // rate *= dir*bpmrate*(1.+m_pMasterRate->get()*m_pRateRange->get());

        //qDebug() << "bpmrate " << bpmrate << ", master " << m_pMasterRate->get();

        // If searching in progress...
        if (m_pRateSearch->get()!=0.)
        {
            rate = m_pRateSearch->get();
//            m_pScale->setFastMode(true);
        }
//         else
//             m_pScale->setFastMode(false);

        //qDebug() << "rateslider " << rateSlider->get() << ", ratedir " << m_pRateDir->get() << ", wheel " << wheel->get() << ", scratch " << m_pControlScratch->get();

        //qDebug() << "rate " << rate << ", bpmrate " << bpmrate << ", file srate " << (double)file_srate_old << ", play srate " << (double)getPlaySrate() << ", baserate " << baserate;

/*
        //
        // Beat event control. Assume forward play
        //

        // Search for next beat
        ReaderExtractBeat *readerbeat = reader->getBeatPtr();
        bool *beatBuffer = (bool *)readerbeat->getBasePtr();
        int nextBeatPos;
        int beatBufferPos = bufferpos_play*((CSAMPLE)readerbeat->getBufferSize()/(CSAMPLE)READBUFFERSIZE);
        int i;
        for (i=beatBufferPos+1; i<beatBufferPos+readerbeat->getBufferSize(); i++)
            if (beatBuffer[i%readerbeat->getBufferSize()])
                break;
        if (beatBuffer[i%readerbeat->getBufferSize()])
            // Next beat was found
            nextBeatPos = (i%readerbeat->getBufferSize())*(READBUFFERSIZE/readerbeat->getBufferSize());
        else
            // No next beat was found
            nextBeatPos = bufferpos_play+readerbeat->getBufferSize();

        double event = beatEventControl->get();
        if (event > 0.)
        {
            qDebug() << "event: " << event << ", playpos " << bufferpos_play << ", nextBeatPos " << nextBeatPos;
            //
            // Play next event
            //

            // Reset beat event control
        beatEventControl->set(0.);

            if (oldEvent>0.)
            {
                // Adjust bufferplaypos
                bufferpos_play = nextBeatPos;

                // Search for a new next beat position
                ReaderExtractBeat *readerbeat = reader->getBeatPtr();
                bool *beatBuffer = (bool *)readerbeat->getBasePtr();

                int beatBufferPos = bufferpos_play*((CSAMPLE)readerbeat->getBufferSize()/(CSAMPLE)READBUFFERSIZE);
                int i;
                for (i=beatBufferPos+1; i<beatBufferPos+readerbeat->getBufferSize(); i++)
                {
    //                qDebug() << "i " << i;
                    if (beatBuffer[i%readerbeat->getBufferSize()])
                        break;
                }
                if (beatBuffer[i%readerbeat->getBufferSize()])
                    // Next beat was found
                    nextBeatPos = (i%readerbeat->getBufferSize())*(READBUFFERSIZE/readerbeat->getBufferSize());
                else
                    // No next beat was found
                    nextBeatPos = -1;
            }

            oldEvent = 1.;
        }
        else if (oldEvent==0.)
            nextBeatPos = -1;

   //        qDebug() << "NextBeatPos :" << nextBeatPos << ", bufDiff: " << READBUFFERSIZE/readerbeat->getBufferSize();
 */



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

        // Determine playback direction
        bool backwards = false;
        if (rate<0.)
            backwards = true;

        //qDebug() << "rate: " << rate << ", filepos_play: " << filepos_play << ", file_length_old " << file_length_old;

        if ((rate==0.) || (filepos_play==0. && backwards) ||
            (filepos_play==(float)file_length_old && !backwards))
        {
            rampOut(pOut, iBufferSize);
            bCurBufferPaused = true;
        }
        else
        {
            // Check if we are at the boundaries of the file
            if ((filepos_play<0. && backwards) || (filepos_play>file_length_old && !backwards))
            {
                //qDebug() << "buffer out of range, filepos_play " << filepos_play << ", length " << file_length_old << "i";

                if (!m_bLastBufferPaused)
                    rampOut(pOut, iBufferSize);
                bCurBufferPaused = true;
            }
            else
            {
                double idx;
                CSAMPLE * output;
                if (!m_bSeekBeat)
                {
                    // Are we looping?
                    if (buttonLoop->get() && !m_bLoopActive && !m_bLoopMeasureTime)
                    {
                        // If all loop samples are in the current buffer, copy them to temporary playback buffer
//                         qDebug() << "temp file pos " << m_dTempFilePos << ", loop len " << m_dLoopLength << ", added " << m_dTempFilePos+m_dLoopLength << ", filepos end " << filepos_end;
                        if (readerinfo && m_dTempFilePos+m_dLoopLength<(double)filepos_end)
                        {
                            // Copy loop
                            int pos = (int)(bufferpos_play-(filepos_play-m_dTempFilePos));
                            while (pos<0)
                                pos += READBUFFERSIZE;
                            for (int i=0; i<m_dLoopLength; ++i)
                            {
                                m_pTempBuffer[i] = m_pWaveBuffer[pos];
                                pos = (pos+1)%READBUFFERSIZE;
                            }
                            m_pScale->clear();
                            m_bLoopActive = true;
                        }
                    }

                    if (buttonLoop->get() && m_bLoopActive)
                    {
                        // Current position in temporaray buffer
                        double dTempPlayPos = filepos_play-m_dTempFilePos;

//                         qDebug() << "file playpos " << filepos_play << ", loop file start " << m_dTempFilePos << ", temp buffer pos " << dTempPlayPos << ", loop length " << m_dLoopLength;

                        //Q_ASSERT(dTempPlayPos>=0. && dTempPlayPos<m_dLoopLength);

                        while (dTempPlayPos>m_dLoopLength)
                            dTempPlayPos -= m_dLoopLength;
                        while (dTempPlayPos<0.)
                            dTempPlayPos += m_dLoopLength;

                        // Perform scaling of the temporary buffer into buffer
                        int request = iBufferSize;

                        //Q_ASSERT(request>0 && request<=iBufferSize);

                        output = m_pScale->scale(dTempPlayPos, request, m_pTempBuffer, (int)m_dLoopLength);

                        double dNewPos = m_pScale->getNewPlaypos();
                        if (dNewPos>=m_dLoopLength)
                            dNewPos -= m_dLoopLength;

                        idx = bufferpos_play + (dNewPos-dTempPlayPos);

//                         qDebug() << "idx " << idx;

//                         int i;
//                         for (i=0; i<request; i++)
//                             pOutput[i] = output[i];

                        /*
                           if (request<iBufferSize)
                           {

                           output = m_pScale->scale(0, iBufferSize-request, m_pTempBuffer, m_dLoopLength);

                           for (i=request; i<iBufferSize; i++)
                            pOutput[i] = output[i-request];

                           idx -= m_dLoopLength;
                           }
                         */



//                         output = m_pScale->scale(dTempCurBufferPlayPos, iBufferSize, m_pTempBuffer, 100000);
                        // "Correctly" advance playpos
//                         idx = bufferpos_play + (m_pScale->getNewPlaypos()-dTempCurBufferPlayPos);
                    }
                    else
                    {

                        // Perform scaling of Reader buffer into buffer.
                        output = m_pScale->scale(bufferpos_play, iBufferSize);
                        idx = m_pScale->getNewPlaypos();
                        // qDebug() << "idx " << idx << ", buffer pos " << bufferpos_play << ", play " << filepos_play;
                    }
                }
                else
                {
                    // Check if the crossfade is finished
                    if (m_bSeekCrossfade && m_dSeekCrossfadeEndFilePos<filepos_play)
                    {
                        m_bSeekBeat = false;

                        //qDebug() << "seek & crossfading done, play " << filepos_play << ", cross end " << m_dSeekCrossfadeEndFilePos;

                        // Even if we are in the process of doing a beat
                        // syncronious seek, we do scaling of the playpos buffer here, to advance the playpos correctly.
                        output = m_pScale->scale(bufferpos_play, iBufferSize);
                        idx = m_pScale->getNewPlaypos();

                    }
                    else
                    {
                        //qDebug() << "play " << filepos_play << ", end " << (int)filepos_end;

                        // Current play position in temporary buffer
                        double dTempCurBufferPlayPos = filepos_play-m_dSeekFilePos;

                        // Do the crossfade and set number of buffers before we should switch to the non-temporary
                        // buffer
                        double dCrossfade = m_dBeatInterval/4.;
                        if (dCrossfade==0)
                            dCrossfade = 22050;

                        // Check if enough data has been read from disk, to do the crossfade in the temporary buffer
                        if (readerinfo && filepos_play<filepos_end-dCrossfade && !m_bSeekCrossfade)
                        {
//                             qDebug() << "*play " << filepos_play << ", end " << (int)filepos_end;
                            // qDebug() << "Doing crossfade...";


                            // Distance to next beat from current play position
                            double dSeekStartCrossfade = (m_dBeatFirst + dCrossfade*ceil((filepos_play-m_dBeatFirst)/dCrossfade)) - filepos_play;

                            // Distance to second next beat from current play position
                            double dSeekEndCrossfade = dSeekStartCrossfade + dCrossfade;
                            m_dSeekCrossfadeEndFilePos = filepos_play + dSeekEndCrossfade;

                            //qDebug() << "play " << filepos_play << ", start " << dSeekStartCrossfade << ", end " << dSeekEndCrossfade << ", interval " << m_dBeatInterval;

                            // Do crossfade in temporary buffer
                            for (double i=0.; i<ceil(dCrossfade); ++i)
                            {
                                m_pTempBuffer[(int)(i+dTempCurBufferPlayPos+dSeekStartCrossfade)] = ((dCrossfade-i)/dCrossfade)*m_pTempBuffer[(int)(i+dTempCurBufferPlayPos+dSeekStartCrossfade)] +
                                                                                                    (i/dCrossfade)*m_pWaveBuffer[(int)(i+bufferpos_play+dSeekStartCrossfade)];
                                //qDebug() << "old " << ((dCrossfade-i)/dCrossfade) << "*" << m_pTempSeekBuffer[(int)(i+dTempCurBufferPlayPos+dSeekStartCrossfade)] << ", new " << (i/dCrossfade) << "*" << m_pWaveBuffer[(int)(i+bufferpos_play+dSeekStartCrossfade)];
                            }
                            m_bSeekCrossfade = true;
                        }
                        // Perform scaling of the temporary buffer into buffer
                        output = m_pScale->scale(dTempCurBufferPlayPos, iBufferSize, m_pTempBuffer, kiTempLength);

                        // "Correctly" advance playpos
                        idx = bufferpos_play + (m_pScale->getNewPlaypos()-dTempCurBufferPlayPos);
                    }


                }

                int i;
//                 if (!buttonLoop->get())
                {
                    for (i=0; i<iBufferSize; i++)
                        pOutput[i] = output[i];
                }

/*
                // If a beat occours in current buffer mark it by led or in audio
                // This code currently only works in forward playback.
                ReaderExtractBeat *readerbeat = reader->getBeatPtr();
                if (readerbeat!=0)
                {
                    // Check if we need to set samples from a previos beat mark
                    if (audioBeatMark->get()==1. && m_iBeatMarkSamplesLeft>0)
                    {
                        int to = (int)math_min(m_iBeatMarkSamplesLeft, idx-bufferpos_play);
                        for (int j=0; j<to; j++)
                            pOutput[j] = 30000.;
                        m_iBeatMarkSamplesLeft = math_max(0,m_iBeatMarkSamplesLeft-to);
                    }

                    float *beatBuffer = (float *)readerbeat->getBasePtr();
                    int chunkSizeDiff = READBUFFERSIZE/readerbeat->getBufferSize();
                    //qDebug() << "from " << (int)floor(bufferpos_play) << "-" << (int)floor(idx);
                    //Q_ASSERT( (int)floor(bufferpos_play) <= (int)floor(idx) );
                    for (i=(int)floor(bufferpos_play); i<=(int)floor(idx); i++)
                    {
                        if (((i%chunkSizeDiff)==0) && (beatBuffer[i/chunkSizeDiff]>0.0))
                        {
   //                            qDebug() << "" << i/chunkSizeDiff << ": " << beatBuffer[i/chunkSizeDiff];

                            // Audio beat mark
                            if (audioBeatMark->get()==1.)
                            {
                                int from = (int)(i-bufferpos_play);
                                int to = (int)math_min(i-bufferpos_play+audioBeatMarkLen, idx-bufferpos_play);
                                for (int j=from; j<to; j++)
                                    pOutput[j] = 30000.;
                                m_iBeatMarkSamplesLeft = math_max(0,audioBeatMarkLen-(to-from));

                                //qDebug() << "audio beat mark";
                                //qDebug() << "mark " << i2/chunkSizeDiff << ": " << (int)math_max(0 << "-" << i-bufferpos_play);
                            }
 #ifdef __UNIX__
                            // PowerMate led
   //                            if (powermate!=0)
   //                                 powermate->led();
 #endif
                        }
                    }
                }
 */

                // Write file playpos
                //qDebug() << "filepos_play " << filepos_play << ", idx " << idx << ", bufferpos_play " << bufferpos_play << ", oldlen " << file_length_old << "i";

                filepos_play += (idx-bufferpos_play);
                if (readerinfo && file_length_old>0 && filepos_play>file_length_old)
                {
                    idx -= filepos_play-file_length_old;
                    filepos_play = file_length_old;
                    at_end = true;
                    //qDebug() << "end";
                }
                else if (filepos_play<0)
                {
                    //qDebug() << "start " << filepos_play;
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

                //qDebug() << "bufferpos_play " << idx << ", old " << oldidx;

            }
        }

        //
        // Check if more samples are needed from reader, and wake it up if necessary.
        //
        if (readerinfo && filepos_end>0)
        {
            //qDebug() << "play " << filepos_play << ",\tstart " << filepos_start << ",\tend " << filepos_end << "\t info " << readerinfo << ", len " << file_length_old << ", " << fabs(filepos_play-filepos_start) << "<" << (float)(READCHUNKSIZE*(READCHUNK_NO/2-1));
            //qDebug() << "checking";
            // Part of this if condition is commented out to ensure that more block is
            // loaded at the end of an file to fill the buffer with zeros

            if (!m_bLoopActive)
            {
                if (filepos_play>filepos_end || filepos_play<filepos_start)
                {
                    //                 qDebug() << "wake seek play " << filepos_play << ", start " << filepos_start << ", end " << filepos_end;

                    // Ensure sane operation during too-fast-for-machine forward:
                    //reader->requestSeek(filepos_play);

                    reader->wake();
                }
                else if ((filepos_end - filepos_play < READCHUNKSIZE*(READCHUNK_NO/2-1)))
                {
                    //                 qDebug() << "wake fwd play " << filepos_play << ", start " << filepos_start << ", end " << filepos_end;
                    reader->wake();
                }
                else if (fabs(filepos_play-filepos_start)<(float)(READCHUNKSIZE*(READCHUNK_NO/2-1)))
                {
                    //qDebug() << "wake back";
                    reader->wake();
                }
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

//          qDebug() << "filepos_play " << filepos_play << ", len " << file_length_old << ", back " << backwards << ", play " << playButton->get();

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
                //if (!backwards)
            {
                //qDebug() << "next";
                m_pTrackEnd->set(1.);
            }
                /*
                   else
                   {
                    qDebug() << "stop";
                    playButton->set(0.);
                    //m_pTrackEnd->set(1.);
                   }
                 */
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

        pause.unlock();
    }
    else
    {
        if (!m_bLastBufferPaused)
            rampOut(pOut, iBufferSize);
        bCurBufferPaused = true;
    }

    // Force ramp in if this is the first buffer during a play
    if (m_bLastBufferPaused && !bCurBufferPaused)
    {
        //qDebug() << "ramp in";
        // Ramp from zero
        int iLen = math_min(iBufferSize, kiRampLength);
        float fStep = pOutput[iLen-1]/(float)iLen;
        for (int i=0; i<iLen; ++i)
            pOutput[i] = fStep*i;
    }

    m_bLastBufferPaused = bCurBufferPaused;

    m_fLastSampleValue = pOutput[iBufferSize-1];
//    qDebug() << "last " << m_fLastSampleValue;

    
    //Debugging the weird crackle that occurs when you switch directions - Albert
/*
    qDebug() << pOutput[0] << pOutput[1];
    qDebug() << pOutput[2] << pOutput[3];

    qDebug() << pOutput[iBufferSize-4] << pOutput[iBufferSize-3];
    qDebug() << pOutput[iBufferSize-2] << pOutput[iBufferSize-1];

    pOutput[0] = pOutput[2];
    pOutput[1] = pOutput[3];
*/
    //for (int i=0; i<iBufferSize; ++i)
    //    qDebug() << pOutput[i];

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
