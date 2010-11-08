#ifndef __VINYLCONTROLXWAX_H__
#define __VINYLCONTROLXWAX_H__

#include "vinylcontrol.h"
#include <time.h>

#ifdef _MSC_VER
#include "timecoder.h"
#else
extern "C" {
#include "timecoder.h"
}
#endif

#define XWAX_DEVICE_FRAME 32
#define XWAX_SMOOTHING (128 / XWAX_DEVICE_FRAME) /* result value is in frames */
#define RING_SIZE 20
#define MIN_SIGNAL 75


class VinylControlXwax : public VinylControl
{
    public:
        VinylControlXwax(ConfigObject<ConfigValue> *pConfig, const char *_group);
        virtual ~VinylControlXwax();
    	void ToggleVinylControl(bool enable);
    	bool isEnabled();
    	void AnalyseSamples(short* samples, size_t size);  
        static void freeLUTs();
protected:
	void run();						// main thread loop

private:
	void syncPosition();
	void togglePlayButton(bool on);
	bool checkEnabled(bool was, bool is);
	void resetSteadyPitch(double pitch, double time);
	double checkSteadyPitch(double pitch, double time);
	void enableRecordEndMode();
	void disableRecordEndMode();
	void enableConstantMode();
	bool uiUpdateTime(double time);

	double dFileLength; 			//The length (in samples) of the current song.

	double dOldPos;   				//The position read last time it was polled.
	double dOldDiff;  				//The old difference between the positions. (used to check if the needle's on the record...)
	double dOldPitch;

	bool bNeedleDown; 				//Is the needle on the record? (used for needle dropping)
	bool bSeeking; 					//Are we seeking through the record? (ie. is it moving really fast?)
	bool bHaveSignal;					//Any signal at all?
	bool bAtRecordEnd;
	bool bForceResync;
	int iNewMode;
	double dOldFilePos;
	double dSteadyPitch;
	double dSteadyPitchTime;
	double dUiUpdateTime;

    //Contains information that xwax's code needs internally about the timecode and how to process it.
    struct timecoder_t timecoder;
    static QMutex m_xwaxLUTMutex; /** Static mutex that protects our creation/destruction of the xwax LUTs */
    static bool m_bLUTInitialized;

	short*  m_samples;
	size_t  m_SamplesSize;

	bool		   bShouldClose;
	bool		   bIsRunning;
    bool           m_bNeedleSkipPrevention;      /**< needle skip prevention is now optional (still CD mode force this to be false) */
};        

#endif
