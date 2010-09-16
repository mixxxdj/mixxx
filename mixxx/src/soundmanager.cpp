/***************************************************************************
                          soundmanager.cpp
                             -------------------
    begin                : Sun Aug 15, 2007
    copyright            : (C) 2007 Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>
#include <QtCore>
#include <portaudio.h>
#include "soundmanager.h"
#include "sounddevice.h"
#include "sounddeviceportaudio.h"
#include "engine/enginemaster.h"
#include "controlobjectthreadmain.h"

/** Initializes Mixxx's audio core
 *  @param pConfig The config key table
 *  @param _master A pointer to the audio engine's mastering class.
 */
SoundManager::SoundManager(ConfigObject<ConfigValue> * pConfig, EngineMaster * _master) : QObject()
{
    //qDebug() << "SoundManager::SoundManager()";
    m_pConfig = pConfig;
    m_pMaster = _master;
    m_pStreamBuffers[SOURCE_MASTER] = (CSAMPLE*)_master->getMasterBuffer();
    m_pStreamBuffers[SOURCE_HEADPHONES] = (CSAMPLE*)_master->getHeadphoneBuffer();

    //Note that we deal with input as shorts instead of CSAMPLEs
    m_pReceiverBuffers[RECEIVER_VINYLCONTROL_ONE] = new short[MAX_BUFFER_LEN];
    m_pReceiverBuffers[RECEIVER_VINYLCONTROL_TWO] = new short[MAX_BUFFER_LEN];
    m_pReceiverBuffers[RECEIVER_MICROPHONE] = new short[MAX_BUFFER_LEN];

    iNumDevicesOpenedForOutput = 0;
    iNumDevicesOpenedForInput = 0;
    iNumDevicesHaveRequestedBuffer = 0;
#ifdef __VINYLCONTROL__
    m_VinylControl[0] = 0;
    m_VinylControl[1] = 0;
#endif

    //TODO: Find a better spot for this:
    //Set up a timer to sync Mixxx's ControlObjects on...
    //(We set the timer to fire off
    //connect(&m_controlObjSyncTimer, SIGNAL(timeout()), this, SLOT(sync()));
    //m_controlObjSyncTimer.start(33);
    //m_controlObjSyncTimer->start(m_pConfig->getValueString(ConfigKey("[Soundcard]","Latency")).toInt());

    //These are ControlObjectThreadMains because all the code that
    //uses them is called from the GUI thread (stuff like opening soundcards).
    ControlObjectThreadMain* pControlObjectLatency = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]", "latency")));
    ControlObjectThreadMain* pControlObjectSampleRate = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]", "samplerate")));
    ControlObjectThreadMain* pControlObjectVinylControlMode = new ControlObjectThreadMain(new ControlObject(ConfigKey("[VinylControl]", "Mode")));
    ControlObjectThreadMain* pControlObjectVinylControlMode1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "VinylMode")));
    ControlObjectThreadMain* pControlObjectVinylControlMode2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "VinylMode")));
    //ControlObjectThreadMain* pControlObjectVinylControlEnabled1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]", "vinylcontrol")));
    //ControlObjectThreadMain* pControlObjectVinylControlEnabled2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]", "vinylcontrol")));
//    ControlObjectThreadMain* pControlObjectVinylControlEnabled = new ControlObjectThreadMain(new ControlObject(ConfigKey("[VinylControl]", "Enabled")));
    ControlObjectThreadMain* pControlObjectVinylControlGain = new ControlObjectThreadMain(new ControlObject(ConfigKey("[VinylControl]", "VinylControlGain")));
    ControlObjectThreadMain* pControlObjectVinylControlSignalQuality1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]", "VinylControlQuality")));
    ControlObjectThreadMain* pControlObjectVinylControlSignalQuality2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]", "VinylControlQuality")));
    ControlObjectThreadMain* pControlObjectVinylControlInputStrengthL1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]", "VinylControlInputL")));
    ControlObjectThreadMain* pControlObjectVinylControlInputStrengthR1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]", "VinylControlInputR")));
    ControlObjectThreadMain* pControlObjectVinylControlInputStrengthL2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]", "VinylControlInputL")));
    ControlObjectThreadMain* pControlObjectVinylControlInputStrengthR2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]", "VinylControlInputR")));



    pControlObjectLatency->slotSet(m_pConfig->getValueString(ConfigKey("[Soundcard]","Latency")).toInt());
    pControlObjectSampleRate->slotSet(m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt());
    pControlObjectVinylControlMode1->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","Mode")).toInt());
    pControlObjectVinylControlMode2->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","Mode")).toInt());
    //pControlObjectVinylControlEnabled->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","Enabled")).toInt());
    //pControlObjectVinylControlEnabled1->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","Enabled")).toInt());
    //pControlObjectVinylControlEnabled2->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","Enabled")).toInt());
    pControlObjectVinylControlGain->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","VinylControlGain")).toInt());


    qDebug() << "SampleRate" << pControlObjectSampleRate->get();
    qDebug() << "Latency" << pControlObjectLatency->get();

    //Hack because PortAudio samplerate enumeration is slow as hell on Linux (ALSA dmix sucks, so we can't blame PortAudio)
    m_samplerates.push_back("44100");
    m_samplerates.push_back("48000");
    m_samplerates.push_back("96000");
}

