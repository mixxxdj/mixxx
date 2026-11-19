#include "soundio/sounddevicejack.h"

#include <jack/jack.h>

#include <QLibrary>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

#include "control/controlobject.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerconfig.h"
#include "util/defs.h"
#include "waveform/visualplayposition.h"

namespace {

constexpr int kCpuUsageUpdateRate = 30;
const QString kAppGroup = QStringLiteral("[App]");

QString pathName(const AudioPath& path) {
    QString name;
    switch (path.getType()) {
    case AudioPathType::Main:
        name = QStringLiteral("playback");
        break;
    case AudioPathType::Headphones:
        name = QStringLiteral("headphone");
        break;
    default: {
        static const QRegularExpression nonAlphanumeric(
                QStringLiteral("[^a-z0-9]+"));
        name = AudioPath::getStringFromType(path.getType())
                       .toLower()
                       .replace(nonAlphanumeric, QStringLiteral("_"));
        break;
    }
    }
    if (AudioPath::isIndexed(path.getType()) ||
            path.getType() == AudioPathType::Bus) {
        name += QStringLiteral("_%1").arg(path.getIndex() + 1);
    }
    return name;
}

QString channelName(const AudioPath& path, int channelOffset) {
    const int channelCount = path.getChannelGroup().getChannelCount();
    if (channelCount == 1) {
        return QStringLiteral("C");
    }
    if (channelCount == 2) {
        return channelOffset == 0 ? QStringLiteral("FL") : QStringLiteral("FR");
    }
    return QStringLiteral("CH%1").arg(channelOffset + 1);
}

std::vector<QString> portNames(
        const QList<AudioOutputBuffer>& paths, int channelCount) {
    std::vector<QString> names(static_cast<size_t>(channelCount));
    for (const auto& path : paths) {
        const ChannelGroup channels = path.getChannelGroup();
        const int base = channels.getChannelBase();
        const int count = channels.getChannelCount();
        for (int offset = 0; offset < count; ++offset) {
            names.at(static_cast<size_t>(base + offset)) =
                    QStringLiteral("mixxx_%1_%2")
                            .arg(pathName(path), channelName(path, offset));
        }
    }
    return names;
}

std::vector<QString> portNames(
        const QList<AudioInputBuffer>& paths, int channelCount) {
    std::vector<QString> names(static_cast<size_t>(channelCount));
    for (const auto& path : paths) {
        const ChannelGroup channels = path.getChannelGroup();
        const int base = channels.getChannelBase();
        const int count = channels.getChannelCount();
        for (int offset = 0; offset < count; ++offset) {
            names.at(static_cast<size_t>(base + offset)) =
                    QStringLiteral("mixxx_capture_%1_%2")
                            .arg(pathName(path), channelName(path, offset));
        }
    }
    return names;
}

} // namespace

struct SoundDeviceJack::Impl {
    using ClientOpen =
            jack_client_t* (*)(const char*, jack_options_t, jack_status_t*, ...);
    using ClientClose = int (*)(jack_client_t*);
    using Activate = int (*)(jack_client_t*);
    using Deactivate = int (*)(jack_client_t*);
    using GetSampleRate = jack_nframes_t (*)(jack_client_t*);
    using GetBufferSize = jack_nframes_t (*)(jack_client_t*);
    using FramesSinceCycleStart = jack_nframes_t (*)(const jack_client_t*);
    using SetProcessCallback =
            int (*)(jack_client_t*, JackProcessCallback, void*);
    using SetBufferSizeCallback =
            int (*)(jack_client_t*, JackBufferSizeCallback, void*);
    using OnInfoShutdown =
            void (*)(jack_client_t*, JackInfoShutdownCallback, void*);
    using SetLatencyCallback =
            int (*)(jack_client_t*, JackLatencyCallback, void*);
    using SetSampleRateCallback =
            int (*)(jack_client_t*, JackSampleRateCallback, void*);
    using SetXrunCallback = int (*)(jack_client_t*, JackXRunCallback, void*);
    using PortRegister = jack_port_t* (*)(jack_client_t*,
            const char*,
            const char*,
            unsigned long,
            unsigned long);
    using PortGetBuffer = void* (*)(jack_port_t*, jack_nframes_t);
    using PortGetLatencyRange = void (*)(
            jack_port_t*, jack_latency_callback_mode_t, jack_latency_range_t*);

