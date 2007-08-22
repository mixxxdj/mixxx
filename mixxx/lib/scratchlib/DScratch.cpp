/////////////////////////////////////////////////////////////////////////////
// Name:        DScratch.cpp
// Purpose:
// Author:      Stefan Langhammer, Thomas Rogg
// Modified by:
// Created:     4.12.2005
// Modified:    15.08.2006
// Copyright:   (c) Stefan Langhammer, Thomas Rogg
// Licence:     LGPL
/////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
//	DScratch.cpp - Final Scratch vinyl analyser
// -----------------------------------------------------------------------------


#include "DScratch.h"
#include "DAnalyse.h"


#include <set>
#include <string>


short DScratch::mNumChannels = -1;
short DScratch::m_fAmplifyFactor = 1.0;
bool  DScratch::m_bEnableRIAACorrection = false;

// -----------------------------------------------------------------------------
//	DScratch::DScratch - constructs the DScratch object
// -----------------------------------------------------------------------------

DScratch::DScratch()
{
	
	// Initialize variables
	mActiveDevice		= -1;

	// Get all devices capable of being used by Digital Scratch
#ifdef __DSCRATCH_USE_RTAUDIO__
	mAudio = NULL;
#ifdef __WINDOWS_ASIO__
	ProbeDrivers(RtAudio::WINDOWS_ASIO);
#endif
#ifdef __MACOSX_CORE__
	ProbeDrivers(RtAudio::MACOSX_CORE);
#endif
#ifdef __WINDOWS_DS__
	ProbeDrivers(RtAudio::WINDOWS_DS);
#endif
#ifdef __LINUX_ALSA__
	ProbeDrivers(RtAudio::LINUX_ALSA);
#endif
#ifdef __LINUX_JACK__
	ProbeDrivers(RtAudio::LINUX_JACK);
#endif

#endif /* __DSCRATCH_USE_RTAUDIO__ */
}

// -----------------------------------------------------------------------------
//	DScratch::~DScratch - destructs the DScratch object
// -----------------------------------------------------------------------------

DScratch::~DScratch()
{
#ifdef __DSCRATCH_USE_RTAUDIO__
	if (mAudio)
	{
		CloseRtAudio();
		delete mAudio;
	}
#endif /* __DSCRATCH_USE_RTAUDIO__ */
}

// -----------------------------------------------------------------------------
//	DScratch::SetActiveDevice - sets the device to use
// -----------------------------------------------------------------------------

bool DScratch::SetActiveDevice(short index, short ActiveChannels)
{
#ifdef __DSCRATCH_USE_RTAUDIO__
	mNumChannels = ActiveChannels;
	if(index >= 0 && index < (int)mDeviceNames.size())
	{
		this->InitRtAudio (m_DeviceInfo.at(index).api);
		return OpenRtAudio(m_DeviceInfo.at(index).deviceID, ActiveChannels);
	}
	else
		return false;
#else
	return false;
#endif /* __DSCRATCH_USE_RTAUDIO__ */
}


// -----------------------------------------------------------------------------
//	DScratch::GetActiveDevice - returns the used device, -1 if no device
// -----------------------------------------------------------------------------

int DScratch::GetActiveDevice()
{
	return mActiveDevice;
}


// -----------------------------------------------------------------------------
//	DScratch::SetTimecodeVinyl - sets the timecode vinyl type
// -----------------------------------------------------------------------------
void DScratch::SetTimecodeVinyl(TimecodeVinyl vinylType)
{
	mAnalyser.SetVinyl(int(vinylType));
}
// -----------------------------------------------------------------------------
//	DScratch::SetUseData - initialises to give data yourself, do not use device
// -----------------------------------------------------------------------------
void DScratch::SetUseData(int frequency)
{
#ifdef __DSCRATCH_USE_RTAUDIO__
	CloseRtAudio();
#endif /* __DSCRATCH_USE_RTAUDIO__ */

	mAnalyser.SetFrequency(frequency);
}

