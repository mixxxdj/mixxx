/***************************************************************************
                          vinylcontrolxwax.cpp
                             -------------------
    begin                : Sometime in Summer 2007
    copyright            : (C) 2007 Albert Santoni
                           (C) 2007 Mark Hills
                           Portions of xwax used under the terms of the GPL
    email                : gamegod \a\t users.sf.net
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


/****** TODO *******
   Stuff to maybe implement here
   1) The smoothing thing that xwax does
   2) Tons of cleanup
   3) Speed up needle dropping
   4) Extrapolate small dropouts and keep track of "dynamics"

 ********************/

VinylControlXwax::VinylControlXwax(ConfigObject<ConfigValue> * pConfig, const char * _group) : VinylControl(pConfig, _group)
{
    dOldPos                 = 0.0f;
    dOldDiff                = 0.0f;
    bNeedleDown             = true;
    m_samples               = NULL;
    char * timecode  =  NULL;
    bShouldClose    = false;
    m_bNeedleSkipPrevention = (bool)(m_pConfig->getValueString( ConfigKey( "[VinylControl]", "NeedleSkipPrevention" ) ).toInt());
    
    //this is all needed because libxwax indexes by C-strings
    //so we go and pass libxwax a pointer into our local stack...
    if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEA)
        timecode = (char*)"serato_2a";
    else if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEB) 
        timecode = (char*)"serato_2b";
    else if (strVinylType == MIXXX_VINYL_SERATOCD) {
        timecode = (char*)"serato_cd";
        m_bNeedleSkipPrevention = false;
    }
    else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHSIDEA)
        timecode = (char*)"traktor_a";
    else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHSIDEB)
        timecode = (char*)"traktor_b";
    else {
        qDebug() << "Unknown vinyl type, defaulting to serato_2a";
        timecode = (char*)"serato_2a";
    }

    //qDebug() << "Xwax Vinyl control starting with a sample rate of:" << iSampleRate;
    qDebug() << "Building timecode lookup tables...";

  
    //Initialize the timecoder structure.
    timecoder_init(&timecoder);
    timecoder.rate = iSampleRate;
    
    
    //Build the timecode lookup table.
    if(timecoder_build_lookup(timecode, &timecoder) == -1)
    {
        qDebug() << "ERROR: Failed to build the timecode table!";
        return;
    }

    
    qDebug() << "Starting vinyl control xwax thread";

    //Start this thread (ends up calling-back the function "run()" below)
    start();

    //qDebug() << "Created new VinylControlXwax!";
}

VinylControlXwax::~VinylControlXwax()
{
    // Remove existing samples
    if (m_samples)
        free(m_samples);

    //Cleanup xwax nicely
    timecoder_free_lookup(&timecoder);
    timecoder_clear(&timecoder);

    // Continue the run() function and close it
    lockSamples.lock();
    bShouldClose = true;
    waitForNextInput.wakeAll();
    lockSamples.unlock();

    controlScratch->slotSet(0.0f);
    wait();
}



void VinylControlXwax::AnalyseSamples(short *samples, size_t size)
{
    if (lockSamples.tryLock())
    {
        //Submit the samples to the xwax timecode processor
        timecoder_submit(&timecoder, samples, size);

        //Update the input signal strength
        timecodeInputL->slotSet((float)fabs((float)samples[0]) / SHRT_MAX * 2.0f);
        timecodeInputR->slotSet((float)fabs((float)samples[1]) / SHRT_MAX * 2.0f);

        waitForNextInput.wakeAll();
        lockSamples.unlock();
    }
}


