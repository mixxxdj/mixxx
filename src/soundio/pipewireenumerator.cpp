#include "soundio/pipewireenumerator.h"

#include <pipewire/pipewire.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>
#include <spa/utils/result.h>

#include <QList>
#include <QSharedPointer>
#include <QStringView>
#include <string>

#include "audio/types.h"
#include "control/controlobject.h"
#include "moc_pipewireenumerator.cpp"
#include "soundio/sounddevice.h"
#include "soundio/sounddevicepipewire.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"
#include "util/assert.h"
#include "util/trace.h"
#include "util/types.h"
#include "waveform/visualplayposition.h"

namespace {

constexpr int kCpuUsageUpdateRate = 30; // in 1/s, fits to display frame rate
const QString kAppGroup = QStringLiteral("[App]");

static const char* find_node_name(const struct spa_dict* props) {
    static const char* const name_keys[] = {
            PW_KEY_NODE_NAME,
            PW_KEY_NODE_DESCRIPTION,
            PW_KEY_APP_NAME,
            PW_KEY_MEDIA_NAME,
    };

    for (const char* key : name_keys) {
        const char* name = spa_dict_lookup(props, key);
        if (name) {
            return name;
        }
    }
    return nullptr;
}
} // namespace

PipewireEnumerator::PipewireEnumerator(UserSettingsPointer, SoundManager* pManager)
        : m_pSoundManager(pManager),
          m_pPwThreadLoop(nullptr),
          m_pPwContext(nullptr),
          m_pPwCore(nullptr),
          m_pPwRegistry(nullptr),
          m_pPwMetadata(nullptr),
          m_pPwFilter(nullptr),
          m_initialized(false),
          m_sampleRate(48000),
          m_audioLatencyUsage(kAppGroup, QStringLiteral("audio_latency_usage")),
          m_framesPerBuffer(0) {
    connect(m_pSoundManager,
            &SoundManager::inputRegistered,
            this,
            &PipewireEnumerator::registerInput);
    connect(m_pSoundManager,
            &SoundManager::outputRegistered,
            this,
            &PipewireEnumerator::registerOutput);

    connect(this, &PipewireEnumerator::deviceAdded, m_pSoundManager, &SoundManager::addDevice);
    connect(this, &PipewireEnumerator::deviceRemoved, m_pSoundManager, &SoundManager::removeDevice);

    pw_init(nullptr, nullptr);

    m_pPwThreadLoop = pw_thread_loop_new("mixxx_loop", nullptr);
    spa_zero(m_pwRegistryListener);
    spa_zero(m_pwMetadataListener);
    spa_zero(m_pwFilterListener);

    initialize();
}

PipewireEnumerator::~PipewireEnumerator() {
    pw_thread_loop_stop(m_pPwThreadLoop);

    if (m_pPwFilter) {
        spa_hook_remove(&m_pwFilterListener);
        pw_filter_destroy(m_pPwFilter);
    }

    if (m_pPwMetadata) {
        spa_hook_remove(&m_pwMetadataListener);
        pw_proxy_destroy((struct pw_proxy*)m_pPwMetadata);
    }

    if (m_pPwRegistry) {
        spa_hook_remove(&m_pwRegistryListener);
        pw_proxy_destroy((struct pw_proxy*)m_pPwRegistry);
    }

    if (m_pPwCore) {
        pw_core_disconnect(m_pPwCore);
    }

    if (m_pPwContext) {
        pw_context_destroy(m_pPwContext);
    }

    pw_thread_loop_destroy(m_pPwThreadLoop);
    pw_deinit();
}

