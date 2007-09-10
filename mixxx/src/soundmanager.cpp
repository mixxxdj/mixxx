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
#include "enginemaster.h"


SoundManager::SoundManager(ConfigObject<ConfigValue> * pConfig, EngineMaster * _master) : QObject()
{
    qDebug() << "SoundManager::SoundManager()";
    m_pConfig = pConfig;
    m_pMaster = _master;
    m_pBuffer = new CSAMPLE[MAX_BUFFER_LEN];
    m_pMasterBuffer = new CSAMPLE[MAX_BUFFER_LEN];
    m_pHeadphonesBuffer = new CSAMPLE[MAX_BUFFER_LEN];
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

    ControlObject * pControlObjectLatency  = ControlObject::getControl(ConfigKey("[Master]","latency"));
    ControlObject * pControlObjectSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));

    pControlObjectLatency->set(m_pConfig->getValueString(ConfigKey("[Soundcard]","Latency")).toInt());
    pControlObjectSampleRate->set(m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt());

    qDebug() << "SampleRate" << pControlObjectSampleRate->get();
    qDebug() << "Latency" << pControlObjectLatency->get();

    //Hack because PortAudio samplerate enumeration is slow as hell on Linux
    m_samplerates.push_back("44100");
    m_samplerates.push_back("48000");
    m_samplerates.push_back("96000"); 

#ifdef __PORTAUDIO__
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        qDebug() << "Error:" << Pa_GetErrorText(err);
    }
#endif
}

//Destructor for the SoundManager class. Closes all the devices, cleans up their pointers
//and terminates PortAudio.
SoundManager::~SoundManager()
{
    //TODO: Should only call Pa_Terminate() if Pa_Inititialize() was successful.

    //Clean up devices.
    clearDeviceList();

    Pa_Terminate();
}

//Returns a list of all the devices we've enumerated through PortAudio.
//If filterAPI is the name of an audio API used by PortAudio, this function
//will only return devices that belong to that API. Otherwise, the list will
//contain all devices on all PortAudio-supported APIs.
QList<SoundDevice *> SoundManager::getDeviceList(QString filterAPI)
{
    qDebug() << "SoundManager::getDeviceList";
    if (m_devices.empty())
        this->queryDevices();

    if (filterAPI == "None")
    {
        QList<SoundDevice *> emptyList;
        return emptyList;
    }
    else
    {
        //Create a list of sound devices filtered to the given API.
        QList<SoundDevice *> filteredDeviceList;
        QListIterator<SoundDevice *> dev_it(m_devices);
        while (dev_it.hasNext())
        {
            SoundDevice * device = dev_it.next();
            if (device->getHostAPI() == filterAPI)
                filteredDeviceList.push_back(device);
        }
        return filteredDeviceList;
    }

    return m_devices;
}

//Returns a list of host APIs supported by PortAudio.
QList<QString> SoundManager::getHostAPIList()
{
    QList<QString> apiList;

    for (PaHostApiIndex i = 0; i < Pa_GetHostApiCount(); i++)
    {
        const PaHostApiInfo * api = Pa_GetHostApiInfo(i);
        apiList.push_back(api->name);
    }

    return apiList;
}

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