    QLibrary library{QStringLiteral("libjack.so.0")};
    jack_client_t* client{nullptr};
    ClientOpen clientOpen{nullptr};
    ClientClose clientClose{nullptr};
    Activate activate{nullptr};
    Deactivate deactivate{nullptr};
    GetSampleRate getSampleRate{nullptr};
    GetBufferSize getBufferSize{nullptr};
    FramesSinceCycleStart framesSinceCycleStart{nullptr};
    SetProcessCallback setProcessCallback{nullptr};
    SetBufferSizeCallback setBufferSizeCallback{nullptr};
    OnInfoShutdown onInfoShutdown{nullptr};
    SetLatencyCallback setLatencyCallback{nullptr};
    SetSampleRateCallback setSampleRateCallback{nullptr};
    SetXrunCallback setXrunCallback{nullptr};
    PortRegister portRegister{nullptr};
    PortGetBuffer portGetBuffer{nullptr};
    PortGetLatencyRange portGetLatencyRange{nullptr};
    std::vector<jack_port_t*> inputPorts;
    std::vector<jack_port_t*> outputPorts;
    SoundDeviceJack* device{nullptr};

    template<typename Function>
    bool resolve(Function& function, const char* name) {
        function = reinterpret_cast<Function>(library.resolve(name));
        if (!function) {
            qWarning() << "Could not resolve JACK function" << name;
            return false;
        }
        return true;
    }

    bool loadSymbols() {
        if (!library.load()) {
            qWarning() << "Could not load JACK library:" << library.errorString();
            return false;
        }
        const bool requiredSymbolsLoaded =
                resolve(clientOpen, "jack_client_open") &&
                resolve(clientClose, "jack_client_close") &&
                resolve(activate, "jack_activate") &&
                resolve(deactivate, "jack_deactivate") &&
                resolve(getSampleRate, "jack_get_sample_rate") &&
                resolve(setProcessCallback, "jack_set_process_callback") &&
                resolve(onInfoShutdown, "jack_on_info_shutdown") &&
                resolve(portRegister, "jack_port_register") &&
                resolve(portGetBuffer, "jack_port_get_buffer");
        if (!requiredSymbolsLoaded) {
            library.unload();
            return false;
        }
        setBufferSizeCallback =
                reinterpret_cast<SetBufferSizeCallback>(
                        library.resolve("jack_set_buffer_size_callback"));
        getBufferSize =
                reinterpret_cast<GetBufferSize>(
                        library.resolve("jack_get_buffer_size"));
        framesSinceCycleStart =
                reinterpret_cast<FramesSinceCycleStart>(
                        library.resolve("jack_frames_since_cycle_start"));
        portGetLatencyRange =
                reinterpret_cast<PortGetLatencyRange>(
                        library.resolve("jack_port_get_latency_range"));
        setLatencyCallback =
                reinterpret_cast<SetLatencyCallback>(
                        library.resolve("jack_set_latency_callback"));
        setSampleRateCallback =
                reinterpret_cast<SetSampleRateCallback>(
                        library.resolve("jack_set_sample_rate_callback"));
        setXrunCallback =
                reinterpret_cast<SetXrunCallback>(
                        library.resolve("jack_set_xrun_callback"));
        return true;
    }

    static int process(jack_nframes_t framesPerBuffer, void* userData) {
        auto* self = static_cast<Impl*>(userData);
        return self->device->processCallback(framesPerBuffer);
    }