/** Destructor for the SoundManager class. Closes all the devices, cleans up their pointers
  and terminates PortAudio. */
SoundManager::~SoundManager()
{
    //TODO: Should only call Pa_Terminate() if Pa_Inititialize() was successful.

    //Clean up devices.
    clearDeviceList();

    Pa_Terminate();

    delete [] m_pReceiverBuffers[RECEIVER_VINYLCONTROL_ONE];
    delete [] m_pReceiverBuffers[RECEIVER_VINYLCONTROL_TWO];
    delete [] m_pReceiverBuffers[RECEIVER_MICROPHONE];
}

/** Returns a list of all the devices we've enumerated through PortAudio.
  *
  * @param filterAPI If filterAPI is the name of an audio API used by PortAudio, this function
  * will only return devices that belong to that API. Otherwise, the list will
  * contain all devices on all PortAudio-supported APIs.
  * @param bOutputDevices If bOutputDevices is true, then devices
  *                       supporting audio output will be listed.
  * @param bInputDevices If bInputDevices is true, then devices supporting
  *                      audio input will be listed too.
  */
QList<SoundDevice*> SoundManager::getDeviceList(QString filterAPI, bool bOutputDevices, bool bInputDevices)
{
    //qDebug() << "SoundManager::getDeviceList";
    bool bMatchedCriteria = true;   //Whether or not the current device matched the filtering criteria

    if (m_devices.empty())
        this->queryDevices();

    if (filterAPI == "None")
    {
        QList<SoundDevice*> emptyList;
        return emptyList;
    }
    else
    {
        //Create a list of sound devices filtered to match given API and input/output .
        QList<SoundDevice*> filteredDeviceList;
        QListIterator<SoundDevice*> dev_it(m_devices);
        while (dev_it.hasNext())
        {
            bMatchedCriteria = true;                //Reset this for the next device.
            SoundDevice *device = dev_it.next();
            if (device->getHostAPI() != filterAPI)
                bMatchedCriteria = false;
            if (bOutputDevices)
            {
                 if (device->getNumOutputChannels() <= 0)
                    bMatchedCriteria = false;
            }
            if (bInputDevices)
            {
                if (device->getNumInputChannels() <= 1) //Ignore mono input and no-input devices
                    bMatchedCriteria = false;
            }

            if (bMatchedCriteria)
                filteredDeviceList.push_back(device);
        }
        return filteredDeviceList;
    }

    return m_devices;
}

/** Get a list of host APIs supported by PortAudio.
 *  @return The list of audio APIs supported on the current computer.
 */
QList<QString> SoundManager::getHostAPIList()
{
    QList<QString> apiList;

    for (PaHostApiIndex i = 0; i < Pa_GetHostApiCount(); i++)
    {
        const PaHostApiInfo *api = Pa_GetHostApiInfo(i);
        if (QString(api->name) != "skeleton implementation") apiList.push_back(api->name);
    }

    return apiList;
}

/** Set which host API Mixxx should use.
 *  @param api The host API that you want Mixxx to use.
 *  @return 0 on success, non-zero otherwise.
 */