//Closes all the open sound devices.
void SoundManager::closeDevices()
{
    qDebug() << "SoundManager::closeDevices()";
    QListIterator<SoundDevice *> dev_it(m_devices);
    while (dev_it.hasNext())
    {
        //qDebug() << "closing a device...";
        dev_it.next()->close();
    }
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

//Closes all the devices and empties the list of devices we have.
void SoundManager::clearDeviceList()
{
    qDebug() << "SoundManager::clearDeviceList()";

    //Close the devices first.
    closeDevices();

    //Empty out the list of devices we currently have.
    while (!m_devices.empty())
    {
        SoundDevice * dev = m_devices.takeLast();
        delete dev;
    }
}

//Returns a list of samplerates we will attempt to support.
QList<QString> SoundManager::getSamplerateList()
{
    return m_samplerates;
}

//Creates a list of sound devices that PortAudio sees.
void SoundManager::queryDevices()
{
    qDebug() << "SoundManager::queryDevices()";
    clearDeviceList();

#ifdef __PORTAUDIO__
    int iNumDevices;
    iNumDevices = Pa_GetDeviceCount();
    if(iNumDevices < 0)
    {
        qDebug() << "ERROR: Pa_CountDevices returned" << iNumDevices;
        return;
    }

    const PaDeviceInfo * deviceInfo;
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
            SoundDevicePortAudio * currentDevice = new SoundDevicePortAudio(m_pConfig, this, deviceInfo, i);
            m_devices.push_back((SoundDevice *)currentDevice);
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

    if (api)
    {
#ifdef __LINUX__
        //Check for JACK and use that if it's available, otherwise use ALSA
        if (apiList.contains("JACK Audio Connection Kit"))
            setHostAPI("JACK Audio Connection Kit");
        else
            setHostAPI("ALSA");
#endif
#ifdef __WIN32__
//TODO: Check for ASIO and use that if it's available, otherwise use DirectSound
        if (apiList.contains("ASIO"))
            setHostAPI("ASIO");
        else
            setHostAPI("Windows DirectSound");
#endif
#ifdef __OSX__
        setHostAPI("CoreAudio");
#endif
    }

    if (devices)
    {
        //Set the default master device to be the first device in the list (that matches the API)
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMaster"), ConfigValue(getDeviceList(getHostAPI()).front()->getName()));
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"), ConfigValue(getDeviceList(getHostAPI()).front()->getName()));
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"), ConfigValue(getDeviceList(getHostAPI()).front()->getName()));
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

        if (m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMaster")) == device->getName())
        {
            device->addSource(SOURCE_MASTER);
            bNeedToOpenDeviceForOutput = 1;
        }
        if (m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadphones")) == device->getName())
        {
            device->addSource(SOURCE_HEADPHONES);
            bNeedToOpenDeviceForOutput = 1;
        }
        /*
           if ((m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterLeft")) == device->getName())
            && (m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterRight")) == device->getName()))
           {
            device->addSource(SOURCE_MASTER);
            bNeedToOpenDevice = 1;
           }
           if ((m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadLeft")) == device->getName())
            && (m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadRight")) == device->getName()))
           {
            device->addSource(SOURCE_HEADPHONES);
            bNeedToOpenDevice = 1;
           }*/

        //Connect the soundcard's inputs to the Engine.
        if (m_pConfig->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck1"))  == device->getName())
        {
            device->addReceiver(RECEIVER_VINYLCONTROL_ONE);
            bNeedToOpenDeviceForInput = 1;
        }
        if (m_pConfig->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck2")) == device->getName())
        {
            device->addReceiver(RECEIVER_VINYLCONTROL_TWO);
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

    return 0;
}

void SoundManager::sync()
{
    ControlObject::sync();
    //qDebug() << "sync";

}

//Requests a buffer in the proper format, if we're prepared to give one.
CSAMPLE * SoundManager::requestBuffer(QList<AudioSource> srcs, unsigned long iFramesPerBuffer)
{
    //qDebug() << "SoundManager::requestBuffer()";

    //qDebug() << "numOpenedDevices" << iNumOpenedDevices;
    //qDebug() << "iNumDevicesHaveRequestedBuffer" << iNumDevicesHaveRequestedBuffer;

    //When the first device requests a buffer...
    requestBufferMutex.lock();
    if (iNumDevicesHaveRequestedBuffer == 0)
    {
        //First, sync control parameters with changes from GUI thread
        sync();

        //Process a block of samples for output. iFramesPerBuffer is the
        //number of samples for one channel, but the EngineObject
        //architecture expects number of samples for two channels
        //as input (buffer size) so...
        m_pMaster->process(0, m_pBuffer, iFramesPerBuffer*2);

        //Ok, so now we've got our buffer of interlaced audio containing
        //both the master output and the headphones output.

        //FIXME? Peel the interlaced buffer apart into separate headphone and
        //master buffers, for advanced routing.
        for (int i = 0; i < MAX_BUFFER_LEN; i += 4)
        {
            m_pMasterBuffer[i/2] = m_pBuffer[i];
            m_pMasterBuffer[i/2 + 1] = m_pBuffer[i+1];
            m_pHeadphonesBuffer[i/2] = m_pBuffer[i+2];
            m_pHeadphonesBuffer[i/2 + 1] = m_pBuffer[i+3];
        }
    }
    iNumDevicesHaveRequestedBuffer++;

    if (iNumDevicesHaveRequestedBuffer >= iNumDevicesOpenedForOutput)
        iNumDevicesHaveRequestedBuffer = 0;

    requestBufferMutex.unlock();

    //Depending on what sources are connected to the SoundDevice, pass back certain audio back to
    //the SoundDevice.
    if (srcs.contains(SOURCE_MASTER) && srcs.contains(SOURCE_HEADPHONES))
    {
        return m_pBuffer;
    }

    if (srcs.contains(SOURCE_MASTER))
    {
        return m_pMasterBuffer;
    }

    if (srcs.contains(SOURCE_HEADPHONES))
    {
        return m_pHeadphonesBuffer;
    }

    qDebug() << "Warning: No sources passed to SoundManager::requestBuffer()";
    return m_pBuffer; //Default, shouldn't happen if this function was used properly.
}

//Used by SoundDevices to "push" any audio from their inputs that they have into the mixing engine.
CSAMPLE * SoundManager::pushBuffer(QList<AudioReceiver> recvs, short * inputBuffer, unsigned long iFramesPerBuffer)
{

    if (inputBuffer)
    {
#ifdef __VINYLCONTROL__
        if (recvs.contains(RECEIVER_VINYLCONTROL_ONE))
        {
            if (m_VinylControl[0])
                m_VinylControl[0]->AnalyseSamples(inputBuffer, iFramesPerBuffer);
        }
        if (recvs.contains(RECEIVER_VINYLCONTROL_TWO))
        {
            if (m_VinylControl[1])
                m_VinylControl[1]->AnalyseSamples(inputBuffer, iFramesPerBuffer);
        }
#endif
    }
    //TODO: Add pass-through option here (and push it into EngineMaster)...
    //      (or maybe save it, and then have requestBuffer() push it into EngineMaster)...


    return NULL; //FIXME: Return void instead?
}