void PipewireEnumerator::initialize() {
    if (m_initialized) {
        return;
    }

    if (!m_pPwContext) {
        m_pPwContext = pw_context_new(pw_thread_loop_get_loop(m_pPwThreadLoop), nullptr, 0);
        if (!m_pPwContext) {
            qWarning() << "PipewireEnumerator::initialize pw_context_new "
                          "failed with error:"
                       << spa_strerror(errno);
            return;
        }
    }

    m_pPwCore = pw_context_connect(m_pPwContext, nullptr, 0);

    if (!m_pPwCore) {
        qWarning() << "PipewireEnumerator::initialize pw_context_connect "
                      "failed with error:"
                   << spa_strerror(errno);
        return;
    }

    m_pPwRegistry = pw_core_get_registry(m_pPwCore, PW_VERSION_REGISTRY, 0);
    pw_registry_add_listener(m_pPwRegistry, &m_pwRegistryListener, &registry_events, this);

    // see https://docs.pipewire.org/page_man_pipewire-props_7.html
    // and pipewire/keys.h header
    m_pPwFilter = pw_filter_new(m_pPwCore,
            "mixxx",
            pw_properties_new(PW_KEY_MEDIA_NAME,
                    "Mixxx",
                    PW_KEY_MEDIA_TYPE,
                    "Audio",
                    PW_KEY_MEDIA_CATEGORY,
                    "Duplex",
                    PW_KEY_MEDIA_ROLE,
                    "Production",
                    PW_KEY_MEDIA_CLASS,
                    "Audio/Duplex",
                    PW_KEY_NODE_NAME,
                    "Mixxx",
                    PW_KEY_NODE_NICK,
                    "Mixxx",
                    nullptr));

    pw_filter_add_listener(m_pPwFilter, &m_pwFilterListener, &filter_events, this);

    for (auto it = m_inputs.begin(); it != m_inputs.end(); ++it) {
        it.value() = createInputPorts(it.key());
    }

    for (auto it = m_outputs.begin(); it != m_outputs.end(); ++it) {
        it.value() = createOutputPorts(it.key());
    }

    int res = pw_filter_connect(m_pPwFilter,
            PW_FILTER_FLAG_RT_PROCESS,
            nullptr,
            0);

    VERIFY_OR_DEBUG_ASSERT(res >= 0) {
        qWarning() << "PipewireEnumerator::initialize pw_filter_connect error:"
                   << spa_strerror(res);
    }

    pw_thread_loop_start(m_pPwThreadLoop);

    m_initialized = true;
}

QList<mixxx::audio::SampleRate> PipewireEnumerator::getSampleRates() const {
    return m_samplerates;
}

void PipewireEnumerator::registryEventGlobal(uint32_t id,
        uint32_t,
        const char* pType,
        uint32_t,
        const struct spa_dict* pProps) {
    if (strcmp(pType, PW_TYPE_INTERFACE_Metadata) == 0) {
        const char* name = spa_dict_lookup(pProps, PW_KEY_METADATA_NAME);
        if (strcmp(name, "settings") != 0) {
            return;
        }

        void* data = pw_registry_bind(m_pPwRegistry,
                id,
                PW_TYPE_INTERFACE_Metadata,
                PW_VERSION_METADATA,
                0);
        m_pPwMetadata = static_cast<pw_metadata*>(data);
        pw_metadata_add_listener(m_pPwMetadata, &m_pwMetadataListener, &metadataEvents, this);
    } else if (strcmp(pType, PW_TYPE_INTERFACE_Node) == 0) {
        const char* media_class = spa_dict_lookup(pProps, PW_KEY_MEDIA_CLASS);
        const char* media_type = spa_dict_lookup(pProps, PW_KEY_MEDIA_TYPE);

        bool isAudioNode = (media_class && strstr(media_class, "Audio")) ||
                (media_type && strstr(media_type, "Audio"));

        if (!isAudioNode) {
            return;
        }

        const char* name = find_node_name(pProps);

        m_objects.insert_or_assign(id, Object{Node{}});
        auto pDevice = QSharedPointer<SoundDevicePipewire>::create(
                m_pConfig, m_pSoundManager, this, id, name);
        emit deviceAdded(pDevice);
        // pipewire assigns each object with a unique ID
        // any previous element is either invalid or already removed
        m_soundDevices.insert_or_assign(id, std::move(pDevice));

        // this can be fooled if a different application names its node "Mixxx"
        if (strcmp(name, "Mixxx") == 0) {
            m_filterId = id;
        }
    } else if (strcmp(pType, PW_TYPE_INTERFACE_Port) == 0) {
        const uint32_t node_id = pw_properties_parse_int(spa_dict_lookup(pProps, PW_KEY_NODE_ID));
        if (!m_soundDevices.contains(node_id)) {
            // most likely midi or video node
            return;
        }

        m_objects.insert_or_assign(id, Object{Port(node_id)});
        QSharedPointer<SoundDevicePipewire> pSoundDevice = m_soundDevices.at(node_id);
        pSoundDevice->registerPort(id, pProps);
        m_pSoundManager->updateDeviceChannels(pSoundDevice);

        const char* direction = spa_dict_lookup(pProps, PW_KEY_PORT_DIRECTION);

        if (node_id == m_filterId) {
            QString name(spa_dict_lookup(pProps, PW_KEY_PORT_NAME));
            QStringList list = name.split(':');
            if (strcmp(direction, "in") == 0) {
                QList<AudioInput> keys = m_inputs.keys();
                auto it = std::ranges::find(keys, list.at(0), &AudioPath::getString);
                VERIFY_OR_DEBUG_ASSERT(it != keys.end()) {
                    return;
                }

                if (list.at(1) == "FL") {
                    *m_inputs.value(*it).first = id;
                } else {
                    *m_inputs.value(*it).second = id;
                }
            } else {
                QList<AudioOutput> keys = m_outputs.keys();
                auto it = std::ranges::find(keys, list.at(0), &AudioPath::getString);
                VERIFY_OR_DEBUG_ASSERT(it != keys.end()) {
                    return;
                }

                if (list.at(1) == "FL") {
                    *m_outputs.value(*it).first = id;
                } else {
                    *m_outputs.value(*it).second = id;
                }
            }
        }
    } else if (strcmp(pType, PW_TYPE_INTERFACE_Link) == 0) {
        const uint32_t in_node = pw_properties_parse_int(
                spa_dict_lookup(pProps, PW_KEY_LINK_INPUT_NODE));
        const uint32_t in_port = pw_properties_parse_int(
                spa_dict_lookup(pProps, PW_KEY_LINK_INPUT_PORT));
        const uint32_t out_node = pw_properties_parse_int(
                spa_dict_lookup(pProps, PW_KEY_LINK_OUTPUT_NODE));
        const uint32_t out_port = pw_properties_parse_int(
                spa_dict_lookup(pProps, PW_KEY_LINK_OUTPUT_PORT));

        if (in_node == m_filterId) {
            m_objects.insert_or_assign(id, Object{Link(in_port, out_port)});
            m_soundDevices.at(out_node)->registerLink(id, SPA_DIRECTION_OUTPUT);
        } else if (out_node == m_filterId) {
            m_objects.insert_or_assign(id, Object{Link(in_port, out_port)});
            m_soundDevices.at(in_node)->registerLink(id, SPA_DIRECTION_INPUT);
        }
    }
}

