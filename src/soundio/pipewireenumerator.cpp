#include "soundio/pipewireenumerator.h"

#include <pipewire/pipewire.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>
#include <spa/utils/result.h>

#include <memory>
#include <string>

#include "audio/types.h"
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
          m_ppwThreadLoop(nullptr),
          m_ppwContext(nullptr),
          m_ppwCore(nullptr),
          m_ppwRegistry(nullptr),
          m_ppwMetadata(nullptr),
          m_ppwFilter(nullptr),
          m_soundDevices(std::make_shared<SoundDeviceMap>()),
          m_openedDevices(std::make_shared<DeviceMap>()),
          m_initialized(false),
          m_audioLatencyUsage(kAppGroup, QStringLiteral("audio_latency_usage")) {
    connect(this, &PipewireEnumerator::deviceAdded, m_pSoundManager, &SoundManager::addDevice);
    connect(this, &PipewireEnumerator::deviceRemoved, m_pSoundManager, &SoundManager::removeDevice);

    pw_init(nullptr, nullptr);

    m_ppwThreadLoop = pw_thread_loop_new("mixxx_loop", nullptr);
    spa_zero(m_pwRegistryListener);
    spa_zero(m_pwMetadataListener);
    spa_zero(m_pwFilterListener);
}

PipewireEnumerator::~PipewireEnumerator() {
    pw_thread_loop_stop(m_ppwThreadLoop);

    if (m_ppwFilter) {
        pw_filter_destroy(m_ppwFilter);
    }

    if (m_ppwMetadata) {
        spa_hook_remove(&m_pwMetadataListener);
        pw_proxy_destroy((struct pw_proxy*)m_ppwMetadata);
    }

    if (m_ppwRegistry) {
        spa_hook_remove(&m_pwRegistryListener);
        pw_proxy_destroy((struct pw_proxy*)m_ppwRegistry);
    }

    if (m_ppwCore) {
        pw_core_disconnect(m_ppwCore);
    }

    if (m_ppwContext) {
        pw_context_destroy(m_ppwContext);
    }

    pw_thread_loop_destroy(m_ppwThreadLoop);
    pw_deinit();
}