    static int bufferSizeChanged(jack_nframes_t, void*) {
        return 0;
    }

    static void latencyChanged(
            jack_latency_callback_mode_t mode, void* userData) {
        auto* self = static_cast<Impl*>(userData);
        if (mode != JackPlaybackLatency || !self->portGetLatencyRange) {
            return;
        }
        jack_nframes_t latencyFrames = 0;
        for (auto* port : self->outputPorts) {
            if (!port) {
                continue;
            }
            jack_latency_range_t range{};
            self->portGetLatencyRange(port, mode, &range);
            latencyFrames = std::max(latencyFrames, range.max);
        }
        self->device->m_outputLatencyFrames.store(
                latencyFrames, std::memory_order_release);
    }

    static void serverShutdown(jack_status_t, const char*, void* userData) {
        static_cast<SoundDeviceJack*>(userData)->m_serverShutdown.store(
                true, std::memory_order_release);
    }

    static int sampleRateChanged(jack_nframes_t sampleRate, void* userData) {
        auto* self = static_cast<SoundDeviceJack*>(userData);
        if (sampleRate != self->m_sampleRate.value()) {
            self->m_sampleRateChanged.store(true, std::memory_order_release);
        }
        return 0;
    }

    static int xrun(void* userData) {
        static_cast<SoundDeviceJack*>(userData)->m_pSoundManager->underflowHappened(6);
        return 0;
    }

    void closeClient() {
        if (client) {
            if (device->m_isOpen.exchange(false, std::memory_order_acq_rel) &&
                    !device->m_serverShutdown.load(std::memory_order_acquire)) {
                deactivate(client);
            }
            clientClose(client);
            client = nullptr;
        }
        inputPorts.clear();
        outputPorts.clear();
        library.unload();
    }
};

SoundDeviceJack::SoundDeviceJack(
        UserSettingsPointer config,
        SoundManager* soundManager,
        const QString& deviceName,
        int maxInputChannels,
        int maxOutputChannels,
        mixxx::audio::SampleRate defaultSampleRate,
        unsigned int deviceIndex)
        : SoundDevice(std::move(config), soundManager),
          m_pImpl(std::make_unique<Impl>()),
          m_defaultSampleRate(defaultSampleRate),
          m_audioLatencyUsage(kAppGroup, QStringLiteral("audio_latency_usage")),
          m_timeInAudioCallback(mixxx::Duration::fromSeconds(0)) {
    m_deviceId.name = deviceName;
    m_deviceId.deviceIndex = static_cast<int>(deviceIndex);
    m_strDisplayName = deviceName;
    m_hostAPI = SoundManagerConfig::kAPIJack;
    m_numInputChannels = mixxx::audio::ChannelCount(maxInputChannels);
    m_numOutputChannels = mixxx::audio::ChannelCount(maxOutputChannels);
    m_pImpl->device = this;
}

SoundDeviceJack::~SoundDeviceJack() {
    close();
}

