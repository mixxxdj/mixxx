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
#include <cstring> // for memcpy
#include "soundmanager.h"
#include "sounddevice.h"
#include "sounddeviceportaudio.h"
#include "engine/enginemaster.h"
#include "controlobjectthreadmain.h"
#include "audiopath.h"

/** Initializes Mixxx's audio core
 *  @param pConfig The config key table
 *  @param _master A pointer to the audio engine's mastering class.
 */
SoundManager::SoundManager(ConfigObject<ConfigValue> * pConfig, EngineMaster * _master)
    : QObject()
{
    //qDebug() << "SoundManager::SoundManager()";
    m_pConfig = pConfig;
    m_pMaster = _master;

    iNumDevicesOpenedForOutput = 0;
    iNumDevicesOpenedForInput = 0;
    iNumDevicesHaveRequestedBuffer = 0;

    //Hack because PortAudio samplerate enumeration is slow as hell on Linux (ALSA dmix sucks, so we can't blame PortAudio)
    // this needs to be up here so SoundManagerConfig can get at the list (populated) -- bkgood
    m_samplerates.push_back(44100);
    m_samplerates.push_back(48000);
    m_samplerates.push_back(96000);

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
    ControlObjectThreadMain* pControlObjectVinylControlEnabled = new ControlObjectThreadMain(new ControlObject(ConfigKey("[VinylControl]", "Enabled")));
    ControlObjectThreadMain* pControlObjectVinylControlGain = new ControlObjectThreadMain(new ControlObject(ConfigKey("[VinylControl]", "VinylControlGain")));
    ControlObjectThreadMain* pControlObjectVinylControlSignalQuality1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]", "VinylControlQuality")));
    ControlObjectThreadMain* pControlObjectVinylControlSignalQuality2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]", "VinylControlQuality")));
    ControlObjectThreadMain* pControlObjectVinylControlInputStrengthL1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]", "VinylControlInputL")));
    ControlObjectThreadMain* pControlObjectVinylControlInputStrengthR1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]", "VinylControlInputR")));
    ControlObjectThreadMain* pControlObjectVinylControlInputStrengthL2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]", "VinylControlInputL")));
    ControlObjectThreadMain* pControlObjectVinylControlInputStrengthR2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]", "VinylControlInputR")));

    pControlObjectLatency->slotSet(m_pConfig->getValueString(ConfigKey("[Soundcard]","Latency")).toInt());
    pControlObjectSampleRate->slotSet(m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt());
    pControlObjectVinylControlMode->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","Mode")).toInt());
    pControlObjectVinylControlEnabled->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","Enabled")).toInt());
    pControlObjectVinylControlGain->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","VinylControlGain")).toInt());


    qDebug() << "SampleRate" << pControlObjectSampleRate->get();
    qDebug() << "Latency" << pControlObjectLatency->get();

}

/** Destructor for the SoundManager class. Closes all the devices, cleans up their pointers
  and terminates PortAudio. */
SoundManager::~SoundManager()
{
    //TODO: Should only call Pa_Terminate() if Pa_Inititialize() was successful.

    //Clean up devices.
    clearDeviceList();

    Pa_Terminate();
    // vinyl control proxies and input buffers are freed in closeDevices, called
    // by clearDeviceList -- bkgood
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

    m_sourceBuffers.clear(); // anti-cruft (safe because sources only have
                             // pointers to memory owned by EngineMaster)

    foreach (AudioReceiver recv, m_receiverBuffers.keys()) {
        short *buffer = m_receiverBuffers[recv];
        if (buffer != NULL) {
            delete [] buffer;
            m_receiverBuffers[recv] = buffer = NULL;
        }
    }
    m_receiverBuffers.clear();

#ifdef __VINYLCONTROL__
    // TODO see comment where these objects are created in setupDevices,
    // this should probably be in the dtor or at least somewhere other
    // than here -- bkgood
    while (!m_VinylControl.empty()) {
        VinylControlProxy *vc = m_VinylControl.takeLast();
        if (vc != NULL) {
            delete vc;
        }
    }
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
QList<unsigned int> SoundManager::getSampleRates()
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
        m_pConfig->set(ConfigKey("[Soundcard]","Samplerate"), ConfigValue(44100));
        m_pConfig->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(64));
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