void PipewireEnumerator::initialize() {
    if (m_initialized) {
        qWarning() << "PipewireEnumerator::initialize already initialized";
        return;
    }

    if (!m_ppwContext) {
        m_ppwContext = pw_context_new(pw_thread_loop_get_loop(m_ppwThreadLoop), nullptr, 0);
        if (!m_ppwContext) {
            qWarning() << "PipewireEnumerator::initialize pw_context_new "
                          "failed with error:"
                       << spa_strerror(errno);
            return;
        }
    }

    m_ppwCore = pw_context_connect(m_ppwContext, nullptr, 0);

    if (!m_ppwCore) {
        qWarning() << "PipewireEnumerator::initialize pw_context_connect "
                      "failed with error:"
                   << spa_strerror(errno);
        return;
    }

    m_ppwRegistry = pw_core_get_registry(m_ppwCore, PW_VERSION_REGISTRY, 0);
    pw_registry_add_listener(m_ppwRegistry, &m_pwRegistryListener, &registry_events, this);

    // see https://docs.pipewire.org/page_man_pipewire-props_7.html
    // and pipewire/keys.h header
    m_ppwFilter = pw_filter_new(m_ppwCore,
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

    pw_filter_add_listener(m_ppwFilter, &m_pwFilterListener, &filter_events, this);

    int res = pw_filter_connect(m_ppwFilter,
            PW_FILTER_FLAG_RT_PROCESS,
            nullptr,
            0);

    VERIFY_OR_DEBUG_ASSERT(res >= 0) {
        qWarning() << "pw_filter_connect error:" << spa_strerror(res);
    }

    pw_thread_loop_start(m_ppwThreadLoop);

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

        void* data = pw_registry_bind(m_ppwRegistry,
                id,
                PW_TYPE_INTERFACE_Metadata,
                PW_VERSION_METADATA,
                0);
        m_ppwMetadata = static_cast<pw_metadata*>(data);
        pw_metadata_add_listener(m_ppwMetadata, &m_pwMetadataListener, &metadataEvents, this);
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
        emit deviceAdded(device);
        auto pSoundDevices = std::make_shared<SoundDeviceMap>(*m_soundDevices.load());

        // pipewire assigns each object with a unique ID
        // any previous element is either invalid or already removed
        pSoundDevices->insert_or_assign(id, std::move(device));
        m_soundDevices.store(pSoundDevices);

        if (strcmp(name, "Mixxx") == 0) {
            m_filterId = id;
        }
    } else if (strcmp(pType, PW_TYPE_INTERFACE_Port) == 0) {
        const uint32_t node_id = pw_properties_parse_int(spa_dict_lookup(pProps, PW_KEY_NODE_ID));
        const char* dir = spa_dict_lookup(pProps, PW_KEY_PORT_DIRECTION);
        const bool isInput = strcmp(dir, "in") == 0;

        if (!m_soundDevices.load()->contains(node_id)) {
            // most likely midi or video node
            return;
        }

        m_objects.insert_or_assign(id, Object{Port(node_id)});
        auto pSoundDevices = m_soundDevices.load();
        auto pSoundDevice = pSoundDevices->at(node_id);
        pSoundDevice->registerDevicePort(id, pProps);
        m_pSoundManager->updateDeviceChannels(pSoundDevice);

        if (node_id != m_filterId) {
            return;
        }

        auto portId = getPortIndexFromName(spa_dict_lookup(pProps, PW_KEY_PORT_NAME));
        VERIFY_OR_DEBUG_ASSERT(portId.has_value()) {
            return;
        }

        auto pOpenedDevices = *m_openedDevices.load();

        for (auto& [deviceId, device] : pOpenedDevices) {
            if (isInput) {
                for (auto& port : device.inputs) {
                    if (port.filterPort == portId) {
                        auto devicePorts = pSoundDevices->at(deviceId)->getOutPorts();
                        uint32_t devicePortId = devicePorts[port.devicePort].id;
                        createLink(deviceId, devicePortId, node_id, id);
                    }
                }
            } else {
                for (auto& port : device.outputs) {
                    if (portId == port.filterPort) {
                        auto devicePorts = pSoundDevices->at(deviceId)->getInPorts();
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

    auto pair = m_objects.extract(id);
    Object& object = pair.mapped();

    if (std::get_if<Node>(&object)) {
        auto pSoundDevices = std::make_shared<SoundDeviceMap>(*m_soundDevices.load());
        if (!pSoundDevices->contains(id)) {
            return;
        }

        auto pDevice = pSoundDevices->at(id);
        if (pDevice->isOpen()) {
            pDevice->close();
        }

        qWarning() << "removing device:" << pDevice->getDisplayName();
        pSoundDevices->erase(id);
        m_soundDevices.store(pSoundDevices);
        emit deviceRemoved(pDevice);
        // m_pSoundManager->removeDevice(device);
    } else if (auto* port = std::get_if<Port>(&object)) {
        auto pSoundDevices = m_soundDevices.load();
        VERIFY_OR_DEBUG_ASSERT(pSoundDevices->contains(port->nodeId)) {
            qWarning() << "node" << port->nodeId << "port " << id;
            return;
        }

        auto pSoundDevice = pSoundDevices->at(port->nodeId);
        qWarning() << "removing port:" << id;
        pSoundDevice->unregisterDevicePort(id);
        m_pSoundManager->updateDeviceChannels(pSoundDevice);
    }
}

std::vector<SoundDevicePointer> PipewireEnumerator::queryDevices() const {
    std::vector<SoundDevicePointer> devices;
    auto pSoundDevices = m_soundDevices.load();
    for (const auto& [id, pDevice] : *pSoundDevices) {
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
    return m_openedDevices.load()->contains(id);
}

void PipewireEnumerator::openDevice(uint32_t id,
        const std::set<uint8_t>& inChans,
        const std::set<uint8_t>& outChans,
        mixxx::audio::SampleRate rate,
        uint32_t framesPerBuffer) {
    VERIFY_OR_DEBUG_ASSERT(m_initialized) {
        qWarning() << "PipewireEnumerator::openDevice called when "
                      "uninitialized, this should not happen";
        return;
    }

    auto pOpenedDevices = std::make_shared<DeviceMap>(*m_openedDevices.load());

    VERIFY_OR_DEBUG_ASSERT(!pOpenedDevices->contains(id)) {
        qWarning() << "device:" << id << "already open";
        return;
    }

    pw_thread_loop_lock(m_ppwThreadLoop);

    if (rate != m_sampleRate.value() || framesPerBuffer != m_framesPerBuffer) {
        std::string rateStr = "1/" + std::to_string(rate);
        std::string latencyStr = std::to_string(framesPerBuffer) + "/" + std::to_string(rate);

        spa_dict_item items[] = {
                SPA_DICT_ITEM_INIT(PW_KEY_NODE_RATE, rateStr.c_str()),
                SPA_DICT_ITEM_INIT(PW_KEY_NODE_LATENCY, latencyStr.c_str()),
        };
        spa_dict properties = SPA_DICT_INIT(items, 2);

        int res = pw_filter_update_properties(m_ppwFilter, nullptr, &properties);
        if (res >= 0) {
            m_sampleRate = mixxx::audio::SampleRate(rate);
            m_framesPerBuffer = framesPerBuffer;
        } else {
            qWarning() << "pw_filter_update_properties failed:" << spa_strerror(res);
            qWarning() << "Unable to set requested samplerate and buffer size";
        }
    }

    size_t numInPorts = 0;
    size_t numOutPorts = 0;

    for (auto& [id, device] : *pOpenedDevices) {
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
        void* port_data = pw_filter_add_port(m_ppwFilter,
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
        void* port_data = pw_filter_add_port(m_ppwFilter,
                SPA_DIRECTION_OUTPUT,
                PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                0,
                props,
                nullptr,
                0);
        outputs.emplace_back(port_data, i, filterPortIndex);
    }
    pw_thread_loop_unlock(m_ppwThreadLoop);

    // qWarning() << "PipewireEnumerator::openDevice" << inChans.size() <<
    // outChans.size() << inputs.size() << outputs.size();
    // auto newDevices = *m_openedDevices.load();
    pOpenedDevices->emplace(id, Device{std::move(inputs), std::move(outputs)});
    m_openedDevices.store(pOpenedDevices);
    // m_openedDevices.emplace(id, Device{std::move(inputs), std::move(outputs)});
}

void PipewireEnumerator::closeDevice(uint32_t id) {
    VERIFY_OR_DEBUG_ASSERT(m_initialized) {
        qWarning() << "PipewireEnumerator::closeDevice called when "
                      "uninitialized, this should not happen";
        return;
    }

    auto pOpenedDevices = std::make_shared<DeviceMap>(*m_openedDevices.load());
    VERIFY_OR_DEBUG_ASSERT(pOpenedDevices->contains(id)) {
        qWarning() << "device:" << id << "not opened";
        return;
    }

    auto& device = pOpenedDevices->at(id);

    pw_thread_loop_lock(m_ppwThreadLoop);
    for (auto& port : device.inputs) {
        pw_filter_remove_port(port.pPortData);
    }

    for (auto& port : device.outputs) {
        pw_filter_remove_port(port.pPortData);
    }
    pw_thread_loop_unlock(m_ppwThreadLoop);

    pOpenedDevices->erase(id);
    m_openedDevices.store(pOpenedDevices);
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

    auto pOpenedDevices = m_openedDevices.load();
    auto pSoundDevices = m_soundDevices.load();

    for (auto& [id, device] : *pOpenedDevices) {
        auto pSoundDevice = pSoundDevices->at(id);
        auto& ports = device.inputs;
        for (const auto& port : ports) {
            void* pBuffer = pw_filter_get_dsp_buffer(port.pPortData, framesPerBuffer);
            pSoundDevice->writeInput(static_cast<float*>(pBuffer),
                    port.devicePort,
                    framesPerBuffer);
        }
        m_pSoundManager->pushInputBuffers(pSoundDevice->inputs(), framesPerBuffer);
    }

    m_pSoundManager->onDeviceOutputCallback(framesPerBuffer);

    for (auto& [id, device] : *pOpenedDevices) {
        auto pSoundDevice = pSoundDevices->at(id);
        auto& ports = device.outputs;
        for (const auto& port : ports) {
            void* pBuffer = pw_filter_get_dsp_buffer(port.pPortData, framesPerBuffer);
            if (!pBuffer) {
                continue;
            }
            SampleUtil::clear(static_cast<float*>(pBuffer), framesPerBuffer);
            pSoundDevice->writeOutput(static_cast<float*>(pBuffer),
                    port.devicePort,
                    framesPerBuffer);
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

    struct pw_proxy* pProxy = static_cast<pw_proxy*>(pw_core_create_object(m_ppwCore,
            "link-factory",
            PW_TYPE_INTERFACE_Link,
            PW_VERSION_LINK,
            &props,
            0));
    if (pProxy) {
        pw_proxy_destroy(pProxy);
    }
}

mixxx::audio::SampleRate PipewireEnumerator::getDefaultSampleRate() const {
    return m_defaultSampleRate;
}