void PipewireEnumerator::registryEventGlobalRemove(unsigned int id) {
    if (!m_objects.contains(id)) {
        return;
    }

    auto pair = m_objects.extract(id);
    Object& object = pair.mapped();

    if (std::get_if<Node>(&object)) {
        if (!m_soundDevices.contains(id)) {
            return;
        }

        QSharedPointer<SoundDevicePipewire> pDevice = m_soundDevices.at(id);
        if (pDevice->isOpen()) {
            pDevice->close();
        }

        m_soundDevices.erase(id);
        emit deviceRemoved(pDevice);
        // m_pSoundManager->removeDevice(device);
    } else if (Port* port = std::get_if<Port>(&object)) {
        VERIFY_OR_DEBUG_ASSERT(m_soundDevices.contains(port->node)) {
            return;
        }

        QSharedPointer<SoundDevicePipewire> pSoundDevice = m_soundDevices.at(port->node);
        pSoundDevice->unregisterPort(id);
        m_pSoundManager->updateDeviceChannels(pSoundDevice);
    } else if (Link* link = std::get_if<Link>(&object)) {
        Port input = std::get<Port>(m_objects.at(link->input));
        Port output = std::get<Port>(m_objects.at(link->output));

        if (input.node == m_filterId) {
            m_soundDevices.at(output.node)->unregisterLink(id, SPA_DIRECTION_OUTPUT);
        } else if (output.node == m_filterId) {
            m_soundDevices.at(input.node)->unregisterLink(id, SPA_DIRECTION_INPUT);
        }
    }
}

std::vector<SoundDevicePointer> PipewireEnumerator::queryDevices() const {
    std::vector<SoundDevicePointer> devices;
    for (const auto& [id, pDevice] : m_soundDevices) {
        devices.push_back(pDevice);
    }

    return devices;
}

