/***************************************************************************
                          playerportaudiov19.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
                                           (C) 2006/2007 by Albert Santoni
                                           (C) 2006 by Adam Davison
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

// Mathstuff required for Win32
#include "mathstuff.h"
//Added by qt3to4:
#include <Q3ValueList>
#include <QtDebug>
#include <QWaitCondition>

bool PlayerPortAudio::m_painited = false;

PlayerPortAudio::PlayerPortAudio(ConfigObject<ConfigValue> * config, QString api_name) : Player(config)
{
    for (int i = 0; i < MAX_AUDIODEVICES; i++)
    {
        m_devId[i]      = -1;
        m_inputDevId[i] = -1;
        m_pStream[i]    =  0;
        m_iChannels[i]  = -1;
    }

    m_iNumberOfBuffers = 2;
    m_iMasterLeftCh = -1;
    m_iMasterRigthCh = -1;
    m_iHeadLeftCh = -1;
    m_iHeadRightCh = -1;
    m_iNumActiveDevices = 0;
    m_iPreviousDevIndex = -1;
#ifdef __VINYLCONTROL__
    m_VinylControl[0] = NULL;
    m_VinylControl[1] = NULL;
#endif

    m_HostAPI = api_name;
    m_bInit = false;
}

PlayerPortAudio::~PlayerPortAudio()
{
    //Close all the audio devices.
    close();

    if (m_bInit) {
        Pa_Terminate();
        m_painited = false;
    }
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

#ifdef __VINYLCONTROL__
    //Create a new VinylControl object so that it updates it's settings.
    //TODO: FIX THIS SAMPLERATE HACKAGE
    int iTempSrate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt();
    if (iTempSrate <= 0)
        iTempSrate = 44100;
    m_VinylControl[0] = new VinylControlProxy(m_pConfig, "[Channel1]", iTempSrate);
    m_VinylControl[1] = new VinylControlProxy(m_pConfig, "[Channel2]", 44100);
#endif

    // Find out which device to open. Select the first one listed as either Master Left,
    // Master Right, Head Left, Head Right. If other devices are requested for opening
    // than the one selected here, set them to "None" in the config database

    // For the record, with MAX_AUDIODEVICES currently set to 2, we're referring to
    // Master Left/Right as device 1 and Headphones as device 2 (unless Master and
    // Headphones are on the same device, then we only use device 1.)
    PaDeviceIndex id[MAX_AUDIODEVICES];
    PaDeviceIndex id_input[MAX_AUDIODEVICES];
    int iChannelMax[MAX_AUDIODEVICES]; /** Maximum number of channels needed */
    PaDeviceIndex temp_id = -1;
    QString name;

    for (int i = 0; i < MAX_AUDIODEVICES; i++)
    {
        iChannelMax[i] = -1;
        id[i] = -1;
        id_input[i] = -1;
    }

    m_iMasterLeftCh = -1;
    m_iMasterRigthCh = -1;
    m_iHeadLeftCh = -1;
    m_iHeadRightCh = -1;

    waitForNextOutput.wakeAll();

    // Master left
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterLeft"));
    temp_id = getDeviceID(name);
    if (temp_id >= 0)
    {
        if (getChannelNo(name)>=0)
        {
            id[0] = temp_id;
            iChannelMax[0] = getChannelNo(name);
            m_iMasterLeftCh = getChannelNo(name)-1;
        }
    }

    // Master right
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterRight"));
    temp_id = getDeviceID(name);
    if (getChannelNo(name)>=0                      //Make sure we got a valid number of channels
        && ((id[0]==-1 && temp_id>=0)              //Make sure the left channel and this (right) channel IDs are valid
            || (temp_id!=-1 && id[0]==temp_id))) //No idea...
    {
        id[0] = math_max(temp_id, id[0]);
        iChannelMax[0] = math_max(iChannelMax[0], getChannelNo(name));
        m_iMasterRigthCh = getChannelNo(name)-1;
    }

    // Head left
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadLeft"));
    temp_id = getDeviceID(name);
    if (getChannelNo(name)>=0 && ((id[1]==-1 && temp_id>=0) || (temp_id!=-1 && id[1]==temp_id)))
    {
        id[1] = temp_id;
        iChannelMax[1] = math_max(iChannelMax[1], getChannelNo(name));
        m_iHeadLeftCh = getChannelNo(name)-1;
    }

    // Head right
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadRight"));
    temp_id = getDeviceID(name);
    if (getChannelNo(name)>=0 && ((id[1]==-1 && temp_id>=0) || (temp_id!=-1 && id[1]==temp_id)))
    {
        id[1] = temp_id;
        iChannelMax[1] = math_max(iChannelMax[1], getChannelNo(name));
        m_iHeadRightCh = getChannelNo(name)-1;
    }

    // Inputs
    name = m_pConfig->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck1"));
    temp_id = getInputDeviceID(name);
    if (temp_id != -1)
        id_input[0] = temp_id;

    name = m_pConfig->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck2"));
    temp_id = getInputDeviceID(name);
    if (temp_id != -1)
        id_input[1] = temp_id;

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
    int iChannels[MAX_AUDIODEVICES];
    if (id[0] == id[1]) //Check if we're supposed to use the same soundcard for both Master and Headphone output
    {
        iChannels[0] = math_max(iChannelMax[0], iChannelMax[1]);
        iChannels[1] = -1;
    }
    else
    {
        iChannels[0] = iChannelMax[0];
        iChannels[1] = iChannelMax[1];
    }

    // Sample rate
    int iSrate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt();
    if (iSrate <= 0)
        iSrate = 44100;

    // Get latency in msec
    int iLatencyMSec = m_pConfig->getValueString(ConfigKey("[Soundcard]","Latency")).toInt();

    if (iLatencyMSec <= 0)     //Make sure we don't get a crazy latency value.
        iLatencyMSec = 100;

    // Latency in samples
    //int iLatencySamples = (int)((float)(iSrate*iChannels)/1000.f*(float)iLatencyMSec); //Pre-multisoundcard output
    int iLatencySamples = (int)((float)(iSrate*math_max(iChannels[0], iChannels[1]))/1000.f*(float)iLatencyMSec);

    // Round to the nearest multiple of 4.
    iLatencySamples -= (iLatencySamples % 4);
    iLatencySamples += 4;

    qDebug("iLatencySamples: %i", iLatencySamples);

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
    PaStreamCallback * callback = paV19Callback;
    qDebug("PortAudio: kiMaxFrameSize: %i, iLatencyMSec: %i", kiMaxFrameSize, iLatencyMSec);
    qDebug("PortAudio: id[0] %i, sr %i, ch[0] %i, bufsize %i, bufno %i, req. latency %i msec", id[0], iSrate, iChannels[0], iFramesPerBuffer, m_iNumberOfBuffers, iLatencyMSec);
    qDebug("PortAudio: id[1] %i, sr %i, ch[1] %i, bufsize %i, bufno %i, req. latency %i msec", id[1], iSrate, iChannels[1], iFramesPerBuffer, m_iNumberOfBuffers, iLatencyMSec);

    qDebug("Device 1 indices (id[0]: %i, id_input[0]: %i)", id[0], id_input[0]);
    qDebug("Device 2 indices (id[1]: %i, id_input[1]: %i)", id[1], id_input[1]);


    //If all devices are set to "None", then just return.
    if ((id[0] < 0) && (id[1] < 0))
        return false;

    PaStreamParameters outputParams[MAX_AUDIODEVICES];
    PaStreamParameters inputParams[MAX_AUDIODEVICES];
    bool bDeviceAlreadyOpened = false;

    //Set up and open each soundcard that we need!
    for (int i = 0; i < MAX_AUDIODEVICES; i++)
    {
        bDeviceAlreadyOpened = false;

        for (int j = 0; j < i; j++)         //Check to see if we've already opened this device (eg. Maybe we're already using it for Master output)
        {
            if (j != i)
            {
                if (id[j] == id[i] && id[j] != -1) {
                    bDeviceAlreadyOpened = true;
                }
            }
        }
        if (((id[i] != -1) && !bDeviceAlreadyOpened) || (id_input[i] != -1))         //Make sure we're supposed to open this device...
        {
            qDebug("PortAudio: Trying to open device id %i, with channels %i at samplerate %i", id[i], iChannels[i], iSrate);
            PaStreamParameters * p_outputParams;
            outputParams[i].device = id[i];
            outputParams[i].channelCount = iChannels[i];
            outputParams[i].sampleFormat = paFloat32;
            outputParams[i].suggestedLatency = ((float)iLatencyMSec) / 1000.0f;             //Latency in seconds.
            outputParams[i].hostApiSpecificStreamInfo = NULL;

            PaStreamParameters * p_inputParams;            //TODO: Does this need to be made an array like inputParams?
            inputParams[i].device = id_input[i];             //TODO: Isn't the "input" device always the same as the output device? (if there is an output device?)
            //	     Maybe that should be restricted in the preferences GUI...
            // NO, it's not always the same as the output device! Consider using 4 outputs on your
            // master soundcard, and a different soundcard for input. The master's id is the same as the first
            // for the second stream, but the input id for the second stream is something different.
            inputParams[i].channelCount = 2;
            inputParams[i].sampleFormat = paInt16;             //As needed by scratchlib
            inputParams[i].suggestedLatency = ((float)iLatencyMSec) / 1000.0f;             //Latency in seconds.
            inputParams[i].hostApiSpecificStreamInfo = NULL;

            if (id[i] < 0 || (i > 0 && (id[1] == id[0])))             //TODO: comment this the same as the below input one.... kinda
                p_outputParams = NULL;
            else
                p_outputParams = &(outputParams[i]);

            if (id_input[i] < 0)             //If we're not supposed to open an input device, set the pointer to be null.
                p_inputParams = NULL;                 //NULL input params in Pa_OpenStream means don't open the device for input.
            else
                p_inputParams = &(inputParams[i]);

            PaError err = paNoError;

            // Set up a struct containing all the data we want to pass back to the callback
            callbackStuff[i].player = this;
            callbackStuff[i].devIndex = i;         //The audio device's index (NOT THE PORTAUDIO ID!!!)

            // Try open device using iChannelMax
            err = Pa_OpenStream(&m_pStream[i],
                                p_inputParams,                                  // Input parameters
                                p_outputParams,                                 // Output parameters
                                iSrate,                                                 // Sample rate
                                iFramesPerBuffer,                               // Frames per buffer
                                paClipOff,                                              // Stream flags
                                callback,                                               // Stream callback
                                &(callbackStuff[i]));                                      // Pointer passed to the callback function

            if (err == paNoError)
                m_iChannels[i] = iChannels[i];
            else
            {
                // Try open device using maximum supported channels by soundcard
                iChannels[i] = Pa_GetDeviceInfo(id[i])->maxOutputChannels;
                outputParams[i].channelCount = iChannels[i];
                err = Pa_OpenStream(&m_pStream[i],
                                    p_inputParams,                              // Input parameters
                                    p_outputParams,                             // Output parameters
                                    iSrate,                                                     // Sample rate
                                    iFramesPerBuffer,                                   // Frames per buffer
                                    paClipOff,                                                  // Stream flags
                                    callback,                                                   // Stream callback
                                    &(callbackStuff[i]));                                          // Pointer passed to the callback function
                if (err==paNoError)
                    m_iChannels[i] = iChannels[i];
            }

            if( err != paNoError )         //*** TODO/WARNING: I don't think this block makes any sense whatsoever... - Albert April 21/07
            {
                // Try open device using only two channels
                outputParams[i].channelCount = 2;
                err = Pa_OpenStream(&m_pStream[i],
                                    p_inputParams,                              // Input parameters
                                    p_outputParams,                             // Output parameters
                                    iSrate,                                                     // Sample rate
                                    iFramesPerBuffer,                                   // Frames per buffer
                                    paClipOff,                                                  // Stream flags
                                    callback,                                                   // Stream callback
                                    &(callbackStuff[i]));                                          // Pointer passed to the callback function
                if (err==paNoError)
                {
                    m_iChannels[i] = 2;

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

            if( err != paNoError )         //Make sure we opened the soundcard successfully.
            {
                qDebug("PortAudio: Open stream error: %s", Pa_GetErrorText(err));
                qDebug("PortAudio: More error info: %s", Pa_GetLastHostErrorInfo()->errorText);

                m_devId[i] = -1;
                m_inputDevId[i] = -1;
                m_iChannels[i] = -1;

                return false;
            }

            m_devId[i] = id[i];
            m_inputDevId[i] = id_input[i];

            // Update SRATE and Latency ControlObjects
            m_pControlObjectSampleRate->queueFromThread((double)iSrate);
            m_pControlObjectLatency->queueFromThread((double)iLatencyMSec);

            // Start stream
            err = Pa_StartStream(m_pStream[i]);
            if (err != paNoError)
            {
                qDebug("PortAudio: Start stream %i error: %s", i, Pa_GetErrorText(err));
                m_pStream[i] = 0;
            }
            else
                qDebug("PortAudio: Started stream %i successfully", i);

            //If the Master and Headphone devices are the same, or the Headphone device is set to "None",
            //then we can just break out of this loop:
            /*if ((id[0] == id[1]) || (id[1] == -1))
               {
                    calculateNumActiveDevices();
                    return true;
               }*/
        }
    }

    /*
       //Print out all the information about the soundcards and their channels.
       qDebug("PortAudio: ==Soundcard Summary==");
       qDebug("Device 1 index: %i", m_devId[0]);
       qDebug("Device 2 index: %i", m_devId[1]);
       qDebug("Channels for device 1: %i", m_iChannels[0]);
       qDebug("Channels for device 2: %i", m_iChannels[1]);
       qDebug("m_iMasterLeftCh: %i",m_iMasterLeftCh);
       qDebug("m_iMasterRightCh: %i",m_iMasterRigthCh);
       qDebug("m_iHeadLeftCh: %i",m_iHeadLeftCh);
       qDebug("m_iHeadRightCh: %i",m_iHeadRightCh);
     */

    calculateNumActiveDevices();
    return true;
}

//Find out how many active/open soundcards there are.
void PlayerPortAudio::calculateNumActiveDevices()
{
    int count = 0;

    for (int i = 0; i < MAX_AUDIODEVICES; i++)
    {
        if (m_devId[i] != -1)
            count++;
    }
    m_iNumActiveDevices = count;
}

void PlayerPortAudio::close()
{
    //waitForNextOutput.wakeAll();

    for (int i = 0; i < MAX_AUDIODEVICES; i++)
    {
        m_devId[i] = -1;
        m_iChannels[i] = 0;

        // Stop streams
        if (m_pStream[i])
        {
            PaError err = Pa_StopStream(m_pStream[i]);
            if( err != paNoError )
                qDebug("PortAudio: Stop stream %i error: %s,", i, Pa_GetErrorText(err));

            // Close streams
            err = Pa_CloseStream(m_pStream[i]);
            if( err != paNoError )
                qDebug("PortAudio: Close stream %i error: %s", i, Pa_GetErrorText(err));
        }

        m_pStream[i] = 0;
        waitForNextOutput.wakeAll();
        calculateNumActiveDevices();
    }

    //waitForNextOutput.wakeAll();

    m_iMasterLeftCh = -1;
    m_iMasterRigthCh = -1;
    m_iHeadLeftCh = -1;
    m_iHeadRightCh = -1;
}

void PlayerPortAudio::setDefaults()
{
    // Get list of interfaces
    QStringList interfaces = getInterfaces();

    // Set first interfaces to master left
    QStringListIterator it(interfaces);

    if (it.hasNext())
    {
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue((it.next())));
    }
    else
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue("None"));

    // Set second interface to master right
    if (it.hasNext())
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue((it.next())));
    else
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue("None"));

    // Set head left and right to none
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadLeft"),ConfigValue("None"));
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadRight"),ConfigValue("None"));

    // Set default sample rate
    QStringList srates = getSampleRates();
    QStringListIterator itSRates(srates);
    while (it.hasNext())
    {
        QString s;
        s = itSRates.next();
        m_pConfig->set(ConfigKey("[Soundcard]","Samplerate"),ConfigValue(s));
        if ((s).toInt()>=44100)
            break;
    }

    // Set currently used latency in config database
    //int msec = (int)(1000.*(2.*1024.)/(2.*(float)(*it).toInt()));
    int msec = 100;     // More conservative defaults

    m_pConfig->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(msec));
}