int SoundManager::setHostAPI(QString api)
{
    m_hostAPI = api;
    m_pConfig->set(ConfigKey("[Soundcard]","SoundApi"), ConfigValue(api));

    return 0;
}
//FIXME: Unused
QString SoundManager::getHostAPI()
{
    return m_hostAPI;
}

/** Closes all the open sound devices.
 *
 *  Because multiple soundcards might be open, this member function
 *  simply runs through the list of all known soundcards (from PortAudio)
 *  and attempts to close them all. Closing a soundcard that isn't open
 *  is safe.
 */
void SoundManager::closeDevices()
{
    //qDebug() << "SoundManager::closeDevices()";
    QListIterator<SoundDevice*> dev_it(m_devices);

    //requestBufferMutex.lock(); //Ensures we don't kill a stream in the middle of a callback call.
                                 //Note: if we're using Pa_StopStream() (like now), we don't need
                                 //      to lock. PortAudio stops the threads nicely.
    while (dev_it.hasNext())
    {
        //qDebug() << "closing a device...";
        dev_it.next()->close();
    }
    //requestBufferMutex.unlock();

    //requestBufferMutex.lock();
    iNumDevicesOpenedForOutput = 0;
    iNumDevicesOpenedForInput = 0;
    iNumDevicesHaveRequestedBuffer = 0;
    //requestBufferMutex.unlock();

#ifdef __VINYLCONTROL__
    if (m_VinylControl[0])
        delete m_VinylControl[0];
    if (m_VinylControl[1])
        delete m_VinylControl[1];
    if (m_VinylControl[0])
        m_VinylControl[0] = NULL;
    if (m_VinylControl[1])
        m_VinylControl[1] = NULL;

#endif
}

/** Closes all the devices and empties the list of devices we have. */
void SoundManager::clearDeviceList()
{
    //qDebug() << "SoundManager::clearDeviceList()";

    //Close the devices first.
    closeDevices();

    //Empty out the list of devices we currently have.
    while (!m_devices.empty())
    {
        SoundDevice* dev = m_devices.takeLast();
        delete dev;
    }
    
#ifdef __PORTAUDIO__
    Pa_Terminate();
#endif
}

/** Returns a list of samplerates we will attempt to support.
 *  @return The list of available samplerates.
 */
QList<QString> SoundManager::getSamplerateList()
{
    return m_samplerates;
}

//Creates a list of sound devices that PortAudio sees.
void SoundManager::queryDevices()
{
    //qDebug() << "SoundManager::queryDevices()";
    clearDeviceList();

#ifdef __PORTAUDIO__
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        qDebug() << "Error:" << Pa_GetErrorText(err);
        return;
    }
    
    int iNumDevices;
    iNumDevices = Pa_GetDeviceCount();
    if(iNumDevices < 0)
    {
        qDebug() << "ERROR: Pa_CountDevices returned" << iNumDevices;
        return;
    }

    const PaDeviceInfo* deviceInfo;
    for (int i = 0; i < iNumDevices; i++)
    {
        deviceInfo = Pa_GetDeviceInfo(i);
        /* deviceInfo fields for quick reference:
            int 	structVersion
            const char * 	name
            PaHostApiIndex 	hostApi
            int 	maxInputChannels
            int 	maxOutputChannels
            PaTime 	defaultLowInputLatency
            PaTime 	defaultLowOutputLatency
            PaTime 	defaultHighInputLatency
            PaTime 	defaultHighOutputLatency
            double 	defaultSampleRate
         */
        const PaHostApiInfo * apiInfo = NULL;
        apiInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
        //if (apiInfo->name == m_hostAPI)
        {
            SoundDevicePortAudio *currentDevice = new SoundDevicePortAudio(m_pConfig, this, deviceInfo, i);
            m_devices.push_back((SoundDevice*)currentDevice);
        }
    }


#endif
}

