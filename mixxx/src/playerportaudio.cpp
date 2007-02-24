/***************************************************************************
                          playerportaudiov19.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    					   (C) 2006 by Albert Santoni
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

#include "playerportaudio.h"
#include "controlobject.h"

PlayerPortAudio::PlayerPortAudio(ConfigObject<ConfigValue> *config, QString api_name) : Player(config)
{
    m_devId = -1;
    m_iNumberOfBuffers = 2;
    m_iChannels = -1;
    m_iMasterLeftCh = -1;
    m_iMasterRigthCh = -1;
    m_iHeadLeftCh = -1;
    m_iHeadRightCh = -1;
    m_pStream = 0;
    m_HostAPI = api_name;
    m_bInit = false;
}

PlayerPortAudio::~PlayerPortAudio()
{
    if (m_devId>=0)
        close();
    if (m_bInit)
        Pa_Terminate();
}

bool PlayerPortAudio::initialize()
{
    PaError err = Pa_Initialize();
    if (err!=paNoError)
    {
        qDebug("PortAudio error: %s", Pa_GetErrorText(err));
        m_bInit = false;
    }
    else
        m_bInit = true;

    return m_bInit;
}

bool PlayerPortAudio::open()
{
    Player::open();

    // Find out which device to open. Select the first one listed as either Master Left,
    // Master Right, Head Left, Head Right. If other devices are requested for opening
    // than the one selected here, set them to "None" in the config database
    PaDeviceIndex id = -1;
    PaDeviceIndex temp = -1;
    QString name;
    /** Maximum number of channels needed */
    int iChannelMax = -1;

    m_iMasterLeftCh = -1;
    m_iMasterRigthCh = -1;
    m_iHeadLeftCh = -1;
    m_iHeadRightCh = -1;

    // Master left
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterLeft"));
    temp = getDeviceID(name);
    if (temp>=0)
    {
        if (getChannelNo(name)>=0)
        {
            id = temp;
            iChannelMax = getChannelNo(name);
            m_iMasterLeftCh = getChannelNo(name)-1;
        }
    }

    // Master right
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterRight"));
    temp = getDeviceID(name);
    if (getChannelNo(name)>=0 && ((id==-1 && temp>=0) || (temp!=-1 && id==temp)))
    {
        id = temp;
        iChannelMax = math_max(iChannelMax, getChannelNo(name));
        m_iMasterRigthCh = getChannelNo(name)-1;
    }

    // Head left
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadLeft"));
    temp = getDeviceID(name);
    if (getChannelNo(name)>=0 && ((id==-1 && temp>=0) || (temp!=-1 && id==temp)))
    {
        id = temp;
        iChannelMax = math_max(iChannelMax, getChannelNo(name));
        m_iHeadLeftCh = getChannelNo(name)-1;
    }

    // Head right
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadRight"));
    temp = getDeviceID(name);
    if (getChannelNo(name)>=0 && ((id==-1 && temp>=0) || (temp!=-1 && id==temp)))
    {
        id = temp;
        iChannelMax = math_max(iChannelMax, getChannelNo(name));
        m_iHeadRightCh = getChannelNo(name)-1;
    }

    // Check if any of the devices in the config database needs to be set to "None"
    if (m_iMasterLeftCh<0)
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue("None"));
    if (m_iMasterRigthCh<0)
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue("None"));
    if (m_iHeadLeftCh<0)
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadLeft"),ConfigValue("None"));
    if (m_iHeadRightCh<0)
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadRight"),ConfigValue("None"));

    // Number of channels to open
    int iChannels = iChannelMax;

    // Sample rate
    int iSrate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt();
	if (iSrate <= 0)
		iSrate = 44100;

    // Get latency in msec
    int iLatencyMSec = m_pConfig->getValueString(ConfigKey("[Soundcard]","Latency")).toInt();

	if (iLatencyMSec <= 0) //Make sure we don't get a crazy latency value.
		iLatencyMSec = 150;

    // Latency in samples
    int iLatencySamples = (int)((float)(iSrate*iChannels)/1000.f*(float)iLatencyMSec);

    // Apply simple rule to determine number of buffers
    if (iLatencySamples/kiMaxFrameSize<2)
        m_iNumberOfBuffers = 2;
    else
        m_iNumberOfBuffers = iLatencySamples/kiMaxFrameSize;

    // Frame size...    
    int iFramesPerBuffer = iLatencySamples/m_iNumberOfBuffers;

    // Ensure the chosen configuration is valid
    //TODO: Fix this in our PortAudio-v19 implementation. PA19 ditches the Pa_GetMinNumBuffers() function, as it's
    //		not needed anymore. 
    /*
    if (m_iNumberOfBuffers<Pa_GetMinNumBuffers(iFramesPerBuffer,iSrate))
    {
        m_iNumberOfBuffers = Pa_GetMinNumBuffers(iFramesPerBuffer,iSrate);
    
        iLatencyMSec = (1000*iFramesPerBuffer*m_iNumberOfBuffers)/(iSrate*iChannels);
        m_pConfig->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(iLatencyMSec));
    }
    */

    // Callback function to use
    PaStreamCallback *callback = paV19Callback;
	qDebug("PortAudio: kiMaxFrameSize: %i, iLatencyMSec: %i", kiMaxFrameSize, iLatencyMSec);
    qDebug("PortAudio: id %i, sr %i, ch %i, bufsize %i, bufno %i, req. latency %i msec", id, iSrate, iChannels, iFramesPerBuffer, m_iNumberOfBuffers, iLatencyMSec);

    if (id<0)
        return false;

	PaStreamParameters outputParams;
	outputParams.device = id;
	outputParams.channelCount = iChannels;
	outputParams.sampleFormat = paFloat32;
	outputParams.suggestedLatency = ((float)iLatencyMSec) / 1000.0f; //Latency in seconds.
	outputParams.hostApiSpecificStreamInfo = NULL;

    PaError err = paNoError;
       
	// Try open device using iChannelMax
    err = Pa_OpenStream(&m_pStream,
    					NULL,				// Input parameters
    					&outputParams, 		// Output parameters
    					iSrate,				// Sample rate
    					iFramesPerBuffer,	// Frames per buffer
    					paClipOff,			// Stream flags
    					callback,			// Stream callback
    					this);              // Pointer passed to the callback function


    // Try open device using iChannelMax
    /*err = Pa_OpenStream(&m_pStream, paNoDevice, 0, paFloat32, 0,
                        id,                 // Id of output device
                        iChannels,          // Number of output channels
                        paFloat32,          // Output sample format
                        0,                  // Extra info. Not used.
                        iSrate,             // Sample rate
                        iFramesPerBuffer,   // Frames per buffer
                        m_iNumberOfBuffers,   // Number of buffers
                        paClipOff,          // we won't output out of range samples so don't bother clipping them
                        callback,           // Callback function
                        this);              // Pointer passed to the callback function
    */
    if (err == paNoError)
        m_iChannels = iChannels;
    else
    {
        // Try open device using maximum supported channels by soundcard
        iChannels = Pa_GetDeviceInfo(id)->maxOutputChannels;
        outputParams.channelCount = iChannels;
	    err = Pa_OpenStream(&m_pStream,
	    					NULL,				// Input parameters
	    					&outputParams, 		// Output parameters
	    					iSrate,				// Sample rate
	    					iFramesPerBuffer,	// Frames per buffer
	    					paClipOff,			// Stream flags
	    					callback,			// Stream callback
	    					this);              // Pointer passed to the callback function
        if (err==paNoError)
            m_iChannels = iChannels;
    }

    if( err != paNoError )
    {
        // Try open device using only two channels
        outputParams.channelCount = 2;
	    err = Pa_OpenStream(&m_pStream,
	    					NULL,				// Input parameters
	    					&outputParams, 		// Output parameters
	    					iSrate,				// Sample rate
	    					iFramesPerBuffer,	// Frames per buffer
	    					paClipOff,			// Stream flags
	    					callback,			// Stream callback
	    					this);              // Pointer passed to the callback function
        if (err==paNoError)
        {
            m_iChannels = 2;

            // Update channel variables and config database
            if (m_iMasterLeftCh>1)
            {
                m_iMasterLeftCh = -1;
                m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue("None"));
            }
            if (m_iMasterRigthCh>1)
            {
                m_iMasterRigthCh = -1;
                m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue("None"));
            }
            if (m_iHeadLeftCh>1)
            {
                m_iHeadLeftCh = -1;
                m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadLeft"),ConfigValue("None"));
            }
            if (m_iHeadRightCh>1)
            {
                m_iHeadRightCh = -1;
                m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadRight"),ConfigValue("None"));
            }

        }
    }

    if( err != paNoError )
    {
        qDebug("PortAudio: Open stream error: %s", Pa_GetErrorText(err));
        //err = Pa_GetLastHostErrorInfo();
		qDebug("PortAudio: More error info: %s", Pa_GetLastHostErrorInfo()->errorText);

        m_devId = -1;
        m_iChannels = -1;

        return false;
    }

    m_devId = id;

    // Update SRATE and Latency ControlObjects
    m_pControlObjectSampleRate->queueFromThread((double)iSrate);
    m_pControlObjectLatency->queueFromThread((double)iLatencyMSec);

    // Start stream
    err = Pa_StartStream(m_pStream);
    if (err != paNoError)
        qDebug("PortAudio: Start stream error: %s", Pa_GetErrorText(err));

    return true;
}