// -----------------------------------------------------------------------------
//	DScratch::SetCalibration - Set the peak tolerance range and reset timecodes/s
// -----------------------------------------------------------------------------
void DScratch::SetCalibration(float calibrationValue)
{
	mAnalyser.SetCalibration(calibrationValue);
}

// -----------------------------------------------------------------------------
//	DScratch::SetAmplify - Set Amplifier multiplier
// -----------------------------------------------------------------------------
void	DScratch::SetAmplify(float factor)
{
	m_fAmplifyFactor = factor;
}

// -----------------------------------------------------------------------------
//	DScratch::EnableRIAACorrection - enables the Reverse RIAA algorithm on the 
//									 input signal. Needed to connect record
//									 players without a preamp.							
// -----------------------------------------------------------------------------
void	DScratch::EnableRIAACorrection(bool enable)
{
	m_bEnableRIAACorrection = enable;
}

// -----------------------------------------------------------------------------
//	DScratch::AnalyseData - analyses the given data
// -----------------------------------------------------------------------------

void	DScratch::AnalyseData(short *data, int numData)
{
	mAnalyser.Analyse(data, numData);
}


// -----------------------------------------------------------------------------
//	DScratch::GetSpeed - returns the current speed of the vinyl
// -----------------------------------------------------------------------------

double	DScratch::GetSpeed()
{
	return mAnalyser.GetSpeed();
}


// -----------------------------------------------------------------------------
//	DScratch::GetPosition - returns the current position of the vinyl
// -----------------------------------------------------------------------------

double	DScratch::GetPosition()
{
	return mAnalyser.GetPosition();
}

// -----------------------------------------------------------------------------
//	DScratch::GetTimecodesPerSecond - returns detected timecodes/s
// -----------------------------------------------------------------------------
int		DScratch::GetTimecodesPerSecond()
{
	return mAnalyser.GetTimecodesPerSecond();
}

// -----------------------------------------------------------------------------
//	DScratch::GetTimecodesPerSecond - returns the current input volume peak
// -----------------------------------------------------------------------------
int		DScratch::GetVolumePeak()
{
	return mAnalyser.GetVolumePeak();
}
// -----------------------------------------------------------------------------
//	DScratch::GetDeviceName - returns the name of the given device
// -----------------------------------------------------------------------------

bool	DScratch::GetDeviceName(int index, string &name)
{
#ifdef __DSCRATCH_USE_RTAUDIO__
	if (index < 0 || index >= int(mDeviceNames.size()))
		return false;
	name = mDeviceNames.at(index);
	return true;
#else
	return false;
#endif /* __DSCRATCH_USE_RTAUDIO__ */
}


// -----------------------------------------------------------------------------
//	DScratch::GetDeviceIndex - returns the index of the given device
// -----------------------------------------------------------------------------

int		DScratch::GetDeviceIndex(const char *name)
{
#ifdef __DSCRATCH_USE_RTAUDIO__
	size_t i;

	for(i = 0; i < mDeviceNames.size(); i++)
	{
		if(name == mDeviceNames.at(i))
			return (int)i;
	}
#endif /* __DSCRATCH_USE_RTAUDIO__ */

	return -1;
}


// -----------------------------------------------------------------------------
//	DScratch::GetNumDevices - returns the number of devices found
// -----------------------------------------------------------------------------

int		DScratch::GetNumDevices()
{
#ifdef __DSCRATCH_USE_RTAUDIO__
	return (int)mDeviceNames.size();
#else
	return 0;
#endif /* __DSCRATCH_USE_RTAUDIO__ */
}

// -----------------------------------------------------------------------------
//	DScratch::GetNumChannels - returns the number of channels
// -----------------------------------------------------------------------------
int		DScratch::GetNumChannels(int device)
{
	#ifdef __DSCRATCH_USE_RTAUDIO__
		return m_DeviceInfo.at(device).channels;
	#else
		return -1;
	#endif
}

