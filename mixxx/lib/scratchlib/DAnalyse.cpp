/////////////////////////////////////////////////////////////////////////////
// Name:        DAnalyse.cpp
// Purpose:
// Author:      Stefan Langhammer, Thomas Rogg
// Modified by:
// Created:     4.12.2005
// Modified:    15.08.2006
// Copyright:   (c) Stefan Langhammer, Thomas Rogg
// Licence:     LGPL
/////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
//	DAnalyse.cpp - Final Scratch vinyl analyser - analyser class
// -----------------------------------------------------------------------------

#include "DAnalyse.h"
#include "timeCodeArray.h"
#include <cstdio>

#define fabs_MACRO(x)	((x) < 0 ? -(x) : (x))

// -----------------------------------------------------------------------------
//	DAnalyse::DAnalyse - analyser constructor
// -----------------------------------------------------------------------------

DAnalyse::DAnalyse()
{
	mVinyl					= DSCRATCH_VINYL_FINALSCRATCH;
	mNumTimecodePeaks		= 32; // Default for Final Scratch
	SetThreshold			(1); //percent

	mTimecodesPerSecond		= 0;
	mTimecodesPerSecondTemp = 0;
	mTimeFullSeconds		= 0;
}


// -----------------------------------------------------------------------------
//	DAnalyse::DAnalyse - analyser destructor
// -----------------------------------------------------------------------------

DAnalyse::~DAnalyse()
{
}


// -----------------------------------------------------------------------------
//	DAnalyse::SetVinyl - sets the vinyl we're using
// -----------------------------------------------------------------------------

void DAnalyse::SetVinyl(int vinyl)
{
	mVinyl = vinyl;
	if		(mVinyl == DSCRATCH_VINYL_FINALSCRATCH)
		mNumTimecodePeaks = 32;
	else if (mVinyl == DSCRATCH_VINYL_MIXVIBES)
		mNumTimecodePeaks = 91;
}


// -----------------------------------------------------------------------------
//	DAnalyse::SetFrequency - sets the frequency of the data
// -----------------------------------------------------------------------------

void DAnalyse::SetFrequency(int frequency)
{
	mFrequency = frequency;

	// Reinitialize
	mTime = 0.0f;

	mAmplitudeDir[0] = DSCRATCH_AMPLITUDE_STAYING_SAME;
	mAmplitudeDir[1] = DSCRATCH_AMPLITUDE_STAYING_SAME;

	mPeaks.clear();

	mNumBitIdedPeaks = 0;

	mSpeed = 1.0f;
	mPosition = 0.0f; // Timecode position in seconds

	lastLowestPeak[0].amplitude		= 0;
	lastLowestPeak[0].channel		= DSCRATCH_PEAK_CHANNEL_LEFT;
	lastLowestPeak[0].bit			= DSCRATCH_BIT_UNDEFINED;
	lastLowestPeak[1].amplitude		= 0;
	lastLowestPeak[1].channel		= DSCRATCH_PEAK_CHANNEL_RIGHT;
	lastLowestPeak[1].bit			= DSCRATCH_BIT_UNDEFINED;
	
	lastHighestPeak[0].amplitude	= 0;
	lastHighestPeak[0].channel		= DSCRATCH_PEAK_CHANNEL_LEFT;
	lastHighestPeak[0].bit			= DSCRATCH_BIT_UNDEFINED;
	lastHighestPeak[1].amplitude	= 0;
	lastHighestPeak[1].channel		= DSCRATCH_PEAK_CHANNEL_RIGHT;
	lastHighestPeak[1].bit			= DSCRATCH_BIT_UNDEFINED;
}	


// -----------------------------------------------------------------------------
//	DAnalyse::SetThreshold - sets the threshold in percent of the dynamic range
// -----------------------------------------------------------------------------
void DAnalyse::SetThreshold			(int percent)
{
	// 2^16 = 65536
	m_iThreshold = int (65536 * (percent/100.0f));
}

// -----------------------------------------------------------------------------
//	DAnalyse::SetCalibration - Set the peak tolerance range and reset timecodes/s
// -----------------------------------------------------------------------------
void DAnalyse::SetCalibration(float calibrationValue)
{
	m_fCalibrationValue = calibrationValue;

	// Reset the timecodes/second counter (important for the calibration)
	mTimeFullSeconds		= 0;
	mTimecodesPerSecond		= 0;
	mTimecodesPerSecondTemp = 0;
}