#ifdef __VINYLCONTROL__
    //Initialize vinyl control
    // TODO this ought to be done in the ctor or something. Not here. Really
    // shouldn't be any reason for these to be reinitialized every time the
    // audio prefs are updated. Will require work in DlgPrefVinyl -- bkgood
    m_VinylControl.append(new VinylControlProxy(m_pConfig, "[Channel1]"));
    m_VinylControl.append(new VinylControlProxy(m_pConfig, "[Channel2]"));
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
            AudioSource src(
                AudioSource::MASTER,
                m_pConfig->getValueString(ConfigKey("[Soundcard]", "ChannelMaster")).toInt()
            );

            err = device->addSource(src);
            if (err != 0)
                return err;
            m_sourceBuffers[src] = m_pMaster->getMasterBuffer();
            bNeedToOpenDeviceForOutput = 1;
        }
        if (m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadphones")) == device->getInternalName())
        {
            AudioSource src(
                AudioSource::HEADPHONES,
                m_pConfig->getValueString(ConfigKey("[Soundcard]", "ChannelHeadphones")).toInt()
            );

			err = device->addSource(src);
			if (err != 0)
                return err;
            m_sourceBuffers[src] = m_pMaster->getHeadphoneBuffer();
            bNeedToOpenDeviceForOutput = 1;
        }

        //Connect the soundcard's inputs to the Engine.
        if (m_pConfig->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck1"))  == device->getInternalName())
        {
            AudioReceiver recv(
                AudioReceiver::VINYLCONTROL,
                m_pConfig->getValueString(ConfigKey("[VinylControl]", "ChannelInputDeck1")).toInt(),
                0 // first vc deck
            );

            err = device->addReceiver(recv);
            if (err != 0)
                return err;
            if (!m_receiverBuffers.contains(recv)) {
                m_receiverBuffers[recv] = new short[MAX_BUFFER_LEN];
            }
            bNeedToOpenDeviceForInput = 1;
        }
        if (m_pConfig->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck2")) == device->getInternalName())
        {
            AudioReceiver recv(
                AudioReceiver::VINYLCONTROL,
                m_pConfig->getValueString(ConfigKey("[VinylControl]", "ChannelInputDeck2")).toInt(),
                1 // second vc deck
            );

            err = device->addReceiver(recv);
            if (err != 0)
                return err;
            if (!m_receiverBuffers.contains(recv)) {
                m_receiverBuffers[recv] = new short[MAX_BUFFER_LEN];
            }
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

QString SoundManager::getHostAPI() const {
    return m_hostAPI;
}

/** Set which host API Mixxx should use.
 *  @param api The host API that you want Mixxx to use.
 */
void SoundManager::setHostAPI(QString api) {
    m_hostAPI = api;
}

float SoundManager::getSampleRate() const {
    return m_sampleRate;
}

void SoundManager::setSampleRate(float sampleRate) {
    // trust input is good? -- bkgood
    m_sampleRate = sampleRate;
}

unsigned int SoundManager::getFramesPerBuffer() const {
    return m_framesPerBuffer;
}

void SoundManager::setFramesPerBuffer(unsigned int framesPerBuffer) {
    // check that it's a power of 2 because otherwise we
    // get crashes and slowdowns (and bears, oh my) -- bkgood
    // next line is INT_MAX because I can't think of a better limit
    for (unsigned int i = 1; i < INT_MAX; i *= 2) {
        if (i >= framesPerBuffer) {
            framesPerBuffer = i;
            break;
        }
    }
    m_framesPerBuffer = framesPerBuffer;
}

/**
 * Adds an AudioSource going to the given device. Doesn't take effect
 * until setupDevices(). Not thread safe, only intended to be called
 * from the main thread.
 */
void SoundManager::addSource(SoundDevice *device, AudioSource source) {
    m_audioSources.append(QPair<SoundDevice*, AudioSource>(device, source));
}

/**
 * Adds an AudioReceiver going to the given device. Doesn't take effect
 * until setupDevices(). Not thread safe, only intended to be called
 * from the main thread.
 */
void SoundManager::addReceiver(SoundDevice *device, AudioReceiver receiver) {
    m_audioReceivers.append(QPair<SoundDevice*, AudioReceiver>(device, receiver));
}

void SoundManager::sync()
{
    ControlObject::sync();
    //qDebug() << "sync";

}

//Requests a buffer in the proper format, if we're prepared to give one.
QHash<AudioSource, const CSAMPLE*>
SoundManager::requestBuffer(QList<AudioSource> srcs, unsigned long iFramesPerBuffer)
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
	return m_sourceBuffers;
}

//Used by SoundDevices to "push" any audio from their inputs that they have into the mixing engine.
void SoundManager::pushBuffer(QList<AudioReceiver> recvs, short * inputBuffer,
                              unsigned long iFramesPerBuffer, unsigned int iFrameSize)
{
//    m_receiverBuffers[RECEIVER_VINYLCONTROL_ONE]

    //short vinylControlBuffer1[iFramesPerBuffer * 2];
    //short vinylControlBuffer2[iFramesPerBuffer * 2];
    //short *vinylControlBuffer1 = (short*) alloca(iFramesPerBuffer * 2 * sizeof(short));
    //short *vinylControlBuffer2 = (short*) alloca(iFramesPerBuffer * 2 * sizeof(short));

    //memset(vinylControlBuffer1, 0, iFramesPerBuffer * iFrameSize * sizeof(*vinylControlBuffer1));

    // IMPORTANT -- Mixxx should ALWAYS be the owner of whatever input buffer we're using,
    // otherwise we double-free (well, PortAudio frees and then we free) and everything
    // goes to hell -- bkgood

    /** If the framesize is only 2, then we only have one pair of input channels
     *  That means we don't have to do any deinterlacing, and we can pass
     *  the audio on to its intended destination. */
    // this special casing is probably not worth keeping around. It had a speed
    // advantage before because it just assigned a pointer instead of copying data,
    // but this meant we couldn't free all the receiver buffer pointers, because some
    // of them might potentially be owned by portaudio. Not freeing them means we leak
    // memory in certain cases -- bkgood
    if (iFrameSize == 2)
    {
        QListIterator<AudioReceiver> recvItr(recvs);
        while (recvItr.hasNext()) {
            AudioReceiver recv = recvItr.next();
            if (recv.getType() == AudioReceiver::VINYLCONTROL) {
                memcpy(m_receiverBuffers[recv], inputBuffer,
                        sizeof(*inputBuffer) * iFrameSize * iFramesPerBuffer);
            }
        }
    }

/*
    //If we have two stereo input streams (interlaced as one), then
    //break them up into two separate interlaced streams
    if (iFrameSize == 4)
    {
        for (int i = 0; i < iFramesPerBuffer; i++) //For each frame of audio
        {
            m_receiverBuffers[RECEIVER_VINYLCONTROL_ONE][i*2    ] = inputBuffer[i*iFrameSize    ];
            m_receiverBuffers[RECEIVER_VINYLCONTROL_ONE][i*2 + 1] = inputBuffer[i*iFrameSize + 1];
            m_receiverBuffers[RECEIVER_VINYLCONTROL_TWO][i*2    ] = inputBuffer[i*iFrameSize + 2];
            m_receiverBuffers[RECEIVER_VINYLCONTROL_TWO][i*2 + 1] = inputBuffer[i*iFrameSize + 3];
        }
        //Set the pointers to point to the de-interlaced input audio
        vinylControlBuffer1 = m_receiverBuffers[RECEIVER_VINYLCONTROL_ONE];
        vinylControlBuffer2 = m_receiverBuffers[RECEIVER_VINYLCONTROL_TWO];
    }
*/
    else { //More than two channels of input (iFrameSize > 2)
        //Do crazy deinterleaving of the audio into the correct m_receiverBuffers.
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
                ChannelGroup chanGroup = recv.getChannelGroup();
				int iLocalFrameBase = (iFrameBase/iFrameSize) * chanGroup.getChannelCount();

				for (iChannel = 0; iChannel < chanGroup.getChannelCount(); iChannel++) //this will make sure a sample from each channel is copied
				{
					//output[iFrameBase + src.channelBase + iChannel] += outputAudio[src.type][iLocalFrameBase + iChannel] * SHRT_CONVERSION_FACTOR;
			        m_receiverBuffers[recv][iLocalFrameBase + iChannel] = inputBuffer[iFrameBase + chanGroup.getChannelBase() + iChannel];
                }
			}
        }
    }

    if (inputBuffer)
    {
#ifdef __VINYLCONTROL__
        QListIterator<AudioReceiver> devItr(recvs);
        while (devItr.hasNext())
        {
            AudioReceiver recv = devItr.next();
            if (recv.getType() == AudioReceiver::VINYLCONTROL) {
                unsigned int index = recv.getIndex();
                Q_ASSERT(index < 2); // XXX we only do two vc decks atm -- bkgood
                if (m_VinylControl[index] && m_receiverBuffers.contains(recv)) {
                    m_VinylControl[index]->AnalyseSamples(m_receiverBuffers[recv], iFramesPerBuffer);
                }
            }
        }
#endif
    }
    //TODO: Add pass-through option here (and push it into EngineMaster)...
    //      (or maybe save it, and then have requestBuffer() push it into EngineMaster)...
}
