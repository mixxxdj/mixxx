#include "soundio/pipewireenumerator.h"

#include <pipewire/pipewire.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>

#include <cstdlib>

#include "moc_pipewireenumerator.cpp"
#include "soundio/sounddevice.h"
#include "soundio/sounddevicepipewire.h"
#include "soundio/soundmanager.h"

namespace {

static const char* find_node_name(const struct spa_dict* props) {
    static const char* const name_keys[] = {
            PW_KEY_NODE_NAME,
            PW_KEY_NODE_DESCRIPTION,
            PW_KEY_APP_NAME,
            PW_KEY_MEDIA_NAME,
    };

    SPA_FOR_EACH_ELEMENT_VAR(name_keys, key) {
        const char* name = spa_dict_lookup(props, *key);
        if (name) {
            return name;
        }
    }
    return nullptr;
}

} // namespace

PipewireEnumerator::PipewireEnumerator(
        [[maybe_unused]] UserSettingsPointer pConfig, SoundManager* pManager)
        : m_pManager(pManager) {
    pw_init(nullptr, nullptr);

    m_pThreadLoop = pw_thread_loop_new("mixxx_pw_loop", nullptr);
    m_pContext = pw_context_new(pw_thread_loop_get_loop(m_pThreadLoop), nullptr, 0);
    m_pCore = pw_context_connect(m_pContext, nullptr, 0);
    m_pRegistry = pw_core_get_registry(m_pCore, PW_VERSION_REGISTRY, 0);

    spa_zero(m_registryListener);
    pw_registry_add_listener(m_pRegistry, &m_registryListener, &registry_events, this);

    pw_thread_loop_start(m_pThreadLoop);
}

PipewireEnumerator::~PipewireEnumerator() {
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

void PipewireEnumerator::registryEventGlobalOuter(void* data,
        uint32_t id,
        uint32_t permissions,
        const char* type,
        uint32_t version,
        const struct spa_dict* props) {
    ((PipewireEnumerator*)data)->registryEventGlobal(id, permissions, type, version, props);
}

void PipewireEnumerator::registryEventGlobal(uint32_t id,
        [[maybe_unused]] uint32_t permissions,
        const char* type,
        [[maybe_unused]] uint32_t version,
        const struct spa_dict* props) {
    if (strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0) {
        const char* name = spa_dict_lookup(props, PW_KEY_METADATA_NAME);
        if (strcmp(name, "settings")) {
            return;
        }

        void* data = pw_registry_bind(m_pRegistry,
                id,
                PW_TYPE_INTERFACE_Metadata,
                PW_VERSION_METADATA,
                0);
        m_pMetadata = static_cast<pw_metadata*>(data);
        pw_metadata_add_listener(m_pMetadata, &m_metadataListener, &metadataEvents, this);
    } else if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
        const char* media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
        const char* media_type = spa_dict_lookup(props, PW_KEY_MEDIA_TYPE);

        bool isAudioNode = (media_class && strstr(media_class, "Audio")) ||
                (media_type && strstr(media_type, "Audio"));

        if (!isAudioNode) {
            return;
        }

        const char* name = find_node_name(props);

        // exclude mixxx own node
        if (strcmp(name, "mixxx")) {
            m_objects.insert_or_assign(id, Object{Node{}});
            auto device = QSharedPointer<SoundDevicePipewire>::create(
                    m_pConfig, m_pManager, this, id, name);
            emit deviceAdded(device);
            m_soundDevices.insert_or_assign(id, std::move(device));
        }
    } else if (strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
        const uint32_t node_id = pw_properties_parse_int(spa_dict_lookup(props, PW_KEY_NODE_ID));
        const uint32_t port_id = pw_properties_parse_int(spa_dict_lookup(props, PW_KEY_PORT_ID));
        const char* dir = spa_dict_lookup(props, PW_KEY_PORT_DIRECTION);
        const spa_direction direction =
                strcmp(dir, "out") ? SPA_DIRECTION_INPUT : SPA_DIRECTION_OUTPUT;

        if (m_soundDevices.contains(node_id)) {
            m_objects.insert_or_assign(id, Object{Port(port_id, node_id, direction)});
            m_soundDevices[node_id]->registerDevicePort(id, props);
        } else {
            for (auto& [device_id, device] : m_soundDevices) {
                if (device->isOpen()) {
                    uint32_t filterId = pw_filter_get_node_id(device->getFilter());
                    if (node_id == filterId) {
                        m_soundDevices[device_id]->registerFilterPort(id, props);
                    }
                }
            }
        }
    } else if (strcmp(type, PW_TYPE_INTERFACE_Link) == 0) {
        const int out_port = pw_properties_parse_int(
                spa_dict_lookup(props, PW_KEY_LINK_OUTPUT_PORT));
        const int in_port = pw_properties_parse_int(spa_dict_lookup(props, PW_KEY_LINK_INPUT_PORT));

        if (m_objects.contains(in_port)) {
            m_objects.insert_or_assign(id, Object{Link(in_port)});
        } else if (m_objects.contains(out_port)) {
            m_objects.insert_or_assign(id, Object{Link(out_port)});
        }

        const int out_node = pw_properties_parse_int(
                spa_dict_lookup(props, PW_KEY_LINK_OUTPUT_NODE));

        if (m_soundDevices.contains(out_node)) {
            m_soundDevices[out_node]->registerLink(id, SPA_DIRECTION_INPUT, props);
            return;
        }

        const int in_node = pw_properties_parse_int(spa_dict_lookup(props, PW_KEY_LINK_INPUT_NODE));

        if (m_soundDevices.contains(in_node)) {
            m_soundDevices[in_node]->registerLink(id, SPA_DIRECTION_OUTPUT, props);
            return;
        }
    }
}