// -----------------------------------------------------------------------------
//	DAnalyse::Analyse - analyse sampled data
// -----------------------------------------------------------------------------

bool DAnalyse::Analyse(short *samples, int numSamples)
{
    mbNewTimecodeDetected = false;
	int channel, i;
	channel = 0;
	mVolumePeak = 0;
	for(i = 0; i < numSamples; i++)
	{
		// we have crossed zero? Store the peak
		if	(	mAmplitudeDir[channel] == DSCRATCH_AMPLITUDE_GOING_DOWN && *samples - lastLowestPeak[channel].amplitude > 2000)
		{
			lastLowestPeak[channel].type = DSCRATCH_PEAK_TYPE_LOW;
			AddPeak(lastLowestPeak[channel]);
			lastHighestPeak[channel].amplitude = *samples;
			lastLowestPeak[channel].amplitude = *samples;
			mAmplitudeDir[channel] = DSCRATCH_AMPLITUDE_GOING_UP;
		}
		if	(	mAmplitudeDir[channel] == DSCRATCH_AMPLITUDE_GOING_UP && lastHighestPeak[channel].amplitude - *samples > 2000)
		{
			lastHighestPeak[channel].type = DSCRATCH_PEAK_TYPE_HIGH;
			AddPeak(lastHighestPeak[channel]);
			lastHighestPeak[channel].amplitude = *samples;
			lastLowestPeak[channel].amplitude = *samples;
			mAmplitudeDir[channel] = DSCRATCH_AMPLITUDE_GOING_DOWN;
		}
		
		// Initialize direction
		if	(	mAmplitudeDir[channel] == DSCRATCH_AMPLITUDE_STAYING_SAME)
		{
			if		(*samples  <0) mAmplitudeDir[channel] = DSCRATCH_AMPLITUDE_GOING_DOWN;
			else if (*samples  >0) mAmplitudeDir[channel] = DSCRATCH_AMPLITUDE_GOING_UP;
		}
		
		// Check lowest peak
		if (*samples < lastLowestPeak[channel].amplitude)
		{
			lastLowestPeak[channel].amplitude = *samples;
			lastLowestPeak[channel].time = mTime + (i >> 1) * (1000.0f / mFrequency);
		}
		// Check highest peak
		if (*samples > lastHighestPeak[channel].amplitude)
		{
			lastHighestPeak[channel].amplitude = *samples;
			lastHighestPeak[channel].time = mTime + (i >> 1) * (1000.0f / mFrequency);
		}

		if (fabs_MACRO(*samples) > mVolumePeak)
			mVolumePeak = fabs_MACRO(*samples);

		samples++;
		channel = !channel;
	}
	// Get new time
	mTime += numSamples/2 * (1000.0f / mFrequency);
	return mbNewTimecodeDetected;
}

// -----------------------------------------------------------------------------
//	DAnalyse::AddPeak - add a new peak to the peak list
// -----------------------------------------------------------------------------

