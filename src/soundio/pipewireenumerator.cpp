#include "soundio/pipewireenumerator.h"

#include <pipewire/pipewire.h>
#include <qendian.h>
#include <qlogging.h>
#include <qobject.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>
#include <spa/utils/result.h>

#include <QList>
#include <QMessageBox>
#include <QMetaObject>
#include <QSharedPointer>
#include <QStringView>
#include <ranges>
#include <string>

#include "audio/types.h"
#include "control/controlobject.h"
#include "moc_pipewireenumerator.cpp"
#include "preferences/configobject.h"
#include "preferences/dialog/dlgprefsound.h"
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
constexpr int kDefaultBufferSize = 1024;
const QString kAppGroup = QStringLiteral("[App]");

static std::string find_node_name(uint32_t id, const struct spa_dict* props) {
    std::string name;
    static const char* const nameKeys[] = {
            PW_KEY_NODE_DESCRIPTION,
            PW_KEY_NODE_NICK,
            PW_KEY_MEDIA_NAME,
            PW_KEY_APP_NAME,
            PW_KEY_NODE_NAME,
    };

    for (const char* key : nameKeys) {
        const char* prop = spa_dict_lookup(props, key);
        if (prop) {
            name = prop;
            break;
        }
    }

    const char* mediaClass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
    static const char* const subtypes[] = {
            "Output",
            "Input",
            "Source",
            "Sink",
            "Duplex",
    };

    if (mediaClass) {
        for (const char* subtype : subtypes) {
            if (std::strstr(mediaClass, subtype)) {
                name = name + " (" + subtype + ")";
            }
        }
    } else {
        const char* mediaCategory = spa_dict_lookup(props, PW_KEY_MEDIA_CATEGORY);
        if (mediaCategory) {
            name = name + " (" + mediaCategory + ")";
        }
    }

    // worst case scenario, we still have node ID as name
    return name + ":" + std::to_string(id);
}
} // namespace

PipewireEnumerator::PipewireEnumerator(
        UserSettingsPointer pConfig, SoundManager* pManager)
        : m_pSoundManager(pManager),
          m_pConfig(pConfig),
          m_pPwThreadLoop(nullptr),
          m_pPwContext(nullptr),
          m_pPwCore(nullptr),
          m_pPwRegistry(nullptr),
          m_pPwMetadata(nullptr),
          m_pPwFilter(nullptr),
          m_audioLatencyUsage(kAppGroup, QStringLiteral("audio_latency_usage")),
          m_coPipewirePatchbaySync(ConfigKey(
                  kAppGroup, QStringLiteral("pipewire_patchbay_sync"))),
          m_coBufferSize(ConfigKey(kAppGroup, QStringLiteral("buffer_size")),
                  true,
                  false,
                  false,
                  kDefaultBufferSize),
          m_coLatencyParamsMismatch(ConfigKey(
                  kAppGroup, QStringLiteral("latency_params_mismatch"))),
          m_coOutputLatencyMs(kAppGroup, QStringLiteral("output_latency_ms")),
          m_coSamplerate(kAppGroup, QStringLiteral("samplerate")),
          m_forceQuantum(false),
          m_forceSamplerate(false),
          m_samplerate(48000),
          m_bufferSize(kDefaultBufferSize) {
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
    spa_zero(m_pwCoreListener);
    spa_zero(m_pwRegistryListener);
    spa_zero(m_pwMetadataListener);
    spa_zero(m_pwFilterListener);
}

PipewireEnumerator::~PipewireEnumerator() {
    deinitialize();
    if (m_pPwContext) {
        pw_context_destroy(m_pPwContext);
    }
    pw_deinit();
}

