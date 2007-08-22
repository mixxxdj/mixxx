/***************************************************************************
                          sounddeviceportaudio.cpp
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
#include "vinylcontrolxwax.h"


/****** TODO *******
Stuff to maybe implement here
1) The smoothing thing that xwax does
2) ....?

********************/

VinylControlXwax::VinylControlXwax(ConfigObject<ConfigValue> *pConfig, const char *_group) : VinylControl(pConfig, _group)
{ 
	dOldPos			= 0.0f;
	dOldDiff		= 0.0f;
	bNeedleDown		= true;
	m_samples		= NULL;
    char* timecode  =  NULL;
    bIsRunning      = true;
    bShouldClose    = false;
 
	if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEA)
		timecode = "serato_2a";
	else if (strVinylType == MIXXX_VINYL_SERATOCV02VINYLSIDEB)
	    timecode = "serato_2b";
	else if (strVinylType == MIXXX_VINYL_SERATOCD)
	    timecode = "serato_cd";
	else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHSIDEA)
	    timecode = "traktor_a";
	else if (strVinylType == MIXXX_VINYL_TRAKTORSCRATCHSIDEB)
	    timecode = "traktor_b";

	//Enable or disable RIAA correction
	
	//Set the sample rate
	qDebug("Xwax Vinyl control starting with a sample rate of: %i", iSampleRate);
   
    qDebug("Building timecode lookup tables...\n");
    
    
    if(timecoder_build_lookup(timecode) == -1) 
    {
        qDebug("ERROR: Failed to build the timecode table!");
        return;
    }   
   
    timecoder_init(&timecoder);


	//(This ends up calling-back the function "run()" below.)
    start();

	qDebug("Created new VinylControlXwax!\n");
	
		
	if (bRelativeMode)
		qDebug("Relative mode enabled!");	
	if (bScratchMode)
	{
		qDebug("********************");
		qDebug("SCRATCH MODE ENABLED");
		qDebug("********************");
	}
}

VinylControlXwax::~VinylControlXwax()
{
	// Remove existing samples
	if (m_samples)
		free(m_samples);

    //Cleanup xwax nicely
    timecoder_free_lookup();
    timecoder_clear(&timecoder);

	// Continue the run() function and close it
	bShouldClose = true;
	waitForNextInput.wakeAll();
	terminate();
}



void VinylControlXwax::AnalyseSamples(short* samples, size_t size)
{
	lockSamples.lock();
	timecoder_submit(&timecoder, samples, size);
	lockSamples.unlock();

	waitForNextInput.wakeAll();
}


