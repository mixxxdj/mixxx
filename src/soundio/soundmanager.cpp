#include "soundio/soundmanager.h"

#include <portaudio.h>

#include <QLibrary>
#include <QThread>
#include <QtGlobal>
#include <cstring> // for memcpy and strcmp

#include "control/controlobject.h"
#include "engine/enginemixer.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "moc_soundmanager.cpp"
#include "soundio/sounddevice.h"
#include "soundio/sounddevicenetwork.h"
#include "soundio/sounddevicenotfound.h"
#include "soundio/sounddeviceportaudio.h"
#include "soundio/soundmanagerutil.h"
#include "util/cmdlineargs.h"
#include "util/compatibility/qatomic.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/versionstore.h"
#include "vinylcontrol/defs_vinylcontrol.h"

#ifdef Q_OS_IOS
#include "soundio/soundmanagerios.h"
#endif

typedef PaError (*SetJackClientName)(const char *name);

namespace {

const QString kAppGroup = QStringLiteral("[App]");

#define CPU_OVERLOAD_DURATION 500 // in ms

struct DeviceMode {
    SoundDevicePointer pDevice;
    bool isInput;
    bool isOutput;
};

#ifdef __LINUX__
constexpr unsigned int kSleepSecondsAfterClosingDevice = 5;
#endif
} // anonymous namespace

SoundManager::SoundManager(UserSettingsPointer pConfig,
        EngineMixer* pEngineMixer)
        : m_pEngineMixer(pEngineMixer),
          m_pConfig(pConfig),
          m_paInitialized(false),
          m_config(this),
          m_pErrorDevice(nullptr),
          m_underflowHappened(0),
          m_underflowUpdateCount(0),
          m_audioLatencyOverloadCount({kAppGroup, QStringLiteral("audio_latency_overload_count")}),
          m_audioLatencyOverload({kAppGroup, QStringLiteral("audio_latency_overload")}) {
    // TODO(xxx) some of these ControlObject are not needed by soundmanager, or are unused here.
    // It is possible to take them out?
    m_pControlObjectSoundStatusCO = new ControlObject(
            ConfigKey("[SoundManager]", "status"));
    m_pControlObjectSoundStatusCO->set(SOUNDMANAGER_DISCONNECTED);

    m_pControlObjectVinylControlGainCO = new ControlObject(
            ConfigKey(VINYL_PREF_KEY, "gain"));

    //Hack because PortAudio samplerate enumeration is slow as hell on Linux (ALSA dmix sucks, so we can't blame PortAudio)
    m_samplerates.push_back(mixxx::audio::SampleRate(44100));
    m_samplerates.push_back(mixxx::audio::SampleRate(48000));
    m_samplerates.push_back(mixxx::audio::SampleRate(96000));

    m_pNetworkStream = QSharedPointer<EngineNetworkStream>(
            new EngineNetworkStream(2, 0));

    queryDevices();

    if (!m_config.readFromDisk()) {
        m_config.loadDefaults(this, SoundManagerConfig::ALL);
    }
    checkConfig();
    // Don't write config to disk, yet -- it may be reset to defaults in case
    // previously configured devices were not found.
    // Write new config after MixxxMainWindow::noOutputDlg where the user has
    // a chance to keep the previous sound config (exit).
}

SoundManager::~SoundManager() {
    // Clean up devices.
    const bool sleepAfterClosing = false;
    clearDeviceList(sleepAfterClosing);

    if (m_paInitialized) {
        Pa_Terminate();
        m_paInitialized = false;
    }
    // vinyl control proxies and input buffers are freed in closeDevices, called
    // by clearDeviceList -- bkgood

    delete m_pControlObjectSoundStatusCO;
    delete m_pControlObjectVinylControlGainCO;
}

