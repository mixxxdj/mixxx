/**
 * @file soundmanager.cpp
 * @author Albert Santoni <gamegod at users dot sf dot net>
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20070815
 */

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>
#include <cstring> // for memcpy and strcmp

#ifdef __PORTAUDIO__
#include <QLibrary>
#include <portaudio.h>
#endif // ifdef __PORTAUDIO__

#include "soundmanager.h"
#include "sounddevice.h"
#include "sounddeviceportaudio.h"
#include "sounddevicenetwork.h"
#include "engine/enginemaster.h"
#include "engine/enginebuffer.h"
#include "soundmanagerutil.h"
#include "controlobject.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "sampleutil.h"
#include "util/cmdlineargs.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "util/sleep.h"

#ifdef __PORTAUDIO__
typedef PaError (*SetJackClientName)(const char *name);
#endif

namespace {
#ifdef __LINUX__
const unsigned int kSleepSecondsAfterClosingDevice = 5;
#endif
} // anonymous namespace

SoundManager::SoundManager(ConfigObject<ConfigValue> *pConfig,
                           EngineMaster *pMaster)
        : m_pMaster(pMaster),
          m_pConfig(pConfig),
#ifdef __PORTAUDIO__
          m_paInitialized(false),
          m_jackSampleRate(-1),
#endif
          m_pErrorDevice(NULL) {

#ifdef __PORTAUDIO__
    qDebug() << "PortAudio version:" << Pa_GetVersion()
             << "text:" << Pa_GetVersionText();
#endif

    // TODO(xxx) some of these ControlObject are not needed by soundmanager, or are unused here.
    // It is possible to take them out?
    m_pControlObjectSoundStatusCO = new ControlObject(ConfigKey("[SoundManager]", "status"));
    m_pControlObjectSoundStatusCO->set(SOUNDMANAGER_DISCONNECTED);
    m_pControlObjectVinylControlGainCO = new ControlObject(ConfigKey(VINYL_PREF_KEY, "gain"));

    //Hack because PortAudio samplerate enumeration is slow as hell on Linux (ALSA dmix sucks, so we can't blame PortAudio)
    m_samplerates.push_back(44100);
    m_samplerates.push_back(48000);
    m_samplerates.push_back(96000);

    m_pNetworkStream = QSharedPointer<EngineNetworkStream>(
            new EngineNetworkStream(2, 0));

    queryDevices();

    if (!m_config.readFromDisk()) {
        m_config.loadDefaults(this, SoundManagerConfig::ALL);
    }
    checkConfig();
    m_config.writeToDisk(); // in case anything changed by applying defaults
}

SoundManager::~SoundManager() {
    // Clean up devices.
    const bool sleepAfterClosing = false;
    clearDeviceList(sleepAfterClosing);

#ifdef __PORTAUDIO__
    if (m_paInitialized) {
        Pa_Terminate();
        m_paInitialized = false;
    }
#endif
    // vinyl control proxies and input buffers are freed in closeDevices, called
    // by clearDeviceList -- bkgood

    delete m_pControlObjectSoundStatusCO;
    delete m_pControlObjectVinylControlGainCO;
}

QList<SoundDevice*> SoundManager::getDeviceList(
    QString filterAPI, bool bOutputDevices, bool bInputDevices) {
    //qDebug() << "SoundManager::getDeviceList";

    if (filterAPI == "None") {
        QList<SoundDevice*> emptyList;
        return emptyList;
    }

    // Create a list of sound devices filtered to match given API and
    // input/output.
    QList<SoundDevice*> filteredDeviceList;

    foreach (SoundDevice* device, m_devices) {
        // Skip devices that don't match the API, don't have input channels when
        // we want input devices, or don't have output channels when we want
        // output devices.
        if (device->getHostAPI() != filterAPI ||
                (bOutputDevices && device->getNumOutputChannels() <= 0) ||
                (bInputDevices && device->getNumInputChannels() <= 0)) {
            continue;
        }
        filteredDeviceList.push_back(device);
    }
    return filteredDeviceList;
}

QList<QString> SoundManager::getHostAPIList() const {
    QList<QString> apiList;

    for (PaHostApiIndex i = 0; i < Pa_GetHostApiCount(); i++) {
        const PaHostApiInfo* api = Pa_GetHostApiInfo(i);
        if (api && QString(api->name) != "skeleton implementation") {
            apiList.push_back(api->name);
        }
    }

    return apiList;
}