void VinylControlXwax::run()
{
	int	iTPS = 0;	// Timecodes per second
	dVinylPosition	= 0.0f;
	dVinylPitch		= 0.0f;
	dOldPitch       = 0.0f;
	double dTemp	= 0.0f;
	double dPlayPos = 0.0f; 
	//bool absoluteMode = true;
	float filePosition = 0.0f;
	//bool bScratchMode = true;
	double dVinylPitchRange = 1.0f;
	double dTotalSpeed = 0.0f;
	double dTempCount = 0.0f;	
	bool bNeedsPosSync = false;

    int when, alive, pitch_unavailable;

	//This shouldn't be needed (the preferences dialog does it)
	if (bScratchMode)
		bRelativeMode = true;

	bShouldClose = false;
	while(true)
	{
	    lockSamples.lock();
		waitForNextInput.wait(&lockSamples);
		lockSamples.unlock(); //Is this correct?
		
		if (bShouldClose)
			return;	
				
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
			*/

		//Set the pre-amp/amplification/input signal boost.
		//int prefAmp = m_pConfig->getValueString(ConfigKey("[VinylControl]","VinylControlGain")).toInt();
		//scratch->SetAmplify( (float) prefAmp/100. + 1);

		//Set relative mode
		bRelativeMode = (bool)m_pConfig->getValueString(ConfigKey("[VinylControl]","RelativeMode")).toInt();
		
		//Set scratch mode
		bScratchMode = (bool)m_pConfig->getValueString(ConfigKey("[VinylControl]","ScratchMode")).toInt();

		// Analyse the input samples
        int iPosition = -1;
        iPosition = timecoder_get_position(&timecoder, &when);	
         
        dVinylPosition = iPosition;
        dVinylPosition = dVinylPosition / 1000.0f;
        dVinylPosition -= iLeadInTime;	//Add the lead-in
          
        //qDebug() << "iLeadInTime:" << iLeadInTime << "dVinylPosition" << dVinylPosition;
        
        alive = timecoder_get_alive(&timecoder);
        dOldPitch = dVinylPitch;
        pitch_unavailable = timecoder_get_pitch(&timecoder, &dVinylPitch);
        
        if(alive && !pitch_unavailable)
        {
            //dVinylPitch = (dOldPitch * (XWAX_SMOOTHING - 1) + dVinylPitch) / XWAX_SMOOTHING;
            //qDebug("dVinylPosition: %f, dVinylPitch: %f, when: %d", dVinylPosition, dVinylPitch, when);
        }
        
		//qDebug("original dVinylPitch: %f", dVinylPitch);
		
		//THIS IS ONLY NEEDED for non-pitch1.0:
		//dVinylPitch = dVinylPitch / 0.340f; //Normalize it (33 RPM = 1.0)
		
		dVinylScratch = dVinylPitch; //Use this value to instruct Mixxx for scratching/seeking.
		dVinylPitch = dVinylPitch - 1.0f; //Shift the 33 RPM value (33 RPM = 0.0)
		dVinylPitch = dVinylPitch / 0.080f; //Normalize to the pitch range. (8% = 1.0)
	
		//Re-get the duration, just in case a track hasn't been loaded yet...
		duration = ControlObject::getControl(ConfigKey(group, "duration"));
		
		//Next we set the range that the pitch can go before we consider the turntable to be "seeking".
		//(In absolute mode, when seeking is finished the position is re-synced, which causes a jump.
		// We set a more generous pitch range in absolute mode in order to avoid accidentally resyncing
		// the position. 
		// (For example, if a turntable is at +8% pitch, it'll naturally fluctuate a bit - that is, it'll
		//  fluctuate above +8% pitch. Now, we'll read this in as a pitch value that's greater than 1.0f,
		//  and so our algorithm would think the turntable is seeking, when it's not. In order to work around
		//  these fluctuations, we simply make it so that the turntable's pitch must be read as greater than
		//	1.20f before we say that it's seeking.
		
		if (bRelativeMode) //Relative mode
			dVinylPitchRange = 1.0f;	//The correct pitch range (if it's going faster than this, it's seeking.)
		else //Absolute mode
			dVinylPitchRange = 1.20f; 	//A wider pitch range to account for turntables' speed fluctuations.
			
		
		//Find out whether or not VinylControl is enabled.
		bIsRunning = (bool)m_pConfig->getValueString(ConfigKey("[VinylControl]","Enabled")).toInt();
		//qDebug("VinylControl: bIsRunning=%i", bIsRunning);
		
		if (duration != NULL && bIsRunning)
		{
			filePosition = playPos->get() * duration->get(); //Get the playback position in the file in seconds.
				
			//qDebug("diff in positions: %f", fabs(dVinylPosition - dOldPos));
			//if (dVinylPosition != -1.0f)
			
			// When the Vinyl position has been changed by 0.1seconds
			//if (fabs(dVinylPosition - dOldPos) > 0.01)
			//if (dVinylPosition > 0.0f || bScratchMode)
			if((alive && !pitch_unavailable))
			{
				dTemp = 0;
				
				dTotalSpeed += dVinylScratch;
				dTempCount++;
				
				//qDebug("Average speed: %f", (double)dTotalSpeed/dTempCount);
				
				//Useful debug message for tracking down the problem of the vinyl's position "drifting":
				//qDebug("Ratio of vinyl's position and Mixxx's: %f", fabs(dVinylPosition/filePosition));
				dDriftControl =  ((dVinylPosition/filePosition) - 1)/100 * 4.0f;
				//dDriftControl = 0.0f;
				//qDebug("dDriftControl: %f", dDriftControl);
				//qDebug("Xwax says the time is: %f", dVinylPosition);
				//qDebug("Mixxx says the time is: %f", filePosition);
				//qDebug("dVinylPitch: %f", dVinylPitch);			

				//If the needle just got placed on the record, or playback just resumed
				//from a standstill...
				if (bNeedleDown == false  && !bRelativeMode) //&& bSeeking == false
				{
					//qDebug("STATE: playback just started");
					controlScratch->queueFromThread(0.0f);
					playButton->queueFromThread(1.0f);
					
					bNeedsPosSync = true; //Schedule a position resync, whenever we actually start getting the position signal again.				
					bNeedleDown = true; //The needle is now down/the record is playing.
					dTemp = 0;
				}

                //If we need a resync, and we have the timecode position, then resync (sometimes we don't get enough signal right away)
                if (bNeedsPosSync && (dVinylPosition > 0.0f) && !bRelativeMode)
                {
                	syncPosition();	//Reposition Mixxx
                	bNeedsPosSync = false;
                }

				bNeedleDown = true;
				
				//If it looks like the turntable is seeking... (ie. we're moving
				//the vinyl really fast in either direction), or we're in scratch mode...
				//(in scratch mode, we always consider the turntable to be seeking, therefore we always
				// use the "controlScratch" control object to control playback.... we never adjust the pitch/rate.)			
				if ((dVinylPitch > dVinylPitchRange) || (dVinylPitch < -dVinylPitchRange) || (bScratchMode))
				{
					//qDebug("STATE: seeking");
					bSeeking = true;
					playButton->queueFromThread(0.0f);
					rateSlider->queueFromThread(0.0f);
					controlScratch->queueFromThread(dVinylScratch);
				}
				else { //We're not seeking... just regular playback
					//qDebug("STATE: regular playback");
					if (bSeeking == true && !bRelativeMode  && dVinylPosition > 0.0f) //If we've just stopped seeking, and are playing normal again...
						syncPosition();
					bSeeking = false;
					controlScratch->queueFromThread(0.0f);
					playButton->queueFromThread(1.0f);
				}

				//If we're not seeking, sync Mixxx's pitch with the turntable's pitch.
				if (!bSeeking) {
					syncPitch(dVinylPitch);
				}
				
				dOldPos = dVinylPosition;			
			
			}	
		    else
			{
			    //qDebug("STATE: Needle up?");
				playButton->queueFromThread(0.0f);
				controlScratch->queueFromThread(0.0f);
				dTemp++;
				if (dTemp > 8)
				{
				    bNeedleDown = false;
				    bNeedsPosSync = true;
				    dTemp = 10; //Don't cha be overflowin' matey, yar.
				}    
				//playPos->queueFromThread(0.0f);
			}
		}
	}
}

//Synchronize the pitch of the external turntable with Mixxx's pitch.
void VinylControlXwax::syncPitch(double pitch)
{
	//The dVinylPitch variable's range (from DAnalyse.h in scratchlib) is
	//from 1.0 +- 00%
	pitch += dDriftControl; //Apply the drift control to it, to keep the vinyl and Mixxx in sync.
	rateSlider->queueFromThread(pitch); //rateSlider has a range of -1.0 to 1.0
	//qDebug("pitch: %f", pitch);
}

//Synchronize the position of the timecoded vinyl with Mixxx's position.
void VinylControlXwax::syncPosition()
{
	float filePosition = playPos->get() * duration->get();
	//if (fabs(filePosition - dVinylPosition) > 5.00)
	
	playPos->queueFromThread(dVinylPosition / duration->get()); //VinylPos in seconds / total length of song
	
	//playPos->queueFromThread(dVinylPosition / (15.0f * 60.0f)); //VinylPos in seconds / (total length of vinyl)
}

bool VinylControlXwax::isEnabled()
{ 
    return bIsRunning;
}

void VinylControlXwax::ToggleVinylControl(bool enable) 
{
    bIsRunning = enable;
}
