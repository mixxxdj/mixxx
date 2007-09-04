#ifndef __VINYLCONTROL_H__
#define __VINYLCONTROL_H__

#include <qthread.h>
#include "configobject.h"
#include "controlobject.h"

#define MIXXX_VINYL_FINALSCRATCH "Final Scratch (crappy)"
#define MIXXX_VINYL_MIXVIBESDVSCD "MixVibes DVS CD"
#define MIXXX_VINYL_SERATOCV02VINYLSIDEA "Serato CV02 Vinyl, Side A"
#define MIXXX_VINYL_SERATOCV02VINYLSIDEB "Serato CV02 Vinyl, Side B"
#define MIXXX_VINYL_SERATOCD "Serato CD"
#define MIXXX_VINYL_TRAKTORSCRATCHSIDEA "Traktor Scratch, side A"
#define MIXXX_VINYL_TRAKTORSCRATCHSIDEB "Traktor Scratch, side B"


//TODO: Make this an EngineObject instead one day? (need to route all the input audio through the engine that way too...)

class VinylControl : public QThread
{
    public:
        VinylControl(ConfigObject<ConfigValue> *pConfig, const char *_group);
        ~VinylControl();
    	virtual void ToggleVinylControl(bool enable) = 0;
    	virtual bool isEnabled() = 0;
    	/*virtual void syncPitch(double pitch) = 0;
    	virtual void syncPosition() = 0; */
    	virtual void AnalyseSamples(short* samples, size_t size) = 0;  
    	virtual float getSpeed();
    	virtual float getTimecodeStrength();
    protected:
	    virtual void run() = 0;						// main thread loop
	    
	    QString strVinylType;	 
        ConfigObject<ConfigValue> *m_pConfig;	/** Pointer to config database */
        const char* group;
    	ControlObject *playButton;		//The ControlObject used to start/stop playback of the song.
    	ControlObject *playPos;			//The ControlObject used to change the playback position in the song.
    	ControlObject *controlScratch;	//The ControlObject used to seek when the record is spinning fast.
    	ControlObject *rateSlider;		//The ControlObject used to change the speed/pitch of the song.
    	ControlObject *reverseButton;	//The ControlObject used to reverse playback of the song. 
    	ControlObject *duration;		//The ControlObject used to get the duration of the current song.    
        int iLeadInTime;				//The lead-in time...
    	float dVinylPitch; 			//The speed/pitch of the timecoded vinyl as read by scratchlib.
    	double dVinylPosition; 			//The position of the needle on the record as read by scratchlib.
	    double dVinylScratch;
        double dDriftControl;           //The difference between Mixxx's position and the needle's position on the record.
                                        //... these two values naturally drift apart, so we need to keep making adjustments to the pitch
                                        //to stop it from getting bad.
        float fTimecodeStrength;        //The signal strength of the timecode on the vinyl.

	    int iSampleRate;
	    bool bIsRunning;
	    int iRIAACorrection;
    	bool bRelativeMode;
    	bool bScratchMode;
	QWaitCondition waitForNextInput;
	QMutex         lockInput;
	QMutex		   lockSamples;
};

#endif