void PipewireEnumerator::registryEventGlobalRemove(void* data, unsigned int id) {
    PipewireEnumerator* pEnumerator = static_cast<PipewireEnumerator*>(data);

    if (!pEnumerator->m_objects.contains(id)) {
        return;
    }

    Object& object = pEnumerator->m_objects.at(id);

    if (std::get_if<Node>(&object)) {
        auto& device = pEnumerator->m_soundDevices.extract(id).mapped();
        device->close();
        emit pEnumerator->deviceRemoved(device);
    } else if (auto* port = std::get_if<Port>(&object)) {
        pEnumerator->m_soundDevices[port->nodeId]->unregisterDevicePort(port->id, port->direction);
    } else if (auto* link = std::get_if<Link>(&object)) {
        if (pEnumerator->m_objects.contains(link->portId)) {
            const Port& port = std::get<Port>(pEnumerator->m_objects[link->portId]);
            pEnumerator->m_soundDevices[port.nodeId]->unregisterLink(id, port.id, port.direction);
        }
    }
}

pw_core* PipewireEnumerator::getCore() {
    return m_pCore;
}

pw_thread_loop* PipewireEnumerator::getThreadLoop() {
    return m_pThreadLoop;
}

std::vector<SoundDevicePointer> PipewireEnumerator::queryDevices() const {
    std::vector<SoundDevicePointer> devices{};

    for (const auto& [id, device] : m_soundDevices) {
        devices.push_back(device);
    }

    return devices;
}

std::vector<std::string> PipewireEnumerator::getAPIs() const {
    return std::vector{std::string("PipeWire")};
}

void PipewireEnumerator::initialize() {
    connect(this,
            &PipewireEnumerator::deviceAdded,
            m_pManager,
            &SoundManager::addDevice,
            Qt::QueuedConnection);
    connect(this,
            &PipewireEnumerator::deviceRemoved,
            m_pManager,
            &SoundManager::removeDevice,
            Qt::QueuedConnection);
    connect(this,
            &PipewireEnumerator::portAdded,
            m_pManager,
            &SoundManager::updateDevice,
            Qt::QueuedConnection);
    connect(this,
            &PipewireEnumerator::portRemoved,
            m_pManager,
            &SoundManager::updateDevice,
            Qt::QueuedConnection);
    connect(this,
            &PipewireEnumerator::linkAdded,
            m_pManager,
            &SoundManager::updateDevice,
            Qt::QueuedConnection);
    connect(this,
            &PipewireEnumerator::linkRemoved,
            m_pManager,
            &SoundManager::updateDevice,
            Qt::QueuedConnection);
}

int PipewireEnumerator::metadataProperty(void* data,
        [[maybe_unused]] uint32_t id,
        const char* key,
        [[maybe_unused]] const char* type,
        const char* value) {
    PipewireEnumerator* pEnumerator = static_cast<PipewireEnumerator*>(data);

    if (strcmp(key, "clock.allowed-rates") == 0) {
        // parse json arrays like [ 44100, 48000, 96000 ]
        QString s = value;
        s.remove('[');
        s.remove(']');

        QStringList parts = s.split(',', Qt::SkipEmptyParts);

        for (const QString& part : parts) {
            pEnumerator->m_samplerates.push_back(mixxx::audio::SampleRate(part.trimmed().toInt()));
        }
        pEnumerator->m_pManager->checkConfig();
    }
    return 0;
}