//Attempt to set up some sane default sound device settings.
//The parameters control what stuff gets set to the defaults.other
void SoundManager::setDefaults(bool api, bool devices, bool other)
{
    qDebug() << "SoundManager: Setting defaults";

    QList<QString> apiList = getHostAPIList();

    if (api && !apiList.isEmpty())
    {
#ifdef __LINUX__
        //Check for JACK and use that if it's available, otherwise use ALSA
        if (apiList.contains(MIXXX_PORTAUDIO_JACK_STRING))
            setHostAPI(MIXXX_PORTAUDIO_JACK_STRING);
        else
            setHostAPI(MIXXX_PORTAUDIO_ALSA_STRING);
#endif
#ifdef __WINDOWS__
//TODO: Check for ASIO and use that if it's available, otherwise use DirectSound
//        if (apiList.contains(MIXXX_PORTAUDIO_ASIO_STRING))
//            setHostAPI(MIXXX_PORTAUDIO_ASIO_STRING);
//        else
//Existence of ASIO doesn't necessarily mean you've got ASIO devices
//Do something more advanced one day if you like - Adam
            setHostAPI(MIXXX_PORTAUDIO_DIRECTSOUND_STRING);
#endif
#ifdef __APPLE__
        setHostAPI(MIXXX_PORTAUDIO_COREAUDIO_STRING);
#endif
    }

    if (devices)
    {
        //Set the default master device to be the first ouput device in the list (that matches the API)
		QList<SoundDevice *> qlistAPI = getDeviceList(getHostAPI(), true, false);
		if(! qlistAPI.isEmpty())
		{
			m_pConfig->set(ConfigKey("[Soundcard]","DeviceMaster"), ConfigValue(qlistAPI.front()->getInternalName()));
			m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"), ConfigValue(qlistAPI.front()->getInternalName()));
			m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"), ConfigValue(qlistAPI.front()->getInternalName()));
			m_pConfig->set(ConfigKey("[Soundcard]","ChannelMaster"), ConfigValue(QString::number(0)));
			m_pConfig->set(ConfigKey("[Soundcard]","ChannelHeadphones"), ConfigValue(QString::number(2)));
		}
    }

    if (other)
    {
        //Default samplerate, latency
        m_pConfig->set(ConfigKey("[Soundcard]","Samplerate"), ConfigValue(48000));
        m_pConfig->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(46));
    }
}

