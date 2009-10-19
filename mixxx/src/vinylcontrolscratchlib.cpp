/***************************************************************************
                          vinylcontrol.cpp  -  description
                             -------------------
    begin                : Tue June 6 2006
    copyright            : (C) 2006 by Albert Santoni
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

#include <math.h>
#include <QDebug>
#include "vinylcontrolscratchlib.h"

#include "DAnalyse.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"


VinylControlScratchlib::VinylControlScratchlib(ConfigObject<ConfigValue> * pConfig, const char * _group) : VinylControl(pConfig, _group)
{
    dVinylPosition  = 0.0f;
    dOldPos                 = 0.0f;
    dOldDiff                = 0.0f;
    bNeedleDown             = true;
    m_samples               = NULL;


    //Create the "DAnalyse" object that interacts with scratchlib.
    analyzer = new DAnalyse();

    if (strVinylType == MIXXX_VINYL_FINALSCRATCH)
        analyzer->SetVinyl(DSCRATCH_VINYL_FINALSCRATCH);
    else if (strVinylType == MIXXX_VINYL_MIXVIBESDVSCD)
        analyzer->SetVinyl(DSCRATCH_VINYL_MIXVIBES);

    //Enable or disable RIAA correction
    /*
       if (iRIAACorrection == 1)
            scratch->EnableRIAACorrection(true);
       else
            scratch->EnableRIAACorrection(false);
     */

    //Set the calibration value
    analyzer->SetCalibration(MIXXX_CALIBRATION_VALUE);

    //Set the sample rate
    qDebug() << "Vinyl control starting with a sample rate of:" << iSampleRate;
    analyzer->SetFrequency(iSampleRate);

    //(This ends up calling-back the function "run()" below.)
    start();

    qDebug() << "Created new VinylControlScratchlib!\n";
}

VinylControlScratchlib::~VinylControlScratchlib()
{
    // Remove existing samples
    if (m_samples)
        free(m_samples);

    // Close scratch
    delete analyzer;

    // Continue the run() function and close it
    lockSamples.lock();
    bShouldClose = true;
    waitForNextInput.wakeAll();
    lockSamples.unlock();
    wait();
}


void VinylControlScratchlib::AnalyseSamples(short * samples, size_t size)
{
    lockSamples.lock();

//TODO: This copying of samples is totally unnecessary because this whole process
//      is blocking... (we could just call analyzer->Analyse() here instead of in run())
    if (m_samples)
        free(m_samples);
    m_SamplesSize = size;

    m_samples = (short *)malloc(size*sizeof(short));
    short * ptr = m_samples;
    for (int i = 0; i < size; i++)
    {
        *ptr = *samples;
        *ptr++;
        *samples++;
    }

    if (strVinylType == MIXXX_VINYL_FINALSCRATCH)
        timecodeQuality->slotSet(analyzer->GetTimecodesPerSecond() / 32);
    else if (strVinylType == MIXXX_VINYL_MIXVIBESDVSCD)
        timecodeQuality->slotSet(analyzer->GetTimecodesPerSecond() / 32);

    waitForNextInput.wakeAll();
    lockSamples.unlock();
}