int PipewireEnumerator::metadataProperty(
        void* data, uint32_t, const char* key, const char*, const char* value) {
    PipewireEnumerator* pEnumerator = static_cast<PipewireEnumerator*>(data);

    if (strcmp(key, "clock.rate") == 0) {
        pEnumerator->m_defaultSampleRate = mixxx::audio::SampleRate(std::atoi(value));
    } else if (strcmp(key, "clock.allowed-rates") == 0) {
        qDebug() << "PipewireEnumerator::metadataProperty clock.allowed-rates" << value;
        // parse json arrays like [ 44100, 48000, 96000 ]
        QString s = value;
        s.remove('[');
        s.remove(']');

        const QStringList parts = s.split(',', Qt::SkipEmptyParts);

        for (const QString& part : parts) {
            pEnumerator->m_samplerates.push_back(mixxx::audio::SampleRate(part.trimmed().toInt()));
        }
    }
    return 0;
}

bool PipewireEnumerator::isOpen(uint32_t id) {
    return std::ranges::find(m_openedDevices, id) != m_openedDevices.end();
}

std::string PipewireEnumerator::openDevice(const SoundDevicePipewire& device,
        mixxx::audio::SampleRate sampleRate,
        SINT framesPerBuffer) {
    std::string result;
    VERIFY_OR_DEBUG_ASSERT(m_initialized) {
        qWarning() << "PipewireEnumerator::openDevice called when "
                      "uninitialized, this should not happen";
        return "PipewireEnumerator uninitialized";
    }

    if (sampleRate != m_sampleRate || framesPerBuffer != m_framesPerBuffer) {
        setLatency(sampleRate, framesPerBuffer);
    }

    int deviceId = device.getDeviceId().deviceIndex;

    VERIFY_OR_DEBUG_ASSERT(std::ranges::find(m_openedDevices, deviceId) == m_openedDevices.end()) {
        qWarning() << "SoundDevicePipewire:" << deviceId << "already open";
        return "Device already open";
    }

    pw_thread_loop_lock(m_pPwThreadLoop);

    // device.inputs() corresponds to output ports of device node
    QList<AudioInput> inKeys = m_inputs.keys();
    for (const AudioInputBuffer& input : device.inputs()) {
        auto it = std::ranges::find_if(inKeys, [input](const AudioPath& path) {
            return path.getType() == input.getType() && path.getIndex() == input.getIndex();
        });

        VERIFY_OR_DEBUG_ASSERT(it != inKeys.end()) {
            continue;
        }

        std::pair<uint32_t*, uint32_t*> filterPorts = m_inputs.value(*it);
        ChannelGroup channelGroup = input.getChannelGroup();
        unsigned char channelBase = channelGroup.getChannelBase();
        unsigned char channelCount = channelGroup.getChannelCount().value();
        auto ports = device.getOutPorts();

        if (channelCount == 1) {
            uint32_t filterPort = channelBase % 2 ? *filterPorts.second : *filterPorts.first;
            result += createLink(deviceId, ports[channelBase].id, m_filterId, filterPort);
        } else {
            result += createLink(deviceId, ports[channelBase].id, m_filterId, *filterPorts.first);
            result += createLink(deviceId,
                    ports[channelBase + 1].id,
                    m_filterId,
                    *filterPorts.second);
        }
    }

    // device.outputs() corresponds to input ports of device node
    QList<AudioOutput> outKeys = m_outputs.keys();
    for (const AudioOutputBuffer& output : device.outputs()) {
        auto it = std::ranges::find_if(outKeys, [output](const AudioPath& path) {
            return path.getType() == output.getType() && path.getIndex() == output.getIndex();
        });

        VERIFY_OR_DEBUG_ASSERT(it != outKeys.end()) {
            continue;
        }

        std::pair<uint32_t*, uint32_t*> filterPorts = m_outputs.value(*it);
        ChannelGroup channelGroup = output.getChannelGroup();
        unsigned char channelBase = channelGroup.getChannelBase();
        unsigned char channelCount = channelGroup.getChannelCount().value();
        auto ports = device.getInPorts();

        if (channelCount == 1) {
            uint32_t filterPort = channelBase % 2 ? *filterPorts.second : *filterPorts.first;
            result += createLink(m_filterId, filterPort, deviceId, ports[channelBase].id);
        } else {
            result += createLink(m_filterId, *filterPorts.first, deviceId, ports[channelBase].id);
            result += createLink(m_filterId,
                    *filterPorts.second,
                    deviceId,
                    ports[channelBase + 1].id);
        }
    }
    pw_thread_loop_unlock(m_pPwThreadLoop);
    m_openedDevices.push_back(deviceId);
    return result;
}

