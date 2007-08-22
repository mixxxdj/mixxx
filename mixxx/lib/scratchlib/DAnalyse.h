// -----------------------------------------------------------------------------
//	DAnalyse.h - Final Scratch vinyl analyser - analyser class
// -----------------------------------------------------------------------------

#ifndef __DANALYSE_H__
#define __DANALYSE_H__

#include <list>

const int DSCRATCH_PEAK_CHANNEL_LEFT		= 0;
const int DSCRATCH_PEAK_CHANNEL_RIGHT		= 1;

const int DSCRATCH_PEAK_TYPE_LOW			= 0;
const int DSCRATCH_PEAK_TYPE_HIGH			= 1;
const int DSCRATCH_PEAK_TYPE_ZERO			= 2;

const int DSCRATCH_AMPLITUDE_STAYING_SAME	= 0;
const int DSCRATCH_AMPLITUDE_GOING_UP		= 1;
const int DSCRATCH_AMPLITUDE_GOING_DOWN		= -1;

const int DSCRATCH_BIT_UNDEFINED			= -1;
const int DSCRATCH_BIT_LOW					= 0;
const int DSCRATCH_BIT_HIGH					= 1;

const int DSCRATCH_NUM_PEAKS_TILL_ZERO		= 10;

const int DSCRATCH_VINYL_FINALSCRATCH		= 0;
const int DSCRATCH_VINYL_MIXVIBES			= 1;

struct DPeak
{
	int type;
	short amplitude;

	short channel;	// 0 left, 1 right
	float time;		// peak recording timestamp

	int bit;		// LOW
};

class DAnalyse
{
public:
	DAnalyse();
	~DAnalyse();

	/////////////////////////////////////
	// Methods
	/////////////////////////////////////
public:
	void	SetVinyl				(int vinyl);
	void	SetInputChannels		(short NumChannels);
	void	SetFrequency			(int frequency);
	void	SetThreshold			(int percent);
	void	SetCalibration			(float calibrationValue);
	bool	Analyse					(short *samples, int numSamples);
	double	GetSpeed				();
	double	GetPosition				();
	int		GetTimecodesPerSecond	();
	int		GetVolumePeak			();

private:
	void AddPeak		(DPeak &peak);
	void CalculateSpeed	();
	void DetectTimecode	();

	/////////////////////////////////////
	// Variables
	/////////////////////////////////////
private:
	int				mVinyl;				// DSCRATCH_VINYL_FINALSCRATCH = 0; DSCRATCH_VINYL_MIXVIBES = 1;
	float			m_fCalibrationValue;// tolerance range in which a HIGH/LOW will be detected as a HIGH or a LOW
	int 			mFrequency;			// Sample frequency in hz
	int 			m_iThreshold;		// amplitude threshold ( see SetThreshold method )
	short			mNumChannels;		// Number of channels we receive in Analyse(short*samples, int numSamples);

	unsigned short	mNumTimecodePeaks;	// Necessary peaks for the timecode detection

	float			mTime;				// continuous recording time
	int				mTimeFullSeconds;	// rounded mTime to check the Timecodes/seconds

	float			mSpeed;				// Playback speed 1.0 = +/- 0%
	float			mPosition;			// Timecode position in seconds

	int				mVolumePeak;		// peak of the current input buffer

	DPeak			lastLowestPeak[2];	// last Lowest  DPeak on both channels
	DPeak			lastHighestPeak[2]; // last Highest DPeak on both channels
	int				mAmplitudeDir[2];	// waveform direction of both channels

	std::list<DPeak>mPeaks;				// list of identified peaks				
	int				mNumBitIdedPeaks;	// identified "correct peaks" counter
	bool			mbNewTimecodeDetected; 

protected:
	
	int mTimecodesPerSecond, mTimecodesPerSecondTemp;
};

#endif /* __DANALYSE_H__ */