QStringList PlayerPortAudio::getInterfaces()
{
    //qDebug("PortAudio: getInterfaces()");

    QStringList result;
    const PaHostApiInfo * apiInfo = NULL;
    const PaDeviceInfo * devInfo = NULL;
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
            //qDebug("Api name: %s", apiInfo->name);
            qDebug(devInfo->name);
            //qDebug("m_HostAPI: " + m_HostAPI + "devInfo->hostApi: " + new QString(devInfo->hostApi));

            //... and make sure the interface matches the API we've selected.
            if (m_HostAPI == apiInfo->name)
            {
                //qDebug("name %s, API %i, maxOutputChannels: %i", devInfo->name, devInfo->hostApi, devInfo->maxOutputChannels);

                for (int j=1; j <= devInfo->maxOutputChannels; ++j)
                    result.append(QString("%1 (channel %2)").arg(devInfo->name).arg(j));
            }
        }
    }
    //qDebug("PortAudio: getInterfaces() end");
    return result;
}

/**
 * Get the audio input/capture interfaces from PortAudio
 */
QStringList PlayerPortAudio::getInputInterfaces()
{
    qDebug("PortAudio: getInputInterfaces()");

    QStringList result;
    const PaHostApiInfo * apiInfo = NULL;
    const PaDeviceInfo * devInfo = NULL;
    QString api;

    if (m_HostAPI == "None")
        return result;

    PaDeviceIndex numDevices = Pa_GetDeviceCount();

    for (int i = 0; i < numDevices; i++)
    {
        devInfo = Pa_GetDeviceInfo(i);

        // Add the device if it is an input device:
        //if (devInfo != NULL && devInfo->maxOutputChannels > 0)
        if (devInfo != NULL && (devInfo->maxInputChannels > 0))
        {
            apiInfo = Pa_GetHostApiInfo(devInfo->hostApi);
            //api = apiInfo->name;
            qDebug("Api name: %s", apiInfo->name);
            qDebug(devInfo->name);
            //qDebug("m_HostAPI: " + m_HostAPI + "devInfo->hostApi: " + new QString(devInfo->hostApi));

            //... and make sure the interface matches the API we've selected.
            if (m_HostAPI == apiInfo->name )
            {
                qDebug("name %s, API %i, maxInputChannels: %i", devInfo->name, devInfo->hostApi, devInfo->maxInputChannels);

                result.append(QString("%1").arg(devInfo->name));

                //for (int j=1; j <= devInfo->maxInputChannels; ++j)
                // 	result.append(QString("%1 (channel %2)").arg(devInfo->name).arg(j));
            }
        }
    }
    qDebug("PortAudio: getInputInterfaces() end");
    return result;
}


