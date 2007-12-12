#ifndef __VINYLCONTROL_H__
#define __VINYLCONTROL_H__

#include <qthread.h>
#include "configobject.h"
#include "controlobject.h"

class ControlObjectThreadMain;

#define MIXXX_VINYL_FINALSCRATCH "Final Scratch (crappy)"
#define MIXXX_VINYL_MIXVIBESDVSCD "MixVibes DVS CD"
#define MIXXX_VINYL_SERATOCV02VINYLSIDEA "Serato CV02 Vinyl, Side A"
#define MIXXX_VINYL_SERATOCV02VINYLSIDEB "Serato CV02 Vinyl, Side B"
#define MIXXX_VINYL_SERATOCD "Serato CD"
#define MIXXX_VINYL_TRAKTORSCRATCHSIDEA "Traktor Scratch, side A"
#define MIXXX_VINYL_TRAKTORSCRATCHSIDEB "Traktor Scratch, side B"

#define MIXXX_VCMODE_ABSOLUTE 0
#define MIXXX_VCMODE_RELATIVE 1
#define MIXXX_VCMODE_SCRATCH  2

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
    	ControlObjectThreadMain *playButton;		//The ControlObject used to start/stop playback of the song.
    	ControlObjectThreadMain *playPos;			//The ControlObject used to change the playback position in the song.
    	ControlObjectThreadMain *controlScratch;	//The ControlObject used to seek when the record is spinning fast.
    	ControlObjectThreadMain *rateSlider;		//The ControlObject used to change the speed/pitch of the song.
    	ControlObjectThreadMain *reverseButton;	//The ControlObject used to reverse playback of the song. 
    	ControlObjectThreadMain *duration;		//The ControlObject used to get the duration of the current song.    
    	ControlObjectThreadMain *mode;            //The ControlObject used to get the vinyl control mode (absolute/relative/scratch)
    	ControlObjectThreadMain *enabled;         //The ControlObject used to get if the vinyl control is enabled or disabled.
    	ControlObjectThreadMain *rateRange;         //The ControlObject used to the get the pitch range from the prefs.
        int iLeadInTime;				//The lead-in time...
    	float dVinylPitch; 			//The speed/pitch of the timecoded vinyl as read by scratchlib.
    	double dVinylPosition; 			//The position of the needle on the record as read by scratchlib.
	    double dVinylScratch;
        double dDriftControl;           //The difference between Mixxx's position and the needle's position on the record.
                                        //... these two values naturally drift apart, so we need to keep making adjustments to the pitch
                                        //to stop it from getting bad.
        float fTimecodeStrength;        //The signal strength of the timecode on the vinyl.
        float fRateRange;               //The pitch range setting from Mixxx's preferences

	    unsigned long iSampleRate;
	    bool bIsEnabled;
	    int iRIAACorrection;
    	int iVCMode;
	QWaitCondition waitForNextInput;
	QMutex         lockInput;
	QMutex		   lockSamples;
};

#endif
