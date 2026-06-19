#include "soundio/pipewireenumerator.h"

#include <pipewire/pipewire.h>
#include <qlogging.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>

#include "moc_pipewireenumerator.cpp"
#include "soundio/sounddevice.h"
#include "soundio/sounddevicepipewire.h"
#include "soundio/soundmanager.h"
#include "util/assert.h"
#include "util/sample.h"
#include "util/trace.h"
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

// Calculates the port index from port name, since port name is very
// convenient property to access. That also mean that any changes in
// port naming scheme take in account this function.
static std::optional<uint32_t> getPortIndexFromName(const char* name) {
    std::string_view view(name);

    auto pos = view.find(':');
    if (pos == std::string_view::npos) {
        return std::nullopt;
    }

    uint32_t value;
    auto [ptr, ec] = std::from_chars(
            view.data() + pos + 1,
            view.data() + view.size(),
            value);

    if (ec == std::errc{}) {
        return value;
    }

    return std::nullopt;
}

} // namespace

PipewireEnumerator::PipewireEnumerator(UserSettingsPointer, SoundManager* pManager)
        : m_pSoundManager(pManager),
          m_initialized(false),
          m_audioLatencyUsage(kAppGroup, QStringLiteral("audio_latency_usage")) {
    connect(this, &PipewireEnumerator::deviceAdded, m_pSoundManager, &SoundManager::addDevice);
    connect(this, &PipewireEnumerator::deviceRemoved, m_pSoundManager, &SoundManager::removeDevice);

    pw_init(nullptr, nullptr);

    m_pThreadLoop = pw_thread_loop_new("mixxx_loop", nullptr);
    m_pContext = pw_context_new(pw_thread_loop_get_loop(m_pThreadLoop), nullptr, 0);
    m_pCore = pw_context_connect(m_pContext, nullptr, 0);
    m_pRegistry = pw_core_get_registry(m_pCore, PW_VERSION_REGISTRY, 0);

    // see https://docs.pipewire.org/page_man_pipewire-props_7.html
    // and pipewire/keys.h header
    m_pFilter = pw_filter_new(m_pCore,
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

    spa_zero(m_registryListener);
    spa_zero(m_metadataListener);
    spa_zero(m_filterListener);

    pw_registry_add_listener(m_pRegistry, &m_registryListener, &registry_events, this);
    pw_filter_add_listener(m_pFilter, &m_filterListener, &filter_events, this);

    pw_filter_connect(m_pFilter,
            PW_FILTER_FLAG_RT_PROCESS,
            nullptr,
            0);

    pw_thread_loop_start(m_pThreadLoop);
}

PipewireEnumerator::~PipewireEnumerator() {
    pw_thread_loop_stop(m_pThreadLoop);
    spa_hook_remove(&m_registryListener);
    spa_hook_remove(&m_metadataListener);
    pw_proxy_destroy((struct pw_proxy*)m_pRegistry);
    pw_proxy_destroy((struct pw_proxy*)m_pMetadata);
    pw_core_disconnect(m_pCore);
    pw_context_destroy(m_pContext);
    pw_thread_loop_destroy(m_pThreadLoop);
    pw_deinit();
}

QList<mixxx::audio::SampleRate> PipewireEnumerator::getSampleRates() const {
    if (m_samplerates.empty()) {
        return QList<mixxx::audio::SampleRate>{
                mixxx::audio::SampleRate(44100),
                mixxx::audio::SampleRate(48000),
                mixxx::audio::SampleRate(96000),
        };
    }

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

        void* data = pw_registry_bind(m_pRegistry,
                id,
                PW_TYPE_INTERFACE_Metadata,
                PW_VERSION_METADATA,
                0);
        m_pMetadata = static_cast<pw_metadata*>(data);
        pw_metadata_add_listener(m_pMetadata, &m_metadataListener, &metadataEvents, this);
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
        auto device = QSharedPointer<SoundDevicePipewire>::create(
                m_pConfig, m_pSoundManager, this, id, name);
        m_soundDevices.insert_or_assign(id, std::move(device));
        emit deviceAdded(device);

        if (strcmp(name, "Mixxx") == 0) {
            m_filterId = id;
        }
    } else if (strcmp(pType, PW_TYPE_INTERFACE_Port) == 0) {
        const uint32_t node_id = pw_properties_parse_int(spa_dict_lookup(pProps, PW_KEY_NODE_ID));
        const char* dir = spa_dict_lookup(pProps, PW_KEY_PORT_DIRECTION);
        const bool isInput = strcmp(dir, "in") == 0;

        if (!m_soundDevices.contains(node_id)) {
            // most likely midi or video node
            return;
        }

        m_objects.insert_or_assign(id, Object{Port(node_id)});
        auto soundDevice = m_soundDevices[node_id];
        soundDevice->registerDevicePort(id, pProps);
        m_pSoundManager->updateDeviceChannels(soundDevice);

        if (node_id != m_filterId) {
            return;
        }

        auto port_id = getPortIndexFromName(spa_dict_lookup(pProps, PW_KEY_PORT_NAME));
        VERIFY_OR_DEBUG_ASSERT(!port_id.has_value()) {
            return;
        }

        for (auto& [deviceId, device] : m_openedDevices) {
            if (isInput) {
                for (auto& port : device.inputs) {
                    if (port.filterPort == port_id) {
                        auto devicePorts = m_soundDevices[deviceId]->getOutPorts();
                        uint32_t devicePortId = devicePorts[port.devicePort].id;
                        createLink(deviceId, devicePortId, node_id, id);
                    }
                }
            } else {
                for (auto& port : device.outputs) {
                    if (port_id == port.filterPort) {
                        auto devicePorts = m_soundDevices[deviceId]->getInPorts();
                        uint32_t devicePortId = devicePorts[port.devicePort].id;
                        createLink(node_id, id, deviceId, devicePortId);
                    }
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

        if (in_node == m_filterId || out_node == m_filterId) {
            m_objects.insert_or_assign(id, Object{Link(in_port, out_port)});
        }
    }
}

void PipewireEnumerator::registryEventGlobalRemove(unsigned int id) {
    if (!m_objects.contains(id)) {
        return;
    }

    Object& object = m_objects.at(id);

    if (std::get_if<Node>(&object)) {
        const auto& node = m_soundDevices.extract(id);
        auto& device = node.mapped();
        if (device->isOpen()) {
            device->close();
        }
        emit deviceRemoved(device);
    } else if (auto* port = std::get_if<Port>(&object)) {
        auto& device = m_soundDevices[port->nodeId];
        device->unregisterDevicePort(id);
        m_pSoundManager->updateDeviceChannels(device);
    }

    m_objects.erase(id);
}

std::vector<SoundDevicePointer> PipewireEnumerator::queryDevices() const {
    std::vector<SoundDevicePointer> devices{};
    for (const auto& [id, device] : m_soundDevices) {
        devices.push_back(device);
    }

    return devices;
}

void PipewireEnumerator::initialize() {
}

int PipewireEnumerator::metadataProperty(
        void* data, uint32_t, const char* key, const char*, const char* value) {
    PipewireEnumerator* pEnumerator = static_cast<PipewireEnumerator*>(data);

    if (strcmp(key, "clock.allowed-rates") == 0) {
        // parse json arrays like [ 44100, 48000, 96000 ]
        QString s = value;
        s.remove('[');
        s.remove(']');

        const QStringList parts = s.split(',', Qt::SkipEmptyParts);

        for (const QString& part : parts) {
            pEnumerator->m_samplerates.push_back(mixxx::audio::SampleRate(part.trimmed().toInt()));
        }
        pEnumerator->m_pSoundManager->checkConfig();
    }
    return 0;
}

bool PipewireEnumerator::isOpen(uint32_t id) {
    return m_openedDevices.contains(id);
}

void PipewireEnumerator::openDevice(
        uint32_t id, const std::set<uint8_t> inChans, const std::set<uint8_t> outChans) {
    VERIFY_OR_DEBUG_ASSERT(!m_openedDevices.contains(id)) {
        qWarning() << "device:" << id << "already open";
        return;
    }

    pw_thread_loop_lock(m_pThreadLoop);

    size_t numInPorts = 0;
    size_t numOutPorts = 0;

    for (auto& [id, device] : m_openedDevices) {
        numInPorts += device.inputs.size();
        numOutPorts += device.outputs.size();
    }

    // these correspond to the AudioInputs and filter inputs
    // and device outputs
    std::vector<Device::Port> inputs;
    for (uint8_t i : inChans) {
        size_t filterPortIndex = inputs.size() + numInPorts;
        pw_properties* props = pw_properties_new(
                // see pipewire/keys.h header
                PW_KEY_FORMAT_DSP,
                "32 bit float mono audio",
                nullptr);
        pw_properties_setf(props, PW_KEY_PORT_NAME, "in:%zu", filterPortIndex);
        void* port_data = pw_filter_add_port(m_pFilter,
                SPA_DIRECTION_INPUT,
                PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                0,
                props,
                nullptr,
                0);
        inputs.emplace_back(port_data, i, filterPortIndex);
    }
    // these correspond to the AudioInputs and filter outputs
    // and device inputs
    std::vector<Device::Port> outputs;
    for (uint8_t i : outChans) {
        size_t filterPortIndex = outputs.size() + numOutPorts;
        pw_properties* props = pw_properties_new(
                PW_KEY_FORMAT_DSP, "32 bit float mono audio", nullptr);
        pw_properties_setf(props, PW_KEY_PORT_NAME, "out:%zu", filterPortIndex);
        void* port_data = pw_filter_add_port(m_pFilter,
                SPA_DIRECTION_OUTPUT,
                PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                0,
                props,
                nullptr,
                0);
        outputs.emplace_back(port_data, i, filterPortIndex);
    }
    pw_thread_loop_unlock(m_pThreadLoop);

    // qWarning() << "PipewireEnumerator::openDevice" << inChans.size() <<
    // outChans.size() << inputs.size() << outputs.size();
    m_openedDevices.emplace(id, Device{std::move(inputs), std::move(outputs)});
}

void PipewireEnumerator::closeDevice(uint32_t id) {
    VERIFY_OR_DEBUG_ASSERT(m_openedDevices.contains(id)) {
        qWarning() << "device:" << id << "not opened";
        return;
    }

    auto& device = m_openedDevices[id];

    pw_thread_loop_lock(m_pThreadLoop);
    for (auto& port : device.inputs) {
        pw_filter_remove_port(port.pPortData);
    }

    for (auto& port : device.outputs) {
        pw_filter_remove_port(port.pPortData);
    }
    pw_thread_loop_unlock(m_pThreadLoop);

    m_openedDevices.erase(id);
}

void PipewireEnumerator::callback(const spa_io_position* pos) {
    // This must be the very first call, else timeInfo becomes invalid
    m_clkRefTimer.restart().toDoubleSeconds();
    VisualPlayPosition::setCallbackEntryToDacSecs(
            pos->clock.delay / pos->clock.rate.denom, m_clkRefTimer);

    Trace trace("SoundDevicePw::callbackProcessClkRef");

    if (pos->clock.xrun > xrun_duration) {
        xrun_duration = pos->clock.xrun;
        m_pSoundManager->underflowHappened(6);
    }

    const uint64_t framesPerBuffer = pos->clock.duration;
    m_pSoundManager->processUnderflowHappened(framesPerBuffer);

    for (auto& [id, device] : m_openedDevices) {
        auto soundDevice = m_soundDevices[id];
        auto& ports = device.inputs;
        for (const auto& port : ports) {
            void* buffer = pw_filter_get_dsp_buffer(port.pPortData, framesPerBuffer);
            soundDevice->writeInput(static_cast<float*>(buffer), port.devicePort, framesPerBuffer);
        }
        m_pSoundManager->pushInputBuffers(soundDevice->inputs(), framesPerBuffer);
    }

    m_pSoundManager->onDeviceOutputCallback(framesPerBuffer);

    for (auto& [id, device] : m_openedDevices) {
        auto& soundDevice = m_soundDevices[id];
        auto& ports = device.outputs;
        for (const auto& port : ports) {
            void* buffer = pw_filter_get_dsp_buffer(port.pPortData, framesPerBuffer);
            if (!buffer) {
                continue;
            }
            SampleUtil::clear(static_cast<float*>(buffer), framesPerBuffer);
            soundDevice->writeOutput(static_cast<float*>(buffer), port.devicePort, framesPerBuffer);
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

void PipewireEnumerator::createLink(uint32_t outNodeId,
        uint32_t outPortId,
        uint32_t inNodeId,
        uint32_t inPortId) {
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

    struct pw_proxy* proxy = (struct pw_proxy*)pw_core_create_object(m_pCore,
            "link-factory",
            PW_TYPE_INTERFACE_Link,
            PW_VERSION_LINK,
            &props,
            0);
    if (proxy) {
        pw_proxy_destroy(proxy);
    }
}