QStringList PlayerPortAudio::getSampleRates()
{
    // Returns a sorted list of supported sample rates of the currently opened device.
    // If no device is open, return the list of sample rates supported by the
    // default device
    qDebug("PortAudio: getSampleRates()");

    PaError err;
    PaDeviceIndex id = m_devId[0];              //TODO: This is a hack to pick the samplerates from the first soundcard....
    //Figure out something smarter...
    //Something smarter = poll the samplerates from both soundcards, and
    //only display the samlerates that are supported by both. - Albert April 30, 2007
    //The problem with doing this right now is that Pa_IsFormatSupported when using
    //ALSA takes ages for some reason, and polling both soundcards here would
    //double this 5 second freeze that Mixxx experiences because of Pa_IsFormatSupported
    //(buggily) blocking.
    //qDebug("m_devId[0]: %d", m_devId[0]);
    if (id<0)
        id = Pa_GetDefaultOutputDevice();

    //const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(id);

    PaStreamParameters outputParams;
    outputParams.device = id;
    outputParams.channelCount = 2;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = .150;     //Latency in seconds.
    outputParams.hostApiSpecificStreamInfo = NULL;

    Q3ValueList<double> desiredSampleRates; //A list of all the sample rates we're going to suggest.
    Q3ValueList<double> validSampleRates; //A list containing all the supported sample rates.

    //Here's all the sample rates we're going to suggest to PortAudio. We check which ones
    //are actually supported below.
    desiredSampleRates.append(11025.0);
    desiredSampleRates.append(22050.0);
    desiredSampleRates.append(44100.0);
    desiredSampleRates.append(48000.0);
    desiredSampleRates.append(96000.0);

    // Sample rates
    //if (devInfo)
    {
        for (unsigned int j=0; j < desiredSampleRates.count(); j++)
        {
            //Check if each sample rate is supported, if so, add them to the list of supported sample rates.
            qDebug("PortAudio: checking if sample rate is supported...");
            err = Pa_IsFormatSupported(NULL, &outputParams, desiredSampleRates[j]);
            if (err == paFormatIsSupported)     //The format IS supported.
            {
                validSampleRates.append(desiredSampleRates[j]);
                qDebug("Supported...");
            }
            else
            {
                qDebug("PortAudio error: %s, id was: %d", Pa_GetErrorText(err), id);
            }
        }
    }

    //If for some reason our enumeration of the sample rates failed, throw
    //in some default values. (PortAudio might be being sketchy...)
    if (validSampleRates.count() == 0)
    {
        validSampleRates.append(44100.0);
        validSampleRates.append(48000.0);
        validSampleRates.append(96000.0);
    }

    // Sort list
#ifndef QT3_SUPPORT
    qSort(validSampleRates);
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
    const PaHostApiInfo * apiInfo = NULL;

    //We need to initialize PortAudio before we find out what APIs are present.
    //Even if this gets called while PortAudio is already initialized, the docs
    //say this is OK (I think...).

    PaError err = paNoError;

    // So this little hackfest saves buggy drivers from being really buggy - AD
    if (!m_painited) {
        err = Pa_Initialize();
        m_painited = true;
    }

    if (err == paNoError)
    {
        for (int i = 0; i < Pa_GetHostApiCount(); i++)
        {
            apiInfo = Pa_GetHostApiInfo(i);
            //qDebug("Api name: %s", apiInfo->name);
            apiList.append(apiInfo->name);
        }
//		Pa_Terminate();
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
    //qDebug("PortAudio: getDeviceID(" + name + ")");
    PaDeviceIndex no = Pa_GetDeviceCount();
    for (int i=0; i<no; i++)
    {
        const PaDeviceInfo * devInfo = Pa_GetDeviceInfo(i);

        // Add the device if it is an output device:
        if (devInfo != 0 && devInfo->maxOutputChannels > 0)
        {
            for (int j = 1; j <= devInfo->maxOutputChannels; ++j)
                if (QString("%1 (channel %2)").arg(devInfo->name).arg(j) == name)
                {
                    //qDebug("PortAudio: getDeviceID(" + name + "), returning device id %i", i);
                    return i;
                }
        }
    }
    return -1;
}


PaDeviceIndex PlayerPortAudio::getInputDeviceID(QString name)
{
    qDebug("PortAudio: getInputDeviceID(" + name + ")");
    PaDeviceIndex no = Pa_GetDeviceCount();
    for (int i=0; i<no; i++)
    {
        const PaDeviceInfo * devInfo = Pa_GetDeviceInfo(i);

        // Add the device if it is an input device:
        if (devInfo != 0 && devInfo->maxOutputChannels > 0)
        {
            for (int j = 1; j <= devInfo->maxInputChannels; ++j)
            {
                //qDebug("Comparing: " + QString("%1").arg(devInfo->name));
                //qDebug("and: " + name);
                if (QString("%1").arg(devInfo->name) == name)
                {
                    qDebug("PortAudio: getInputDeviceID(" + name + "), returning device id %i", i);
                    return i;
                }
            }
        }
    }
    qDebug("PortAudio: getInputDeviceID(" + name + "), returning device id -1");
    return -1;
}

PaDeviceIndex PlayerPortAudio::getChannelNo(QString name)
{
    PaDeviceIndex no = Pa_GetDeviceCount();
    for (int i=0; i<no; i++)
    {
        const PaDeviceInfo * devInfo = Pa_GetDeviceInfo(i);

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

/** -------- ------------------------------------------------------
        Purpose: This callback function gets called everytime a soundcard runs out of samples to play.
                         This is where the soundcard/PortAudio ask Mixxx for more samples, and get them in
                         an orderly fashion (*thread safety). This is also where the "routing" of audio samples
                         take place.
        -------- ------------------------------------------------------
 */
int PlayerPortAudio::callbackProcess(unsigned long framesPerBuffer, float * out, short * in, int devIndex)
{
    //if (m_iBufferSize==0)
    //    m_iBufferSize = iBufferSize*m_iNumberOfBuffers;

    static float * tmp; //Leave this as static - it's shared between two threads.
    float * output = out;
    int i;

    /*
       if (!out)
       {
        qDebug() << "output is NULL!";
       prevDevice.lock();
       m_iPreviousDevIndex = devIndex; //Save this devIndex as the previous one
        prevDevice.unlock();
        waitForNextOutput.wakeAll(); //Allow the other thread to give at 'er
        return 0;
       }*/

    prevDevice.lock();
    int iPreviousDevIndex = m_iPreviousDevIndex;
    prevDevice.unlock();

    if (iPreviousDevIndex == devIndex && m_iNumActiveDevices > 1)
    {
        lockSamples.lock();
        waitForNextOutput.wait(&lockSamples);
        lockSamples.unlock();
    }

    //Only fill the buffer with sound data from Mixxx once.
    lockSamples.lock();
    if (devIndex == 0)
    {
        tmp = prepareBuffer(iBufferSize);
    }
    lockSamples.unlock();

    // Reset sample for each open channel
    for (i=0; i<framesPerBuffer*m_iChannels[devIndex]; i++)
        output[i] = 0.;

    // Copy to output buffer
    for (i=0; i<iBufferSize; i++)
    {
        if (devIndex == 0) //For the first sound device
        {
            if (m_iMasterLeftCh>=0  && m_iChannels[devIndex]>=1) { output[m_iMasterLeftCh]  += tmp[(i*4)  ]/32768.;}
            if (m_iMasterRigthCh>=0 && m_iChannels[devIndex]>=2) output[m_iMasterRigthCh] += tmp[(i*4)+1]/32768.;
            if (m_iHeadLeftCh>=0    && m_iChannels[devIndex]>=3) output[m_iHeadLeftCh]    += tmp[(i*4)+2]/32768.;
            if (m_iHeadRightCh>=0   && m_iChannels[devIndex]>=4) output[m_iHeadRightCh]   += tmp[(i*4)+3]/32768.;
        }
        else if (devIndex == 1)         //If there's a second sound device, route the headphones to it.
        {
            //if (m_iMasterLeftCh>=0  && m_iChannels[devIndex]>=1) { output[m_iMasterLeftCh]  += tmp[(i*4)  ]/32768.;}
            //if (m_iMasterRigthCh>=0 && m_iChannels[devIndex]>=2) output[m_iMasterRigthCh] += tmp[(i*4)+1]/32768.;
            //qDebug() << "m_iHeadLeftCh:" << m_iHeadLeftCh << "devIndex: " << devIndex << "tmp[blah]" << tmp[(i*4)+2]/32768.;
            if (m_iHeadLeftCh>=0    && m_iChannels[devIndex]>=1) output[m_iHeadLeftCh]    += tmp[(i*4)+2]/32768.;
            if (m_iHeadRightCh>=0   && m_iChannels[devIndex]>=2) output[m_iHeadRightCh]   += tmp[(i*4)+3]/32768.;
            //qDebug("headphones!");
        }
        for (int j=0; j < m_iChannels[devIndex]; ++j)
            *output++;
    }

    if (in)
    {
#ifdef __VINYLCONTROL__
        m_VinylControl[devIndex]->AnalyseSamples(in, iBufferSize);
#endif
    }

    prevDevice.lock();
    m_iPreviousDevIndex = devIndex; //Save this devIndex as the previous one
    prevDevice.unlock();
    waitForNextOutput.wakeAll();     //Allow the other thread to give at 'er

    return 0;
}


/* -------- ------------------------------------------------------
   Purpose: Wrapper function to call processing loop function,
            implemented as a method in a class. Used in PortAudio,
            which knows nothing about C++.
   Input:   .
   Output:  -
   -------- ------------------------------------------------------ */
int paV19Callback(const void * inputBuffer, void * outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo * timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void * _callbackStuff)
{
    /*
       //Variables that are used in the human-readable form of function call from hell (below).
       static PlayerPortAudio* _player;
       static int devIndex;
       _player = ((PAPlayerCallbackStuff*)_callbackStuff)->player;
       devIndex = ((PAPlayerCallbackStuff*)_callbackStuff)->devIndex;
     */

    //Human-readable form of the function call from hell:
    //return _player->callbackProcess(framesPerBuffer, (float *)outputBuffer, devIndex);

    //Function call from hell (might provide a little bit of cheapo thread safety to do it this way):
    return ((PAPlayerCallbackStuff *)_callbackStuff)->player->callbackProcess(framesPerBuffer, (float *)outputBuffer, (short *)inputBuffer, ((PAPlayerCallbackStuff *)_callbackStuff)->devIndex);
}

