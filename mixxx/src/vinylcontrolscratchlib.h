/***************************************************************************
                          vinylcontrol.h -  description
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

#ifndef VINYLCONTROLSCRATCHLIB_H
#define VINYLCONTROLSCRATCHLIB_H

#include <qthread.h>
#include "DAnalyse.h"
#include "vinylcontrol.h"

#define MIXXX_VINYLTYPE 0			//The type of timecoded vinyl used. (FinalScratch currently)
#define MIXXX_VINYL_LEADIN 30 		//30 seconds of lead-in time on the timecoded vinyl
#define MIXXX_CALIBRATION_VALUE 1.30 //The calibration value for scratchlib.

class VinylControlScratchlib : public VinylControl
{

public:
    //VinylControlScratchlib();
	VinylControlScratchlib(ConfigObject<ConfigValue> *pConfig, const char *_group);
    virtual ~VinylControlScratchlib();
	void ToggleVinylControl(bool enable);
	bool isEnabled();     
	void AnalyseSamples(short* samples, size_t size);

protected:
	void run();						// main thread loop

private:
	void syncPitch(double pitch);
	void syncPosition();

	DAnalyse *analyzer;				//This is our main interface to scratchlib.	


	double dFileLength; 			//The length (in samples) of the current song.
	double dOldPos;   				//The position read last time it was polled.
	double dOldDiff;  				//The old difference between the positions. (used to check if the needle's on the record...)
	double dOldPitch;
	double dDriftControl;           //The difference between Mixxx's position and the needle's position on the record.
	                                //... these two values naturally drift apart, so we need to keep making adjustments to the pitch
	                                //to stop it from getting bad.
	bool bNeedleDown; 				//Is the needle on the record? (used for needle dropping)
	bool bSeeking; 					//Are we seeking through the record? (ie. is it moving really fast?)

	short*  m_samples;
	size_t  m_SamplesSize;

	bool		   bShouldClose;
	bool		   bIsRunning;
};

#endif