SoundDeviceStatus SoundDeviceJack::open(bool isClkRefDevice, int) {
    if (isOpen()) {
        return SoundDeviceStatus::Ok;
    }
    m_pImpl->closeClient();
    m_serverShutdown.store(false, std::memory_order_release);
    m_sampleRateChanged.store(false, std::memory_order_release);
    if (m_audioOutputs.isEmpty() && m_audioInputs.isEmpty()) {
        m_lastError = QStringLiteral("No JACK input or output ports were configured");
        return SoundDeviceStatus::Error;
    }
    if (!isClkRefDevice) {
        m_lastError = QStringLiteral(
                "The native JACK client must be the Mixxx audio clock reference");
        return SoundDeviceStatus::Error;
    }
    if (!m_sampleRate.isValid()) {
        m_lastError = QStringLiteral("Invalid JACK sample rate");
        return SoundDeviceStatus::Error;
    }
    if (!m_pImpl->loadSymbols()) {
        m_lastError = QStringLiteral("Could not load the JACK client API");
        return SoundDeviceStatus::Error;
    }

    jack_status_t status = {};
    m_pImpl->client = m_pImpl->clientOpen(
            "mixxx", JackNoStartServer, &status);
    if (!m_pImpl->client) {
        m_lastError = QStringLiteral("Could not open a JACK client (status %1)")
                              .arg(static_cast<unsigned int>(status));
        m_pImpl->library.unload();
        return SoundDeviceStatus::Error;
    }

    const auto serverSampleRate =
            m_pImpl->getSampleRate(m_pImpl->client);
    if (serverSampleRate != m_sampleRate.value()) {
        m_lastError = QStringLiteral(
                "JACK server sample rate (%1 Hz) does not match "
                "the configured Mixxx sample rate (%2 Hz)")
                              .arg(serverSampleRate)
                              .arg(m_sampleRate.value());
        m_pImpl->closeClient();
        return SoundDeviceStatus::Error;
    }
    const jack_nframes_t initialBufferSize = m_pImpl->getBufferSize
            ? m_pImpl->getBufferSize(m_pImpl->client)
            : m_configFramesPerBuffer;
    m_lastCallbackEntryToDacSecs =
            static_cast<double>(initialBufferSize) / m_sampleRate.toDouble();

    const int configuredOutputChannels = [&] {
        int channels = 0;
        for (const auto& output : std::as_const(m_audioOutputs)) {
            const ChannelGroup group = output.getChannelGroup();
            channels = std::max<int>(
                    channels,
                    group.getChannelBase() + group.getChannelCount());
        }
        return channels;
    }();
    const int configuredInputChannels = [&] {
        int channels = 0;
        for (const auto& input : std::as_const(m_audioInputs)) {
            const ChannelGroup group = input.getChannelGroup();
            channels = std::max<int>(
                    channels,
                    group.getChannelBase() + group.getChannelCount());
        }
        return channels;
    }();

    auto outputNames = portNames(m_audioOutputs, configuredOutputChannels);
    auto inputNames = portNames(m_audioInputs, configuredInputChannels);
    m_pImpl->outputPorts.resize(static_cast<size_t>(configuredOutputChannels));
    m_pImpl->inputPorts.resize(static_cast<size_t>(configuredInputChannels));
    for (int channel = 0; channel < configuredOutputChannels; ++channel) {
        if (outputNames.at(static_cast<size_t>(channel)).isEmpty()) {
            continue;
        }
        const QByteArray portName =
                outputNames.at(static_cast<size_t>(channel)).toUtf8();
        auto* port = m_pImpl->portRegister(m_pImpl->client,
                portName.constData(),
                JACK_DEFAULT_AUDIO_TYPE,
                JackPortIsOutput,
                0);
        if (!port) {
            m_lastError = QStringLiteral("Could not register JACK output port %1")
                                  .arg(QString::fromUtf8(portName));
            m_pImpl->closeClient();
            return SoundDeviceStatus::Error;
        }
        m_pImpl->outputPorts.at(static_cast<size_t>(channel)) = port;
    }
    for (int channel = 0; channel < configuredInputChannels; ++channel) {
        if (inputNames.at(static_cast<size_t>(channel)).isEmpty()) {
            continue;
        }
        const QByteArray portName =
                inputNames.at(static_cast<size_t>(channel)).toUtf8();
        auto* port = m_pImpl->portRegister(m_pImpl->client,
                portName.constData(),
                JACK_DEFAULT_AUDIO_TYPE,
                JackPortIsInput,
                0);
        if (!port) {
            m_lastError = QStringLiteral("Could not register JACK input port %1")
                                  .arg(QString::fromUtf8(portName));
            m_pImpl->closeClient();
            return SoundDeviceStatus::Error;
        }
        m_pImpl->inputPorts.at(static_cast<size_t>(channel)) = port;
    }

    m_inputChannelCount = configuredInputChannels;
    m_outputChannelCount = configuredOutputChannels;
    m_inputBuffer.resize(
            static_cast<size_t>(kMaxEngineFrames) * m_inputChannelCount);
    m_outputBuffer.resize(
            static_cast<size_t>(kMaxEngineFrames) * m_outputChannelCount);
    m_pImpl->onInfoShutdown(
            m_pImpl->client, &Impl::serverShutdown, this);
    if (m_pImpl->setBufferSizeCallback) {
        if (m_pImpl->setBufferSizeCallback(
                    m_pImpl->client, &Impl::bufferSizeChanged, this) != 0) {
            qWarning() << "Could not install the JACK buffer-size callback";
        }
    }
    if (m_pImpl->setLatencyCallback) {
        if (m_pImpl->setLatencyCallback(
                    m_pImpl->client, &Impl::latencyChanged, m_pImpl.get()) != 0) {
            qWarning() << "Could not install the JACK latency callback";
        }
    }
    if (m_pImpl->setSampleRateCallback) {
        if (m_pImpl->setSampleRateCallback(
                    m_pImpl->client, &Impl::sampleRateChanged, this) != 0) {
            qWarning() << "Could not install the JACK sample-rate callback";
        }
    }
    if (m_pImpl->setXrunCallback) {
        if (m_pImpl->setXrunCallback(
                    m_pImpl->client, &Impl::xrun, this) != 0) {
            qWarning() << "Could not install the JACK xrun callback";
        }
    }
    if (m_pImpl->setProcessCallback(
                m_pImpl->client, &Impl::process, m_pImpl.get()) != 0) {
        m_lastError = QStringLiteral("Could not install the JACK process callback");
        m_pImpl->closeClient();
        return SoundDeviceStatus::Error;
    }

    m_serverShutdown.store(false, std::memory_order_release);
    m_bufferTooLarge.store(false, std::memory_order_release);
    m_outputLatencyFrames.store(0, std::memory_order_release);
    m_lastCallbackEntryToDacLatencyFrames = 0;
    m_timeInAudioCallback = mixxx::Duration::fromSeconds(0);
    m_framesSinceAudioLatencyUsageUpdate = 0;
    m_clkRefTimer.start();
    if (m_pImpl->activate(m_pImpl->client) != 0) {
        m_lastError = QStringLiteral("Could not activate the JACK client");
        m_pImpl->closeClient();
        return SoundDeviceStatus::Error;
    }
    m_isOpen.store(true, std::memory_order_release);
    if (m_serverShutdown.load(std::memory_order_acquire) ||
            m_sampleRateChanged.load(std::memory_order_acquire)) {
        m_lastError = m_serverShutdown.load(std::memory_order_acquire)
                ? QStringLiteral("The JACK server shut down while opening the client")
                : QStringLiteral("The JACK server sample rate changed while opening the client");
        m_pImpl->closeClient();
        return SoundDeviceStatus::Error;
    }

    Impl::latencyChanged(JackPlaybackLatency, m_pImpl.get());
    const double latencySeconds =
            m_outputLatencyFrames.load(std::memory_order_acquire) /
            m_sampleRate.toDouble();
    ControlObject::set(
            ConfigKey(kAppGroup, QStringLiteral("output_latency_ms")),
            latencySeconds * 1000.0);
    ControlObject::set(
            ConfigKey(kAppGroup, QStringLiteral("samplerate")),
            m_sampleRate);

    m_lastError.clear();
    qInfo() << "Opened native JACK device" << m_deviceId.debugName()
            << "with" << configuredInputChannels << "input and"
            << configuredOutputChannels << "output channels";
    return SoundDeviceStatus::Ok;
}

