/***************************************************************************
                          vinylcontrolxwax.cpp
                             -------------------
    begin                : Sometime in Summer 2007
    copyright            : (C) 2007 Albert Santoni
                           (C) 2007 Mark Hills
                           (C) 2011 Owen Williams
                           Portions of xwax used under the terms of the GPL
    current maintainer   : Owen Williams
    email                : owilliams@mixxx.org
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
#include <limits.h>
#include "vinylcontrolxwax.h"
#include "controlobjectthreadmain.h"
#include <math.h>


/****** TODO *******
   Stuff to maybe implement here
   1) The smoothing thing that xwax does
   2) Tons of cleanup
   3) Speed up needle dropping
   4) Extrapolate small dropouts and keep track of "dynamics"

 ********************/

bool VinylControlXwax::m_bLUTInitialized = false;
QMutex VinylControlXwax::s_xwaxLUTMutex;

VinylControlXwax::VinylControlXwax(ConfigObject<ConfigValue> * pConfig, QString group) : VinylControl(pConfig, group)
{
    dOldPos                 = 0.0f;
    m_samples               = NULL;
    char * timecode  =  NULL;
    bShouldClose    = false;
    bForceResync    = false;
    iOldMode        = MIXXX_VCMODE_ABSOLUTE;
    dUiUpdateTime   = -1.0f;
    m_bNeedleSkipPrevention = (bool)(m_pConfig->getValueString( ConfigKey( "[VinylControl]", "needle_skip_prevention" ) ).toInt());
    signalenabled->slotSet(m_pConfig->getValueString( ConfigKey( "[VinylControl]", "show_signal_quality" ) ).toInt());

    dLastTrackSelectPos = 0.0;
    dCurTrackSelectPos = 0.0;
    trackSelector = trackLoader = NULL;
    bTrackSelectMode = false;

    tSinceSteadyPitch = QTime();
    m_pSteadySubtle = new SteadyPitch(0.08);
    m_pSteadyGross = new SteadyPitch(0.5);

    iQualPos = 0;
    iQualFilled = 0;

    m_bCDControl = false;

    //this is all needed because libxwax indexes by C-strings
    //so we go and pass libxwax a pointer into our local stack...
    if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEA) {
        timecode = (char*)"serato_2a";
    }
    else if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEB) {
        timecode = (char*)"serato_2b";
    }
    else if (strVinylType == MIXXX_VINYL_SERATOCD) {
        timecode = (char*)"serato_cd";
        m_bNeedleSkipPrevention = false;
        m_bCDControl = true;
    }
    else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHSIDEA) {
        timecode = (char*)"traktor_a";
    }
    else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHSIDEB) {
        timecode = (char*)"traktor_b";
    }
    else if (strVinylType == MIXXX_VINYL_MIXVIBESDVS) {
    	timecode = (char*)"mixvibes_v2";
   	}
    else {
        qDebug() << "Unknown vinyl type, defaulting to serato_2a";
        timecode = (char*)"serato_2a";
    }
    
    timecode_def *tc_def = timecoder_find_definition(timecode);
    if (tc_def == NULL)
    {
        qDebug() << "Error finding timecode definition for " << timecode << ", defaulting to serato_2a";
        timecode = (char*)"serato_2a";
        tc_def = timecoder_find_definition(timecode);
    }

    double speed = 1.0f;
    if (strVinylSpeed == MIXXX_VINYL_SPEED_45)
        speed = 1.35f;

    //qDebug() << "Xwax Vinyl control starting with a sample rate of:" << iSampleRate;
    qDebug() << "Building timecode lookup tables for" << strVinylType << "with speed" << strVinylSpeed;


    //Initialize the timecoder structure.
    s_xwaxLUTMutex.lock(); //Static mutex! We don't want two threads doing this!

    timecoder_init(&timecoder, tc_def, speed, iSampleRate);
    timecoder_monitor_init(&timecoder, MIXXX_VINYL_SCOPE_SIZE);
    //Note that timecoder_init will not double-malloc the LUTs, and after this we are guaranteed
    //that the LUT has been generated unless we ran out of memory.
    m_bLUTInitialized = true;
    m_uiSafeZone = timecoder_get_safe(&timecoder);
    //}
    s_xwaxLUTMutex.unlock();

    qDebug() << "Starting vinyl control xwax thread";

    //Start this thread (ends up calling-back the function "run()" below)
    start();
}