void VinylControlScratchlib::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("VinylControlScratchlib %1").arg(++id));
    
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

        if (bShouldClose)
        {
            lockSamples.unlock();
            return;
        }
        //TODO: Move all these config object get*() calls to an "updatePrefs()" function,
        //		and make that get called when any options get changed in the preferences dialog, rather than
        //		polling everytime we get a buffer.

        //Enable or disable RIAA correction
        /*
           int iRIAACorrection =  m_pConfig->getValueString(ConfigKey("[VinylControl]","InputRIAACorrection")).toInt();
           if (iRIAACorrection == 1)
                scratch->EnableRIAACorrection(true);
           else
                scratch->EnableRIAACorrection(false);

           //Set the pre-amp/amplification/input signal boost.
           int prefAmp = m_pConfig->getValueString(ConfigKey("[VinylControl]","VinylControlGain")).toInt();
           scratch->SetAmplify( (float) prefAmp/100. + 1);
         */

        //Set relative mode
        //bRelativeMode = (bool)m_pConfig->getValueString(ConfigKey("[VinylControl]","RelativeMode")).toInt();

        //Set scratch mode
        //bScratchMode = (bool)m_pConfig->getValueString(ConfigKey("[VinylControl]","ScratchMode")).toInt();

        // Analyse the input samples
        analyzer->Analyse(this->m_samples, this->m_SamplesSize);

        lockSamples.unlock();

        //Vinyl control mode
        iVCMode = mode->get();

        //Check if vinyl control is enabled...
        bIsEnabled = enabled->get();

        //Get the pitch range from the prefs.
        fRateRange = rateRange->get();


        // Get Pitch and Position

        dVinylPosition  = analyzer->GetPosition();              //Timecode position in seconds. (Position of the needle)
        dVinylPosition -= iLeadInTime;                                  //Add the lead-in
        dVinylPitch             = analyzer->GetSpeed();                 //Get the playback speed of the vinyl in.

        /*
           OK, something weird has happened. Scratchlib started reporting dVinylPitch as RPM as a fraction:
                My original comment was:
                        //eg. Seems to be in RPM/100 now - eg. 0.33 is 33 RPM)
           However, it now seems to have switched back to being 33 RPM = 1.0 for some reason. I'll refer to
           this scheme as being "pitch1.0" code below.
           -- Albert - April 2, 2007
         */

        //qDebug() << "original dVinylPitch: " << dVinylPitch;

        //THIS IS ONLY NEEDED for non-pitch1.0:
        //dVinylPitch = dVinylPitch / 0.340f; //Normalize it (33 RPM = 1.0)

        dVinylScratch = dVinylPitch;         //Use this value to instruct Mixxx for scratching/seeking.
        dVinylPitch = dVinylPitch - 1.0f;         //Shift the 33 RPM value (33 RPM = 0.0)
        dVinylPitch = dVinylPitch / fRateRange;         //Normalize to the pitch range. (8% = 1.0)

        //Re-get the duration, just in case a track hasn't been loaded yet...
        //FIXME: Commented out by Albert during ControlObjectThread-ification - Nov 3/07
        //duration = ControlObject::getControl(ConfigKey(group, "duration"));

        //Next we set the range that the pitch can go before we consider the turntable to be "seeking".
        //(In absolute mode, when seeking is finished the position is re-synced, which causes a jump.
        // We set a more generous pitch range in absolute mode in order to avoid accidentally resyncing
        // the position.
        // (For example, if a turntable is at +8% pitch, it'll naturally fluctuate a bit - that is, it'll
        //  fluctuate above +8% pitch. Now, we'll read this in as a pitch value that's greater than 1.0f,
        //  and so our algorithm would think the turntable is seeking, when it's not. In order to work around
        //  these fluctuations, we simply make it so that the turntable's pitch must be read as greater than
        //	1.20f before we say that it's seeking.

        if (iVCMode == MIXXX_VCMODE_RELATIVE)         //Relative mode
            dVinylPitchRange = 1.0f;                    //The correct pitch range (if it's going faster than this, it's seeking.)
        else if (iVCMode == MIXXX_VCMODE_ABSOLUTE)        //Absolute mode
            dVinylPitchRange = 1.20f;                   //A wider pitch range to account for turntables' speed fluctuations.
        else
            dVinylPitchRange = 1.0f;



        if (duration != NULL && bIsEnabled)
        {
            filePosition = playPos->get() * duration->get();             //Get the playback position in the file in seconds.

            //qDebug() << "diff in positions: " << fabs(dVinylPosition - dOldPos);
            //if (dVinylPosition != -1.0f)

            // When the Vinyl position has been changed by 0.1seconds
            if (fabs(dVinylPosition - dOldPos) > 0.01)
            {
                dTemp = 0;

                dTotalSpeed += dVinylScratch;
                dTempCount++;

                //qDebug() << "Average speed: " << (double)dTotalSpeed/dTempCount;

                //Useful debug message for tracking down the problem of the vinyl's position "drifting":
                //qDebug() << "Ratio of vinyl's position and Mixxx's: " << fabs(dVinylPosition/filePosition);
                dDriftControl =  ((dVinylPosition/filePosition) - 1) * 8.0f;
                //qDebug() << "dDriftControl: " << dDriftControl;
                //qDebug() << "Scratchlib says the time is: " << dVinylPosition;
                //qDebug() << "Mixxx says the time is: " << filePosition;

                //qDebug() << "dVinylPitch: " << dVinylPitch;

                //If it looks like the turntable is seeking... (ie. we're moving
                //the vinyl really fast in either direction), or we're in scratch mode...
                //(in scratch mode, we always consider the turntable to be seeking, therefore we always
                // use the "controlScratch" control object to control playback.... we never adjust the pitch/rate.)
                if ((dVinylPitch > dVinylPitchRange) || (dVinylPitch < -dVinylPitchRange) || (iVCMode == MIXXX_VCMODE_SCRATCH))
                {
                    //qDebug() << "STATE: seeking";
                    bSeeking = true;
                    playButton->slotSet(0.0f);
                    rateSlider->slotSet(0.0f);
                    controlScratch->slotSet(dVinylScratch);
                }
                else {                 //We're not seeking... just regular playback
                                       //qDebug() << "STATE: regular playback";
                    if (bSeeking == true && (iVCMode == MIXXX_VCMODE_ABSOLUTE)  && dVinylPosition > 0.0f)                     //If we've just stopped seeking, and are playing normal again...
                        syncPosition();
                    bSeeking = false;
                    controlScratch->slotSet(0.0f);
                    playButton->slotSet(1.0f);
                }

                //If the needle just got placed on the record, or playback just resumed
                //from a standstill...
                if (bNeedleDown == false && bSeeking == false && (iVCMode == MIXXX_VCMODE_ABSOLUTE))
                {
                    //qDebug() << "STATE: playback just started";
                    controlScratch->slotSet(0.0f);
                    playButton->slotSet(1.0f);
                    syncPosition();                     //Reposition Mixxx

                    bNeedleDown = true;                     //The needle is now down/the record is playing.
                    dTemp = 0;
                }

                bNeedleDown = true;

                //If we're not seeking, sync Mixxx's pitch with the turntable's pitch.
                if (!bSeeking) {
                    syncPitch(dVinylPitch);
                }

                if (dVinylPosition < 0.0f)
                {
                    playButton->slotSet(0.0f);
                    //playPos->slotSet(0.0f);
                }


                dOldPos = dVinylPosition;

            }
            else             //Either the needle is stopped, up off the record, or we simply skipped some timecodes...
            {
                dTemp++;
                if (dTemp > 20) {                 //If the needle is actually stopped/off the record...
                    int volPeak = analyzer->GetVolumePeak();
                    //qDebug() << "******Needle up? with volume peak:"+QString("%1").arg(volPeak)+"\n";
                    if (bNeedleDown == true && (iVCMode == MIXXX_VCMODE_ABSOLUTE))
                        syncPosition();
                    controlScratch->slotSet(0.0f);
                    playButton->slotSet(0.0f);
                    bNeedleDown = false;
                    dTemp = 21;                     //Make sure we don't overflow eventually..
                }
            }
        }
    }
}