void PipewireEnumerator::closeDevice(uint32_t id) {
    VERIFY_OR_DEBUG_ASSERT(m_initialized) {
        qWarning() << "PipewireEnumerator::closeDevice called when "
                      "uninitialized, this should not happen";
        return;
    }

    auto deviceId = std::ranges::find(m_openedDevices, id);

    VERIFY_OR_DEBUG_ASSERT(deviceId != m_openedDevices.end()) {
        qWarning() << "device:" << id << "not opened";
        return;
    }

    QSharedPointer<SoundDevicePipewire> pDevice = m_soundDevices.at(*deviceId);

    // device m_inLinks and m_outLinks are cleared by link registryEventGlobalRemove
    for (uint32_t link : pDevice->getInLinks()) {
        destroyLink(link);
    }

    for (uint32_t link : pDevice->getOutLinks()) {
        destroyLink(link);
    }

    m_openedDevices.erase(deviceId);
}

void PipewireEnumerator::callback(const spa_io_position* pos) {
    // This must be the very first call, else timeInfo becomes invalid
    m_clkRefTimer.restart().toDoubleSeconds();
    VisualPlayPosition::setCallbackEntryToDacSecs(
            pos->clock.delay / pos->clock.rate.denom, m_clkRefTimer);

    Trace trace("SoundDevicePw::callbackProcessClkRef");

#if PW_CHECK_VERSION(0, 3, 50)
    if (pos->clock.xrun > xrun_duration) {
        xrun_duration = pos->clock.xrun;
        m_pSoundManager->underflowHappened(6);
    }
#endif

    const uint32_t sampleRate = pos->clock.rate.denom;
    const uint64_t framesPerBuffer = pos->clock.duration;

    if (sampleRate != m_sampleRate || framesPerBuffer != m_framesPerBuffer) {
        qWarning() << "PipewireEnumerator::callback"
                      "requested"
                   << m_framesPerBuffer << "samples at" << m_sampleRate << "hz,"
                                                                           "provided"
                   << framesPerBuffer << "samples at" << sampleRate << "hz";
        setLatency(sampleRate, framesPerBuffer);
    }

    qDebug() << "PipewireEnumerator::callback" << sampleRate << framesPerBuffer;
    m_pSoundManager->processUnderflowHappened(framesPerBuffer);

    for (uint32_t deviceId : m_openedDevices) {
        QSharedPointer<SoundDevicePipewire> device = m_soundDevices.at(deviceId);
        QList<AudioInputBuffer> deviceInputs = device->inputs();
        for (const AudioInputBuffer& input : deviceInputs) {
            ChannelGroup channelGroup = input.getChannelGroup();
            const int iChannelCount = channelGroup.getChannelCount();
            const int iChannelBase = channelGroup.getChannelBase();
            CSAMPLE* pInputBuffer = input.getBuffer();

            std::pair<uint32_t*, uint32_t*> ports = m_inputs.value(input);

            if (iChannelCount == 1) {
                void* portData = iChannelBase % 2 ? ports.second : ports.first;
                const float* buffer = static_cast<const float*>(
                        pw_filter_get_dsp_buffer(portData, framesPerBuffer));
                if (buffer) {
                    for (uint64_t i = 0; i < framesPerBuffer; i++) {
                        pInputBuffer[i * 2] = buffer[i];
                        pInputBuffer[i * 2 + 1] = buffer[i];
                    }
                }
            } else {
                const float* bufferFL = static_cast<const float*>(
                        pw_filter_get_dsp_buffer(ports.first, framesPerBuffer));
                if (bufferFL) {
                    for (uint64_t i = 0; i < framesPerBuffer; i++) {
                        pInputBuffer[iChannelBase + i * 2] = bufferFL[i];
                    }
                }

                const float* bufferFR = static_cast<const float*>(
                        pw_filter_get_dsp_buffer(ports.second, framesPerBuffer));
                if (bufferFR) {
                    for (uint64_t i = 0; i < framesPerBuffer; i++) {
                        pInputBuffer[iChannelBase + 1 + i * 2] = bufferFR[i];
                    }
                }
            }
        }
        m_pSoundManager->pushInputBuffers(deviceInputs, framesPerBuffer);
    }

    m_pSoundManager->onDeviceOutputCallback(framesPerBuffer);

    for (uint32_t deviceId : m_openedDevices) {
        QSharedPointer<SoundDevicePipewire> device = m_soundDevices.at(deviceId);
        for (const AudioOutputBuffer& output : device->outputs()) {
            ChannelGroup chanGroup = output.getChannelGroup();
            const int iChannelCount = chanGroup.getChannelCount();
            const int iChannelBase = chanGroup.getChannelBase();
            const CSAMPLE* pOutputBuffer = output.getBuffer();

            std::pair<uint32_t*, uint32_t*> ports = m_outputs.value(output);

            if (iChannelCount == 1) {
                void* portData = iChannelBase % 2 ? ports.second : ports.first;
                float* buffer = static_cast<float*>(
                        pw_filter_get_dsp_buffer(portData, framesPerBuffer));
                if (buffer) {
                    for (uint64_t i = 0; i < framesPerBuffer; i++) {
                        buffer[i] = pOutputBuffer[iChannelBase + i * 2];
                    }
                }
            } else {
                float* bufferFL = static_cast<float*>(
                        pw_filter_get_dsp_buffer(ports.first, framesPerBuffer));
                if (bufferFL) {
                    for (uint64_t i = 0; i < framesPerBuffer; i++) {
                        bufferFL[i] = pOutputBuffer[iChannelBase + i * 2];
                    }
                }

                float* bufferFR = static_cast<float*>(
                        pw_filter_get_dsp_buffer(ports.second, framesPerBuffer));
                if (bufferFR) {
                    for (uint64_t i = 0; i < framesPerBuffer; i++) {
                        bufferFR[i] = pOutputBuffer[iChannelBase + 1 + i * 2];
                    }
                }
            }
        }
    }

    updateAudioLatencyUsage(framesPerBuffer);
}