void PlayerPortAudio::close()
{
    m_devId = -1;
    m_iChannels = 0;
    m_iMasterLeftCh = -1;
    m_iMasterRigthCh = -1;
    m_iHeadLeftCh = -1;
    m_iHeadRightCh = -1;

    // Stop stream
    if (m_pStream)
    {
        PaError err = Pa_StopStream(m_pStream);
        if( err != paNoError )
            qDebug("PortAudio: Stop stream error: %s,", Pa_GetErrorText(err));
    }

    // Close stream
    PaError err = Pa_CloseStream(m_pStream);
    if( err != paNoError )
        qDebug("PortAudio: Close stream error: %s", Pa_GetErrorText(err));
    m_pStream = 0;
}

void PlayerPortAudio::setDefaults()
{
    // Get list of interfaces
    QStringList interfaces = getInterfaces();

    // Set first interfaces to master left
    QStringList::iterator it = interfaces.begin();
    if (it!=interfaces.end())
    {
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue((*it)));
    }
    else
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue("None"));

    // Set second interface to master right
    ++it;
    if (it!=interfaces.end())
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue((*it)));
    else
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue("None"));

    // Set head left and right to none
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadLeft"),ConfigValue("None"));
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadRight"),ConfigValue("None"));

    // Set default sample rate
    QStringList srates = getSampleRates();
    it = srates.begin();
    while (it!=srates.end())
    {
        m_pConfig->set(ConfigKey("[Soundcard]","Samplerate"),ConfigValue((*it)));
        if ((*it).toInt()>=44100)
            break;
        ++it;
    }

    
    // Set currently used latency in config database