//Synchronize the pitch of the external turntable with Mixxx's pitch.
void VinylControlScratchlib::syncPitch(double pitch)
{
    //The dVinylPitch variable's range (from DAnalyse.h in scratchlib) is
    //from 1.0 +- 00%
    if (iVCMode == MIXXX_VCMODE_ABSOLUTE)  //Only apply drift control when we want to stay synced with the vinyl's position.
        pitch += dDriftControl;     //Apply the drift control to it, to keep the vinyl and Mixxx in sync.
    rateSlider->slotSet(pitch);     //rateSlider has a range of -1.0 to 1.0
    //qDebug() << "pitch: " << pitch;
}

//Synchronize the position of the timecoded vinyl with Mixxx's position.
void VinylControlScratchlib::syncPosition()
{
    float filePosition = playPos->get() * duration->get();
    if (fabs(filePosition - dVinylPosition) > 5.00)
        playPos->slotSet(dVinylPosition / duration->get());         //VinylPos in seconds / total length of song
    //playPos->slotSet(dVinylPosition / (15.0f * 60.0f)); //VinylPos in seconds / (total length of vinyl)
}

bool VinylControlScratchlib::isEnabled()
{
    return bIsEnabled;
}

void VinylControlScratchlib::ToggleVinylControl(bool enable)
{
    bIsEnabled = enable;
}