void PipewireEnumerator::updateAudioLatencyUsage(const SINT framesPerBuffer) {
    m_framesSinceAudioLatencyUsageUpdate += framesPerBuffer;
    if (m_framesSinceAudioLatencyUsageUpdate > (m_sampleRate.toDouble() / kCpuUsageUpdateRate)) {
        double secInAudioCb = m_timeInAudioCallback.toDoubleSeconds();
        m_audioLatencyUsage.set(
                secInAudioCb / (m_framesSinceAudioLatencyUsageUpdate / m_sampleRate.toDouble()));
        m_timeInAudioCallback = mixxx::Duration::fromSeconds(0);
        m_framesSinceAudioLatencyUsageUpdate = 0;
        // qDebug() << m_audioLatencyUsage
        //          << m_audioLatencyUsage->get();
    }
    // measure time in Audio callback at the very last
    m_timeInAudioCallback += m_clkRefTimer.elapsed();
}

void PipewireEnumerator::destroyLink(uint32_t id) {
    pw_thread_loop_lock(m_pPwThreadLoop);
    pw_registry_destroy(m_pPwRegistry, id);
    pw_thread_loop_unlock(m_pPwThreadLoop);
}

std::string PipewireEnumerator::createLink(uint32_t outNodeId,
        uint32_t outPortId,
        uint32_t inNodeId,
        uint32_t inPortId) {
    // qDebug() << "PipewireEnumerator::createLink" << outNodeId << outPortId <<
    // inNodeId << inPortId;
    spa_dict_item items[6];
    spa_dict props = SPA_DICT_INIT(items, 0);

    std::string strOutNode = std::to_string(outNodeId);
    std::string strOutPort = std::to_string(outPortId);
    std::string strInNode = std::to_string(inNodeId);
    std::string strInPort = std::to_string(inPortId);

    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_LINK_OUTPUT_NODE, strOutNode.c_str());
    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_LINK_OUTPUT_PORT, strOutPort.c_str());
    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_LINK_INPUT_NODE, strInNode.c_str());
    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_LINK_INPUT_PORT, strInPort.c_str());
    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_OBJECT_LINGER, "true");

    struct pw_proxy* pProxy = static_cast<pw_proxy*>(pw_core_create_object(m_pPwCore,
            "link-factory",
            PW_TYPE_INTERFACE_Link,
            PW_VERSION_LINK,
            &props,
            0));
    if (pProxy) {
        pw_proxy_destroy(pProxy);
        return {};
    }

    return "createLink failed: outNodeId: " +
            std::to_string(outNodeId) +
            "outPortId: " + std::to_string(outPortId) +
            "inNodeId: " + std::to_string(inNodeId) +
            "inPortId: " + std::to_string(inPortId);
}