void PipewireEnumerator::initialize() {
    if (m_initialized) {
        return;
    }

    if (!m_pPwContext) {
        m_pPwContext = pw_context_new(pw_thread_loop_get_loop(m_pPwThreadLoop), nullptr, 0);
        if (!m_pPwContext) {
            qDebug() << "PipewireEnumerator::initialize pw_context_new "
                        "failed with error:"
                     << spa_strerror(errno);
            return;
        }
    }

    m_pPwCore = pw_context_connect(m_pPwContext, nullptr, 0);

    if (!m_pPwCore) {
        qDebug() << "PipewireEnumerator::initialize pw_context_connect "
                    "failed with error:"
                 << spa_strerror(errno);
        return;
    }
    pw_core_add_listener(m_pPwCore, &m_pwCoreListener, &coreEvents, this);

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
                    // we need to limit the maximum quantum pipewire will give us
                    // since the callback can run before we configure any quantum
                    // from preference page (by using external patchbay)
                    PW_KEY_NODE_MAX_LATENCY,
                    "4096/44100",
                    // this ensures that audio callback runs even when no link is
                    // connected, so we don't use network clock unless forced
                    PW_KEY_NODE_ALWAYS_PROCESS,
                    "true",
                    nullptr));

    pw_filter_add_listener(m_pPwFilter, &m_pwFilterListener, &filter_events, this);

    for (auto& [input, ports] : m_inputs) {
        createInputPorts(input, ports);
    }

    for (auto& [output, ports] : m_outputs) {
        createOutputPorts(output, ports);
    }

    int res = pw_filter_connect(m_pPwFilter,
            PW_FILTER_FLAG_RT_PROCESS,
            nullptr,
            0);

    VERIFY_OR_DEBUG_ASSERT(res >= 0) {
        qDebug() << "PipewireEnumerator::initialize pw_filter_connect error:"
                 << spa_strerror(res);
    }

    // queue an event when device enumeration is complete
    // so we can compare from Mixxx config and configure the devices
    m_coreSyncSeq = pw_core_sync(m_pPwCore, PW_ID_CORE, 0);

    pw_thread_loop_start(m_pPwThreadLoop);

    m_initialized = true;
}