bool SoundDeviceJack::isOpen() const {
    return m_isOpen.load(std::memory_order_acquire) &&
            !m_serverShutdown.load(std::memory_order_acquire) &&
            !m_sampleRateChanged.load(std::memory_order_acquire);
}

SoundDeviceStatus SoundDeviceJack::close() {
    m_pImpl->closeClient();
    m_isOpen.store(false, std::memory_order_release);
    m_serverShutdown.store(false, std::memory_order_release);
    m_sampleRateChanged.store(false, std::memory_order_release);
    m_inputBuffer.clear();
    m_outputBuffer.clear();
    m_inputChannelCount = 0;
    m_outputChannelCount = 0;
    return SoundDeviceStatus::Ok;
}

void SoundDeviceJack::readProcess(SINT) {
}

void SoundDeviceJack::writeProcess(SINT) {
}

QString SoundDeviceJack::getError() const {
    if (m_serverShutdown.load(std::memory_order_acquire)) {
        return QStringLiteral("The JACK server shut down");
    }
    if (m_sampleRateChanged.load(std::memory_order_acquire)) {
        return QStringLiteral("The JACK server sample rate changed; reopen audio devices");
    }
    if (m_bufferTooLarge.load(std::memory_order_acquire)) {
        return QStringLiteral("JACK buffer size exceeds Mixxx's maximum audio block size");
    }
    return m_lastError;
}