//    int msec = (int)(1000.*(2.*1024.)/(2.*(float)(*it).toInt()));
	int msec = 100; // More conservative defaults
    
    m_pConfig->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(msec));
}

QStringList PlayerPortAudio::getInterfaces()
{
	qDebug("PortAudio: getInterfaces()");
	
    QStringList result;
	const PaHostApiInfo* apiInfo = NULL;
	const PaDeviceInfo* devInfo = NULL;
	QString api;

	if (m_HostAPI == "None")
		return result;
	
    PaDeviceIndex numDevices = Pa_GetDeviceCount();
    
    for (int i = 0; i < numDevices; i++)
    {
        devInfo = Pa_GetDeviceInfo(i);
        
        // Add the device if it is an output device:
        //if (devInfo != NULL && devInfo->maxOutputChannels > 0)
        if (devInfo != NULL)
        {
        	apiInfo = Pa_GetHostApiInfo(devInfo->hostApi);
        	//api = apiInfo->name;
			qDebug("Api name: %s", apiInfo->name);
			qDebug(devInfo->name);
        	//qDebug("m_HostAPI: " + m_HostAPI + "devInfo->hostApi: " + new QString(devInfo->hostApi));
        
        	//... and make sure the interface matches the API we've selected.
        	if (m_HostAPI == apiInfo->name)
        	{
            	qDebug("name %s, API %i, maxOutputChannels: %i", devInfo->name, devInfo->hostApi, devInfo->maxOutputChannels);
				
            	for (int j=1; j <= devInfo->maxOutputChannels; ++j)
                	result.append(QString("%1 (channel %2)").arg(devInfo->name).arg(j));
            }
        }
    }
	qDebug("PortAudio: getInterfaces() end");
    return result;
}