#ifdef __DSCRATCH_USE_RTAUDIO__

// -----------------------------------------------------------------------------
//	DScratch::ProbeDrivers - probes the drivers
// -----------------------------------------------------------------------------
void	DScratch::ProbeDrivers(RtAudio::RtAudioApi api)
{
	// open audio device
	RtAudio rt(api);
	std::set<std::string> names;
	RtAudioDeviceInfo info;
	int numDevices, i;
	string name;

	numDevices = rt.getDeviceCount();

	// enumerate devices
	for(i = 1; i <= numDevices; i++)
	{
		try
		{
			info = rt.getDeviceInfo(i);
		}
		catch(RtError &)
		{
			continue;
		}

		if(info.probed && info.inputChannels >= 2)
		{
			if(names.find(info.name) == names.end())
			{
				// and store INPUT device informations

				char temp[400];
				sprintf( temp,     "%-40s %s\n  %i input channels | %10s | highest freq: %i hz",	info.name.c_str(),
					GetRtAudioApiString(api).c_str(), info.inputChannels, GetAudioFormatAsString(&info.nativeFormats).c_str(),
					info.sampleRates.at(info.sampleRates.size()-1));
				DeviceInfo devInfo;
				devInfo.api = api;
				devInfo.deviceID = i;
				devInfo.channels = info.inputChannels;
				m_DeviceInfo.push_back(devInfo);
				name = temp;
				mDeviceNames.push_back(name);
				names.insert(info.name);
			}
		}
	}
}


// -----------------------------------------------------------------------------
//	DScratch::InitRtAudio - set the driver type ASIO, DSOUND,...
// -----------------------------------------------------------------------------

void	DScratch::InitRtAudio(RtAudio::RtAudioApi driver)
{
#ifdef __DSCRATCH_USE_RTAUDIO__
	if (mAudio)
		delete mAudio;
	
	mAudio = new RtAudio(driver);
#endif /* __DSCRATCH_USE_RTAUDIO__ */
}

// -----------------------------------------------------------------------------
//	DScratch::OpenRtAudio - opens the RtAudio stream
// -----------------------------------------------------------------------------

bool	DScratch::OpenRtAudio(short device, short ActiveChannels)
{
	int bufferSize;

	// Close audio if open
	CloseRtAudio();

	// Open audio
	bufferSize = DSCRATCH_RTAUDIO_BUFFER_SIZE;
	RtAudioDeviceInfo info = mAudio->getDeviceInfo(device);
	
	// Use the highest sample frequency
	int freq = info.sampleRates.at(info.sampleRates.size()-1);
	freq = 44100; //Hacked by Albert
	mAnalyser.SetFrequency(freq);

	try {
		// SampleFormat note
		// we need RTAUDIO_SINT16 as output format for the analysing part.
		mAudio->openStream(0, 0, device, ActiveChannels,
		RTAUDIO_SINT16, freq, &bufferSize,
		DSCRATCH_RTAUDIO_NUM_BUFFERS);
	}
	catch (RtError &error) {
		error.printMessage();
		CloseRtAudio();
		return false;
	}
	mActiveDevice = device;

	try {
		mAudio->setStreamCallback(RtAudioCallback, &mAnalyser);
	}
	catch (RtError &error) {
		error.printMessage();
		CloseRtAudio();
		return false;
	}
	try {
		mAudio->startStream();
	}
	catch (RtError &error) {
		error.printMessage();
		CloseRtAudio();
		return false;
	}
	return true;
}


// -----------------------------------------------------------------------------
//	DScratch::CloseRtAudio - closes the RtAudio stream
// -----------------------------------------------------------------------------

void	DScratch::CloseRtAudio()
{
	if(mActiveDevice == -1)
		return;

	try
	{
		mAudio->stopStream();
		mAudio->closeStream();
	}
	catch(RtError &)
	{
	}

	mActiveDevice	= -1;
}


