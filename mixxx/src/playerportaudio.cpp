/***************************************************************************
                          playerportaudio.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

int bufferIdxSlave = 0;


PlayerPortAudio::PlayerPortAudio(ConfigObject<ConfigValue> *config, ControlObject *pControl) : Player(config,pControl)
{
    m_devId = -1;
    m_iChannels = -1;
    m_iMasterLeftCh = -1;
    m_iMasterRigthCh = -1;
    m_iHeadLeftCh = -1;
    m_iHeadRightCh = -1;
    m_pStream = 0;

    PaError err = Pa_Initialize();
    if (err!=paNoError)
        qFatal("PortAudio: Initialization error");
}

PlayerPortAudio::~PlayerPortAudio()
{
    Pa_Terminate();
}

bool PlayerPortAudio::open()
{
    Player::open();

    // Find out which device to open. Select the first one listed as either Master Left,
    // Master Right, Head Left, Head Right. If other devices are requested for opening
    // than the one selected here, set them to "None" in the config database
    PaDeviceID id = -1;
    PaDeviceID temp = -1;
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
        iChannelMax = max(iChannelMax, getChannelNo(name));
        m_iMasterRigthCh = getChannelNo(name)-1;
    }

    // Head left
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadLeft"));
    temp = getDeviceID(name);
    if (getChannelNo(name)>=0 && ((id==-1 && temp>=0) || (temp!=-1 && id==temp)))
    {
        id = temp;
        iChannelMax = max(iChannelMax, getChannelNo(name));
        m_iHeadLeftCh = getChannelNo(name)-1;
    }

    // Head right
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadRight"));
    temp = getDeviceID(name);
    if (getChannelNo(name)>=0 && ((id==-1 && temp>=0) || (temp!=-1 && id==temp)))
    {
        id = temp;
        iChannelMax = max(iChannelMax, getChannelNo(name));
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

    // Setup latency
    int iNumberOfBuffers;
    int iFramesPerBuffer;
    int iLatency = iSrate*(m_pConfig->getValueString(ConfigKey("[Soundcard]","Latency")).toFloat()/1000.);

    if (iLatency/kiMaxFrameSize<2)
        iNumberOfBuffers = 2;
    else
        iNumberOfBuffers = iLatency/kiMaxFrameSize;

    iFramesPerBuffer = iLatency/iNumberOfBuffers;


    // Callback function to use
    PortAudioCallback *callback = paCallback;

    qDebug("id %i, sr %i, ch %i, bufsize %i, bufno %i", id, iSrate, iChannels, iFramesPerBuffer, iNumberOfBuffers);

    if (id<0)
        return false;

    PaError err = paNoError;

    // Try open device using iChannelMax
    err = Pa_OpenStream(&m_pStream, paNoDevice, 0, paFloat32, 0,
                        id,                 // Id of output device
                        iChannels,          // Number of output channels
                        paFloat32,          // Output sample format
                        0,                  // Extra info. Not used.
                        iSrate,             // Sample rate
                        iFramesPerBuffer,   // Frames per buffer
                        iNumberOfBuffers,   // Number of buffers
                        paClipOff,          // we won't output out of range samples so don't bother clipping them
                        callback,           // Callback function
                        this);              // Pointer passed to the callback function
    if (err == paNoError)
        m_iChannels = iChannels;
    else
    {
        // Try open device using maximum supported channels by soundcard
        iChannels = Pa_GetDeviceInfo(id)->maxOutputChannels;
        err = Pa_OpenStream(&m_pStream, paNoDevice, 0, paFloat32, 0,
                        id,                 // Id of output device
                        iChannels,          // Number of output channels
                        paFloat32,          // Output sample format
                        0,                  // Extra info. Not used.
                        iSrate,             // Sample rate
                        iFramesPerBuffer,   // Frames per buffer
                        iNumberOfBuffers,   // Number of buffers
                        paClipOff,          // we won't output out of range samples so don't bother clipping them
                        callback,           // Callback function
                        this);              // Pointer passed to the callback function
        if (err==paNoError)
            m_iChannels = iChannels;
    }

    if( err != paNoError )
    {
        // Try open device using only two channels
        err = Pa_OpenStream(&m_pStream, paNoDevice, 0, paFloat32, 0,
                        id,                 // Id of output device
                        2,                  // Number of output channels
                        paFloat32,          // Output sample format
                        0,                  // Extra info. Not used.
                        iSrate,             // Sample rate
                        iFramesPerBuffer,   // Frames per buffer
                        iNumberOfBuffers,   // Number of buffers
                        paClipOff,          // we won't output out of range samples so don't bother clipping them
                        callback,           // Callback function
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
        qWarning("PortAudio: Open stream error: %s", Pa_GetErrorText(err));
        err = Pa_GetHostError();

        m_devId = -1;
        m_iChannels = -1;

        return false;
    }

    m_devId = id;

    // Update SRATE in EngineObject
    setPlaySrate(iSrate);

    // Start stream
    err = Pa_StartStream(m_pStream);
    if (err != paNoError)
        qWarning("PortAudio: Start stream error: %s", Pa_GetErrorText(err));

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
            qWarning("PortAudio: Stop stream error: %s,", Pa_GetErrorText(err));
    }

    // Close stream
    PaError err = Pa_CloseStream(m_pStream);
    if( err != paNoError )
        qWarning("PortAudio: Close stream error: %s", Pa_GetErrorText(err));
    m_pStream = 0;
}

void PlayerPortAudio::setDefaults()
{
    // Get list of interfaces
    QStringList interfaces = getInterfaces();

    // Set first interfaces to master left
    QStringList::iterator it = interfaces.begin();
    if (*it)
    {
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue((*it)));
    }
    else
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue("None"));

    // Set second interface to master right
    ++it;
    if (*it)
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue((*it)));
    else
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue("None"));

    // Set head left and right to none
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadLeft"),ConfigValue("None"));
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadRight"),ConfigValue("None"));

    // Set default sample rate
    QStringList srates = getSampleRates();
    it = srates.begin();
    while (*it)
    {
        m_pConfig->set(ConfigKey("[Soundcard]","Samplerate"),ConfigValue((*it)));

        if ((*it)>=44100)
            break;
    }

    // Set currently used latency in config database
    int msec = 1000*(2*1024)/(*it).toInt();
    m_pConfig->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(msec));
}

QStringList PlayerPortAudio::getInterfaces()
{
    QStringList result;

    int no = Pa_CountDevices();
    for (int i=0; i<no; i++)
    {
        const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(i);

        // Add the device if it is an output device:
        if (devInfo!=0 && devInfo->maxOutputChannels > 0)
        {
            for (int j=1; j<=devInfo->maxOutputChannels; ++j)
                result.append(QString("%1 (channel %2)").arg(devInfo->name).arg(j));
        }
    }

    return result;
}

QStringList PlayerPortAudio::getSampleRates()
{

    // Returns the list of supported sample rates of the currently opened device.
    // If no device is open, return the list of sample rates supported by the
    // default device
    PaDeviceID id = m_devId;
    if (id<0)
        id = Pa_GetDefaultOutputDeviceID();

    const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(id);
    QStringList result;

    // Sample rates
    if (devInfo->numSampleRates > 0)
    {
        for (int j=0; j<devInfo->numSampleRates; j++)
            result.append(QString("%1").arg((int)devInfo->sampleRates[j]));
    }
    else
    {
        // If we're just given a range of samplerates, then just
        // assume some standard rates:
        result.append(QString("%1").arg(11025));
        result.append(QString("%1").arg(22050));
        result.append(QString("%1").arg(44100));
        result.append(QString("%1").arg(48000));
    }

    return result;
}

PaDeviceID PlayerPortAudio::getDeviceID(QString name)
{
    int no = Pa_CountDevices();
    for (int i=0; i<no; i++)
    {
        const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(i);

        // Add the device if it is an output device:
        if (devInfo!=0 && devInfo->maxOutputChannels > 0)
        {
            for (int j=1; j<=devInfo->maxOutputChannels; ++j)
                if (QString("%1 (channel %2)").arg(devInfo->name).arg(j) == name)
                    return i;
        }
    }
    return -1;
}

PaDeviceID PlayerPortAudio::getChannelNo(QString name)
{
    int no = Pa_CountDevices();
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
int paCallback(void *, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      PaTimestamp, void *pPlayer)
{
    return ((PlayerPortAudio *)pPlayer)->callbackProcess(framesPerBuffer, (float *)outputBuffer);
}