QStringList PlayerPortAudio::getSampleRates()
{
    // Returns a sorted list of supported sample rates of the currently opened device.
    // If no device is open, return the list of sample rates supported by the
    // default device
    qDebug("PortAudio: getSampleRates()");
    
    PaError err;
    PaDeviceIndex id = m_devId;
    qDebug("m_devId: %d", m_devId);
    if (id<0)
        id = Pa_GetDefaultOutputDevice();

    const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(id);

	PaStreamParameters outputParams;
	outputParams.device = id;
	outputParams.channelCount = 2;
	outputParams.sampleFormat = paFloat32;
	outputParams.suggestedLatency = .150; //Latency in seconds.
	outputParams.hostApiSpecificStreamInfo = NULL;

    QValueList<double> desiredSampleRates; //A list of all the sample rates we're going to suggest.
    QValueList<double> validSampleRates; //A list containing all the supported sample rates.
    
    //Here's all the sample rates we're going to suggest to PortAudio. We check which ones
    //are actually supported below.
    desiredSampleRates.append(11025.0);
    desiredSampleRates.append(22050.0);
    desiredSampleRates.append(44100.0);
    desiredSampleRates.append(48000.0); 
    desiredSampleRates.append(96000.0);
    
    // Sample rates
    if (devInfo)
    {
        for (unsigned int j=0; j < desiredSampleRates.count(); j++)
        {
        	//Check if each sample rate is supported, if so, add them to the list of supported sample rates.
        	qDebug("PortAudio: checking if sample rate is supported...");
        	err = Pa_IsFormatSupported(NULL, &outputParams, desiredSampleRates[j]);
        	if (err == paFormatIsSupported) //The format IS supported.
        	{
        		validSampleRates.append(desiredSampleRates[j]);
        		qDebug("Supported...");
			}
			else
			{
				qDebug("PortAudio: %s, id was: %d", Pa_GetErrorText(err), id);
			}
		}
    }

	//If for some reason our enumeration of the sample rates failed, throw
	//in some default values. (PortAudio might be being sketchy...)
	if (validSampleRates.count() == 0)
	{
		validSampleRates.append(44100.0);
		validSampleRates.append(96000.0);
	}

    // Sort list
#ifndef QT3_SUPPORT
    qHeapSort(validSampleRates);
#endif

    // Convert srlist to stringlist
    QStringList result;
    for (unsigned int i = 0; i < validSampleRates.count(); ++i)
        result.append(QString("%1").arg((*validSampleRates.at(i))));    
    
    qDebug("PortAudio: getSampleRates() end");
        
    return result;
}