VinylControlXwax::~VinylControlXwax()
{
    // Remove existing samples
    if (m_samples)
        free(m_samples);
        
    delete m_pSteadySubtle;
    delete m_pSteadyGross;

    //Cleanup xwax nicely
    timecoder_monitor_clear(&timecoder);
    timecoder_clear(&timecoder);
    m_bLUTInitialized = false;

    // Continue the run() function and close it
    lockSamples.lock();
    bShouldClose = true;
    waitForNextInput.wakeAll();
    lockSamples.unlock();

    controlScratch->slotSet(0.0f);
    wait();
}

//static
void VinylControlXwax::freeLUTs()
{
    s_xwaxLUTMutex.lock(); //Static mutex! We don't want two threads doing this!
    if (m_bLUTInitialized) {
        timecoder_free_lookup(); //Frees all the LUTs in xwax.
        m_bLUTInitialized = false;
    }
    s_xwaxLUTMutex.unlock();
}


void VinylControlXwax::AnalyseSamples(const short *samples, size_t size)
{
    if (lockSamples.tryLock())
    {
        //Submit the samples to the xwax timecode processor
        timecoder_submit(&timecoder, samples, size);

        bHaveSignal = fabs((float)samples[0]) + fabs((float)samples[1]) > MIN_SIGNAL;
        //qDebug() << "signal?" << bHaveSignal;

        waitForNextInput.wakeAll();
        lockSamples.unlock();
    }
}

unsigned char* VinylControlXwax::getScopeBytemap()
{
    return timecoder.mon;
}