// -----------------------------------------------------------------------------
//	DScratch::RtAudioCallback - RtAudio callback
// -----------------------------------------------------------------------------

int		DScratch::RtAudioCallback(char *buffer, int bufferSize, void *data)
{
	// Because RtAudio does not provide a mechanism for allowing the user to specify particular channels (or ports) 
	// of a device, it simply opens the first N enumerated ports for input (version 3.03)
	// RT-Audio delivers the samples in "interleaved format"
	// |Channel1-l|Channel1-r|Channel2-l|Channel2-r|Channel1-l|Channel1-r|...
	if (mNumChannels > 2)
	{
		short*	ptr1 = (short*)buffer;
		short*	ptr2 = (short*)buffer;
		int i;
		int size = bufferSize * mNumChannels;
		for(i = 0; i < size; i++)
		{
			// skip the samples up to the input channels
			if ( i%mNumChannels == 0)
			{
				for(int j =0; j<mNumChannels-2;j++)
					ptr1++;
				for(int j =0; j<(mNumChannels-2)/2;j++)
					i++;
				continue;
			}
			*ptr2 = *ptr1;
			ptr1++;
			ptr2++;
		}
	}


	// apply Inverse RIAA filter
	if (m_bEnableRIAACorrection)
	{
		inv_riaa_filter	((short*)buffer, bufferSize * mNumChannels);
	}


	// Amplify input signal
	short*	ptr1 = (short*)buffer;
	int size = bufferSize * mNumChannels;
	for(int i = 0; i < size; i++)
	{
		if (*ptr1 < 0)
			*ptr1 *= m_fAmplifyFactor;
		else if (*ptr1 > 0)
			*ptr1 *= m_fAmplifyFactor;
		ptr1++;
	}

	// Analyse it
	((DAnalyse *)data)->Analyse((short *)buffer, bufferSize*2);
	return 0;
}
// -----------------------------------------------------------------------------
//	DScratch::GetRtAudioApiString - return the api name
// -----------------------------------------------------------------------------
string	DScratch::GetRtAudioApiString(RtAudio::RtAudioApi api)
{
	if (api == RtAudio::IRIX_AL)
		return "IRIX_AL";
	if (api == RtAudio::LINUX_ALSA)
		return "LINUX_ALSA";
	if (api == RtAudio::LINUX_JACK)
		return "LINUX_JACK";
	if (api == RtAudio::LINUX_OSS)
		return "LINUX_OSS";
	if (api == RtAudio::MACOSX_CORE)
		return "MACOSX_CORE";
	if (api == RtAudio::WINDOWS_ASIO)
		return "WINDOWS_ASIO";
	if (api == RtAudio::WINDOWS_DS)
		return "WINDOWS_DS";
	return "UNSPECIFIED";
}
// -----------------------------------------------------------------------------
//	DScratch::GetAudioFormatAsString - return the api name as string
// -----------------------------------------------------------------------------
string	DScratch::GetAudioFormatAsString(RtAudioFormat* format)
{
	string strSampleFormat = "Unknown";
		if ( *format & RTAUDIO_FLOAT64 )
			strSampleFormat = "RTAUDIO_FLOAT64";
		else if ( *format & RTAUDIO_FLOAT32 )
			strSampleFormat = "RTAUDIO_FLOAT32";
		else if ( *format & RTAUDIO_SINT32 )
			strSampleFormat = "RTAUDIO_SINT32";
		else if ( *format & RTAUDIO_SINT24 )
			strSampleFormat = "RTAUDIO_SINT24";
		else if ( *format & RTAUDIO_SINT16 )
			strSampleFormat = "RTAUDIO_SINT16";
		else if ( *format & RTAUDIO_SINT8 )
			strSampleFormat = "RTAUDIO_SINT8";
		return strSampleFormat;
}
#endif /* __DSCRATCH_USE_RTAUDIO__ */
