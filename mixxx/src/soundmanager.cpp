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

    pControlObjectVinylControlMode->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","Mode")).toInt());
    pControlObjectVinylControlEnabled->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","Enabled")).toInt());
    pControlObjectVinylControlGain->slotSet(m_pConfig->getValueString(ConfigKey("[VinylControl]","VinylControlGain")).toInt());

    //Hack because PortAudio samplerate enumeration is slow as hell on Linux (ALSA dmix sucks, so we can't blame PortAudio)
    m_samplerates.push_back(44100);
    m_samplerates.push_back(48000);
    m_samplerates.push_back(96000);

    queryDevices(); // initializes PortAudio so SMConfig:loadDefaults can do
                    // its thing if it needs to

    if (!m_config.readFromDisk()) {
        m_config.loadDefaults(this, SoundManagerConfig::ALL);
    }
    checkConfig();
    m_config.writeToDisk(); // in case anything changed by applying defaults

    // TODO(bkgood) do these really need to be here? they're set in
    // SoundDevicePortAudio::open
    pControlObjectLatency->slotSet(m_config.getFramesPerBuffer() / m_config.getSampleRate() * 1000);
    pControlObjectSampleRate->slotSet(m_config.getSampleRate());
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
QList<QString> SoundManager::getHostAPIList() const
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

    m_outputBuffers.clear(); // anti-cruft (safe because outputs only have
                             // pointers to memory owned by EngineMaster)

    foreach (AudioInput in, m_inputBuffers.keys()) {
        short *buffer = m_inputBuffers[in];
        if (buffer != NULL) {
            delete [] buffer;
            m_inputBuffers[in] = buffer = NULL;
        }
    }
    m_inputBuffers.clear();

#ifdef __VINYLCONTROL__
    // TODO(bkgood) see comment where these objects are created in setupDevices,
    // this should probably be in the dtor or at least somewhere other
    // than here.
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
QList<unsigned int> SoundManager::getSampleRates() const
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
    // now tell the prefs that we updated the device list -- bkgood
    emit(devicesUpdated());
#endif
}

//Opens all the devices chosen by the user in the preferences dialog, and establishes
//the proper connections between them and the mixing engine.
int SoundManager::setupDevices()
{
    qDebug() << "SoundManager::setupDevices()";
    int err = 0;
    iNumDevicesOpenedForOutput = iNumDevicesOpenedForInput = 0;
    int devicesAttempted = 0;
    int devicesOpened = 0;

    // close open devices, close running vinyl control proxies
    closeDevices();
#ifdef __VINYLCONTROL__
    //Initialize vinyl control
    // TODO(bkgood) this ought to be done in the ctor or something. Not here. Really
    // shouldn't be any reason for these to be reinitialized every time the
    // audio prefs are updated. Will require work in DlgPrefVinyl.
    m_VinylControl.append(new VinylControlProxy(m_pConfig, "[Channel1]"));
    m_VinylControl.append(new VinylControlProxy(m_pConfig, "[Channel2]"));
#endif
    foreach (SoundDevice *device, m_devices) {
        bool isInput = false;
        bool isOutput = false;
        device->clearInputs();
        device->clearOutputs();
        foreach (AudioInput in, m_config.getInputs().values(device->getInternalName())) {
            isInput = true;
            err = device->addInput(in);
            if (err != OK)
                return err;
            if (!m_inputBuffers.contains(in)) {
                // TODO(bkgood) look into allocating this with the frames per
                // buffer value from SMConfig
                m_inputBuffers[in] = new short[MAX_BUFFER_LEN];
            }
        }
        foreach (AudioOutput out, m_config.getOutputs().values(device->getInternalName())) {
            isOutput = true;
            // following keeps us from asking for a channel buffer EngineMaster
            // doesn't have -- bkgood
            if (out.getType() == AudioOutput::DECK
                    && out.getIndex() >= m_pMaster->numChannels()) continue;
            err = device->addOutput(out);
            if (err != OK)
                return err;
            // TODO(bkgood) this would be nicer as something like
            // EngineMaster::getBuffer(AudioPathType type, uint index = 0);
            // but I don't want to mess with enginemaster if I can help it
            // before hydra's merged
            switch (out.getType()) {
            case AudioPath::MASTER:
                m_outputBuffers[out] = m_pMaster->getMasterBuffer();
                break;
            case AudioPath::HEADPHONES:
                m_outputBuffers[out] = m_pMaster->getHeadphoneBuffer();
                break;
            case AudioPath::DECK:
                m_outputBuffers[out] = m_pMaster->getChannelBuffer(out.getIndex());
                break;
            default:
                break;
            }
        }
        if (isInput || isOutput) {
            device->setSampleRate(m_config.getSampleRate());
            device->setFramesPerBuffer(m_config.getFramesPerBuffer());
            ++devicesAttempted;
            err = device->open();
            if (err != OK) {
                return err;
            } else {
                ++devicesOpened;
                if (isOutput)
                    ++iNumDevicesOpenedForOutput;
                if (isInput)
                    ++iNumDevicesOpenedForInput;
            }
        }
    }

    qDebug() << "iNumDevicesOpenedForOutput:" << iNumDevicesOpenedForOutput;
    qDebug() << "iNumDevicesOpenedForInput:" << iNumDevicesOpenedForInput;

    // returns OK if we were able to open all the devices the user
    // wanted
    if (devicesAttempted == devicesOpened) {
        return OK;
    }
    return ERR;
}