void SoundManager::closeDevices(bool sleepAfterClosing) {
    //qDebug() << "SoundManager::closeDevices()";

    bool closed = false;
    foreach (SoundDevice* pDevice, m_devices) {
        if (pDevice->isOpen()) {
            // NOTE(rryan): As of 2009 (?) it has been safe to close() a SoundDevice
            // while callbacks are active.
            pDevice->close();
            closed = true;
        }
    }

    if (closed && sleepAfterClosing) {
#ifdef __LINUX__
        // Sleep for 5 sec to allow asynchronously sound APIs like "pulse" to free
        // its resources as well
        sleep(kSleepSecondsAfterClosingDevice);
#endif
    }

    m_pErrorDevice = NULL;

    // TODO(rryan): Should we do this before SoundDevice::close()? No! Because
    // then the callback may be running when we call
    // onInputDisconnected/onOutputDisconnected.
    foreach (SoundDevice* pDevice, m_devices) {
        foreach (AudioInput in, pDevice->inputs()) {
            // Need to tell all registered AudioDestinations for this AudioInput
            // that the input was disconnected.
            for (QHash<AudioInput, AudioDestination*>::const_iterator it =
                         m_registeredDestinations.find(in);
                 it != m_registeredDestinations.end() && it.key() == in; ++it) {
                it.value()->onInputUnconfigured(in);
            }
        }
        foreach (AudioOutput out, pDevice->outputs()) {
            // Need to tell all registered AudioSources for this AudioOutput
            // that the output was disconnected.
            for (QHash<AudioOutput, AudioSource*>::const_iterator it =
                         m_registeredSources.find(out);
                 it != m_registeredSources.end() && it.key() == out; ++it) {
                it.value()->onOutputDisconnected(out);
            }
        }
    }

    while (!m_inputBuffers.isEmpty()) {
        CSAMPLE* pBuffer = m_inputBuffers.takeLast();
        if (pBuffer != NULL) {
            SampleUtil::free(pBuffer);
        }
    }
    m_inputBuffers.clear();

    // Indicate to the rest of Mixxx that sound is disconnected.
    m_pControlObjectSoundStatusCO->set(SOUNDMANAGER_DISCONNECTED);
}

void SoundManager::clearDeviceList(bool sleepAfterClosing) {
    //qDebug() << "SoundManager::clearDeviceList()";

    // Close the devices first.
    closeDevices(sleepAfterClosing);

    // Empty out the list of devices we currently have.
    while (!m_devices.empty()) {
        SoundDevice* dev = m_devices.takeLast();
        delete dev;
    }

#ifdef __PORTAUDIO__
    if (m_paInitialized) {
        Pa_Terminate();
        m_paInitialized = false;
    }
#endif
}

QList<unsigned int> SoundManager::getSampleRates(QString api) const {
#ifdef __PORTAUDIO__
    if (api == MIXXX_PORTAUDIO_JACK_STRING) {
        // queryDevices must have been called for this to work, but the
        // ctor calls it -bkgood
        QList<unsigned int> samplerates;
        samplerates.append(m_jackSampleRate);
        return samplerates;
    }
#endif
    return m_samplerates;
}

QList<unsigned int> SoundManager::getSampleRates() const {
    return getSampleRates("");
}

void SoundManager::queryDevices() {
    //qDebug() << "SoundManager::queryDevices()";
    queryDevicesPortaudio();
    queryDevicesMixxx();

    // now tell the prefs that we updated the device list -- bkgood
    emit(devicesUpdated());
}

void SoundManager::clearAndQueryDevices() {
    const bool sleepAfterClosing = true;
    clearDeviceList(sleepAfterClosing);
    queryDevices();
}