void DAnalyse::AddPeak(DPeak &peak)
{
	std::list<DPeak>::iterator iter;
	DPeak *firstPeaks[3];
	float diffDiv;

	if(mPeaks.size() > 3)
	{
		// simple Treshold
		iter = mPeaks.begin();
		iter++;iter++;
		DPeak *lastPeakOnChannel = &*iter;
		int dynamic = fabs_MACRO( (lastPeakOnChannel->amplitude+65536) + (peak.amplitude+65536) );
		if( dynamic < m_iThreshold)
			return;
		

		// We need the 1st,3rd,5th peak to get peaks of one channel
		iter = mPeaks.begin();		
		iter++;
		firstPeaks[0] = &*iter;
		iter++;
		iter++;
		firstPeaks[1] = &*iter;

		diffDiv = fabs_MACRO(firstPeaks[0]->amplitude - firstPeaks[1]->amplitude) / (float) fabs_MACRO(firstPeaks[0]->amplitude - peak.amplitude);

		// if the new peak has the same "distance" to peak 0,
		// as the peak 1 to peak 0 (with a little variance)
		/*
		if(diffDiv			< (mVinyl == DSCRATCH_VINYL_MIXVIBES ? 1.07 : 1.2)
		&& (1 / diffDiv)	< (mVinyl == DSCRATCH_VINYL_MIXVIBES ? 1.07 : 1.2))	
		*/

		if(diffDiv			< m_fCalibrationValue
		&& (1 / diffDiv)	< m_fCalibrationValue)	
		{
			if(firstPeaks[1]->bit != DSCRATCH_BIT_UNDEFINED)
			{
				peak.bit = firstPeaks[1]->bit;				//assign bit value of peak 1
				mNumBitIdedPeaks++;
			}
		}
		// The peak is outside the variance of distance "peak0-peak1"
		else
		{
			// the last peak was a positive sine mountain ?
			if(firstPeaks[0]->type == DSCRATCH_PEAK_TYPE_HIGH)
			{
				// Then the new should be a low. Therefore if the
				// new peak is larger than the old, store a high bit, else a low.
				peak.bit = firstPeaks[1]->amplitude > peak.amplitude ? DSCRATCH_BIT_HIGH : DSCRATCH_BIT_LOW;
			}
			else
			{
				// The last peak was a negative sine mountain.
				// Then the new should be a high. Therefore if the
				// new peak is larger than the old, store a low bit, else a high.
				peak.bit = firstPeaks[1]->amplitude > peak.amplitude ? DSCRATCH_BIT_LOW : DSCRATCH_BIT_HIGH;
			}
			if(firstPeaks[1]->bit == peak.bit)
			{
				// Bits are same... Paradox
				peak.bit = DSCRATCH_BIT_UNDEFINED;
				mNumBitIdedPeaks = 0;
			}
			else
			{
				mNumBitIdedPeaks++;
			}
		}
	}

	mPeaks.push_front(peak);
	if(mPeaks.size() == mNumTimecodePeaks * 2)
		mPeaks.pop_back();

	DetectTimecode();
}

// -----------------------------------------------------------------------------
//	DAnalyse::DetectTimecode - try to detect a timecode
// -----------------------------------------------------------------------------