SoundManagerConfig SoundManager::getConfig() const {
    return m_config;
}

int SoundManager::setConfig(SoundManagerConfig config) {
    int err = OK;
    m_config = config;
    checkConfig();
    err = setupDevices();
    if (err == OK) {
        m_config.writeToDisk();
    }
    // certain parts of mixxx rely on this being here, for the time being, just
    // letting those be -- bkgood
    m_pConfig->set(ConfigKey("[Soundcard]","Samplerate"), ConfigValue(m_config.getSampleRate()));
    return err;
}

void SoundManager::checkConfig() {
    if (!m_config.checkAPI(*this)) {
        m_config.setAPI(DEFAULT_API);
        m_config.loadDefaults(this, SoundManagerConfig::API | SoundManagerConfig::DEVICES);
    }
    if (!m_config.checkSampleRate(*this)) {
        m_config.setSampleRate(DEFAULT_SAMPLE_RATE);
        m_config.loadDefaults(this, SoundManagerConfig::OTHER);
    }
    // latency checks itself for validity on SMConfig::setLatency()
}

void SoundManager::sync()
{
    ControlObject::sync();
    //qDebug() << "sync";

}

//Requests a buffer in the proper format, if we're prepared to give one.
QHash<AudioOutput, const CSAMPLE*>
SoundManager::requestBuffer(QList<AudioOutput> outputs, unsigned long iFramesPerBuffer)
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
    return m_outputBuffers;
}

//Used by SoundDevices to "push" any audio from their inputs that they have into the mixing engine.
void SoundManager::pushBuffer(QList<AudioInput> inputs, short * inputBuffer,
                              unsigned long iFramesPerBuffer, unsigned int iFrameSize)
{
//    m_inputBuffers[RECEIVER_VINYLCONTROL_ONE]

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
        QListIterator<AudioInput> inputItr(inputs);
        while (inputItr.hasNext()) {
            AudioInput in = inputItr.next();
            if (in.getType() == AudioInput::VINYLCONTROL) {
                memcpy(m_inputBuffers[in], inputBuffer,
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
            m_inputBuffers[RECEIVER_VINYLCONTROL_ONE][i*2    ] = inputBuffer[i*iFrameSize    ];
            m_inputBuffers[RECEIVER_VINYLCONTROL_ONE][i*2 + 1] = inputBuffer[i*iFrameSize + 1];
            m_inputBuffers[RECEIVER_VINYLCONTROL_TWO][i*2    ] = inputBuffer[i*iFrameSize + 2];
            m_inputBuffers[RECEIVER_VINYLCONTROL_TWO][i*2 + 1] = inputBuffer[i*iFrameSize + 3];
        }
        //Set the pointers to point to the de-interlaced input audio
        vinylControlBuffer1 = m_inputBuffers[RECEIVER_VINYLCONTROL_ONE];
        vinylControlBuffer2 = m_inputBuffers[RECEIVER_VINYLCONTROL_TWO];
    }
*/
    else { //More than two channels of input (iFrameSize > 2)
        //Do crazy deinterleaving of the audio into the correct m_inputBuffers.
        //iFrameBase is the "base sample" in a frame (ie. the first sample in a frame)
        for (unsigned int iFrameBase=0; iFrameBase < iFramesPerBuffer*iFrameSize; iFrameBase += iFrameSize)
        {
            //Deinterlace the input audio data from the portaudio buffer
            //We iterate through the receiver list to find out what goes into each buffer.
            //Data is deinterlaced in the order of the list
            QListIterator<AudioInput> inputItr(inputs);
            int iChannel;
            while (inputItr.hasNext())
            {
                AudioInput in = inputItr.next();
                ChannelGroup chanGroup = in.getChannelGroup();
                int iLocalFrameBase = (iFrameBase/iFrameSize) * chanGroup.getChannelCount();

                for (iChannel = 0; iChannel < chanGroup.getChannelCount(); iChannel++)
                    //this will make sure a sample from each channel is copied
                {
                    //output[iFrameBase + src.channelBase + iChannel] +=
                    //  outputAudio[src.type][iLocalFrameBase + iChannel] * SHRT_CONVERSION_FACTOR;
                    m_inputBuffers[in][iLocalFrameBase + iChannel] =
                        inputBuffer[iFrameBase + chanGroup.getChannelBase() + iChannel];
                }
            }
        }
    }

    if (inputBuffer)
    {
#ifdef __VINYLCONTROL__
        QListIterator<AudioInput> inputItr(inputs);
        while (inputItr.hasNext())
        {
            AudioInput in = inputItr.next();
            if (in.getType() == AudioInput::VINYLCONTROL) {
                unsigned int index = in.getIndex();
                Q_ASSERT(index < 2); // XXX we only do two vc decks atm -- bkgood
                if (m_VinylControl[index] && m_inputBuffers.contains(in)) {
                    m_VinylControl[index]->AnalyseSamples(m_inputBuffers[in], iFramesPerBuffer);
                }
            }
        }
#endif
    }
    //TODO: Add pass-through option here (and push it into EngineMaster)...
    //      (or maybe save it, and then have requestBuffer() push it into EngineMaster)...
}