void VinylControlXwax::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("VinylControlXwax %1").arg(++id));

    dVinylPosition  = 0.0f;
    dVinylPitch     = 0.0f;
    dOldPitch       = 0.0f;
    //bool absoluteMode = true;
    int iPosition = -1;
    float filePosition = 0.0f;
    //bool bScratchMode = true;
    double dDriftAmt = 0.0f;
    double dPitchRing[RING_SIZE];
    int ringPos = 0;
    int ringFilled = 0;
    double cur_duration = -1.0f;
    double old_duration = -1.0f;
    int reportedMode = 0;
    bool reportedPlayButton = 0;
    tSinceSteadyPitch.start();

    double when; //unused, needed for calling xwax

    bShouldClose = false;

    iVCMode = mode->get();

    while(true)
    {
        lockSamples.lock();
        waitForNextInput.wait(&lockSamples);
        lockSamples.unlock();

        if (bShouldClose)
            return;

        //TODO: Move all these config object get*() calls to an "updatePrefs()" function,
        //        and make that get called when any options get changed in the preferences dialog, rather than
        //        polling everytime we get a buffer.


        //Check if vinyl control is enabled...
        bIsEnabled = checkEnabled(bIsEnabled, enabled->get());

        //Get the pitch range from the prefs.
        fRateRange = rateRange->get();

        if(bHaveSignal)
        {
            //Always analyse the input samples
            iPosition = timecoder_get_position(&timecoder, &when);
            //Notify the UI if the timecode quality is good
            establishQuality(iPosition != -1);
        }

        //are we even playing and enabled at all?
        if (!bIsEnabled)
            continue;

        dVinylPitch = timecoder_get_pitch(&timecoder);

        //if no track loaded, let track selection work but that's it
        if (duration == NULL)
        {
            //until I can figure out how to detect "track 2" on serato CD,
            //don't try track selection
            if (!m_bCDControl)
            {
                bTrackSelectMode = true;
                doTrackSelection(false, dVinylPitch, iPosition);
            }
            continue;
        }
        //qDebug() << m_group << id << iPosition << when << dVinylPitch;

        cur_duration = duration->get();


        //Has a new track been loaded?
        //FIXME? we should really sync on all track changes
        if (cur_duration != old_duration)
        {
            bForceResync=true;
            bTrackSelectMode = false; //just in case
            old_duration = cur_duration;
            //duration from the control object is an integer.  We need
            //more precision:
            fTrackDuration = trackSamples->get() / 2 / trackSampleRate->get();

            //we were at record end, so turn it off and restore mode
            if(atRecordEnd)
            {
                disableRecordEndMode();
                if (iOldMode == MIXXX_VCMODE_CONSTANT)
                    iVCMode = MIXXX_VCMODE_RELATIVE;
                else
                    iVCMode = iOldMode;
            }
        }

        //make sure dVinylPosition only has good values
        if (iPosition != -1)
        {
            dVinylPosition = iPosition;
            dVinylPosition = dVinylPosition / 1000.0f;
            dVinylPosition -= iLeadInTime;
        }



        //Initialize drift control to zero in case we don't get any position data to calculate it with.
        dDriftControl = 0.0f;

        filePosition = playPos->get() * fTrackDuration;             //Get the playback position in the file in seconds.

        reportedMode = mode->get();
        reportedPlayButton = playButton->get();

        if (iVCMode != reportedMode)
        {
            //if we are playing, don't allow change
            //to absolute mode (would cause sudden track skip)
            if (reportedPlayButton && reportedMode == MIXXX_VCMODE_ABSOLUTE)
            {
                iVCMode = MIXXX_VCMODE_RELATIVE;
                mode->slotSet((double)iVCMode);
            }
            else //go ahead and switch
            {
                iVCMode = reportedMode;
                if (reportedMode == MIXXX_VCMODE_ABSOLUTE)
                    bForceResync = true;
               }

            //if we are out of error mode...
               if (vinylStatus->get() == VINYL_STATUS_ERROR && iVCMode == MIXXX_VCMODE_RELATIVE)
            {
                vinylStatus->slotSet(VINYL_STATUS_OK);
            }
        }

        //if looping has been enabled, don't allow absolute mode
        if (loopEnabled->get() && iVCMode == MIXXX_VCMODE_ABSOLUTE)
        {
            iVCMode = MIXXX_VCMODE_RELATIVE;
            mode->slotSet((double)iVCMode);
        }
        //are we newly playing near the end of the record?  (in absolute mode, this happens
        //when the filepos is past safe (more accurate),
        //but it can also happen in relative mode if the vinylpos is nearing the end
        //If so, change to constant mode so DJ can move the needle safely
        
        if (!atRecordEnd && reportedPlayButton)
        {
            if (iVCMode == MIXXX_VCMODE_ABSOLUTE)
            {
                if ((filePosition + iLeadInTime) * 1000.0f  > m_uiSafeZone &&
                    !bForceResync) //corner case: we are waiting for resync so don't enable just yet
                    enableRecordEndMode();
            }
            else if (iVCMode == MIXXX_VCMODE_RELATIVE || iVCMode == MIXXX_VCMODE_CONSTANT)
            {
                if (iPosition != -1 && iPosition > m_uiSafeZone)
                    enableRecordEndMode();
            }
        }

        if (atRecordEnd)
        {
            //if atRecordEnd was true, maybe it no longer applies:

            if (!reportedPlayButton)
            {
                //if we turned off play button, also disable
                disableRecordEndMode();
            }
            else if (iPosition != -1 &&
                     iPosition <= m_uiSafeZone &&
                     dVinylPosition > 0 &&
                     checkSteadyPitch(dVinylPitch, filePosition) > 0.5)

            {
                //if good position, and safe, and not in leadin, and steady,
                //disable
                disableRecordEndMode();
            }

            if (atRecordEnd)
            {
                //ok, it's still valid, blink
                if ((reportedPlayButton && (int)(filePosition * 2.0f) % 2) ||
                    (!reportedPlayButton && (int)(iPosition / 500.0f) % 2))
                    vinylStatus->slotSet(VINYL_STATUS_WARNING);
                else
                    vinylStatus->slotSet(VINYL_STATUS_DISABLED);
            }
        }

        //check here for position > safe, and if no record end mode,
        //then trigger track selection mode.  just pass position to it
        //and ignore pitch
        
        if (!atRecordEnd)
        {
            if (iPosition != -1 && iPosition > m_uiSafeZone)
            {
                //only enable if pitch is steady, though.  Heavy scratching can
                //produce crazy results and trigger this mode
                if (bTrackSelectMode || checkSteadyPitch(dVinylPitch, filePosition) > 0.1)
                {
                    //until I can figure out how to detect "track 2" on serato CD,
                    //don't try track selection
                    if (!m_bCDControl)
                    {
                        if (!bTrackSelectMode)
                        {
                            qDebug() << "position greater than safe, select mode" << iPosition << m_uiSafeZone;
                            bTrackSelectMode = true;
                            togglePlayButton(FALSE);
                               resetSteadyPitch(0.0f, 0.0f);
                            controlScratch->slotSet(0.0f);
                        }
                        doTrackSelection(true, dVinylPitch, iPosition);
                    }

                    //hm I wonder if track will keep playing while this happens?
                    //not sure what we want to do here...  probably enforce
                    //stopped deck.

                    //but if constant mode...  nah, force stop.
                    continue;
                }
                //if it's not steady yet we process as normal
            }
            else
            {
                //so we're not unsafe.... but
                //if no position, but we were in select mode, do select mode
                if (iPosition == -1 && bTrackSelectMode)
                {
                    //qDebug() << "no position, but were in select mode";
                    doTrackSelection(false, dVinylPitch, iPosition);

                    //again, force stop?
                    continue;
                }
                else if (bTrackSelectMode)
                {
                    //qDebug() << "discontinuing select mode, selecting track";
                    if (trackLoader == NULL)
                        trackLoader = new ControlObjectThread(ControlObject::getControl(ConfigKey(m_group,"LoadSelectedTrack")));

                    if (!trackLoader)
                        qDebug() << "ERROR: couldn't get track loading object?";
                    else
                    {
                        trackLoader->slotSet(1.0);
                        trackLoader->slotSet(0.0); //I think I have to do this...
                    }
                    //if position is known and safe then no track select mode
                    bTrackSelectMode = false;
                }
            }
        }

        if (iVCMode == MIXXX_VCMODE_CONSTANT)
        {
            //when we enabled constant mode we set the rate slider
            //now we just either set scratch val to 0 (stops playback)
            //or 1 (plays back at that rate)

            if (reportedPlayButton)
                controlScratch->slotSet(rateDir->get() * (rateSlider->get() * fRateRange) + 1.0f);
            else
                controlScratch->slotSet(0.0f);

            //is there any reason we'd need to do anything else?
            continue;
        }

        //CONSTANT MODE NO LONGER APPLIES...

        // When there's a timecode signal available
        // This is set when we analyze samples (no need for lock I think)
        if(bHaveSignal)
        {
            //POSITION: MAYBE  PITCH: YES

            //We have pitch, but not position.  so okay signal but not great (scratching / cueing?)
            //qDebug() << "Pitch" << dVinylPitch;

            if (iPosition != -1)
            {
                //POSITION: YES  PITCH: YES
                //add a value to the pitch ring (for averaging / smoothing the pitch)
                //qDebug() << fabs(((dVinylPosition - dOldPos) * (dVinylPitch / fabs(dVinylPitch))));

                //save the absolute amount of drift for when we need to estimate vinyl position
                dDriftAmt = dVinylPosition - filePosition;

                //qDebug() << "drift" << dDriftAmt;

                if (bForceResync)
                {
                    //if forceresync was set but we're no longer absolute,
                    //it no longer applies
                    //if we're in relative mode then we'll do a sync
                    //because it might select a cue
                    if (iVCMode == MIXXX_VCMODE_ABSOLUTE || (iVCMode == MIXXX_VCMODE_RELATIVE && cueing->get()))
                    {
                        syncPosition();
                        resetSteadyPitch(dVinylPitch, dVinylPosition);
                    }
                    bForceResync = false;
                }
                else if (fabs(dVinylPosition - filePosition) > 0.1f &&
                        dVinylPosition < -2.0f)
                {
                    //At first I thought it was a bug to resync to leadin in relative mode,
                    //but after using it that way it's actually pretty convenient.
                    //qDebug() << "Vinyl leadin";
                    syncPosition();
                    resetSteadyPitch(dVinylPitch, dVinylPosition);
                    if (uiUpdateTime(filePosition))
                        rateSlider->slotSet(rateDir->get() * (fabs(dVinylPitch) - 1.0f) / fRateRange);
                }
                else if (iVCMode == MIXXX_VCMODE_ABSOLUTE && (fabs(dVinylPosition - dOldPos) >= 15.0f))
                {
                    //If the position from the timecode is more than a few seconds off, resync the position.
                    //qDebug() << "resync position (>15.0 sec)";
                    //qDebug() << dVinylPosition << dOldPos << dVinylPosition - dOldPos;
                    syncPosition();
                    resetSteadyPitch(dVinylPitch, dVinylPosition);
                }
                else if (iVCMode == MIXXX_VCMODE_ABSOLUTE && m_bNeedleSkipPrevention &&
                        fabs(dVinylPosition - dOldPos) > 0.4 &&
                        (tSinceSteadyPitch.elapsed() < 400 || reportedPlayButton))
                {
                    //red alert, moved wrong direction or jumped forward a lot,
                    //and we were just playing nicely...
                    //move to constant mode and keep playing
                    qDebug() << "WARNING: needle skip detected!:";
                    qDebug() << filePosition << dOldFilePos << dVinylPosition << dOldPos;
                    qDebug() << (dVinylPosition - dOldPos) * (dVinylPitch / fabs(dVinylPitch));
                    //try setting the rate to the steadypitch value
                    enableConstantMode(m_pSteadySubtle->steadyValue());
                    vinylStatus->slotSet(VINYL_STATUS_ERROR);
                }
                else if (iVCMode == MIXXX_VCMODE_ABSOLUTE && m_bCDControl &&
                    fabs(dVinylPosition - dOldPos) >= 0.1f)
                {
                    //qDebug() << "CDJ resync position (>0.1 sec)";
                    syncPosition();
                    resetSteadyPitch(dVinylPitch, dVinylPosition);
                }
                else if (playPos->get() >= 1.0 && dVinylPitch > 0)
                {
                    //end of track, force stop
                    togglePlayButton(false);
                    resetSteadyPitch(0.0f, 0.0f);
                    controlScratch->slotSet(0.0f);
                    ringPos = 0;
                    ringFilled = 0;
                    continue;
                }
                else
                {
                    togglePlayButton(checkSteadyPitch(dVinylPitch, filePosition) > 0.5);
                }

                //Calculate how much the vinyl's position has drifted from it's timecode and compensate for it.
                //(This is caused by the manufacturing process of the vinyl.)
                if (fabs(dDriftAmt) > 0.1 && fabs(dDriftAmt) < 5.0) {
                    dDriftControl = dDriftAmt;
                } else {
                    dDriftControl = 0.0;
                }

                dOldPos = dVinylPosition;
            }
            else
            {
                //POSITION: NO  PITCH: YES
                //if we don't have valid position, we're not playing so reset time to current
                //estimate vinyl position

                if (playPos->get() >= 1.0 && dVinylPitch > 0)
                {
                    //end of track, force stop
                    togglePlayButton(false);
                    resetSteadyPitch(0.0f, 0.0f);
                    controlScratch->slotSet(0.0f);
                    ringPos = 0;
                    ringFilled = 0;
                    continue;
                }

                if (iVCMode == MIXXX_VCMODE_ABSOLUTE &&
                    fabs(dVinylPitch) < 0.05 &&
                    fabs(dDriftAmt) >= 0.3f)
                {
                    //qDebug() << "slow, out of sync, syncing position";
                    syncPosition();
                }

                dOldPos = filePosition + dDriftAmt;

                if (dVinylPitch > 0.2)
                {
                    togglePlayButton(checkSteadyPitch(dVinylPitch, filePosition) > 0.5);
                }
            }

            //playbutton status may have changed
            reportedPlayButton = playButton->get();

            if (reportedPlayButton)
            {
                //only add to the ring if pitch is stable
                dPitchRing[ringPos] = dVinylPitch;
                if(ringFilled < RING_SIZE)
                    ringFilled++;
                ringPos = (ringPos + 1) % RING_SIZE;
            }
            else
            {
                //reset ring if pitch isn't steady
                ringPos = 0;
                ringFilled = 0;
            }

            //only smooth when we have good position (no smoothing for scratching)
            double averagePitch = 0.0f;
            if (iPosition != -1 && reportedPlayButton)
            {
                for (int i=0; i<ringFilled; i++)
                {
                    averagePitch += dPitchRing[i];
                }
                averagePitch /= ringFilled;
                //round out some of the noise
                averagePitch = (double)(int)(averagePitch * 10000.0f);
                averagePitch /= 10000.0f;
            }
            else
                averagePitch = dVinylPitch;

            if (iVCMode == MIXXX_VCMODE_ABSOLUTE)
            {
                controlScratch->slotSet(dVinylPitch + dDriftControl);
                if (iPosition != -1 && reportedPlayButton && uiUpdateTime(filePosition))
                {
                    rateSlider->slotSet(rateDir->get() * (fabs(dVinylPitch + dDriftControl) - 1.0f) / fRateRange);
                    dUiUpdateTime = filePosition;
                }
            }
            else if (iVCMode == MIXXX_VCMODE_RELATIVE)
            {
                controlScratch->slotSet(averagePitch);
                if (iPosition != -1 && reportedPlayButton && uiUpdateTime(filePosition))
                {
                    rateSlider->slotSet(rateDir->get() * (fabs(averagePitch) - 1.0f) / fRateRange);
                    dUiUpdateTime = filePosition;
                }
            }

            dOldPitch = dVinylPitch;
            dOldFilePos = filePosition;
        }
        else //No pitch data available (the needle is up/stopped.... or *really* crappy signal)
        {
            //POSITION: NO  PITCH: NO
            //if it's been a long time, we're stopped.
            //if it hasn't been long, and we're preventing needle skips,
            //let the track play a wee bit more before deciding we've stopped

            rateSlider->slotSet(0.0f);

            if (iVCMode == MIXXX_VCMODE_ABSOLUTE &&
                fabs(dVinylPosition - filePosition) >= 0.1f)
            {
                //qDebug() << "stopped, out of sync, syncing position";
                syncPosition();
            }

            if(fabs(filePosition - dOldFilePos) >= 0.1 ||
                !m_bNeedleSkipPrevention ||
                filePosition == dOldFilePos)
            {
                //We are not playing any more
                togglePlayButton(FALSE);
                resetSteadyPitch(0.0f, 0.0f);
                controlScratch->slotSet(0.0f);
                //resetSteadyPitch(dVinylPitch, filePosition);
                //Notify the UI that the timecode quality is garbage/missing.
                m_fTimecodeQuality = 0.0f;
                ringPos = 0;
                ringFilled = 0;
                iQualPos = 0;
                iQualFilled = 0;
                bForceResync=true;
                vinylStatus->slotSet(VINYL_STATUS_OK);
            }
        }
    }
}