void SoundManager::queryDevicesPortaudio() {
#ifdef __PORTAUDIO__
    PaError err = paNoError;
    if (!m_paInitialized) {
#ifdef Q_OS_LINUX
        setJACKName();
#endif
        err = Pa_Initialize();
        m_paInitialized = true;
    }
    if (err != paNoError) {
        qDebug() << "Error:" << Pa_GetErrorText(err);
        m_paInitialized = false;
        return;
    }

    int iNumDevices = Pa_GetDeviceCount();
    if (iNumDevices < 0) {
        qDebug() << "ERROR: Pa_CountDevices returned" << iNumDevices;
        return;
    }

    const PaDeviceInfo* deviceInfo;
    for (int i = 0; i < iNumDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        if (!deviceInfo) {
            continue;
        }
        /* deviceInfo fields for quick reference:
            int     structVersion
            const char *    name
            PaHostApiIndex  hostApi
            int     maxInputChannels
            int     maxOutputChannels
            PaTime  defaultLowInputLatency
            PaTime  defaultLowOutputLatency
            PaTime  defaultHighInputLatency
            PaTime  defaultHighOutputLatency
            double  defaultSampleRate
         */
        SoundDevicePortAudio* currentDevice = new SoundDevicePortAudio(
                m_pConfig, this, deviceInfo, i);
        m_devices.push_back(currentDevice);
        if (!strcmp(Pa_GetHostApiInfo(deviceInfo->hostApi)->name,
                    MIXXX_PORTAUDIO_JACK_STRING)) {
            m_jackSampleRate = deviceInfo->defaultSampleRate;
        }
    }
#endif
}

void SoundManager::queryDevicesMixxx() {
    SoundDeviceNetwork* currentDevice = new SoundDeviceNetwork(
            m_pConfig, this, m_pNetworkStream);
    m_devices.push_back(currentDevice);
}