QList<SoundDevicePointer> SoundManager::getDeviceList(
        const QString& filterAPI, bool bOutputDevices, bool bInputDevices) const {
    //qDebug() << "SoundManager::getDeviceList";

    if (filterAPI == SoundManagerConfig::kDefaultAPI) {
        return QList<SoundDevicePointer>();
    }

    // Create a list of sound devices filtered to match given API and
    // input/output.
    QList<SoundDevicePointer> filteredDeviceList;

    for (const auto& pDevice: m_devices) {
        // Skip devices that don't match the API, don't have input channels when
        // we want input devices, or don't have output channels when we want
        // output devices. If searching for both input and output devices,
        // make sure to include any devices that have >0 channels.
        const bool hasOutputs = pDevice->getNumOutputChannels().isValid();
        const bool hasInputs = pDevice->getNumInputChannels().isValid();
        if (pDevice->getHostAPI() != filterAPI ||
                (bOutputDevices && !bInputDevices && !hasOutputs) ||
                (bInputDevices && !bOutputDevices && !hasInputs) ||
                (!hasInputs && !hasOutputs)) {
            continue;
        }
        filteredDeviceList.push_back(pDevice);
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
    for (const auto& pDevice : std::as_const(m_devices)) {
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
        QThread::sleep(kSleepSecondsAfterClosingDevice);
#endif
    }

    // TODO(rryan): Should we do this before SoundDevice::close()? No! Because
    // then the callback may be running when we call
    // onInputDisconnected/onOutputDisconnected.
    for (const auto& pDevice : std::as_const(m_devices)) {
        for (const auto& in: pDevice->inputs()) {
            // Need to tell all registered AudioDestinations for this AudioInput
            // that the input was disconnected.
            for (auto it = m_registeredDestinations.constFind(in);
                 it != m_registeredDestinations.constEnd() && it.key() == in; ++it) {
                it.value()->onInputUnconfigured(in);
                m_pEngineMixer->onInputDisconnected(in);
            }
        }
        for (const auto& out: pDevice->outputs()) {
            // Need to tell all registered AudioSources for this AudioOutput
            // that the output was disconnected.
            for (auto it = m_registeredSources.constFind(out);
                 it != m_registeredSources.constEnd() && it.key() == out; ++it) {
                it.value()->onOutputDisconnected(out);
            }
        }
    }

    while (!m_inputBuffers.isEmpty()) {
        CSAMPLE* pBuffer = m_inputBuffers.takeLast();
        if (pBuffer != nullptr) {
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
    m_devices.clear();
    m_pErrorDevice.clear();

    if (m_paInitialized) {
        Pa_Terminate();
        m_paInitialized = false;
    }
}

QList<mixxx::audio::SampleRate> SoundManager::getSampleRates(const QString& api) const {
    if (api == MIXXX_PORTAUDIO_JACK_STRING) {
        // queryDevices must have been called for this to work, but the
        // ctor calls it -bkgood
        QList<mixxx::audio::SampleRate> samplerates;
        if (m_jackSampleRate.isValid()) {
            samplerates.append(m_jackSampleRate);
        }
        return samplerates;
    }
    return m_samplerates;
}

QList<mixxx::audio::SampleRate> SoundManager::getSampleRates() const {
    return getSampleRates("");
}

void SoundManager::queryDevices() {
    //qDebug() << "SoundManager::queryDevices()";
    queryDevicesPortaudio();
    queryDevicesMixxx();

    // now tell the prefs that we updated the device list -- bkgood
    emit devicesUpdated();
}

void SoundManager::clearAndQueryDevices() {
    const bool sleepAfterClosing = true;
    clearDeviceList(sleepAfterClosing);
    queryDevices();
}

void SoundManager::queryDevicesPortaudio() {
    PaError err = paNoError;
    if (!m_paInitialized) {
#ifdef Q_OS_LINUX
        setJACKName();
#endif
#ifdef Q_OS_IOS
        mixxx::initializeAVAudioSession();
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

    // PaDeviceInfo structs have a PaHostApiIndex member, but PortAudio
    // unfortunately provides no good way to associate this with a persistent,
    // unique identifier for the API. So, build a QHash to do that and pass
    // it to the SoundDevicePortAudio constructor.
    QHash<PaHostApiIndex, PaHostApiTypeId> paApiIndexToTypeId;
    for (PaHostApiIndex i = 0; i < Pa_GetHostApiCount(); i++) {
        const PaHostApiInfo* api = Pa_GetHostApiInfo(i);
        if (api && QString(api->name) != "skeleton implementation") {
            paApiIndexToTypeId.insert(i, api->type);
        }
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
        const auto deviceTypeId = paApiIndexToTypeId.value(deviceInfo->hostApi);
        auto currentDevice = SoundDevicePointer(new SoundDevicePortAudio(
                m_pConfig, this, deviceInfo, deviceTypeId, i));
        m_devices.push_back(currentDevice);
        if (!strcmp(Pa_GetHostApiInfo(deviceInfo->hostApi)->name,
                    MIXXX_PORTAUDIO_JACK_STRING)) {
            m_jackSampleRate = static_cast<mixxx::audio::SampleRate::value_t>(
                    deviceInfo->defaultSampleRate);
        }
    }
}

void SoundManager::queryDevicesMixxx() {
    auto currentDevice = SoundDevicePointer(new SoundDeviceNetwork(
            m_pConfig, this, m_pNetworkStream));
    m_devices.append(currentDevice);
}

SoundDeviceStatus SoundManager::setupDevices() {
    // NOTE(rryan): Big warning: This function is concurrent with calls to
    // pushBuffer and onDeviceOutputCallback until closeDevices() below.

    qDebug() << "SoundManager::setupDevices()";
    m_pControlObjectSoundStatusCO->set(SOUNDMANAGER_CONNECTING);
    SoundDeviceStatus status = SoundDeviceStatus::Ok;
    // NOTE(rryan): Do not clear m_pClkRefDevice here. If we didn't touch the
    // SoundDevice that is the clock reference, then it is safe to leave it as
    // it was. Clearing it causes the engine to stop being processed which
    // results in a stuttering noise (sometimes a loud buzz noise at low
    // latencies) when changing devices.
    //m_pClkRefDevice = NULL;
    m_pErrorDevice.clear();
    int outputDevicesOpened = 0;
    int inputDevicesOpened = 0;

    // NOTE(rryan): Documenting for future people touching this class. If you
    // would like to remove the fact that we close all the devices first and
    // then re-open them, I'm with you! The problem is that SoundDevicePortAudio
    // and SoundManager are not thread safe and the way that mutual exclusion
    // between the Qt main thread and the PortAudio callback thread is achieved
    // is that we shut off the PortAudio callbacks for all devices by closing
    // every device first. We then update all the SoundDevice settings
    // (configured AudioInputs/AudioOutputs) and then we re-open them.
    //
    // If you want to solve this issue, you should start by separating the PA
    // callback from the logic in SoundDevicePortAudio. They should communicate
    // via message passing over a request/response FIFO.

    // Instead of clearing m_pClkRefDevice and then assigning it directly,
    // compute the new one then atomically hand off below.
    SoundDevicePointer pNewMainClockRef;

    m_audioLatencyOverloadCount.set(0);

    // load with all configured devices.
    // all found devices are removed below
    QSet<SoundDeviceId> devicesNotFound = m_config.getDevices();

    // pair is isInput, isOutput
    QVector<DeviceMode> toOpen;
    bool haveOutput = false;
    // loop over all available devices
    for (const auto& pDevice : std::as_const(m_devices)) {
        DeviceMode mode = {pDevice, false, false};
        pDevice->clearInputs();
        pDevice->clearOutputs();
        m_pErrorDevice = pDevice;
        const auto inputs = m_config.getInputs().values(pDevice->getDeviceId());
        for (const auto& in : inputs) {
            mode.isInput = true;
            // TODO(bkgood) look into allocating this with the frames per
            // buffer value from SMConfig
            AudioInputBuffer aib(in, SampleUtil::alloc(kMaxEngineSamples));
            status = pDevice->addInput(aib);
            if (status != SoundDeviceStatus::Ok) {
                SampleUtil::free(aib.getBuffer());
                goto closeAndError;
            }

            m_inputBuffers.append(aib.getBuffer());

            // Check if any AudioDestination is registered for this AudioInput
            // and call the onInputConnected method.
            for (auto it = m_registeredDestinations.find(in);
                    it != m_registeredDestinations.end() && it.key() == in;
                    ++it) {
                it.value()->onInputConfigured(in);
                m_pEngineMixer->onInputConnected(in);
            }
        }
        QList<AudioOutput> outputs =
                m_config.getOutputs().values(pDevice->getDeviceId());

        // Statically connect the Network Device to the Sidechain
        if (pDevice->getDeviceId().name == kNetworkDeviceInternalName) {
            AudioOutput out(AudioPathType::RecordBroadcast,
                    0,
                    mixxx::audio::ChannelCount::stereo(),
                    0);
            outputs.append(out);
            if (m_config.getForceNetworkClock() && !jackApiUsed()) {
                pNewMainClockRef = pDevice;
            }
        }

        for (const auto& out : std::as_const(outputs)) {
            mode.isOutput = true;
            if (pDevice->getDeviceId().name != kNetworkDeviceInternalName) {
                haveOutput = true;
            }
            // following keeps us from asking for a channel buffer EngineMixer
            // doesn't have -- bkgood
            const CSAMPLE* pBuffer = m_registeredSources.value(out)->buffer(out).data();
            if (pBuffer == nullptr) {
                qDebug() << "AudioSource returned null for" << out.getString();
                continue;
            }

            AudioOutputBuffer aob(out, pBuffer);
            status = pDevice->addOutput(aob);
            if (status != SoundDeviceStatus::Ok) {
                goto closeAndError;
            }

            if (!m_config.getForceNetworkClock() || jackApiUsed()) {
                if (out.getType() == AudioPathType::Main) {
                    pNewMainClockRef = pDevice;
                } else if ((out.getType() == AudioPathType::Deck ||
                                   out.getType() == AudioPathType::Bus) &&
                        !pNewMainClockRef) {
                    pNewMainClockRef = pDevice;
                }
            }

            // Check if any AudioSource is registered for this AudioOutput and
            // call the onOutputConnected method.
            for (auto it = m_registeredSources.find(out);
                    it != m_registeredSources.end() && it.key() == out;
                    ++it) {
                it.value()->onOutputConnected(out);
            }
        }

        if (mode.isInput || mode.isOutput) {
            pDevice->setSampleRate(m_config.getSampleRate());
            pDevice->setConfigFramesPerBuffer(m_config.getFramesPerBuffer());
            toOpen.append(mode);
        }
    }

    for (const auto& mode: toOpen) {
        SoundDevicePointer pDevice = mode.pDevice;
        m_pErrorDevice = pDevice;

        // If we have not yet set a clock source then we use the first
        // output pDevice
        if (pNewMainClockRef.isNull() &&
                (!haveOutput || mode.isOutput)) {
            pNewMainClockRef = pDevice;
            qWarning() << "Output sound device clock reference not set! Using"
                       << pDevice->getDisplayName();
        }

        int syncBuffers = m_config.getSyncBuffers();
        // If we are in safe mode and using experimental polling support, use
        // the default of 2 sync buffers instead.
        if (CmdlineArgs::Instance().getSafeMode() && syncBuffers == 0) {
            syncBuffers = 2;
        }
        status = pDevice->open(pNewMainClockRef == pDevice, syncBuffers);
        if (status != SoundDeviceStatus::Ok) {
            goto closeAndError;
        }
        devicesNotFound.remove(pDevice->getDeviceId());
        if (mode.isOutput) {
            ++outputDevicesOpened;
        }
        if (mode.isInput) {
            ++inputDevicesOpened;
        }
    }

    if (pNewMainClockRef) {
        qDebug() << "Using" << pNewMainClockRef->getDisplayName()
                 << "as output sound device clock reference";
    } else {
        qWarning() << "No output devices opened, no clock reference device set";
    }

    qDebug() << outputDevicesOpened << "output sound devices opened";
    qDebug() << inputDevicesOpened << "input sound devices opened";
    for (const auto& device : std::as_const(devicesNotFound)) {
        qWarning() << device << "not found";
    }

    m_pControlObjectSoundStatusCO->set(
            outputDevicesOpened > 0 ?
                    SOUNDMANAGER_CONNECTED : SOUNDMANAGER_DISCONNECTED);

    // returns OK if we were able to open all the devices the user wanted
    if (devicesNotFound.isEmpty()) {
        emit devicesSetup();
        return SoundDeviceStatus::Ok;
    }
    m_pErrorDevice = SoundDevicePointer(
            new SoundDeviceNotFound(devicesNotFound.constBegin()->name));
    return SoundDeviceStatus::ErrorDeviceCount;

closeAndError:
    const bool sleepAfterClosing = false;
    closeDevices(sleepAfterClosing);
    return status;
}

SoundDevicePointer SoundManager::getErrorDevice() const {
    return m_pErrorDevice;
}

QString SoundManager::getErrorDeviceName() const {
    SoundDevicePointer pDevice = getErrorDevice();
    if (pDevice) {
        return pDevice->getDisplayName();
    }
    return tr("a device");
}

QString SoundManager::getLastErrorMessage(SoundDeviceStatus status) const {
    QString error;
    QString deviceName(tr("a device"));
    QString detailedError(tr("An unknown error occurred"));
    SoundDevicePointer pDevice = getErrorDevice();
    if (pDevice) {
        deviceName = pDevice->getDisplayName();
        detailedError = pDevice->getError();
    }
    switch (status) {
    case SoundDeviceStatus::ErrorDuplicateOutputChannel:
        error = tr("Two outputs cannot share channels on \"%1\"").arg(deviceName);
        break;
    default:
        error = tr("Error opening \"%1\"").arg(deviceName) + "\n" + detailedError;
        break;
    }
    return error;
}

SoundManagerConfig SoundManager::getConfig() const {
    return m_config;
}

void SoundManager::closeActiveConfig() {
    // Close open devices. After this call we will not get any more
    // onDeviceOutputCallback() or pushBuffer() calls because all the
    // SoundDevices are closed. closeDevices() blocks and can take a while.
    const bool sleepAfterClosing = true;
    closeDevices(sleepAfterClosing);
}

SoundDeviceStatus SoundManager::setConfig(const SoundManagerConfig& config) {
    SoundDeviceStatus status = SoundDeviceStatus::Ok;
    m_config = config;
    checkConfig();

    closeActiveConfig();

    status = setupDevices();
    if (status == SoundDeviceStatus::Ok) {
        m_config.writeToDisk();
    }
    return status;
}

void SoundManager::checkConfig() {
    if (!m_config.checkAPI()) {
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

void SoundManager::onDeviceOutputCallback(const SINT iFramesPerBuffer) {
    // Produce a block of samples for output. EngineMixer expects stereo
    // samples so multiply iFramesPerBuffer by 2.
    m_pEngineMixer->process(iFramesPerBuffer * 2);
}

void SoundManager::pushInputBuffers(const QList<AudioInputBuffer>& inputs,
                                    const SINT iFramesPerBuffer) {
   for (QList<AudioInputBuffer>::ConstIterator i = inputs.begin(),
                 e = inputs.end(); i != e; ++i) {
        const AudioInputBuffer& in = *i;
        CSAMPLE* pInputBuffer = in.getBuffer();
        for (auto it = m_registeredDestinations.constFind(in);
             it != m_registeredDestinations.constEnd() && it.key() == in; ++it) {
            it.value()->receiveBuffer(in, pInputBuffer, iFramesPerBuffer);
        }
    }
}

void SoundManager::writeProcess(SINT framesPerBuffer) const {
    for (const auto& pDevice: m_devices) {
        if (pDevice) {
            pDevice->writeProcess(framesPerBuffer);
        }
    }
}

void SoundManager::readProcess(SINT framesPerBuffer) const {
    for (const auto& pDevice: m_devices) {
        if (pDevice) {
            pDevice->readProcess(framesPerBuffer);
        }
    }
}

void SoundManager::registerOutput(const AudioOutput& output, AudioSource* src) {
    VERIFY_OR_DEBUG_ASSERT(!m_registeredSources.contains(output)) {
        return;
    }
    m_registeredSources.insert(output, src);
    emit outputRegistered(output, src);
}

void SoundManager::registerInput(const AudioInput& input, AudioDestination* dest) {
    // Vinyl control inputs are registered twice, once for timecode and once for
    // passthrough, each with different outputs. So unlike outputs, do not assert
    // that the input has not been registered yet.
    m_registeredDestinations.insert(input, dest);

    emit inputRegistered(input, dest);
}

QList<AudioOutput> SoundManager::registeredOutputs() const {
    return m_registeredSources.keys();
}

QList<AudioInput> SoundManager::registeredInputs() const {
    return m_registeredDestinations.keys();
}

void SoundManager::setJACKName() const {
#ifdef Q_OS_LINUX
    typedef PaError (*SetJackClientName)(const char *name);
    QLibrary portaudio("libportaudio.so.2");
    if (portaudio.load()) {
        SetJackClientName func(
            reinterpret_cast<SetJackClientName>(
                portaudio.resolve("PaJack_SetClientName")));
        if (func) {
            // PortAudio does not make a copy of the string we provide it so we
            // need to make sure it will last forever so we intentionally leak
            // this string.
            char* jackNameCopy = strdup(VersionStore::applicationName().toLocal8Bit().constData());
            if (!func(jackNameCopy)) {
                qDebug() << "JACK client name set";
            }
        } else {
            qWarning() << "failed to resolve JACK name method";
        }
    } else {
        qWarning() << "failed to load portaudio for JACK rename";
    }
#endif
}

void SoundManager::setConfiguredDeckCount(int count) {
    if (getConfiguredDeckCount() == count) {
        // Unchanged
        return;
    }
    m_config.setDeckCount(count);
    checkConfig();
    m_config.writeToDisk();
}

int SoundManager::getConfiguredDeckCount() const {
    return m_config.getDeckCount();
}

void SoundManager::processUnderflowHappened(SINT framesPerBuffer) {
    if (m_underflowUpdateCount == 0) {
        if (atomicLoadRelaxed(m_underflowHappened)) {
            m_audioLatencyOverload.set(1.0);
            m_audioLatencyOverloadCount.set(
                    m_audioLatencyOverloadCount.get() + 1);
            m_underflowUpdateCount = CPU_OVERLOAD_DURATION *
                    m_config.getSampleRate() / framesPerBuffer / 1000;

            m_underflowHappened = 0; // resetting here is not thread safe,
                                     // but that is OK, because we count only
                                     // 1 underflow each 500 ms
        } else {
            m_audioLatencyOverload.set(0.0);
        }
    } else {
        --m_underflowUpdateCount;
    }
}