void VinylControlXwax::enableRecordEndMode()
{
    qDebug() << "record end, setting constant mode";
    vinylStatus->slotSet(VINYL_STATUS_WARNING);
    enableConstantMode();
    atRecordEnd = true;
}

void VinylControlXwax::enableConstantMode()
{
    iOldMode = iVCMode;
    iVCMode = MIXXX_VCMODE_CONSTANT;
    mode->slotSet((double)iVCMode);
    togglePlayButton(true);
    double rate = controlScratch->get();
    rateSlider->slotSet(rateDir->get() * (fabs(rate) - 1.0f) / fRateRange);
    controlScratch->slotSet(rate);
}

void VinylControlXwax::enableConstantMode(double rate)
{
    iOldMode = iVCMode;
    iVCMode = MIXXX_VCMODE_CONSTANT;
    mode->slotSet((double)iVCMode);
    togglePlayButton(true);
    rateSlider->slotSet(rateDir->get() * (fabs(rate) - 1.0f) / fRateRange);
    controlScratch->slotSet(rate);
}

void VinylControlXwax::disableRecordEndMode()
{
    vinylStatus->slotSet(VINYL_STATUS_OK);
    atRecordEnd = false;
    iVCMode = MIXXX_VCMODE_RELATIVE;
    mode->slotSet((double)iVCMode);
}

