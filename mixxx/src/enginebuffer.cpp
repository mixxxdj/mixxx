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

#include "enginebuffer.h"

#include <qevent.h>
#include "configobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "controlbeat.h"
#include "reader.h"
#include "readerextractbeat.h"
#include "enginebufferscalesrc.h"
#include "powermate.h"
#include "wvisualwaveform.h"
#include "visual/visualchannel.h"
#include "mathstuff.h"
#include "player.h"


// Static default values for rate buttons
double EngineBuffer::m_dTemp = 0.01;
double EngineBuffer::m_dTempSmall = 0.001;
double EngineBuffer::m_dPerm = 0.01;
double EngineBuffer::m_dPermSmall = 0.001;

EngineBuffer::EngineBuffer(PowerMate *_powermate, const char *_group)
{
    group = _group;
    powermate = _powermate;

    m_pOtherEngineBuffer = 0;

    m_bTempPress = false;

    m_dAbsPlaypos = 0.;
    m_dBufferPlaypos = 0.;

    // Play button
    playButton = new ControlPushButton(ConfigKey(group, "play"), true);
    connect(playButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlPlay(double)));
    playButton->set(0);

    // Reverse button
    reverseButton = new ControlPushButton(ConfigKey(group, "reverse"));
    reverseButton->set(0);

    // Fwd button
    fwdButton = new ControlPushButton(ConfigKey(group, "fwd"));
    connect(fwdButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlFastFwdBack(double)));
    fwdButton->set(0);

    // Back button
    backButton = new ControlPushButton(ConfigKey(group, "back"));
    connect(backButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlFastFwdBack(double)));
    backButton->set(0);

    // Start button
    startButton = new ControlPushButton(ConfigKey(group, "start"));
    connect(startButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlStart(double)));
    startButton->set(0);

    // End button
    endButton = new ControlPushButton(ConfigKey(group, "end"));
    connect(endButton, SIGNAL(valueChanged(double)), this, SLOT(slotControlEnd(double)));
    endButton->set(0);

    // Cue set button:
    buttonCueSet = new ControlPushButton(ConfigKey(group, "cue_set"));
    connect(buttonCueSet, SIGNAL(valueChanged(double)), this, SLOT(slotControlCueSet(double)));

    // Cue goto button:
    buttonCueGoto = new ControlPushButton(ConfigKey(group, "cue_goto"));
    connect(buttonCueGoto, SIGNAL(valueChanged(double)), this, SLOT(slotControlCueGoto(double)));

    // Cue point
    cuePoint = new ControlObject(ConfigKey(group, "cue_point"));

    // Cue preview button:
    buttonCuePreview = new ControlPushButton(ConfigKey(group, "cue_preview"));
    connect(buttonCuePreview, SIGNAL(valueChanged(double)), this, SLOT(slotControlCuePreview(double)));

    // Playback rate slider
    rateSlider = new ControlPotmeter(ConfigKey(group, "rate"), -1.f, 1.f);

    // Range of rate
    m_pRateRange = new ControlObject(ConfigKey(group, "rateRange"));
    m_pRateRange->set(0.1);
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

    // Scratch controller
    m_pControlScratch = new ControlTTRotary(ConfigKey(group, "scratch"));

    // Slider to show and change song position
    playposSlider = new ControlPotmeter(ConfigKey(group, "playposition"), 0., 1.);
    connect(playposSlider, SIGNAL(valueChanged(double)), this, SLOT(slotControlSeek(double)));

    // Control used to communicate absolute playpos to GUI thread
    //ControlObject *controlabsplaypos = new ControlObject(ConfigKey(group, "absplayposition"));
    //absPlaypos = new ControlEngine(controlabsplaypos);

    // m_pTrackEnd is used to signal when at end of file during playback
    m_pTrackEnd = new ControlObject(ConfigKey(group, "TrackEnd"));

    // Direction of rate slider
    m_pRateDir = new ControlObject(ConfigKey(group, "rate_dir"));

    // TrackEndMode determines what to do at the end of a track
    m_pTrackEndMode = new ControlObject(ConfigKey(group,"TrackEndMode"));

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

    // Control file changed