void DAnalyse::DetectTimecode()
{
	bool timeCode[91]; // 91 is the highest number of peaks we currently need for mixvibes vinyl
	int i;
	
	std::list<DPeak>::iterator iter, iter2;

	if(mNumBitIdedPeaks >= mNumTimecodePeaks )
	{
		CalculateSpeed();
		iter = mPeaks.begin();

		if (mSpeed > 0)	//look for timecodes
		{
			if ( iter->channel != DSCRATCH_PEAK_CHANNEL_LEFT )
			return;
			
			for(i = 0; i < mNumTimecodePeaks; i++)
			{
				timeCode[mNumTimecodePeaks -i - 1] = iter->bit == DSCRATCH_BIT_HIGH;
				iter++;
			}

			// Check for sync
			if(mVinyl == DSCRATCH_VINYL_MIXVIBES
			&& !timeCode[0] && !timeCode[2] && !timeCode[4] && !timeCode[6])
			{
				// MIXVIBES
				unsigned int tc = 0;
				
				for(i = 89; i >= 5; i -= 4)
					tc = (tc << 1) | timeCode[i];

				if(((tc >> 17) & 0x1F) == (tc & 0x1F))
				{
					mPosition = (tc & 0x1FFFF) * 22.0f / 3000.0f;		// 3000 samples / sec
																	// 22 waves / timecode
					mbNewTimecodeDetected = true;
					mTimecodesPerSecondTemp++;

					if (fabs_MACRO(mPeaks.begin()->time - mTimeFullSeconds) > 1000.0)
					{
						mTimeFullSeconds			= int(mPeaks.begin()->time);
						mTimecodesPerSecond			= mTimecodesPerSecondTemp;
						mTimecodesPerSecondTemp		= 0;
					}
					return;
				}
			}
			if(mVinyl == DSCRATCH_VINYL_FINALSCRATCH
			&& timeCode[0] && timeCode[2] && timeCode[4] && !timeCode[6])  // correct :)
			{
				// FINAL SCRATCH
				unsigned int tc = 0;
				
				for(i = 0; i < 32; i += 2)
					tc = (tc << 1) | timeCode[i];
				for(i = 1; i < 32; i += 2)
					tc = (tc << 1) | timeCode[i];

				unsigned int arraySize = 271744; //135872lines *2;
				for (unsigned int i = 0; i < arraySize; i+=2)
				{
					if (tc == gTimeCodeArray[i])
					{
						mPosition = gTimeCodeArray[i+1] / (float)mFrequency;

						mbNewTimecodeDetected = true;
						mTimecodesPerSecondTemp++;
						
						if (fabs_MACRO(mPeaks.begin()->time - mTimeFullSeconds) > 1000.0)
						{
							mTimeFullSeconds			= int(mPeaks.begin()->time);
							mTimecodesPerSecond			= mTimecodesPerSecondTemp;
							mTimecodesPerSecondTemp		= 0;
						}
						return;
					}
				}
			}
		}
	}
	
	// Final Scratch Reverse Peak Counter
	// The FS sine wave has 1200 hz
	if (mPeaks.size() > 5)
	{	
		DPeak *tempPeaks[2];
		iter2 = mPeaks.begin();
		iter2++;
		tempPeaks[0] = &*iter2;		iter2++;
		tempPeaks[1] = &*iter2;		iter2++;

		// check for reverse playback on the both channels
		if		(mVinyl == DSCRATCH_VINYL_FINALSCRATCH)
		{
			if		(	( tempPeaks[0]->channel == DSCRATCH_PEAK_CHANNEL_LEFT  && tempPeaks[1]->type    != tempPeaks[0]->type) ||
						( tempPeaks[0]->channel == DSCRATCH_PEAK_CHANNEL_RIGHT && tempPeaks[1]->type    == tempPeaks[0]->type) )
					{
						mPosition -= 1/4800.0f;		// 1200hz*4
						mbNewTimecodeDetected = true;
					}
		}
		else if (mVinyl == DSCRATCH_VINYL_MIXVIBES)
		{
			if		(	( tempPeaks[0]->channel == DSCRATCH_PEAK_CHANNEL_LEFT  && tempPeaks[1]->type    == tempPeaks[0]->type) ||
						( tempPeaks[0]->channel == DSCRATCH_PEAK_CHANNEL_RIGHT && tempPeaks[1]->type    != tempPeaks[0]->type) )
					{
						mPosition -= 1/12000.0f;		// 3000hz*4
						mbNewTimecodeDetected = true;
					}
		}
	}
}
// -----------------------------------------------------------------------------
//	DAnalyse::CalculateSpeed - determine the current speed
// -----------------------------------------------------------------------------
void DAnalyse::CalculateSpeed()
{
	std::list<DPeak>::iterator iter, iter2, iter3;
	float speed;
	int i, n;
	iter = mPeaks.begin();
	iter++;
	iter2 = mPeaks.begin();
	iter2++;iter2++;
	iter3 = mPeaks.begin();
	iter3++;iter3++;iter3++;

	n = int(mPeaks.size()) - 4;
	if(n > 32)
		n = 32;

	mSpeed = 0;
	for(i = 0; i < n; i++)
	{
		// Final Scratch
		if(mVinyl == DSCRATCH_VINYL_FINALSCRATCH)
		{
			speed = 2.4f * (iter->time - iter3->time);
			if(iter3->channel == DSCRATCH_PEAK_CHANNEL_RIGHT)
			{
				 if(iter2->type != iter3->type)
					speed = -speed;
			}
			else if(iter->type != iter2->type)
					speed = -speed;
		}
		else 
		{
			speed = (iter->time - iter3->time) * 6;
			if(iter3->channel == DSCRATCH_PEAK_CHANNEL_RIGHT)
			{
				 if(iter2->type != iter->type)
					speed = -speed;
			}
			else if(iter3->type != iter2->type)
					speed = -speed;
		}

		iter++;
		iter2++;
		iter3++;

		mSpeed += speed;
	}
	mSpeed /= n;
	mSpeed = 1/mSpeed;
}

// -----------------------------------------------------------------------------
//	DAnalyse::GetSpeed - returns the current speed of the vinyl
// -----------------------------------------------------------------------------

double DAnalyse::GetSpeed()
{
	return mSpeed;
}


// -----------------------------------------------------------------------------
//	DAnalyse::GetPosition - returns the current position of the vinyl
// -----------------------------------------------------------------------------

double DAnalyse::GetPosition()
{
	return mPosition;
}

// -----------------------------------------------------------------------------
//	DAnalyse::GetTimecodesPerSecond - returns detected timecodes/s
// -----------------------------------------------------------------------------

int DAnalyse::GetTimecodesPerSecond()
{
	return mTimecodesPerSecond;
}
// -----------------------------------------------------------------------------
//	DAnalyse::GetVolumePeak - returns the positive volume peak
// -----------------------------------------------------------------------------
int DAnalyse::GetVolumePeak()
{
	return mVolumePeak;
}