Result SoundManager::setupDevices() {
    // NOTE(rryan): Big warning: This function is concurrent with calls to
    // pushBuffer and onDeviceOutputCallback until closeDevices() below.

    qDebug() << "SoundManager::setupDevices()";
    m_pControlObjectSoundStatusCO->set(SOUNDMANAGER_CONNECTING);
    Result err = OK;
    // NOTE(rryan): Do not clear m_pClkRefDevice here. If we didn't touch the
    // SoundDevice that is the clock reference, then it is safe to leave it as
    // it was. Clearing it causes the engine to stop being processed which
    // results in a stuttering noise (sometimes a loud buzz noise at low
    // latencies) when changing devices.
    //m_pClkRefDevice = NULL;
    m_pErrorDevice = NULL;
    int devicesAttempted = 0;
    int devicesOpened = 0;
    int outputDevicesOpened = 0;
    int inputDevicesOpened = 0;

    // filter out any devices in the config we don't actually have
    m_config.filterOutputs(this);
    m_config.filterInputs(this);

    // NOTE(rryan): Documenting for future people touching this class. If you
    // would like to remove the fact that we close all the devices first and
    // then re-open them, I'm with you! The problem is that SoundDevicePortAudio
    // and SoundManager are not thread safe and the way that mutual exclusion
    // between the Qt main thread and the PortAudio callback thread is acheived
    // is that we shut off the PortAudio callbacks for all devices by closing
    // every device first. We then update all the SoundDevice settings
    // (configured AudioInputs/AudioOutputs) and then we re-open them.
    //
    // If you want to solve this issue, you should start by separating the PA
    // callback from the logic in SoundDevicePortAudio. They should communicate
    // via message passing over a request/response FIFO.

    // Instead of clearing m_pClkRefDevice and then assigning it directly,
    // compute the new one then atomically hand off below.
    SoundDevice* pNewMasterClockRef = NULL;

    // pair is isInput, isOutput
    QHash<SoundDevice*, QPair<bool, bool> > toOpen;
    foreach (SoundDevice *device, m_devices) {
        bool isInput = false;
        bool isOutput = false;
        device->clearInputs();
        device->clearOutputs();
        m_pErrorDevice = device;
        foreach (AudioInput in,
                 m_config.getInputs().values(device->getInternalName())) {
            isInput = true;
            // TODO(bkgood) look into allocating this with the frames per
            // buffer value from SMConfig
            AudioInputBuffer aib(in, SampleUtil::alloc(MAX_BUFFER_LEN));
            err = device->addInput(aib) != SOUNDDEVICE_ERROR_OK ? ERR : OK;
            if (err != OK) {
                delete [] aib.getBuffer();
                goto closeAndError;
            }

            m_inputBuffers.append(aib.getBuffer());

            // Check if any AudioDestination is registered for this AudioInput
            // and call the onInputConnected method.
            for (QHash<AudioInput, AudioDestination*>::const_iterator it =
                         m_registeredDestinations.find(in);
                 it != m_registeredDestinations.end() && it.key() == in; ++it) {
                it.value()->onInputConfigured(in);
            }
        }
        QList<AudioOutput> outputs =
                m_config.getOutputs().values(device->getInternalName());

        // Statically connect the Network Device to the Sidechain
        if (device->getInternalName() == kNetworkDeviceInternalName) {
            AudioOutput out(AudioPath::SIDECHAIN, 0, 2, 0);
            outputs.append(out);
        }

        foreach (AudioOutput out, outputs) {
            isOutput = true;
            // following keeps us from asking for a channel buffer EngineMaster
            // doesn't have -- bkgood
            const CSAMPLE* pBuffer = m_registeredSources.value(out)->buffer(out);
            if (pBuffer == NULL) {
                qDebug() << "AudioSource returned null for" << out.getString();
                continue;
            }

            AudioOutputBuffer aob(out, pBuffer);
            err = device->addOutput(aob) != SOUNDDEVICE_ERROR_OK ? ERR : OK;
            if (err != OK) goto closeAndError;
            if (out.getType() == AudioOutput::MASTER) {
                pNewMasterClockRef = device;
            } else if ((out.getType() == AudioOutput::DECK ||
                        out.getType() == AudioOutput::BUS)
                    && !pNewMasterClockRef) {
                pNewMasterClockRef = device;
            }

            // Check if any AudioSource is registered for this AudioOutput and
            // call the onOutputConnected method.
            for (QHash<AudioOutput, AudioSource*>::const_iterator it =
                         m_registeredSources.find(out);
                 it != m_registeredSources.end() && it.key() == out; ++it) {
                it.value()->onOutputConnected(out);
            }
        }

        if (isInput || isOutput) {
            device->setSampleRate(m_config.getSampleRate());
            device->setFramesPerBuffer(m_config.getFramesPerBuffer());
            toOpen[device] = QPair<bool, bool>(isInput, isOutput);
        }
    }

    foreach (SoundDevice *device, toOpen.keys()) {
        QPair<bool, bool> mode(toOpen[device]);
        bool isInput = mode.first;
        bool isOutput = mode.second;
        ++devicesAttempted;
        m_pErrorDevice = device;
        // If we have not yet set a clock source then we use the first
        if (pNewMasterClockRef == NULL) {
            pNewMasterClockRef = device;
            qWarning() << "Output sound device clock reference not set! Using"
                       << device->getDisplayName();
        }
        int syncBuffers = m_config.getSyncBuffers();
        // If we are in safe mode and using experimental polling support, use
        // the default of 2 sync buffers instead.
        if (CmdlineArgs::Instance().getSafeMode() && syncBuffers == 0) {
            syncBuffers = 2;
        }
        err = device->open(pNewMasterClockRef == device, syncBuffers);
        if (err != OK) {
            goto closeAndError;
        } else {
            ++devicesOpened;
            if (isOutput) {
                ++outputDevicesOpened;
            }
            if (isInput) {
                ++inputDevicesOpened;
            }
        }
    }

    if (pNewMasterClockRef) {
        qDebug() << "Using" << pNewMasterClockRef->getDisplayName()
                 << "as output sound device clock reference";
    } else {
        qDebug() << "No output devices opened, no clock reference device set";
    }

    qDebug() << outputDevicesOpened << "output sound devices opened";
    qDebug() << inputDevicesOpened << "input  sound devices opened";

    m_pControlObjectSoundStatusCO->set(
            outputDevicesOpened > 0 ?
                    SOUNDMANAGER_CONNECTED : SOUNDMANAGER_DISCONNECTED);

    // returns OK if we were able to open all the devices the user wanted
    if (devicesAttempted == devicesOpened) {
        emit(devicesSetup());
        return OK;
    }
    m_pErrorDevice = NULL;
    return ERR;

closeAndError:
    const bool sleepAfterClosing = false;
    closeDevices(sleepAfterClosing);
    return err;
}

SoundDevice* SoundManager::getErrorDevice() const {
    return m_pErrorDevice;
}

SoundManagerConfig SoundManager::getConfig() const {
    return m_config;
}

