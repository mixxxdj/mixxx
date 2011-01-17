/***************************************************************************
                          vinylcontrolxwax.cpp
                             -------------------
    begin                : Sometime in Summer 2007
    copyright            : (C) 2007 Albert Santoni
                           (C) 2007 Mark Hills
                           (C) 2010 Owen Williams
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
#include <math.h>


/****** TODO *******
   Stuff to maybe implement here
   1) The smoothing thing that xwax does
   2) Tons of cleanup
   3) Speed up needle dropping
   4) Extrapolate small dropouts and keep track of "dynamics"

 ********************/

bool VinylControlXwax::m_bLUTInitialized = false;
QMutex VinylControlXwax::m_xwaxLUTMutex;

VinylControlXwax::VinylControlXwax(ConfigObject<ConfigValue> * pConfig, const char * _group) : VinylControl(pConfig, _group)
{
    dOldPos                 = 0.0f;
    dOldDiff                = 0.0f;
    bNeedleDown             = true;
    m_samples               = NULL;
    char * timecode  =  NULL;
    bShouldClose    = false;
    bForceResync    = false;
    iOldMode		= MIXXX_VCMODE_ABSOLUTE;
    dUiUpdateTime   = -1.0f;
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
    m_xwaxLUTMutex.lock(); //Static mutex! We don't want two threads doing this!
   
    timecoder_init(&timecoder, timecode, 1.0f, iSampleRate);
    //Note that timecoder_init will not double-malloc the LUTs, and after this we are guaranteed
    //that the LUT has been generated unless we ran out of memory.
    m_bLUTInitialized = true;
    //}
    m_xwaxLUTMutex.unlock();

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
    m_xwaxLUTMutex.lock(); //Static mutex! We don't want two threads doing this!
    if (m_bLUTInitialized) {
        timecoder_free_lookup(); //Frees all the LUTs in xwax.
        m_bLUTInitialized = false;
    }
    m_xwaxLUTMutex.unlock();
}