void VinylControlXwax::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("VinylControlXwax %1").arg(++id));
    
    int iTPS = 0;               // Timecodes per second
    dVinylPosition  = 0.0f;
    dVinylPitch             = 0.0f;
    dOldPitch       = 0.0f;
    double dTemp    = 0.0f;
    double dPlayPos = 0.0f;
    //bool absoluteMode = true;
    float filePosition = 0.0f;
    //bool bScratchMode = true;
    double dVinylPitchRange = 1.0f;
    double dTotalSpeed = 0.0f;
    double dTempCount = 0.0f;

    int when, alive, pitch_unavailable;

    bShouldClose = false;
    while(true)
    {
        lockSamples.lock();
        waitForNextInput.wait(&lockSamples);
        lockSamples.unlock();

        if (bShouldClose)
            return;

        //TODO: Move all these config object get*() calls to an "updatePrefs()" function,
        //		and make that get called when any options get changed in the preferences dialog, rather than
        //		polling everytime we get a buffer.

        //Vinyl control mode
        iVCMode = mode->get();
        //Check if vinyl control is enabled...
        bIsEnabled = enabled->get();

        //Get the pitch range from the prefs.
        fRateRange = rateRange->get();

        // Analyse the input samples
        int iPosition = -1;
        iPosition = timecoder_get_position(&timecoder, &when);

        dVinylPosition = iPosition;
        dVinylPosition = dVinylPosition / 1000.0f;
        //The lead-in time is added in syncPosition() in order to make the calculations
        //in this function simpler.


        //Initialize drift control to zero in case we don't get any position data to calculate it with.
        dDriftControl = 0.0f;

        //qDebug() << "iLeadInTime:" << iLeadInTime << "dVinylPosition" << dVinylPosition;

        alive = timecoder_get_alive(&timecoder);

        pitch_unavailable = timecoder_get_pitch(&timecoder, &dVinylPitch);

        if (duration != NULL && bIsEnabled)
        {
            filePosition = playPos->get() * duration->get();             //Get the playback position in the file in seconds.

            // When there's a timecode signal available
            if((alive && !pitch_unavailable))
            {
                //Notify the UI that the timecode quality is good
                timecodeQuality->slotSet(1.0f);

                //dVinylPitch = (dOldPitch * (XWAX_SMOOTHING - 1) + dVinylPitch) / XWAX_SMOOTHING;
                //qDebug() << "dVinylPosition: " << dVinylPosition << ", dVinylPitch: " << dVinylPitch << ", when: " << when;

                //FIXME (when Mark finished variable samplerates in timecoder)
                //Hack to make other samplerates work with xwax:
                //dVinylPitch *= (iSampleRate/44100);

                dVinylScratch = dVinylPitch;         //Use this value to instruct Mixxx for scratching/seeking.
                dVinylPitch = dVinylPitch - 1.0f;         //Shift the 33 RPM value (33 RPM = 0.0)
                dVinylPitch = dVinylPitch / fRateRange;   //Normalize to the pitch range. (8% = 1.0)


                //Re-get the duration, just in case a track hasn't been loaded yet...
                //duration = ControlObject::getControl(ConfigKey(group, "duration"));

                //If xwax has given us a valid position from the timecode (will be -1.0f if invalid)
                if ((dVinylPosition - iLeadInTime) > 0.0f)
                {
                    //If the position from the timecode is more than a few seconds off, resync the position.
                    if (fabs(dVinylPosition - filePosition - iLeadInTime) > 3.0 && 
                        (iVCMode == MIXXX_VCMODE_ABSOLUTE) &&
                        m_bNeedleSkipPrevention)
                    {
                        syncPosition();
                    }
                   //If we're in CD mode, react to shorter skips (for rapid-fire cueing). There's no needles
                    //with CDJs, so there's no point in trying to prevent needle skips.
                    else if (fabs(dVinylPosition - filePosition - iLeadInTime) > 0.2 &&
                             (iVCMode == MIXXX_VCMODE_ABSOLUTE) &&
                             (!m_bNeedleSkipPrevention)) 
                    {
                        syncPosition();
                    }
                    //else if (fabs(dVinylPosition - iLeadInTime) < 0.3) //Force resync at start 
                    //    syncPosition(); 
                        
                    //Calculate how much the vinyl's position has drifted from it's timecode and compensate for it.
                    //(This is caused by the manufacturing process of the vinyl.)
                    if (filePosition != 0.0f)
                        dDriftControl = (((dVinylPosition-iLeadInTime)/filePosition) - 1)/100 * 4.0f;

                    //Useful debug message for tracking down the problem of the vinyl's position "drifting":
                    //qDebug() << "Ratio of vinyl's position and Mixxx's: " << fabs(dVinylPosition/filePosition);

                    /*
                    qDebug() << "dDriftControl: " << dDriftControl;
                    qDebug() << "Xwax says the time is: " << dVinylPosition;
                    qDebug() << "Mixxx says the time is: " << filePosition;
                    qDebug() << "dVinylPitch: " << dVinylPitch;
                    //qDebug() << "diff in positions:" << fabs(dVinylPosition - filePosition);
                    */
                }
                else if (dVinylPosition > 0.0f) //Valid timecode, but we're in the timecode before the lead-in time...
                {
                    //Move to the start of the song...
                    dVinylPosition = iLeadInTime;
                    syncPosition();
                    //From here, Mixxx should appear to wait at the start of the song until the
                    //timecode passes the lead-in time mark. After that, this code should enter
                    //the "if" statement above this instead of falling into this "else".
                }

                playButton->slotSet(0.0f);
                rateSlider->slotSet(0.0f);

                if (iVCMode == MIXXX_VCMODE_ABSOLUTE)
                    controlScratch->slotSet(dVinylScratch + dDriftControl);
                else
                    controlScratch->slotSet(dVinylScratch);
                //qDebug() << "dVinylScratch" << dVinylScratch << "dDriftControl" << dDriftControl;

                dOldPos = dVinylPosition;
                dOldPitch = dVinylPitch;
            }
            else //No pitch data available (the needle is up/stopped.... or really crappy signal)
            {
                controlScratch->slotSet(0.0f);

                //Notify the UI that the timecode quality is garbage/missing.
                timecodeQuality->slotSet(0.0f);
            }
        }
    }
}

//Synchronize the pitch of the external turntable with Mixxx's pitch.
void VinylControlXwax::syncPitch(double pitch)
{
    //The dVinylPitch variable's range is from 1.0 +- 00%
    if (iVCMode == MIXXX_VCMODE_ABSOLUTE)     //Only apply drift control when we want to stay synced with the vinyl's position.
        pitch += dDriftControl;     //Apply the drift control to it, to keep the vinyl and Mixxx in sync.
    rateSlider->slotSet(pitch);     //rateSlider has a range of -1.0 to 1.0
    //qDebug() << "pitch: " << pitch;
}

//Synchronize the position of the timecoded vinyl with Mixxx's position.
void VinylControlXwax::syncPosition()
{
    float filePosition = playPos->get() * duration->get();
    //if (fabs(filePosition - dVinylPosition) > 5.00)

    dVinylPosition -= iLeadInTime;  //Add the lead-in
    playPos->slotSet(dVinylPosition / duration->get());     //VinylPos in seconds / total length of song

    //playPos->slotSet(dVinylPosition / (15.0f * 60.0f)); //VinylPos in seconds / (total length of vinyl)
}

bool VinylControlXwax::isEnabled()
{
    return bIsEnabled;
}

void VinylControlXwax::ToggleVinylControl(bool enable)
{
    bIsEnabled = enable;
}