//Opens all the devices chosen by the user in the preferences dialog, and establishes
//the proper connections between them and the mixing engine.
int SoundManager::setupDevices()
{
    qDebug() << "SoundManager::setupDevices()";
    int err = 0;
    bool bNeedToOpenDeviceForOutput = 0;
    bool bNeedToOpenDeviceForInput = 0;
    QListIterator<SoundDevice *> deviceIt(m_devices);
    SoundDevice * device;

    //Set sound scale method
    if (m_pMaster)
        m_pMaster->setPitchIndpTimeStretch(m_pConfig->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt());

#ifdef __VINYLCONTROL__
    //Initialize vinyl control
    m_VinylControl[0] = new VinylControlProxy(m_pConfig, "[Channel1]");
    m_VinylControl[1] = new VinylControlProxy(m_pConfig, "[Channel2]");
#endif

    while (deviceIt.hasNext())
    {
        device = deviceIt.next();
        bNeedToOpenDeviceForOutput = 0;
        bNeedToOpenDeviceForInput = 0;

        //Close the device in case it was open.
        device->close();

        //Disconnect the device from any sources/receivers.
        device->clearSources();
        device->clearReceivers();

        //Connect the mixing engine's sound output(s) to the soundcard(s).

        if (m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMaster")) == device->getInternalName())
        {
			AudioSource src;
			src.channelBase = m_pConfig->getValueString(ConfigKey("[Soundcard]", "ChannelMaster")).toInt();
			src.channels = 2;	//TODO: Should we have a mono option?  Surround sound mixing might be cool...
			src.type = SOURCE_MASTER;

            err = device->addSource(src);
            if (err != 0)
                return err;
            bNeedToOpenDeviceForOutput = 1;
        }
        if (m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadphones")) == device->getInternalName())
        {
            AudioSource src;
			src.channelBase = m_pConfig->getValueString(ConfigKey("[Soundcard]", "ChannelHeadphones")).toInt();
			src.channels = 2;
			src.type = SOURCE_HEADPHONES;

			err = device->addSource(src);
			if (err != 0)
                return err;
            bNeedToOpenDeviceForOutput = 1;
        }

        //Connect the soundcard's inputs to the Engine.
        if (m_pConfig->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck1"))  == device->getInternalName())
        {
            AudioReceiver recv;
			recv.channelBase = m_pConfig->getValueString(ConfigKey("[VinylControl]", "ChannelInputDeck1")).toInt();
			recv.channels = 2;
			recv.type = RECEIVER_VINYLCONTROL_ONE;

            err = device->addReceiver(recv);
            if (err != 0)
                return err;
            bNeedToOpenDeviceForInput = 1;
        }
        if (m_pConfig->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck2")) == device->getInternalName())
        {
            AudioReceiver recv;
			recv.channelBase = m_pConfig->getValueString(ConfigKey("[VinylControl]", "ChannelInputDeck2")).toInt();
			recv.channels = 2;
			recv.type = RECEIVER_VINYLCONTROL_TWO;

            err = device->addReceiver(recv);
            if (err != 0)
                return err;
            bNeedToOpenDeviceForInput = 1;
        }

        //Open the device.
        if (bNeedToOpenDeviceForOutput || bNeedToOpenDeviceForInput)
        {
            err = device->open();
            if (err != 0)
                return err;
            else
            {
                iNumDevicesOpenedForOutput += (int)bNeedToOpenDeviceForOutput;
                iNumDevicesOpenedForInput += (int)bNeedToOpenDeviceForInput;
            }
        }
    }

    qDebug() << "iNumDevicesOpenedForOutput:" << iNumDevicesOpenedForOutput;
    qDebug() << "iNumDevicesOpenedForInput:" << iNumDevicesOpenedForInput;

    //Returns non-zero if we have no output devices
    return (iNumDevicesOpenedForOutput == 0);
}

void SoundManager::sync()
{
    ControlObject::sync();
    //qDebug() << "sync";

}

//Requests a buffer in the proper format, if we're prepared to give one.
CSAMPLE ** SoundManager::requestBuffer(QList<AudioSource> srcs, unsigned long iFramesPerBuffer)
{
    //qDebug() << "SoundManager::requestBuffer()";

    //qDebug() << "numOpenedDevices" << iNumOpenedDevices;
    //qDebug() << "iNumDevicesHaveRequestedBuffer" << iNumDevicesHaveRequestedBuffer;

    //When the first device requests a buffer...
    if (requestBufferMutex.tryLock())
    {
        if (iNumDevicesHaveRequestedBuffer == 0)
        {
            //First, sync control parameters with changes from GUI thread
            sync();

            //Process a block of samples for output. iFramesPerBuffer is the
            //number of samples for one channel, but the EngineObject
            //architecture expects number of samples for two channels
            //as input (buffer size) so...
            m_pMaster->process(0, 0, iFramesPerBuffer*2);

        }
        iNumDevicesHaveRequestedBuffer++;

        if (iNumDevicesHaveRequestedBuffer >= iNumDevicesOpenedForOutput)
            iNumDevicesHaveRequestedBuffer = 0;

        requestBufferMutex.unlock();
    }
	return m_pStreamBuffers;
}

//Used by SoundDevices to "push" any audio from their inputs that they have into the mixing engine.
CSAMPLE * SoundManager::pushBuffer(QList<AudioReceiver> recvs, short * inputBuffer,
                                   unsigned long iFramesPerBuffer, unsigned int iFrameSize)
{
    short* vinylControlBuffer1 = NULL; /** Pointer to the buffer containing the vinyl control audio for deck 1*/
    short* vinylControlBuffer2 = NULL; /** Pointer to the buffer containing the vinyl control audio for deck 1*/

//    m_pReceiverBuffers[RECEIVER_VINYLCONTROL_ONE]

    //short vinylControlBuffer1[iFramesPerBuffer * 2];
    //short vinylControlBuffer2[iFramesPerBuffer * 2];
    //short *vinylControlBuffer1 = (short*) alloca(iFramesPerBuffer * 2 * sizeof(short));
    //short *vinylControlBuffer2 = (short*) alloca(iFramesPerBuffer * 2 * sizeof(short));

    //memset(vinylControlBuffer1, 0, iFramesPerBuffer * iFrameSize * sizeof(*vinylControlBuffer1));

    /** If the framesize is only 2, then we only have one pair of input channels
     *  That means we don't have to do any deinterlacing, and we can pass
     *  the audio on to its intended destination. */
    if (iFrameSize == 2)
    {
        vinylControlBuffer1 = inputBuffer;
        vinylControlBuffer2 = inputBuffer;
    }

/*
    //If we have two stereo input streams (interlaced as one), then
    //break them up into two separate interlaced streams
    if (iFrameSize == 4)
    {
        for (int i = 0; i < iFramesPerBuffer; i++) //For each frame of audio
        {
            m_pReceiverBuffers[RECEIVER_VINYLCONTROL_ONE][i*2    ] = inputBuffer[i*iFrameSize    ];
            m_pReceiverBuffers[RECEIVER_VINYLCONTROL_ONE][i*2 + 1] = inputBuffer[i*iFrameSize + 1];
            m_pReceiverBuffers[RECEIVER_VINYLCONTROL_TWO][i*2    ] = inputBuffer[i*iFrameSize + 2];
            m_pReceiverBuffers[RECEIVER_VINYLCONTROL_TWO][i*2 + 1] = inputBuffer[i*iFrameSize + 3];
        }
        //Set the pointers to point to the de-interlaced input audio
        vinylControlBuffer1 = m_pReceiverBuffers[RECEIVER_VINYLCONTROL_ONE];
        vinylControlBuffer2 = m_pReceiverBuffers[RECEIVER_VINYLCONTROL_TWO];
    }
*/
    else { //More than two channels of input (iFrameSize > 2)

        //Do crazy deinterleaving of the audio into the correct m_pReceiverBuffers.

        //iFrameBase is the "base sample" in a frame (ie. the first sample in a frame)
        for (unsigned int iFrameBase=0; iFrameBase < iFramesPerBuffer*iFrameSize; iFrameBase += iFrameSize)
        {
			//Deinterlace the input audio data from the portaudio buffer
			//We iterate through the receiver list to find out what goes into each buffer.
			//Data is deinterlaced in the order of the list
			QListIterator<AudioReceiver> devItr(recvs);
			int iChannel;
			while(devItr.hasNext())
			{
				AudioReceiver recv = devItr.next();
				int iLocalFrameBase = (iFrameBase/iFrameSize) * recv.channels;
				for(iChannel = 0; iChannel < recv.channels; iChannel++)	//this will make sure a sample from each channel is copied
				{
					//output[iFrameBase + src.channelBase + iChannel] += outputAudio[src.type][iLocalFrameBase + iChannel] * SHRT_CONVERSION_FACTOR;
			        m_pReceiverBuffers[recv.type][iLocalFrameBase + iChannel] = inputBuffer[iFrameBase + recv.channelBase + iChannel];
                }
			}
        }
        //Set the pointers to point to the de-interlaced input audio
        vinylControlBuffer1 = m_pReceiverBuffers[RECEIVER_VINYLCONTROL_ONE];
        vinylControlBuffer2 = m_pReceiverBuffers[RECEIVER_VINYLCONTROL_TWO];
    }

/*

*/


    if (inputBuffer)
    {
#ifdef __VINYLCONTROL__
        QListIterator<AudioReceiver> devItr(recvs);
        while(devItr.hasNext())
        {
            AudioReceiver recv = devItr.next();
            if (recv.type == RECEIVER_VINYLCONTROL_ONE)
            {
                //recv.channelBase
                Q_ASSERT(recv.channels == 2); //Stereo data is needed for vinyl control
                if (m_VinylControl[0])
                    m_VinylControl[0]->AnalyseSamples(vinylControlBuffer1, iFramesPerBuffer);
                if (m_pConfig->getValueString(ConfigKey("[VinylControl]", "SingleDeckEnable")).toInt())
                {
                	if (m_VinylControl[1])
                    	m_VinylControl[1]->AnalyseSamples(vinylControlBuffer1, iFramesPerBuffer);
                }
            }
            if (recv.type == RECEIVER_VINYLCONTROL_TWO)
            {
                if (!m_pConfig->getValueString(ConfigKey("[VinylControl]", "SingleDeckEnable")).toInt())
            	{
	                Q_ASSERT(recv.channels == 2); //Stereo data is needed for vinyl control
	                if (m_VinylControl[1])
	                    m_VinylControl[1]->AnalyseSamples(vinylControlBuffer2, iFramesPerBuffer);
	            }
            }
        }
#endif
    }
    //TODO: Add pass-through option here (and push it into EngineMaster)...
    //      (or maybe save it, and then have requestBuffer() push it into EngineMaster)...


    return NULL; //FIXME: Return void instead?
}