mixxx::audio::SampleRate SoundDeviceJack::getDefaultSampleRate() const {
    return m_defaultSampleRate;
}

int SoundDeviceJack::processCallback(unsigned int framesPerBuffer) {
    const double timeSinceLastCallbackSecs =
            m_clkRefTimer.restart().toDoubleSeconds();
    if (framesPerBuffer > kMaxEngineFrames) {
        m_bufferTooLarge.store(true, std::memory_order_release);
    }
    if (framesPerBuffer > kMaxEngineFrames ||
            m_sampleRateChanged.load(std::memory_order_acquire)) {
        for (auto* port : m_pImpl->outputPorts) {
            if (port) {
                auto* buffer = static_cast<CSAMPLE*>(
                        m_pImpl->portGetBuffer(port, framesPerBuffer));
                if (buffer) {
                    std::fill_n(buffer, framesPerBuffer, 0.0f);
                }
            }
        }
        return 0;
    }

    updateCallbackEntryToDacTime(
            framesPerBuffer, timeSinceLastCallbackSecs);
    m_pSoundManager->processUnderflowHappened(
            static_cast<SINT>(framesPerBuffer));

    if (m_inputChannelCount > 0) {
        CSAMPLE* interleavedInput = m_inputBuffer.data();
        std::fill_n(
                interleavedInput,
                static_cast<size_t>(framesPerBuffer) * m_inputChannelCount,
                0.0f);
        for (int channel = 0; channel < m_inputChannelCount; ++channel) {
            auto* port = m_pImpl->inputPorts[static_cast<size_t>(channel)];
            if (!port) {
                continue;
            }
            const auto* portBuffer = static_cast<const CSAMPLE*>(
                    m_pImpl->portGetBuffer(port, framesPerBuffer));
            if (!portBuffer) {
                continue;
            }
            for (unsigned int frame = 0; frame < framesPerBuffer; ++frame) {
                interleavedInput[static_cast<size_t>(frame) *
                                static_cast<size_t>(m_inputChannelCount) +
                        static_cast<size_t>(channel)] =
                        portBuffer[frame];
            }
        }
        composeInputBuffer(
                interleavedInput,
                static_cast<SINT>(framesPerBuffer),
                0,
                m_inputChannelCount);
        m_pSoundManager->pushInputBuffers(
                m_audioInputs, static_cast<SINT>(framesPerBuffer));
    }

    m_pSoundManager->readProcess(static_cast<SINT>(framesPerBuffer));
    m_pSoundManager->onDeviceOutputCallback(
            static_cast<SINT>(framesPerBuffer));

    if (m_outputChannelCount > 0) {
        CSAMPLE* interleavedOutput = m_outputBuffer.data();
        composeOutputBuffer(
                interleavedOutput,
                static_cast<SINT>(framesPerBuffer),
                0,
                m_outputChannelCount);
        for (int channel = 0; channel < m_outputChannelCount; ++channel) {
            auto* port = m_pImpl->outputPorts[static_cast<size_t>(channel)];
            if (!port) {
                continue;
            }
            auto* portBuffer = static_cast<CSAMPLE*>(
                    m_pImpl->portGetBuffer(port, framesPerBuffer));
            if (!portBuffer) {
                continue;
            }
            for (unsigned int frame = 0; frame < framesPerBuffer; ++frame) {
                portBuffer[frame] = interleavedOutput[static_cast<size_t>(frame) *
                                static_cast<size_t>(m_outputChannelCount) +
                        static_cast<size_t>(channel)];
            }
        }
    }
    m_pSoundManager->writeProcess(static_cast<SINT>(framesPerBuffer));
    updateAudioLatencyUsage(framesPerBuffer);
    return 0;
}