void VinylControlXwax::togglePlayButton(bool on)
{
    if (bIsEnabled && (playButton->get() > 0) != on) {
        //switching from on to off -- restart counter for checking needleskip
        if (!on)
            tSinceSteadyPitch.restart();
        playButton->slotSet((float)on);  //and we all float on all right
    }
}

void VinylControlXwax::doTrackSelection(bool valid_pos, double pitch, double position)
{
    //compare positions, fabricating if we don't have position data, and
    //move the selector every so often
    //track will be selected when the needle is moved back to play area
    //track selection can be cancelled by loading a track manually

    const int SELECT_INTERVAL = 150;
    const double NOPOS_SPEED = 0.50;

    if (trackSelector == NULL)
    {
        //this isn't done in the constructor because this object
        //doesn't seem to be created yet
        trackSelector = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Playlist]","SelectTrackKnob")));
        if (trackSelector == NULL)
        {
            qDebug() << "Warning: Track Selector control object NULL";
            return;
        }
    }

    if (!valid_pos)
    {
        if (fabs(pitch) > 0.1)
        {
            //how to estimate how far the record has moved when we don't have a valid
            //position and no mp3 track to compare with???  just add a bullshit amount?
            dCurTrackSelectPos += pitch * NOPOS_SPEED; //MADE UP CONSTANT, needs to be based on frames per second I think
        }
        else //too slow, do nothing
            return;
    }
    else
        dCurTrackSelectPos = position; //if we have valid pos, use it


    //we have position or at least record is moving, so check if we should
    //change location

    if (fabs(dCurTrackSelectPos - dLastTrackSelectPos) > 10.0 * 1000)
    {
        //yeah probably not a valid value
        //qDebug() << "large change in track position, resetting";
        dLastTrackSelectPos = dCurTrackSelectPos;
    }
    else if (fabs(dCurTrackSelectPos - dLastTrackSelectPos) > SELECT_INTERVAL)
    {
        //only adjust by one at a time.  It's no help jumping around
        trackSelector->slotSet((int)(dCurTrackSelectPos - dLastTrackSelectPos) / fabs(dCurTrackSelectPos - dLastTrackSelectPos));
        dLastTrackSelectPos = dCurTrackSelectPos;
    }
}