QStringList PlayerPortAudio::getSoundApiList()
{
	//Note: Need to put the active API at the top of the list.
	QStringList apiList;
	const PaHostApiInfo* apiInfo = NULL;

	//We need to initialize PortAudio before we find out what APIs are present.
	//Even if this gets called while PortAudio is already initialized, the docs
	//say this is OK (I think...).
    PaError err = Pa_Initialize();
    if (err == paNoError)
    {
		for (int i = 0; i < Pa_GetHostApiCount(); i++)
		{
			apiInfo = Pa_GetHostApiInfo(i);
			qDebug("Api name: %s", apiInfo->name);
			apiList.append(apiInfo->name);
		}
		Pa_Terminate();
    }
    else
        qDebug("PortAudio error: %s", Pa_GetErrorText(err));
	
	return apiList;
	
	/*
#ifdef __LINUX__
    return QStringList("OSS (PA)");
#endif
#ifdef __MACX__
    return QStringList("CoreAudio (PA)");
#endif
#ifdef __WIN__
    return QStringList("WMME (PA)");
#endif
*/
	
}


PaDeviceIndex PlayerPortAudio::getDeviceID(QString name)
{
	qDebug("PortAudio: getDeviceID(" + name + ")");
    PaDeviceIndex no = Pa_GetDeviceCount();
    for (int i=0; i<no; i++)
    {
        const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(i);

        // Add the device if it is an output device:
        if (devInfo != 0 && devInfo->maxOutputChannels > 0)
        {
            for (int j = 1; j <= devInfo->maxOutputChannels; ++j)
                if (QString("%1 (channel %2)").arg(devInfo->name).arg(j) == name)
                {
                	qDebug("PortAudio: getDeviceID(" + name + "), returning device id %i", i);
                    return i;
                }
        }
    }
    return -1;
}

PaDeviceIndex PlayerPortAudio::getChannelNo(QString name)
{
    PaDeviceIndex no = Pa_GetDeviceCount();
    for (int i=0; i<no; i++)
    {
        const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(i);

        // Add the device if it is an output device:
        if (devInfo!=0 && devInfo->maxOutputChannels > 0)
        {
            for (int j=1; j<=devInfo->maxOutputChannels; ++j)
                if (QString("%1 (channel %2)").arg(devInfo->name).arg(j) == name)
                    return j;
        }
    }
    return -1;
}

int PlayerPortAudio::callbackProcess(int iBufferSize, float *out)
{
    //if (m_iBufferSize==0)
    //    m_iBufferSize = iBufferSize*m_iNumberOfBuffers;
    
    float *tmp = prepareBuffer(iBufferSize);
    float *output = out;
    int i;

    // Reset sample for each open channel
    for (i=0; i<iBufferSize*m_iChannels; i++)
        output[i] = 0.;

    // Copy to output buffer
    for (i=0; i<iBufferSize; i++)
    {
        if (m_iMasterLeftCh>=0)  output[m_iMasterLeftCh]  += tmp[(i*4)  ]/32768.;
        if (m_iMasterRigthCh>=0) output[m_iMasterRigthCh] += tmp[(i*4)+1]/32768.;
        if (m_iHeadLeftCh>=0)    output[m_iHeadLeftCh]    += tmp[(i*4)+2]/32768.;
        if (m_iHeadRightCh>=0)   output[m_iHeadRightCh]   += tmp[(i*4)+3]/32768.;

        for (int j=0; j<m_iChannels; ++j)
            *output++;
    }

    return 0;
}


/* -------- ------------------------------------------------------
   Purpose: Wrapper function to call processing loop function,
            implemented as a method in a class. Used in PortAudio,
            which knows nothing about C++.
   Input:   .
   Output:  -
   -------- ------------------------------------------------------ */
int paV19Callback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *_player)
{
    return ((PlayerPortAudio *)_player)->callbackProcess(framesPerBuffer, (float *)outputBuffer);
}

