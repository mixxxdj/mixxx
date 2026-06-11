#pragma once

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>
#include <spa/utils/defs.h>

#include <QObject>

#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddeviceenumerator.h"
#include "soundio/sounddevicepipewire.h"
#include "soundio/soundmanager.h"

class PipewireEnumerator : public SoundDeviceEnumerator {
    Q_OBJECT
  public:
    PipewireEnumerator(UserSettingsPointer config,
            SoundManager* sm);
    ~PipewireEnumerator() override;

    QList<mixxx::audio::SampleRate> getSampleRates() const override;
    std::vector<SoundDevicePointer> queryDevices() const override;
    std::vector<std::string> getAPIs() const override;

    void initialize();

    pw_core* getCore();
    pw_thread_loop* getThreadLoop();

  signals:
    void deviceAdded(SoundDevicePointer device);
    void deviceRemoved(SoundDevicePointer device);
    void portAdded(SoundDevicePointer device);
    void portRemoved(SoundDevicePointer device);
    void linkAdded(SoundDevicePointer device);
    void linkRemoved(SoundDevicePointer device);

  private:
    /// Source of all events, like a soundcard is detected/removed, link is created/destroyed
    void registryEventGlobal(uint32_t id,
            uint32_t permissions,
            const char* type,
            uint32_t version,
            const struct spa_dict* props);
    static void registryEventGlobalOuter(void* data,
            uint32_t id,
            uint32_t permissions,
            const char* type,
            uint32_t version,
            const struct spa_dict* props);

    static void registryEventGlobalRemove(void* data, unsigned int id);

    static constexpr struct pw_registry_events registry_events = {
            .version = PW_VERSION_REGISTRY_EVENTS,
            .global = registryEventGlobalOuter,
            .global_remove = registryEventGlobalRemove,
    };

    struct Filter {
        struct Port {
            uint32_t id;
        };

        pw_filter* filter;
        std::unordered_map<std::shared_ptr<SoundDevicePipewire>, std::vector<Port>> devices;
    };

    struct Device {
        pw_device* device;
        pw_node* in_node;
        pw_node* out_node;
        Filter filter;
    };

    std::unordered_map<uint32_t, Device> m_devices;

    static int metadataProperty(void* data,
            uint32_t id,
            const char* key,
            const char* type,
            const char* value);

    static constexpr struct pw_metadata_events metadataEvents = {
            .version = PW_VERSION_METADATA_EVENTS,
            .property = metadataProperty};

    void createSoundDevice(uint32_t id, const char* name);
    void destroySoundDevice(uint32_t id);

    std::unordered_map<uint32_t, QSharedPointer<SoundDevicePipewire>> m_soundDevices;

    struct Link {
        uint32_t portId;
    };

    struct Port {
        // this is not global id, but port.id, which starts from 0
        // using this we can directly index into the port vector
        // on the node from nodeId
        uint32_t id;
        uint32_t nodeId;
        spa_direction direction;
    };

    struct Node {};
    using Object = std::variant<Node, Port, Link>;

    std::unordered_map<uint32_t, Object> m_objects;
    QList<mixxx::audio::SampleRate> m_samplerates;

    SoundManager* m_pManager;
    UserSettingsPointer m_pConfig;

    pw_core* m_pCore;
    pw_registry* m_pRegistry;
    pw_context* m_pContext;
    pw_metadata* m_pMetadata;
    pw_thread_loop* m_pThreadLoop;
    spa_hook m_registryListener;
    spa_hook m_metadataListener;

    bool m_initialized;
};