void VinylControlXwax::AnalyseSamples(short *samples, size_t size)
{
    if (lockSamples.tryLock())
    {
        //Submit the samples to the xwax timecode processor
        timecoder_submit(&timecoder, samples, size);

        //Update the input signal strength
        //qDebug() << group << (float)fabs((float)samples[0]);
        timecodeInputL->slotSet((float)fabs((float)samples[0]) / SHRT_MAX * 2.0f);
        timecodeInputR->slotSet((float)fabs((float)samples[1]) / SHRT_MAX * 2.0f);
        
        bHaveSignal = fabs((float)samples[0]) + fabs((float)samples[1]) > MIN_SIGNAL;
        //qDebug() << "signal?" << bHaveSignal;
        
        waitForNextInput.wakeAll();
        lockSamples.unlock();
    }
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
    double old_duration = -1.0f;
    int reportedMode = 0;
    bool reportedPlayButton = 0;

	float when;

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
        //		and make that get called when any options get changed in the preferences dialog, rather than
        //		polling everytime we get a buffer.

        
        //Check if vinyl control is enabled...
        bIsEnabled = checkEnabled(bIsEnabled, enabled->get());
    
    	//Get the pitch range from the prefs.
        fRateRange = rateRange->get();
        
		//are we even playing and enabled at all?
        if (duration != NULL && bIsEnabled)	
        {
        	// Analyse the input samples
	        iPosition = timecoder_get_position(&timecoder, &when);
        	//qDebug() << group << id << iPosition;
	        
        	double cur_duration = duration->get();
        	//FIXME? we should really sync on all track changes
        	if (cur_duration != old_duration)
        	{
        		bForceResync=true;
        		old_duration = cur_duration;
        	}
        
		    dVinylPosition = iPosition;
		    dVinylPosition = dVinylPosition / 1000.0f;
		    //qDebug() << "dVinylPosition1" << dVinylPosition << iLeadInTime;
		    dVinylPosition -= iLeadInTime;
		    //qDebug() << "dVinylPosition2" << dVinylPosition;
		    
        	//Initialize drift control to zero in case we don't get any position data to calculate it with.
        	dDriftControl = 0.0f;
        	dVinylPitch = timecoder_get_pitch(&timecoder);
            filePosition = playPos->get() * cur_duration;             //Get the playback position in the file in seconds.
            
            reportedMode = mode->get();
            //qDebug() << "cur mode" << reportedMode;
            reportedPlayButton = playButton->get();
            
			if (iVCMode != reportedMode)
		    {
		    	//qDebug() << "cur mode" << iVCMode << "new mode" << reportedMode;
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
			    	bForceResync = true;
			    	if (vinylStatus->get() == VINYL_STATUS_ERROR)
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
					if ((filePosition + iLeadInTime) * 1000.0f  > timecoder_get_safe(&timecoder) &&
						!bForceResync) //corner case: we are waiting for resync so don't enable just yet
						enableRecordEndMode();
				}
				else if (iVCMode == MIXXX_VCMODE_RELATIVE || iVCMode == MIXXX_VCMODE_CONSTANT)
				{
					if (iPosition != -1 && iPosition > timecoder_get_safe(&timecoder))
						enableRecordEndMode();
				}
			}
			
            if (atRecordEnd)
            { 
            	//if atRecordEnd was true, maybe it no longer applies:
            	
            	if ((iVCMode == MIXXX_VCMODE_ABSOLUTE && 
						(filePosition + iLeadInTime) * 1000.0f  <= timecoder_get_safe(&timecoder)))
				{
					//if we are in absolute mode and the file position is in a safe zone now
					disableRecordEndMode();
				}
				else if (!reportedPlayButton) 
				{
					//if we turned off play button, also disable
					disableRecordEndMode();
				}
				else if (iPosition != -1) 
				{
					//if relative mode, and vinylpos is safe
					if (iVCMode == MIXXX_VCMODE_RELATIVE && 
		            	iPosition <= timecoder_get_safe(&timecoder))
		            {
			            disableRecordEndMode();
			        }
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
				//Notify the UI that the timecode quality is good
                timecodeQuality->slotSet(1.0f);
                
                //dVinylPitch = (dOldPitch * (XWAX_SMOOTHING - 1) + dVinylPitch) / XWAX_SMOOTHING;
                				
                //FIXME (when Mark finished variable samplerates in timecoder)
                //Hack to make other samplerates work with xwax:
                //dVinylPitch *= (iSampleRate/44100);

                //Re-get the duration, just in case a track hasn't been loaded yet...
                //duration = ControlObject::getControl(ConfigKey(group, "duration"));
                
				//We have pitch, but not position.  so okay signal but not great (scratching / cueing?)
				//qDebug() << "Pitch" << dVinylPitch;
				if (iPosition != -1)
				{
					//POSITION: YES  PITCH: YES
					//add a value to the pitch ring (for averaging / smoothing the pitch)
					
					dPitchRing[ringPos] = dVinylPitch;
					if(ringFilled < RING_SIZE)
			        	ringFilled++;
			        	
			        //save the absolute amount of drift for when we need to estimate vinyl position
					dDriftAmt = dVinylPosition - filePosition;
					
	            	//qDebug() << "vinyl" << dVinylPosition << "file" << filePosition;
	            	if (bForceResync)
	            	{
	            		//if forceresync was set but we're no longer absolute,
	            		//it no longer applies
	            		if (iVCMode == MIXXX_VCMODE_ABSOLUTE)
	            		{
		            		syncPosition();
	                    	resetSteadyPitch(dVinylPitch, dVinylPosition);
	                    }
	                    bForceResync = false;
	            	}
	            	else if (m_bNeedleSkipPrevention &&
	            		iVCMode == MIXXX_VCMODE_ABSOLUTE &&
	            		filePosition != dOldFilePos &&
	                    (
		                    ((dVinylPosition - dOldPos) * (dVinylPitch / fabs(dVinylPitch)) < 0) ||
		                    ((dVinylPosition - dOldPos) * (dVinylPitch / fabs(dVinylPitch)) > 0.6)
	                    ))
	                {
	                	//red alert, moved wrong direction or jumped forward a lot,
	                	//move to constant mode and keep playing
	                	//TODO: trigger some sort of UI alert so the dj
	                	//can clean the needle
	                	qDebug() << "WARNING: needle skip detected!:";
	                	qDebug() << filePosition << dOldFilePos << dVinylPosition << dOldPos;
	                	enableConstantMode();
	                	resetSteadyPitch(dVinylPitch, dVinylPosition);
	                	vinylStatus->slotSet(VINYL_STATUS_ERROR);
	                }
	                else if (fabs(dVinylPosition - dOldPos) >= 5.0f &&  
	                    (iVCMode == MIXXX_VCMODE_ABSOLUTE))
	                {
	                	//If the position from the timecode is more than a few seconds off, resync the position.
	                	qDebug() << "resync position (>5.0 sec)";
	                	qDebug() << dVinylPosition << dOldPos << dVinylPosition - dOldPos;
	                    syncPosition();
	                    resetSteadyPitch(dVinylPitch, dVinylPosition);
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
                    else if (!m_bNeedleSkipPrevention &&
                    	fabs(dVinylPosition - dOldPos) >= 0.1f &&
                    	iVCMode == MIXXX_VCMODE_ABSOLUTE)
                    {
                    	qDebug() << "CDJ resync position (>0.1 sec)";
                    	syncPosition();
	                    resetSteadyPitch(dVinylPitch, dVinylPosition);
                    }
                    else if (playPos->get() == 1.0 && dVinylPitch > 0)
				    {
				    	//end of track
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
	                dDriftControl = ((filePosition - dVinylPosition)  / dVinylPosition) / 100 * 4.0f;
	                
	                //if we hit the end of the ring, loop around
                    ringPos++;
	                if(ringPos > RING_SIZE)
       				 	ringPos = 0;
		            dOldPos = dVinylPosition;
		        }
		        else
		        {
		        	//POSITION: NO  PITCH: YES
		        	//if we don't have valid position, we're not playing so reset time to current
		            //estimate vinyl position
		            
		            if (playPos->get() == 1.0 && dVinylPitch > 0)
				    {
				    	//end of track
				    	togglePlayButton(false);
				    	resetSteadyPitch(0.0f, 0.0f);
						controlScratch->slotSet(0.0f);
						ringPos = 0;
						ringFilled = 0;
				    	continue;
				    }
		            
		            dOldPos = filePosition + dDriftAmt;
		            if (uiUpdateTime(filePosition))
		        		timecodeQuality->slotSet(0.75f);
		        	
		        	if (dVinylPitch > 0.2)
		        	{	
	                	togglePlayButton(checkSteadyPitch(dVinylPitch, filePosition) > 0.5);
					}		            	
		        }
		        
		        //playbutton status may have changed
		        reportedPlayButton = playButton->get();
		        
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
	            	controlScratch->slotSet(averagePitch + dDriftControl);
	            	if (iPosition != -1 && reportedPlayButton && uiUpdateTime(filePosition))
	            	{
	                	rateSlider->slotSet(rateDir->get() * (fabs(averagePitch + dDriftControl) - 1.0f) / fRateRange);
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
			        timecodeQuality->slotSet(0.0f);
			        ringPos = 0;
			        ringFilled = 0;
			        vinylStatus->slotSet(VINYL_STATUS_OK);
			    }
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

void VinylControlXwax::disableRecordEndMode()
{
	vinylStatus->slotSet(VINYL_STATUS_OK);
	atRecordEnd = false;
	//don't start a new track with constant mode
	if (iVCMode == MIXXX_VCMODE_CONSTANT)
	{
		if (iOldMode == MIXXX_VCMODE_CONSTANT)
			iVCMode = MIXXX_VCMODE_RELATIVE;
		else
			iVCMode = iOldMode;
		mode->slotSet((double)iVCMode);
	}
}

void VinylControlXwax::togglePlayButton(bool on)
{
	if (bIsEnabled && playButton->get() != on)
		playButton->slotSet((float)on);  //and we all float on all right
}

void VinylControlXwax::resetSteadyPitch(double pitch, double time)
{
	dSteadyPitch = pitch;
	dSteadyPitchTime = time;
}

double VinylControlXwax::checkSteadyPitch(double pitch, double time)
{
	//return TRUE if we have established 0.5 secs of steady pitch
	
	if (time < dSteadyPitchTime) //bad values, often happens during resync
	{
		resetSteadyPitch(pitch, time);
		return 0.0;
	}
	
	if (fabs(pitch - dSteadyPitch) < 0.2f)
	{
		return time - dSteadyPitchTime;
	}
	
	//else
	resetSteadyPitch(pitch, time);
	return 0.0;
}

//Synchronize Mixxx's position to the position of the timecoded vinyl.
void VinylControlXwax::syncPosition()
{
	//qDebug() << "sync position";
   	playPos->slotSet(dVinylPosition / duration->get());     //VinylPos in seconds / total length of song
}

bool VinylControlXwax::checkEnabled(bool was, bool is)
{
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