void VinylControlXwax::resetSteadyPitch(double pitch, double time)
{
    m_pSteadySubtle->reset(pitch, time);
    m_pSteadyGross->reset(pitch, time);
}

double VinylControlXwax::checkSteadyPitch(double pitch, double time)
{
    if (m_pSteadyGross->check(pitch, time, loopEnabled->get()) < 0.5) {
        scratching->slotSet(1.0);
    } else {
        scratching->slotSet(0.0);
    }
    return m_pSteadySubtle->check(pitch, time, loopEnabled->get());
}

//Synchronize Mixxx's position to the position of the timecoded vinyl.
void VinylControlXwax::syncPosition()
{
    //qDebug() << "sync position" << dVinylPosition / duration->get();
    vinylSeek->slotSet(dVinylPosition / fTrackDuration);    //VinylPos in seconds / total length of song
}

bool VinylControlXwax::checkEnabled(bool was, bool is)
{
    // if we're not enabled, but the last object was, try turning ourselves on
    // XXX: is this just a race that's working right now?
    if (!is && wantenabled->get() > 0)
    {
        enabled->slotSet(true);
        wantenabled->slotSet(false); //don't try to do this over and over
        return true; //optimism!
    }
    if (was != is)
    {
        //we reset the scratch value, but we don't reset the rate slider.
        //This means if we are playing, and we disable vinyl control,
        //the track will keep playing at the previous rate.
        //This allows for single-deck control, dj handoffs, etc.

        togglePlayButton(playButton->get() || fabs(controlScratch->get()) > 0.05f);
        controlScratch->slotSet(rateDir->get() * (rateSlider->get() * fRateRange) + 1.0f);
        resetSteadyPitch(0.0f, 0.0f);
        bForceResync = true;
        if (!was)
            dOldFilePos = 0.0f;
        iVCMode = mode->get();
        atRecordEnd = false;
    }
    if (is && !was)
    {
        vinylStatus->slotSet(VINYL_STATUS_OK);
    }
    if (!is)
        vinylStatus->slotSet(VINYL_STATUS_DISABLED);

    return is;
}