Result SoundManager::setConfig(SoundManagerConfig config) {
    Result err = OK;
    m_config = config;
    checkConfig();

    // Close open devices. After this call we will not get any more
    // onDeviceOutputCallback() or pushBuffer() calls because all the
    // SoundDevices are closed. closeDevices() blocks and can take a while.
    const bool sleepAfterClosing = true;
    closeDevices(sleepAfterClosing);

    // certain parts of mixxx rely on this being here, for the time being, just
    // letting those be -- bkgood
    // Do this first so vinyl control gets the right samplerate -- Owen W.
    m_pConfig->set(ConfigKey("[Soundcard]","Samplerate"), ConfigValue(m_config.getSampleRate()));

    err = setupDevices();
    if (err == OK) {
        m_config.writeToDisk();
    }
    return err;
}

void SoundManager::checkConfig() {
    if (!m_config.checkAPI(*this)) {
        m_config.setAPI(SoundManagerConfig::kDefaultAPI);
        m_config.loadDefaults(this, SoundManagerConfig::API | SoundManagerConfig::DEVICES);
    }
    if (!m_config.checkSampleRate(*this)) {
        m_config.setSampleRate(SoundManagerConfig::kFallbackSampleRate);
        m_config.loadDefaults(this, SoundManagerConfig::OTHER);
    }

    // Even if we have a two-deck skin, if someone has configured a deck > 2
    // then the configuration needs to know about that extra deck.
    m_config.setCorrectDeckCount(getConfiguredDeckCount());
    // latency checks itself for validity on SMConfig::setLatency()
}

void SoundManager::onDeviceOutputCallback(const unsigned int iFramesPerBuffer) {
    // Produce a block of samples for output. EngineMaster expects stereo
    // samples so multiply iFramesPerBuffer by 2.
    m_pMaster->process(iFramesPerBuffer*2);
}

void SoundManager::pushInputBuffers(const QList<AudioInputBuffer>& inputs,
                                    const unsigned int iFramesPerBuffer) {
   for (QList<AudioInputBuffer>::ConstIterator i = inputs.begin(),
                 e = inputs.end(); i != e; ++i) {
        const AudioInputBuffer& in = *i;
        CSAMPLE* pInputBuffer = in.getBuffer();
        for (QHash<AudioInput, AudioDestination*>::const_iterator it =
                m_registeredDestinations.find(in);
                it != m_registeredDestinations.end() && it.key() == in; ++it) {
            it.value()->receiveBuffer(in, pInputBuffer, iFramesPerBuffer);
        }
    }
}

void SoundManager::writeProcess() {
    QListIterator<SoundDevice*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        SoundDevice* device = dev_it.next();
        if (device) {
            device->writeProcess();
        }
    }
}

void SoundManager::readProcess() {
    QListIterator<SoundDevice*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        SoundDevice* device = dev_it.next();
        if (device) {
            device->readProcess();
        }
    }
}


void SoundManager::registerOutput(AudioOutput output, AudioSource *src) {
    if (m_registeredSources.contains(output)) {
        qDebug() << "WARNING: AudioOutput already registered!";
    }
    m_registeredSources.insert(output, src);
    emit(outputRegistered(output, src));
}

void SoundManager::registerInput(AudioInput input, AudioDestination *dest) {
    if (m_registeredDestinations.contains(input)) {
        // note that this can be totally ok if we just want a certain
        // AudioInput to be going to a different AudioDest -bkgood
        qDebug() << "WARNING: AudioInput already registered!";
    }

    m_registeredDestinations.insertMulti(input, dest);

    emit(inputRegistered(input, dest));
}

QList<AudioOutput> SoundManager::registeredOutputs() const {
    return m_registeredSources.keys();
}

QList<AudioInput> SoundManager::registeredInputs() const {
    return m_registeredDestinations.keys();
}

void SoundManager::setJACKName() const {
#ifdef __PORTAUDIO__
#ifdef Q_OS_LINUX
    typedef PaError (*SetJackClientName)(const char *name);
    QLibrary portaudio("libportaudio.so.2");
    if (portaudio.load()) {
        SetJackClientName func(
            reinterpret_cast<SetJackClientName>(
                portaudio.resolve("PaJack_SetClientName")));
        if (func) {
            if (!func("Mixxx")) qDebug() << "JACK client name set";
        } else {
            qWarning() << "failed to resolve JACK name method";
        }
    } else {
        qWarning() << "failed to load portaudio for JACK rename";
    }
#endif
#endif
}

void SoundManager::setConfiguredDeckCount(int count) {
    m_config.setDeckCount(count);
    checkConfig();
    m_config.writeToDisk();
}

int SoundManager::getConfiguredDeckCount() const {
    return m_config.getDeckCount();
}