void PipewireEnumerator::deinitialize() {
    if (!m_initialized) {
        return;
    }

    pw_thread_loop_stop(m_pPwThreadLoop);

    // clear everything we get through registry
    m_soundDevices.clear();
    m_nodes.clear();
    m_ports.clear();
    m_links.clear();

    // or is it better to m_pSoundManager->removeDevice(device) for every device?
    emit m_pSoundManager->devicesUpdated();

    if (m_pPwFilter) {
        spa_hook_remove(&m_pwFilterListener);
        pw_filter_destroy(m_pPwFilter);
        m_pPwFilter = nullptr;
    }

    if (m_pPwMetadata) {
        spa_hook_remove(&m_pwMetadataListener);
        pw_proxy_destroy((struct pw_proxy*)m_pPwMetadata);
        m_pPwMetadata = nullptr;
    }

    if (m_pPwRegistry) {
        spa_hook_remove(&m_pwRegistryListener);
        pw_proxy_destroy((struct pw_proxy*)m_pPwRegistry);
        m_pPwRegistry = nullptr;
    }

    if (m_pPwCore) {
        spa_hook_remove(&m_pwCoreListener);
        pw_core_disconnect(m_pPwCore);
        m_pPwCore = nullptr;
    }

    m_initialized = false;
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

        std::string name = find_node_name(id, pProps);

        m_nodes.insert_or_assign(id, Node{});
        auto pDevice = QSharedPointer<SoundDevicePipewire>::create(
                m_pConfig, m_pSoundManager, this, id, name);
        emit deviceAdded(pDevice);
        // pipewire assigns each object with a unique ID
        // any previous element is either invalid or already removed
        m_soundDevices.insert_or_assign(id, std::move(pDevice));

        if (name.find("Mixxx") != std::string::npos) {
            uint32_t filterId = pw_filter_get_node_id(m_pPwFilter);
            if (filterId == id) {
                m_filterId = filterId;
            }
        }
    } else if (strcmp(pType, PW_TYPE_INTERFACE_Port) == 0) {
        const uint32_t node_id = pw_properties_parse_int(spa_dict_lookup(pProps, PW_KEY_NODE_ID));
        if (!m_nodes.contains(node_id)) {
            // most likely midi or video node
            return;
        }

        const char* portName = spa_dict_lookup(pProps, PW_KEY_PORT_NAME);
        const char* channel = spa_dict_lookup(pProps, PW_KEY_AUDIO_CHANNEL);
        const char* direction = spa_dict_lookup(pProps, PW_KEY_PORT_DIRECTION);
        const bool isInput = std::strcmp(direction, "in") == 0;

        QSharedPointer<SoundDevicePipewire> pSoundDevice = m_soundDevices.at(node_id);
        m_pSoundManager->updateDeviceChannels(pSoundDevice);
        Node& node = m_nodes.at(node_id);
        Port port{};
        port.node = node_id;
        port.channel = channel ? channel : (portName ? portName : std::to_string(id).c_str());
        port.isInput = isInput;

        if (portName && channel) {
            std::string_view name = portName;
            const size_t last = name.find_last_of("_:-");
            const bool nameContainsChannel = last != std::string_view::npos and
                    port.channel == name.substr(last + 1);
            port.name = nameContainsChannel ? name.substr(0, last) : portName;
            port.name += ':';
        }

        // m_numInputChannels, m_numOutputChannels, m_audioInputs, m_audioOutputs
        // are with respect to Mixxx and not the SoundDevice
        if (isInput) {
            node.inputs.push_back(id);
            pSoundDevice->setNumOutputs(node.inputs.size());
        } else {
            node.outputs.push_back(id);
            pSoundDevice->setNumInputs(node.outputs.size());
        }

        m_ports.insert_or_assign(id, std::move(port));
        m_pSoundManager->updateDeviceChannels(pSoundDevice);

        if (node_id == m_filterId) {
            QString name(spa_dict_lookup(pProps, PW_KEY_PORT_NAME));
            QStringList list = name.split(':');
            if (isInput) {
                auto keys = std::views::keys(m_inputs);
                auto it = std::ranges::find(keys, list.at(0), &AudioPath::getString);
                VERIFY_OR_DEBUG_ASSERT(it != keys.end()) {
                    return;
                }

                if (list.at(1) == "FL") {
                    m_inputs[*it].left.id = id;
                } else {
                    m_inputs[*it].right.id = id;
                }
            } else {
                auto keys = std::views::keys(m_outputs);
                auto it = std::ranges::find(keys, list.at(0), &AudioPath::getString);
                VERIFY_OR_DEBUG_ASSERT(it != keys.end()) {
                    return;
                }

                if (list.at(1) == "FL") {
                    m_outputs.at(*it).left.id = id;
                } else {
                    m_outputs.at(*it).right.id = id;
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

        if (in_node != m_filterId and out_node != m_filterId) {
            return;
        }

        Port& inputPort = m_ports.at(in_port);
        Port& outputPort = m_ports.at(out_port);

        m_links.insert_or_assign(id, Link{in_port, out_port});
        inputPort.links.push_back(id);
        outputPort.links.push_back(id);

        if (in_node == m_filterId) {
            // device output, mixxx input
            for (auto& [input, ports] : m_inputs) {
                if (ports.left.id != in_port and ports.right.id != in_port) {
                    continue;
                }

                const bool portsActive = ports.active.load();
                if (!portsActive) {
                    m_pSoundManager->configureInput(input);
                    ports.active.store(true);
                }
                break;
            }
        }

        if (out_node == m_filterId) {
            // device input, mixxx output
            for (auto& [output, ports] : m_outputs) {
                if (ports.left.id != out_port and ports.right.id != out_port) {
                    continue;
                }

                const bool portsActive = ports.active.load();
                if (!portsActive) {
                    m_pSoundManager->configureOutput(output);
                    ports.active.store(true);
                }
                break;
            }
        }

        if (!static_cast<bool>(m_coPipewirePatchbaySync.get())) {
            QMetaObject::invokeMethod(m_pSoundManager, &SoundManager::invalidateConfig);
        }
    }
}

void PipewireEnumerator::registryEventGlobalRemove(unsigned int id) {
    if (m_nodes.contains(id)) {
        m_nodes.erase(id);
        VERIFY_OR_DEBUG_ASSERT(m_soundDevices.contains(id)) {
            return;
        }

        emit deviceRemoved(m_soundDevices.at(id));
        m_soundDevices.erase(id);
    } else if (m_ports.contains(id)) {
        const Port& port = m_ports.at(id);
        VERIFY_OR_DEBUG_ASSERT(m_soundDevices.contains(port.node)) {
            return;
        }

        QSharedPointer<SoundDevicePipewire> pSoundDevice = m_soundDevices.at(port.node);
        Node& node = m_nodes.at(port.node);

        if (port.isInput) {
            std::erase(node.inputs, id);
            pSoundDevice->setNumOutputs(node.inputs.size());
        } else {
            std::erase(node.outputs, id);
            pSoundDevice->setNumInputs(node.outputs.size());
        }

        m_pSoundManager->updateDeviceChannels(pSoundDevice);
        m_ports.erase(id);
    } else if (m_links.contains(id)) {
        const Link& link = m_links.at(id);
        Port& inputPort = m_ports.at(link.input);
        Port& outputPort = m_ports.at(link.output);
        std::erase(inputPort.links, id);
        std::erase(outputPort.links, id);

        if (outputPort.node == m_filterId) {
            for (auto& [output, ports] : m_outputs) {
                if (ports.left.id == link.output) {
                    Port& rightPort = m_ports.at(ports.right.id);
                    if (outputPort.links.empty() && rightPort.links.empty()) {
                        ports.active.store(false);
                        // close the device if device is disconnected externally
                        ports.activeDevice = 0;
                        m_pSoundManager->unconfigureOutput(output);
                    }
                } else if (ports.right.id == link.output) {
                    Port& leftPort = m_ports.at(ports.left.id);
                    if (outputPort.links.empty() && leftPort.links.empty()) {
                        ports.active.store(false);
                        // close the device if device is disconnected externally
                        ports.activeDevice = 0;
                        m_pSoundManager->unconfigureOutput(output);
                    }
                } else {
                    continue;
                }
            }
        }

        if (inputPort.node == m_filterId) {
            for (auto& [input, ports] : m_inputs) {
                if (ports.left.id == link.input) {
                    Port& rightPort = m_ports.at(ports.right.id);
                    if (inputPort.links.empty() && rightPort.links.empty()) {
                        // close the device if device is disconnected externally
                        ports.activeDevice = 0;
                        ports.active.store(false);
                        m_pSoundManager->unconfigureInput(input);
                    }
                } else if (ports.right.id == link.input) {
                    Port& leftPort = m_ports.at(ports.left.id);
                    if (inputPort.links.empty() && leftPort.links.empty()) {
                        // close the device if device is disconnected externally
                        ports.activeDevice = 0;
                        ports.active.store(false);
                        m_pSoundManager->unconfigureInput(input);
                    }
                } else {
                    continue;
                }
            }
        }

        if (!static_cast<bool>(m_coPipewirePatchbaySync.get())) {
            QMetaObject::invokeMethod(m_pSoundManager, &SoundManager::invalidateConfig);
        }

        m_links.erase(id);
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
    }

    return 0;
}

bool PipewireEnumerator::isOpen(uint32_t id) {
    for (const auto& ports : std::views::values(m_inputs)) {
        if (ports.activeDevice == id) {
            return true;
        }
    }

    for (const auto& ports : std::views::values(m_outputs)) {
        if (ports.activeDevice == id) {
            return true;
        }
    }

    return false;
}

std::string PipewireEnumerator::openDeviceInput(uint32_t deviceId,
        const AudioInput& input) {
    std::string result;
    VERIFY_OR_DEBUG_ASSERT(m_initialized) {
        qDebug() << "PipewireEnumerator::openDevice called when "
                    "uninitialized, this should not happen";
        return "PipewireEnumerator uninitialized";
    }

    if (static_cast<bool>(m_coPipewirePatchbaySync.get())) {
        return {};
    }

    PortPair& ports = m_inputs.at(input);
    ports.activeDevice = deviceId;

    if (ports.active.load()) {
        closePorts(ports);
    }

    ChannelGroup channelGroup = input.getChannelGroup();
    unsigned char channelBase = channelGroup.getChannelBase();
    unsigned char channelCount = channelGroup.getChannelCount().value();
    std::span<const uint32_t> portIds = m_nodes.at(deviceId).outputs;

    pw_thread_loop_lock(m_pPwThreadLoop);

    if (channelCount == 1) {
        result += createLink(deviceId, portIds[channelBase], m_filterId, ports.left.id);
        result += createLink(deviceId, portIds[channelBase], m_filterId, ports.right.id);
    } else {
        result += createLink(deviceId, portIds[channelBase], m_filterId, ports.left.id);
        result += createLink(deviceId,
                portIds[channelBase + 1],
                m_filterId,
                ports.right.id);
    }

    pw_thread_loop_unlock(m_pPwThreadLoop);
    return result;
}

std::string PipewireEnumerator::openDeviceOutput(uint32_t deviceId, const AudioOutput& output) {
    std::string result;
    VERIFY_OR_DEBUG_ASSERT(m_initialized) {
        qDebug() << "PipewireEnumerator::openDevice called when "
                    "uninitialized, this should not happen";
        return "PipewireEnumerator uninitialized";
    }

    if (static_cast<bool>(m_coPipewirePatchbaySync.get())) {
        return {};
    }

    PortPair& ports = m_outputs.at(output);
    ports.activeDevice = deviceId;

    if (ports.active.load()) {
        closePorts(ports);
    }

    ChannelGroup channelGroup = output.getChannelGroup();
    unsigned char channelBase = channelGroup.getChannelBase();
    unsigned char channelCount = channelGroup.getChannelCount().value();

    std::span<const uint32_t> portIds = m_nodes.at(deviceId).inputs;

    pw_thread_loop_lock(m_pPwThreadLoop);

    if (channelCount == 1) {
        uint32_t filterPort = channelBase % 2 ? ports.right.id : ports.left.id;
        result += createLink(m_filterId, filterPort, deviceId, portIds[channelBase]);
    } else {
        result += createLink(m_filterId, ports.left.id, deviceId, portIds[channelBase]);
        result += createLink(m_filterId,
                ports.right.id,
                deviceId,
                portIds[channelBase + 1]);
    }

    pw_thread_loop_unlock(m_pPwThreadLoop);
    return result;
}

void PipewireEnumerator::closePorts(PortPair& ports) {
    const Port& left = m_ports.at(ports.left.id);
    const Port& right = m_ports.at(ports.right.id);

    qDebug() << "PipewireEnumerator::closePorts" << left.links.size() << right.links.size();

    for (uint32_t link : left.links) {
        destroyLink(link);
        qDebug() << "PipewireEnumerator::closePorts" << link;
    }

    for (uint32_t link : right.links) {
        destroyLink(link);
        qDebug() << "PipewireEnumerator::closePorts" << link;
    }
    ports.activeDevice = 0;
}

void PipewireEnumerator::closeDevices() {
    for (auto& [path, ports] : m_inputs) {
        if (ports.active.load()) {
            qDebug() << "PipewireEnumerator::closeDevices" << path.getString();
            closePorts(ports);
        }
    }

    for (auto& [path, ports] : m_outputs) {
        if (ports.active.load()) {
            qDebug() << "PipewireEnumerator::closeDevices" << path.getString();
            closePorts(ports);
        }
    }
}

void PipewireEnumerator::callback(const spa_io_position* pos) {
    // This must be the very first call, else timeInfo becomes invalid
    m_clkRefTimer.restart();
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

    m_pSoundManager->processUnderflowHappened(framesPerBuffer);

    for (const auto& [input, ports] : m_inputs) {
        if (!ports.active.load()) {
            continue;
        }
        CSAMPLE* pInputBuffer = m_pSoundManager->getInputBuffer(input);
        VERIFY_OR_DEBUG_ASSERT(pInputBuffer) {
            return;
        }

        const float* bufferFL = static_cast<const float*>(
                pw_filter_get_dsp_buffer(ports.left.data, framesPerBuffer));

        if (bufferFL) {
            for (uint64_t i = 0; i < framesPerBuffer; i++) {
                pInputBuffer[i * 2] = bufferFL[i];
            }
        } else {
            for (uint64_t i = 0; i < framesPerBuffer; i++) {
                pInputBuffer[i * 2] = 0;
            }
        }

        const float* bufferFR = static_cast<const float*>(
                pw_filter_get_dsp_buffer(ports.right.data, framesPerBuffer));
        if (bufferFR) {
            for (uint64_t i = 0; i < framesPerBuffer; i++) {
                pInputBuffer[i * 2 + 1] = bufferFR[i];
            }
        } else {
            for (uint64_t i = 0; i < framesPerBuffer; i++) {
                pInputBuffer[i * 2 + 1] = 0;
            }
        }
        m_pSoundManager->pushInputBuffer(input, framesPerBuffer);
    }

    m_pSoundManager->onDeviceOutputCallback(framesPerBuffer);

    for (const auto& [output, ports] : m_outputs) {
        if (!ports.active.load()) {
            continue;
        }

        const CSAMPLE* pOutputBuffer = m_pSoundManager->getOutputBuffer(output);

        VERIFY_OR_DEBUG_ASSERT(pOutputBuffer) {
            return;
        }

        float* bufferFL = static_cast<float*>(
                pw_filter_get_dsp_buffer(ports.left.data, framesPerBuffer));
        if (bufferFL) {
            for (uint64_t i = 0; i < framesPerBuffer; i++) {
                bufferFL[i] = pOutputBuffer[i * 2];
            }
        }

        float* bufferFR = static_cast<float*>(
                pw_filter_get_dsp_buffer(ports.right.data, framesPerBuffer));
        if (bufferFR) {
            for (uint64_t i = 0; i < framesPerBuffer; i++) {
                bufferFR[i] = pOutputBuffer[i * 2 + 1];
            }
        }
    }

    const bool configForceQuantum = m_pConfig->getValue(
            ConfigKey(kAppGroup, QStringLiteral("force_buffer_size")), false);
    const bool configForceSamplerate = m_pConfig->getValue(
            ConfigKey(kAppGroup, QStringLiteral("force_samplerate")), false);
    const unsigned int engineSamplerate = static_cast<unsigned int>(m_coSamplerate.get());
    const unsigned int engineBufferSize = static_cast<unsigned int>(m_coBufferSize.get());

    const mixxx::audio::SampleRate configSamplerate = m_pSoundManager->getConfig().getSampleRate();
    const unsigned int configBufferSize = configForceQuantum
            ? m_pSoundManager->getConfig().getFramesPerBuffer()
            // server set buffer size can be arbitrary (non power of 2)
            // here we don't store it in SoundManagerConfig, and instead use ControlObject
            : engineBufferSize;

    const bool forceChanged = configForceQuantum != m_forceQuantum ||
            configForceSamplerate != m_forceSamplerate;
    const bool latencyParamsChanged = configSamplerate != m_samplerate ||
            configBufferSize != m_bufferSize;

    if (forceChanged) {
        m_forceQuantum = configForceQuantum;
        m_forceSamplerate = configForceSamplerate;
    }

    if (latencyParamsChanged) {
        m_samplerate = configSamplerate;
        m_bufferSize = configBufferSize;
    }

    if (forceChanged || latencyParamsChanged) {
        std::string rate = m_forceSamplerate ? std::to_string(m_samplerate) : "";
        std::string quantum = m_forceQuantum ? std::to_string(m_bufferSize) : "";

        const spa_dict_item propItems[] = {
                SPA_DICT_ITEM_INIT(PW_KEY_NODE_FORCE_RATE, rate.c_str()),
                SPA_DICT_ITEM_INIT(PW_KEY_NODE_FORCE_QUANTUM, quantum.c_str()),
        };

        spa_dict properties = SPA_DICT_INIT(propItems, std::size(propItems));

        pw_thread_loop_lock(m_pPwThreadLoop);
        int res = pw_filter_update_properties(m_pPwFilter, nullptr, &properties);
        pw_thread_loop_unlock(m_pPwThreadLoop);

        if (res < 0) {
            qDebug() << "PipewireEnumerator::setLatency "
                        "pw_filter_update_properties failed:"
                     << spa_strerror(res);
            qDebug() << "Unable to set requested samplerate";
        }

        const bool bufferSizeMismatch = m_forceQuantum && (framesPerBuffer != m_bufferSize);
        const bool samplerateMismatch = m_forceSamplerate && (sampleRate != m_samplerate);
        m_coLatencyParamsMismatch.set(bufferSizeMismatch || samplerateMismatch);
    }

    if (sampleRate != engineSamplerate || framesPerBuffer != engineBufferSize) {
        qDebug() << "PipewireEnumerator::callback requested"
                 << m_samplerate << "samples at" << m_bufferSize << "hz, provided"
                 << sampleRate << "samples at" << framesPerBuffer << "hz";

        m_coOutputLatencyMs.set(framesPerBuffer * 1000 / sampleRate);
        m_coSamplerate.set(sampleRate);
        m_coBufferSize.set(framesPerBuffer);
    }

    updateAudioLatencyUsage(sampleRate, framesPerBuffer);
}

void PipewireEnumerator::updateAudioLatencyUsage(SINT samplerate, SINT framesPerBuffer) {
    m_framesSinceAudioLatencyUsageUpdate += framesPerBuffer;
    if (m_framesSinceAudioLatencyUsageUpdate > ((double)samplerate / kCpuUsageUpdateRate)) {
        double secInAudioCb = m_timeInAudioCallback.toDoubleSeconds();
        m_audioLatencyUsage.set(
                secInAudioCb / (m_framesSinceAudioLatencyUsageUpdate / (double)samplerate));
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
    if (input.isHidden()) {
        return;
    }

    auto [it, isInserted] = m_inputs.try_emplace(input);

    if (m_initialized and isInserted) {
        pw_thread_loop_lock(m_pPwThreadLoop);
        createInputPorts(input, it->second);
        pw_thread_loop_unlock(m_pPwThreadLoop);
    }
}

void PipewireEnumerator::registerOutput(const AudioOutput& output, AudioSource*) {
    if (output.isHidden()) {
        return;
    }

    auto [it, isInserted] = m_outputs.try_emplace(output);
    if (m_initialized and isInserted) {
        pw_thread_loop_lock(m_pPwThreadLoop);
        createOutputPorts(output, it->second);
        pw_thread_loop_unlock(m_pPwThreadLoop);
    }
}

// need to pw_thread_loop_lock before calling this
void PipewireEnumerator::createPorts(
        PortPair& ports, std::string_view name, spa_direction direction) {
    pw_properties* props = pw_properties_new(
            // see pipewire/keys.h header
            PW_KEY_FORMAT_DSP,
            "32 bit float mono audio",
            PW_KEY_AUDIO_CHANNEL,
            "FL",
            nullptr);
    // any changes to port name needs to update port name parsing logic in
    // PipewireEnumerator::registryEventGlobal
    pw_properties_setf(props, PW_KEY_PORT_NAME, "%s:FL", name.data());

    ports.left.data = pw_filter_add_port(m_pPwFilter,
            direction,
            PW_FILTER_PORT_FLAG_MAP_BUFFERS,
            0,
            props,
            nullptr,
            0);

    props = pw_properties_new(
            // see pipewire/keys.h header
            PW_KEY_FORMAT_DSP,
            "32 bit float mono audio",
            PW_KEY_AUDIO_CHANNEL,
            "FR",
            nullptr);
    // any changes to port name needs to update port name parsing logic in
    // PipewireEnumerator::registryEventGlobal
    pw_properties_setf(props, PW_KEY_PORT_NAME, "%s:FR", name.data());

    ports.right.data = pw_filter_add_port(m_pPwFilter,
            direction,
            PW_FILTER_PORT_FLAG_MAP_BUFFERS,
            0,
            props,
            nullptr,
            0);
}

// need to pw_thread_loop_lock before calling this
void PipewireEnumerator::createInputPorts(const AudioInput& input, PortPair& ports) {
    std::string inputName = input.getString().toStdString();
    createPorts(ports, inputName, SPA_DIRECTION_INPUT);
}

// need to pw_thread_loop_lock before calling this
void PipewireEnumerator::createOutputPorts(const AudioOutput& output, PortPair& ports) {
    std::string outputName = output.getString().toStdString();
    createPorts(ports, outputName, SPA_DIRECTION_OUTPUT);
}

void PipewireEnumerator::updateFilterLatency(const SINT samplerate, const SINT bufferSize) {
    qDebug() << "PipewireEnumerator::updateFilterLatency" << m_forceQuantum << m_forceSamplerate;

    std::string rate = m_forceSamplerate ? std::to_string(samplerate) : "";
    std::string quantum = m_forceQuantum ? std::to_string(bufferSize) : "";

    const spa_dict_item propItems[] = {
            SPA_DICT_ITEM_INIT(PW_KEY_NODE_FORCE_RATE, rate.c_str()),
            SPA_DICT_ITEM_INIT(PW_KEY_NODE_FORCE_QUANTUM, quantum.c_str()),
    };

    spa_dict properties = SPA_DICT_INIT(propItems, std::size(propItems));

    pw_thread_loop_lock(m_pPwThreadLoop);
    int res = pw_filter_update_properties(m_pPwFilter, nullptr, &properties);
    pw_thread_loop_unlock(m_pPwThreadLoop);

    if (res < 0) {
        qDebug() << "PipewireEnumerator::setLatency "
                    "pw_filter_update_properties failed:"
                 << spa_strerror(res);
        qDebug() << "Unable to set requested samplerate";
    }
}

void PipewireEnumerator::coreEventDone(uint32_t id, int seq) {
    qDebug() << "PipewireEnumerator::coreEventDone" << id << seq << m_coreSyncSeq;
    if (id == 0 && seq == m_coreSyncSeq) {
        QMetaObject::invokeMethod(m_pSoundManager, &SoundManager::devicesEnumerated);
    }
}

void PipewireEnumerator::coreEventError(uint32_t id, int seq, int res, const char* message) {
    qDebug() << "PipewireEnumerator::coreEventError" << id << seq << res << message;
    if (id == 0) {
        if (res == -EPIPE) {
            qDebug() << "Deinitializing PipeWire due to server disconnect";
            QMessageBox::information(nullptr,
                    tr("Information"),
                    tr("PipeWire server disconnected, query devices to reconnect."));
            deinitialize();
        }
    }
}

QString PipewireEnumerator::getChannelString(
        uint32_t id, ChannelGroup channelGroup, bool input) const {
    unsigned char base = channelGroup.getChannelBase();
    mixxx::audio::ChannelCount count = channelGroup.getChannelCount();

    const Node& node = m_nodes.at(id);

    std::span<const uint32_t> ports = input ? node.outputs : node.inputs;
    std::span<const uint32_t> subspan = ports.subspan(base + 1, count - 1);

    // without this 1st port will always take else branch, inserting unnecessary ' '
    const Port& firstPort = m_ports.at(ports[base]);
    std::string_view currentCommonSubstr = firstPort.name;
    std::string channelString = firstPort.name + firstPort.channel;

    for (uint32_t portId : subspan) {
        const Port& port = m_ports.at(portId);
        if (!port.name.empty() and port.name == currentCommonSubstr) {
            channelString = channelString + '/' + port.channel;
        } else {
            channelString = channelString + ' ' + port.name + port.channel;
            currentCommonSubstr = port.name;
        }
    }
    return QString::fromStdString(channelString);
}

bool PipewireEnumerator::nodeHasPorts(const Node& node) {
    for (uint32_t portId : node.inputs) {
        if (!m_ports.at(portId).links.empty()) {
            return true;
        }
    }

    for (uint32_t portId : node.outputs) {
        if (!m_ports.at(portId).links.empty()) {
            return true;
        }
    }

    return false;
}
