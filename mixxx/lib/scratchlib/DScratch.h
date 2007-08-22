// -----------------------------------------------------------------------------
//	DScratch.h - Final Scratch vinyl analyser
// -----------------------------------------------------------------------------
#ifndef __DSCRATCH_H__
#define __DSCRATCH_H__

#include "DAnalyse.h"
#include "InverseRIAAFilter.h"

#include <vector>
using namespace std;

#ifdef __DSCRATCH_USE_RTAUDIO__

#include <RtAudio.h>

const int DSCRATCH_RTAUDIO_BUFFER_SIZE		= 1024;
const int DSCRATCH_RTAUDIO_NUM_BUFFERS		= 8;

// RtAudio device informations structure
struct DeviceInfo
{
	RtAudio::RtAudioApi api;
	int					deviceID;
	int					channels;
};
class RtAudio;
#endif /* __DSCRATCH_USE_RTAUDIO__ */


// Supported vinyl types
enum TimecodeVinyl
{
	TIMECODE_VINYL_FINALSCRATCH,
	TIMECODE_VINYL_MIXVIBES
};

///////////////////////////////////
// Audio analysing class
class DScratch : public InverseRIAAFilter
{
public:
	DScratch();
	~DScratch();

	///////////////////////////////////
	// methods
	///////////////////////////////////
public:
	// Set and get the used device
	bool	SetActiveDevice(short index, short ActiveChannels);
	int		GetActiveDevice();

	// Set the timecode vinyl
	void	SetTimecodeVinyl(TimecodeVinyl vinylType);

	// Set Amplifier factor
	void	SetAmplify(float factor);
	
	// Enable RIAA filter correction
	void	EnableRIAACorrection(bool enable);

	// Give data yourself, do not use default device by rtAudio
	void	SetUseData(int frequency);
	void	AnalyseData(short *data, int numData);

	// Get speed, position, timecodes/s and volume
	double	GetSpeed();
	double	GetPosition();
	int		GetTimecodesPerSecond();
	int		GetVolumePeak();
	void	SetCalibration(float calibrationValue);

	// Get information about devices
	bool	GetDeviceName	(int index, std::string &name);
	int		GetDeviceIndex	(const char *name);
	int		GetNumDevices	();
	int		GetNumChannels	(int device);
	

#ifdef __DSCRATCH_USE_RTAUDIO__
private:
	void	ProbeDrivers		(RtAudio::RtAudioApi api);
	string	GetRtAudioApiString	(RtAudio::RtAudioApi api);
	string	GetAudioFormatAsString(RtAudioFormat* format);

	void	InitRtAudio			(RtAudio::RtAudioApi driver);
	bool	OpenRtAudio			(short device, short ActiveChannels);
	void	CloseRtAudio		();

	static int	RtAudioCallback(char *buffer, int bufferSize, void *data);

	RtAudio*				mAudio;
	// All device informations
	std::vector<DeviceInfo> m_DeviceInfo;
	//Number of channels we receive in Analyse(short*samples, int numSamples);
	static short			mNumChannels;
#endif /* __DSCRATCH_USE_RTAUDIO__ */

	///////////////////////////////////
	// objects & variables
	///////////////////////////////////

public:
	DAnalyse			mAnalyser;
	std::vector<string> mDeviceNames;
	short				mActiveDevice;
private:
	// input signal amplifyer factor
	static short		m_fAmplifyFactor;
	static bool			m_bEnableRIAACorrection;
};

#endif /* __DSCRATCH_H__ */