void SoundDeviceJack::updateCallbackEntryToDacTime(
        unsigned int framesPerBuffer, double timeSinceLastCallbackSecs) {
    const double bufferSizeSecs =
            framesPerBuffer / m_sampleRate.toDouble();
    const uint32_t outputLatencyFrames =
            m_outputLatencyFrames.load(std::memory_order_acquire);
    const bool outputLatencyChanged =
            outputLatencyFrames != m_lastCallbackEntryToDacLatencyFrames;
    m_lastCallbackEntryToDacLatencyFrames = outputLatencyFrames;
    double callbackEntryToDacSecs = 0;
    if (m_pImpl->framesSinceCycleStart) {
        const int64_t callbackStartOffsetFrames =
                m_pImpl->framesSinceCycleStart(m_pImpl->client);
        callbackEntryToDacSecs =
                (static_cast<int64_t>(outputLatencyFrames) -
                        callbackStartOffsetFrames) /
                        m_sampleRate.toDouble() +
                m_clkRefTimer.elapsed().toDoubleSeconds();
    }

    const double timingDiff =
            timeSinceLastCallbackSecs + callbackEntryToDacSecs -
            (m_lastCallbackEntryToDacSecs + bufferSizeSecs);
    if (callbackEntryToDacSecs <= 0 ||
            (!outputLatencyChanged &&
                    timeSinceLastCallbackSecs < bufferSizeSecs * 2 &&
                    std::abs(timingDiff) / bufferSizeSecs > 0.1)) {
        callbackEntryToDacSecs =
                m_lastCallbackEntryToDacSecs + bufferSizeSecs -
                timeSinceLastCallbackSecs;
        callbackEntryToDacSecs =
                std::clamp(callbackEntryToDacSecs, 0.0, bufferSizeSecs * 2);
    }
    m_lastCallbackEntryToDacSecs = callbackEntryToDacSecs;
    VisualPlayPosition::setCallbackEntryToDacSecs(
            callbackEntryToDacSecs, m_clkRefTimer);
}

void SoundDeviceJack::updateAudioLatencyUsage(unsigned int framesPerBuffer) {
    m_framesSinceAudioLatencyUsageUpdate += framesPerBuffer;
    if (m_framesSinceAudioLatencyUsageUpdate >
            (m_sampleRate.value() / kCpuUsageUpdateRate)) {
        const double secondsInAudioCallback =
                m_timeInAudioCallback.toDoubleSeconds();
        m_audioLatencyUsage.set(
                secondsInAudioCallback /
                (m_framesSinceAudioLatencyUsageUpdate / m_sampleRate.toDouble()));
        m_timeInAudioCallback = mixxx::Duration::fromSeconds(0);
        m_framesSinceAudioLatencyUsageUpdate = 0;
    }
    m_timeInAudioCallback += m_clkRefTimer.elapsed();
}