void PipewireEnumerator::registerInput(const AudioInput& input, AudioDestination*) {
    if (m_inputs.contains(input) or input.isHidden()) {
        // duplicate VinylControl signal
        return;
    }

    if (m_initialized) {
        pw_thread_loop_lock(m_pPwThreadLoop);
        m_inputs.insert(input, createInputPorts(input));
        pw_thread_loop_unlock(m_pPwThreadLoop);
    } else {
        m_inputs.insert(input, {});
    }
}

void PipewireEnumerator::registerOutput(const AudioOutput& output, AudioSource*) {
    if (output.isHidden()) {
        return;
    }

    if (m_initialized) {
        pw_thread_loop_lock(m_pPwThreadLoop);
        m_outputs.insert(output, createOutputPorts(output));
        pw_thread_loop_unlock(m_pPwThreadLoop);
    } else {
        m_outputs.insert(output, {});
    }
}

// need to pw_thread_loop_lock before calling this
std::pair<uint32_t*, uint32_t*> PipewireEnumerator::createPorts(
        std::string_view name, spa_direction direction) {
    pw_properties* props = pw_properties_new(
            // see pipewire/keys.h header
            PW_KEY_FORMAT_DSP,
            "32 bit float mono audio",
            nullptr);
    pw_properties_setf(props, PW_KEY_PORT_NAME, "%s:FL", name.data());

    void* leftPort = pw_filter_add_port(m_pPwFilter,
            direction,
            PW_FILTER_PORT_FLAG_MAP_BUFFERS,
            sizeof(uint32_t),
            props,
            nullptr,
            0);

    props = pw_properties_new(
            // see pipewire/keys.h header
            PW_KEY_FORMAT_DSP,
            "32 bit float mono audio",
            nullptr);
    pw_properties_setf(props, PW_KEY_PORT_NAME, "%s:FR", name.data());

    void* rightPort = pw_filter_add_port(m_pPwFilter,
            direction,
            PW_FILTER_PORT_FLAG_MAP_BUFFERS,
            sizeof(uint32_t),
            props,
            nullptr,
            0);
    return std::pair{static_cast<uint32_t*>(leftPort), static_cast<uint32_t*>(rightPort)};
}

// need to pw_thread_loop_lock before calling this
std::pair<uint32_t*, uint32_t*> PipewireEnumerator::createInputPorts(const AudioInput& input) {
    std::string inputName = input.getString().toStdString();
    return createPorts(inputName, SPA_DIRECTION_INPUT);
}

// need to pw_thread_loop_lock before calling this
std::pair<uint32_t*, uint32_t*> PipewireEnumerator::createOutputPorts(const AudioOutput& output) {
    std::string outputName = output.getString().toStdString();
    return createPorts(outputName, SPA_DIRECTION_OUTPUT);
}

void PipewireEnumerator::setLatency(unsigned int sampleRate, unsigned int framesPerBuffer) {
    qWarning() << "PipewireEnumerator::setLatency" << sampleRate << framesPerBuffer;
    std::string rateStr = "1/" + std::to_string(sampleRate);
    std::string latencyStr = std::to_string(framesPerBuffer) + "/" + std::to_string(sampleRate);

    spa_dict_item items[] = {
            SPA_DICT_ITEM_INIT(PW_KEY_NODE_RATE, rateStr.c_str()),
            SPA_DICT_ITEM_INIT(PW_KEY_NODE_LATENCY, latencyStr.c_str()),
    };

    // don't set PW_KEY_NODE_LATENCY if framesPerBuffer is 0 (uninitialized)
    uint32_t numProps = framesPerBuffer == 0 ? 1 : 2;
    spa_dict properties = SPA_DICT_INIT(items, numProps);

    pw_thread_loop_lock(m_pPwThreadLoop);

    int res = pw_filter_update_properties(m_pPwFilter, nullptr, &properties);

    pw_thread_loop_unlock(m_pPwThreadLoop);

    if (res >= 0) {
        m_sampleRate = sampleRate;
        m_framesPerBuffer = framesPerBuffer;
        ControlObject::set(
                ConfigKey(kAppGroup, QStringLiteral("output_latency_ms")),
                m_framesPerBuffer * 1000 / m_sampleRate);
        ControlObject::set(ConfigKey(kAppGroup, QStringLiteral("samplerate")), m_sampleRate);

    } else {
        qWarning() << "PipewireEnumerator::setLatency "
                      "pw_filter_update_properties failed:"
                   << spa_strerror(res);
        qWarning() << "Unable to set requested samplerate";
    }
}