bool VinylControlXwax::isEnabled()
{
    return bIsEnabled;
}

void VinylControlXwax::ToggleVinylControl(bool enable)
{
    bIsEnabled = enable;
}

bool VinylControlXwax::uiUpdateTime(double now)
{
    if (dUiUpdateTime > now || now - dUiUpdateTime > 0.05)
    {
        dUiUpdateTime = now;
        return true;
    }
    return false;
}

void VinylControlXwax::establishQuality(bool quality_sample)
{
    bQualityRing[iQualPos] = quality_sample;
    if(iQualFilled < QUALITY_RING_SIZE)
    {
        iQualFilled++;
    }

    int quality = 0;
    for (int i=0; i<iQualFilled; i++)
    {
        if (bQualityRing[i])
            quality++;
    }

    //qDebug() << "quality" << m_fTimecodeQuality;
    m_fTimecodeQuality = (float)quality / (float)iQualFilled;

    iQualPos = (iQualPos + 1) % QUALITY_RING_SIZE;
}

float VinylControlXwax::getAngle()
{
    double when;
    float pos = timecoder_get_position(&timecoder, &when);

    if (pos == -1)
        return -1.0;

    pos /= 1000.0;

    float rps = timecoder_revs_per_sec(&timecoder);
    //invert angle to make vinyl spin direction correct
    return 360 - ((int)(pos * 360.0 * rps) % 360);
}