//    filechanged = new ControlEngine(controlfilechanged);
//    filechanged->setNotify(this,(EngineMethod)&EngineBuffer::newtrack);

    m_bCuePreview = false;

    setNewPlaypos(0.);

    reader = new Reader(this, &pause);
    read_buffer_prt = reader->getBufferWavePtr();
    file_length_old = -1;
    file_srate_old = 0;
    rate_old = 0;

    m_iBeatMarkSamplesLeft = 0;

    m_bLastBufferPaused = true;
    m_fLastSampleValue = 0;

    // Construct scaling object
    scale = new EngineBufferScaleSRC(reader->getWavePtr());

    oldEvent = 0.;

    // Used in update of playpos slider
    m_iSamplesCalculated = 0;


    reader->start();
}

EngineBuffer::~EngineBuffer()
{
    delete playButton;
    delete wheel;
    delete m_pControlScratch;
    delete rateSlider;
    delete scale;
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

void EngineBuffer::setQuality(int q)
{
    // Change sound interpolation quality
    scale->setQuality(q);
}

void EngineBuffer::setVisual(WVisualWaveform *pVisualWaveform)
{
    VisualChannel *pVisualChannel = 0;
    // Try setting up visuals
    if (pVisualWaveform)
    {
        // Add buffer as a visual channel
        pVisualChannel = pVisualWaveform->add(group);
        reader->addVisual(pVisualChannel);
    }
}

float EngineBuffer::getDistanceNextBeatMark()
{
    float *p = (float *)reader->getBeatPtr()->getBasePtr();
    int size = reader->getBeatPtr()->getBufferSize();

    int pos = (int)(bufferpos_play*size/READBUFFERSIZE);
    //int pos = (int)(bufferpos_play/size);

    //qDebug("s1 %i s2 %i",size,READBUFFERSIZE);

    int i;
    bool found = false;

    for (i=0; i>(pos-size) && !found; --i)
    {
        //qDebug("p[%i] = %f ",(pos+i)%size,p[(pos+i)%size]);
        if (p[(pos-i)%size] > 0.0)
        {
            found = true;
            qDebug("p[%i] = %f ",(pos+i)%size,p[(pos+i)%size]);
        }
    }

    /*
    for (i=0; i<(size-pos) && !found; ++i) {

        //qDebug("p[%i] = %f ",(pos+i)%size,p[(pos+i)%size]);
        if (p[(pos+i)%size] > 0.0) {
            qDebug("found: p[%i] = %f ",(pos+i)%size,p[(pos+i)%size]);
            found = true;
        }
    }
    if( !found )
    */

    if (found)
    {
        //qDebug("found" );

        return ((float)i)*(float)READBUFFERSIZE/(float)size;
        //return ((float)i)*(float)size;
    }
    else
        return 0.f;
}

Reader *EngineBuffer::getReader()
{
    return reader;
}

float EngineBuffer::getBpm()
{
    return bpmControl->get();
}

void EngineBuffer::setOtherEngineBuffer(EngineBuffer *pOtherEngineBuffer)
{
    if (!m_pOtherEngineBuffer)
        m_pOtherEngineBuffer = pOtherEngineBuffer;
    else
        qFatal("EngineBuffer: Other engine buffer already set!");
}

void EngineBuffer::setNewPlaypos(double newpos)
{
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
}

const char *EngineBuffer::getGroup()
{
    return group;
}

float EngineBuffer::getRate()
{
    return rateSlider->get()*m_pRateRange->get();
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

void EngineBuffer::slotControlSeek(double change)
{
    //qDebug("seeking... %f",change);

    // Find new playpos
    double new_playpos = round(change*file_length_old);
    if (!even((int)new_playpos))
        new_playpos--;
    if (new_playpos > file_length_old)
        new_playpos = file_length_old;
    if (new_playpos < 0.)
        new_playpos = 0.;

    // Seek reader
    //qDebug("seek %f",new_playpos);
    reader->requestSeek(new_playpos);

    m_iBeatMarkSamplesLeft = 0;
//    filepos_play_exchange.write(filepos_play);
//    file->seek((long unsigned)filepos_play);
//    visualPlaypos.tryWrite(
}

// Set the cue point at the current play position:
void EngineBuffer::slotControlCueSet(double)
{
    double cue = max(0.,round(filepos_play-Player::getBufferSize()));
    if (!even((int)cue))
        cue--;
    cuePoint->set(cue);
}

// Goto the cue point:
void EngineBuffer::slotControlCueGoto(double pos)
{
    if (pos!=0.)
    {
        // Set cue point if play is not pressed
        if (playButton->get()==0.)
        {
            slotControlCueSet();

            // Start playing
            playButton->set(1.);
        }
        else
        {
            // Seek to cue point
            reader->requestSeek(cuePoint->get());
            m_iBeatMarkSamplesLeft = 0;
        }
    }
}

void EngineBuffer::slotControlCuePreview(double)
{
    // Set cue point if play is not pressed
    if (playButton->get()==0.)
        slotControlCueSet();

    if (buttonCuePreview->get()==0.)
    {
        // Stop playing (set playbutton to stoped) and seek to cue point
        playButton->set(0.);
        m_bCuePreview = false;
        reader->requestSeek(cuePoint->get());
        m_iBeatMarkSamplesLeft = 0;
    }
    else if (!m_bCuePreview)
    {
        // Seek to cue point and start playing
        m_bCuePreview = true;

        if (playButton->get()==0.)
            playButton->set(1.);
        else
        {
            // Seek to cue point
            reader->requestSeek(cuePoint->get());
            m_iBeatMarkSamplesLeft = 0;
        }
    }
}

void EngineBuffer::slotControlPlay(double)
{
    slotControlCueSet();
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
    ReaderExtractBeat *beat = reader->getBeatPtr();
    if (beat!=0)
    {
        // Get file BPM
        CSAMPLE *bpmBuffer = beat->getBpmPtr();
        double filebpm = bpmBuffer[(int)(bufferpos_play*(beat->getBufferSize()/READCHUNKSIZE))];

//        qDebug("user %f, file %f, change %f",bpm, filebpm, bpm/filebpm);

        // Change rate to match new bpm
        rateSlider->set(bpm/filebpm-1.);
    }
}

void EngineBuffer::slotControlRatePermDown(double)
{
    // Adjusts temp rate down if button pressed
    if (buttonRatePermDown->get()==1.)
        rateSlider->sub(m_dPerm);
}

void EngineBuffer::slotControlRatePermDownSmall(double)
{
    // Adjusts temp rate down if button pressed
    if (buttonRatePermDownSmall->get()==1.)
        rateSlider->sub(m_dPermSmall);
}

void EngineBuffer::slotControlRatePermUp(double)
{
    // Adjusts temp rate up if button pressed
    if (buttonRatePermUp->get()==1.)
        rateSlider->add(m_dPerm);
}

void EngineBuffer::slotControlRatePermUpSmall(double)
{
    // Adjusts temp rate up if button pressed
    if (buttonRatePermUpSmall->get()==1.)
        rateSlider->add(m_dPermSmall);
}

void EngineBuffer::slotControlRateTempDown(double)
{
    // Adjusts temp rate down if button pressed, otherwise set to 0.
    if (buttonRateTempDown->get()==1. && !m_bTempPress)
    {
        m_bTempPress = true;
        rateSlider->sub(m_dTemp);
    }
    else if (buttonRateTempDown->get()==0.)
    {
        m_bTempPress = false;
        rateSlider->add(m_dTemp);
    }
}

void EngineBuffer::slotControlRateTempDownSmall(double)
{
    // Adjusts temp rate down if button pressed, otherwise set to 0.
    if (buttonRateTempDownSmall->get()==1. && !m_bTempPress)
    {
        m_bTempPress = true;
        rateSlider->sub(m_dTempSmall);
    }
    else if (buttonRateTempDownSmall->get()==0.)
    {
        m_bTempPress = false;
        rateSlider->add(m_dTempSmall);
    }
}

void EngineBuffer::slotControlRateTempUp(double)
{
    // Adjusts temp rate up if button pressed, otherwise set to 0.
    if (buttonRateTempUp->get()==1. && !m_bTempPress)
    {
        m_bTempPress = true;
        rateSlider->add(m_dTemp);
    }
    else if (buttonRateTempUp->get()==0.)
    {
        m_bTempPress = false;
        rateSlider->sub(m_dTemp);
    }
}

void EngineBuffer::slotControlRateTempUpSmall(double)
{
    // Adjusts temp rate up if button pressed, otherwise set to 0.
    if (buttonRateTempUpSmall->get()==1. && !m_bTempPress)
    {
        m_bTempPress = true;
        rateSlider->add(m_dTempSmall);
    }
    else if (buttonRateTempUpSmall->get()==0.)
    {
        m_bTempPress = false;
        rateSlider->sub(m_dTempSmall);
    }
}

void EngineBuffer::slotControlBeatSync(double)
{
    float fOtherBpm = m_pOtherEngineBuffer->getBpm();
    float fThisBpm  = bpmControl->get();
    float fRateScale;

    if (fOtherBpm>0. && fThisBpm>0.)
    {
        // Test if this buffers bpm is the double of the other one, and find rate scale:
        if ( fabs(fThisBpm*2.-fOtherBpm) < fabs(fThisBpm-fOtherBpm))
            fRateScale = fOtherBpm/(2*fThisBpm) * (1.+m_pOtherEngineBuffer->getRate());
        else if ( fabs(fThisBpm-2.*fOtherBpm) < fabs(fThisBpm-fOtherBpm))
            fRateScale = 2.*fOtherBpm/fThisBpm * (1.+m_pOtherEngineBuffer->getRate());
        else
            fRateScale = fOtherBpm/fThisBpm * (1.+m_pOtherEngineBuffer->getRate());

        // Ensure the rate is within resonable boundaries
        if (fRateScale<2. || fRateScale>0.5)
            // Adjust the rate:
            rateSlider->set(fRateScale-1.);
    }

    adjustPhase();
}

void EngineBuffer::adjustPhase()
{
    // Search for distance from playpos to beat mark of both buffers
    float fThisDistance = getDistanceNextBeatMark();
    float fOtherDistance = m_pOtherEngineBuffer->getDistanceNextBeatMark();

//    qDebug("this %f, other %f diff %f",fThisDistance, fOtherDistance, fOtherDistance-fThisDistance);

    //filepos_play += fOtherDistance-fThisDistance;
    bufferpos_play = bufferpos_play+fOtherDistance-fThisDistance;
    if (bufferpos_play>(double)READBUFFERSIZE)
    {
        bufferpos_play -= (double)READBUFFERSIZE;
    }

    //qDebug("buffer pos %f, file pos %f",bufferpos_play,filepos_play);
}

void EngineBuffer::slotControlFastFwdBack(double v)
{
    if (v==0.)
        scale->setFastMode(false);
    else
        scale->setFastMode(true);
}

void EngineBuffer::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE *pOutput = (CSAMPLE *)pOut;

    bool bCurBufferPaused = false;

    //Q_ASSERT( scale->getNewPlaypos() == 0);
    // pause can be locked if the reader is currently loading a new track.
    if (m_pTrackEnd->get()==0 && pause.tryLock())
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
            reader->setFileposPlay((int)filepos_play);
            reader->setRate(rate_old);

            reader->unlock();
            readerinfo = true;
        }

        //qDebug("filepos_play %f,\tstart %i,\tend %i\t info %i, len %i",filepos_play, filepos_start, filepos_end, readerinfo,file_length_old);



        //
        // Calculate rate
        //

        // Find BPM adjustment factor
        ReaderExtractBeat *beat = reader->getBeatPtr();
        double bpmrate = 1.;
        double filebpm = 0.;
        if (beat!=0)
        {
            CSAMPLE *bpmBuffer = beat->getBpmPtr();
            filebpm = bpmBuffer[(int)(bufferpos_play*(beat->getBufferSize()/READCHUNKSIZE))];
        }

        // Determine direction of playback from reverse button
        double dir = 1.;
        if (reverseButton->get()==1.)
            dir = -1.;

        double baserate = dir*bpmrate*((double)file_srate_old/(double)getPlaySrate());
        if (fwdButton->get()==1.)
            baserate = fabs(baserate)*5.;
        else if (backButton->get()==1.)
            baserate = fabs(baserate)*-5.;

        double rate;
        if (playButton->get()==1. || fwdButton->get()==1. || backButton->get()==1.)
        {
            rate=wheel->get()+(1.+rateSlider->get()*m_pRateRange->get()*m_pRateDir->get())*baserate;

            // Apply scratch
            if (m_pControlScratch->get()<0.)
                rate = rate * (m_pControlScratch->get()-1.);
            else if (m_pControlScratch->get()>0.)
                rate = rate * (m_pControlScratch->get()+1.);
        }
        else
        {
            rate=(wheel->get()+m_pControlScratch->get())*baserate*10.;
        }

        //qDebug("rateslider %f, ratedir %f, wheel %f", rateSlider->get(), m_pRateDir->get(), wheel->get());

        //qDebug("rate %f, bpmrate %f, file srate %f, play srate %f, baserate %f",rate, bpmrate, (double)file_srate_old, (double)getPlaySrate(), baserate);

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
            qDebug("event: %f, playpos %f, nextBeatPos %i",event,bufferpos_play,nextBeatPos);
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
    //                qDebug("i %i",i);
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

//        qDebug("NextBeatPos :%i, bufDiff: %i",nextBeatPos,READBUFFERSIZE/readerbeat->getBufferSize());
*/



        // If the rate has changed, set it in the scale object
        if (rate != rate_old)
        {
            // The rate returned by the scale object can be different from the wanted rate!
            rate_old = rate;
            rate = scale->setRate(rate);
            rate_old = rate;
        }

        bool at_start = false;
        bool at_end = false;

        // Determine playback direction
        bool backwards = false;
        if (rate<0.)
            backwards = true;

//         qDebug("rate: %f, filepos_play: %f, file_length_old %i",rate, filepos_play, file_length_old);

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
                qDebug("buffer out of range");

                rampOut(pOut, iBufferSize);
                bCurBufferPaused = true;
            }
            else
            {
                // Perform scaling of Reader buffer into buffer
                CSAMPLE *output = scale->scale(bufferpos_play, iBufferSize);
                int i;
                for (i=0; i<iBufferSize; i++)
                    pOutput[i] = output[i];
                double idx = scale->getNewPlaypos();

                // If a beat occours in current buffer mark it by led or in audio
                // This code currently only works in forward playback.
                ReaderExtractBeat *readerbeat = reader->getBeatPtr();
                if (readerbeat!=0)
                {
                    // Check if we need to set samples from a previos beat mark
                    if (audioBeatMark->get()==1. && m_iBeatMarkSamplesLeft>0)
                    {
                        int to = (int)min(m_iBeatMarkSamplesLeft, idx-bufferpos_play);
                        for (int j=0; j<to; j++)
                            pOutput[j] = 30000.;
                        m_iBeatMarkSamplesLeft = max(0,m_iBeatMarkSamplesLeft-to);
                    }

                    float *beatBuffer = (float *)readerbeat->getBasePtr();
                    int chunkSizeDiff = READBUFFERSIZE/readerbeat->getBufferSize();
                    //qDebug("from %i-%i",(int)floor(bufferpos_play),(int)floor(idx));
                    //Q_ASSERT( (int)floor(bufferpos_play) <= (int)floor(idx) );
                    for (i=(int)floor(bufferpos_play); i<=(int)floor(idx); i++)
                    {
                        if (((i%chunkSizeDiff)==0) && (beatBuffer[i/chunkSizeDiff]>0.0))
                        {
//                            qDebug("%i: %f",i/chunkSizeDiff,beatBuffer[i/chunkSizeDiff]);

                            // Audio beat mark
                            if (audioBeatMark->get()==1.)
                            {
                                int from = (int)(i-bufferpos_play);
                                int to = (int)min(i-bufferpos_play+audioBeatMarkLen, idx-bufferpos_play);
                                for (int j=from; j<to; j++)
                                    pOutput[j] = 30000.;
                                m_iBeatMarkSamplesLeft = max(0,audioBeatMarkLen-(to-from));

                                qDebug("audio beat mark");
                                //qDebug("mark %i: %i-%i", i2/chunkSizeDiff, (int)max(0,i-bufferpos_play),(int)min(i-bufferpos_play+audioBeatMarkLen, idx));
                            }
#ifdef __UNIX__
                            // PowerMate led
                            if (powermate!=0)
                                powermate->led();
#endif
                        }
                    }
                }

                // Write file playpos
                filepos_play += (idx-bufferpos_play);
                if (file_length_old>0 && filepos_play>file_length_old)
                {
                    idx -= filepos_play-file_length_old;
                    filepos_play = file_length_old;
                    at_end = true;
                    //qDebug("end");
                }
                else if (filepos_play<0)
                {
                    //qDebug("start %f",filepos_play);
                    idx -= filepos_play;
                    filepos_play = 0;
                    at_start = true;
                }

                // Set cue point if play button not pressed
                if (playButton->get()==0.)
                    cuePoint->set(max(0.,filepos_play-Player::getBufferSize()));

                // Ensure valid range of idx
                while (idx>READBUFFERSIZE)
                    idx -= (double)READBUFFERSIZE;
                while (idx<0)
                    idx += (double)READBUFFERSIZE;

                // Write buffer playpos
                bufferpos_play = idx;

                //qDebug("bufferpos_play %f, old %f",idx,oldidx);
            }
        }

        //
        // Check if more samples are needed from reader, and wake it up if necessary.
        //
        if (readerinfo && filepos_end>0)
        {
            //qDebug("play %f,\tstart %i,\tend %i\t info %i, len %i, %f<%f",filepos_play, filepos_start, filepos_end, readerinfo,file_length_old,fabs(filepos_play-filepos_start),(float)(READCHUNKSIZE*(READCHUNK_NO/2-1)));
//            qDebug("checking");
            // Part of this if condition is commented out to ensure that more block is
            // loaded at the end of an file to fill the buffer with zeros
            if ((filepos_end - filepos_play < READCHUNKSIZE*(READCHUNK_NO/2-1)))
            {
                //qDebug("wake fwd play %f, end %i",filepos_play, filepos_end);
                reader->wake();
            }
            else if (fabs(filepos_play-filepos_start)<(float)(READCHUNKSIZE*(READCHUNK_NO/2-1)))
            {
                //qDebug("wake back");
                reader->wake();
            }
            else if (filepos_play>filepos_end || filepos_play<filepos_start)
            {
                reader->requestSeek(filepos_play);
                reader->wake();
            }


            //
            // Check if end or start of file, and playmode, write new rate, playpos and do wakeall
            // if playmode is next file: set next in playlistcontrol
            //

            // Update playpos slider and bpm display if necessary
            m_iSamplesCalculated += iBufferSize;
            if (m_iSamplesCalculated > (getPlaySrate()/UPDATE_RATE))
            {
                if (file_length_old!=0.)
                {
                    double f = max(0.,min(filepos_play,file_length_old));
                    playposSlider->set(f/file_length_old);
                }
                else
                    playposSlider->set(0.);
                bpmControl->set(filebpm);
                rateEngine->set(rate);

                m_iSamplesCalculated = 0;

            }

            // Update buffer and abs position. These variables are not in the ControlObject
            // framework because they need very frequent updates.
            if (m_qPlayposMutex.tryLock())
            {
                m_dBufferPlaypos = bufferpos_play;
                m_dAbsPlaypos = filepos_play;
                m_qPlayposMutex.unlock();
            }
        }

        //qDebug("filepos_play %f, len %i, back %i, play %f",filepos_play,file_length_old, backwards, playButton->get());
        // If playbutton is pressed, check if we are at start or end of track
        if (playButton->get()==1. && m_pTrackEnd->get()==0. &&
            ((filepos_play<=0. && backwards==true) ||
             ((int)filepos_play>=file_length_old && backwards==false)))
        {
            // If end of track mode is set to next, signal EndOfTrack to TrackList,
            // otherwise start looping, pingpong or stop the track
            int m = (int)m_pTrackEndMode->get();
            qDebug("end mode %i",m);
            switch (m)
            {
            case TRACK_END_MODE_STOP:
                qDebug("stop");
                playButton->set(0.);
                m_pTrackEnd->set(1.);
                break;
            case TRACK_END_MODE_NEXT:
                qDebug("next");
                m_pTrackEnd->set(1.);
                break;
            case TRACK_END_MODE_LOOP:
                qDebug("loop");
                slotControlSeek(0.);
                break;
/*
            case TRACK_END_MODE_PING:
                qDebug("Ping not implemented yet");

                if (reverseButton->get()==1.)
                reverseButton->set(0.);
                else
                reverseButton->set(1.);

                break;
*/
            default:
                qDebug("Invalid track end mode: %i",m);
            }
        }

        pause.unlock();
    }
    else
    {
        rampOut(pOut, iBufferSize);
        bCurBufferPaused = true;
    }

    // Force ramp in if this is the first buffer during a play
    if (m_bLastBufferPaused && !bCurBufferPaused)
    {
//         qDebug("ramp in");
        // Ramp from zero
        int iLen = min(iBufferSize, kiRampLength);
        float fStep = pOutput[iLen-1]/(float)iLen;
        for (int i=0; i<iLen; ++i)
            pOutput[i] = fStep*i;
    }

    m_bLastBufferPaused = bCurBufferPaused;

    m_fLastSampleValue = pOutput[iBufferSize-1];
//    qDebug("last %f",m_fLastSampleValue);
}

void EngineBuffer::rampOut(const CSAMPLE *pOut, int iBufferSize)
{
    CSAMPLE *pOutput = (CSAMPLE *)pOut;

//     qDebug("ramp out");

    // Ramp to zero
    int i=0;
    if (m_fLastSampleValue!=0.)
    {
        int iLen = min(iBufferSize, kiRampLength);
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
