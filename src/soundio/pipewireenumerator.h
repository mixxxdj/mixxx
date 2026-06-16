#pragma once

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>
#include <spa/utils/defs.h>

#include <QObject>
#include <set>

#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddeviceenumerator.h"
#include "soundio/sounddevicepipewire.h"
#include "soundio/soundmanager.h"

class PipewireEnumerator : public SoundDeviceEnumerator {
    Q_OBJECT
  public:
    PipewireEnumerator(UserSettingsPointer pConfig,
            SoundManager* pManager);
    ~PipewireEnumerator() override;

    QList<mixxx::audio::SampleRate> getSampleRates() const override;
    std::vector<SoundDevicePointer> queryDevices() const override;
    std::vector<std::string> getAPIs() const override {
        return std::vector<std::string>{"PipeWire"};
    }

    void initialize();

    pw_core* getCore();
    pw_thread_loop* getThreadLoop();

    bool isOpen(uint32_t id);
    void openDevice(uint32_t id, std::set<uint8_t>& inChannels, std::set<uint8_t>& outChannels);
    void closeDevice(uint32_t id);

  signals:
    void deviceAdded(SoundDevicePointer pDevice);
    void deviceRemoved(SoundDevicePointer pDevice);

  private:
    static void registryEventGlobalOuter(void* data,
            uint32_t id,
            uint32_t permissions,
            const char* type,
            uint32_t version,
            const struct spa_dict* props) {
        ((PipewireEnumerator*)data)->registryEventGlobal(id, permissions, type, version, props);
    }

    static void registryEventGlobalRemoveOuter(void* data, uint32_t id) {
        ((PipewireEnumerator*)data)->registryEventGlobalRemove(id);
    }

    static constexpr pw_registry_events registry_events = {
            .version = PW_VERSION_REGISTRY_EVENTS,
            .global = registryEventGlobalOuter,
            .global_remove = registryEventGlobalRemoveOuter,
    };

    static int metadataProperty(void* data,
            uint32_t id,
            const char* key,
            const char* type,
            const char* value);

    static constexpr struct pw_metadata_events metadataEvents = {
            .version = PW_VERSION_METADATA_EVENTS,
            .property = metadataProperty};

    static void callback(void* data, spa_io_position* pos) {
        ((PipewireEnumerator*)data)->callback(pos);
    }

    static constexpr pw_filter_events filter_events{
            .version = PW_VERSION_FILTER_EVENTS,
            .destroy = nullptr,
            .state_changed = nullptr,
            .io_changed = nullptr,
            .param_changed = nullptr,
            .add_buffer = nullptr,
            .remove_buffer = nullptr,
            .process = callback,
            .drained = nullptr,
            .command = nullptr,
    };

    void registryEventGlobal(uint32_t id,
            uint32_t permissions,
            const char* type,
            uint32_t version,
            const struct spa_dict* props);
    void registryEventGlobalRemove(unsigned int id);

    void callback(const spa_io_position* pos);

    void addDevice(uint32_t id);
    void removeDevice(uint32_t id);

    void writeInput(const float* input, int channel, int framesPerBuffer, int offset = 0);
    void writeOutput(float* output, int channel, int framesPerBuffer, int offset = 0);

    void createLink(uint32_t outNodeId,
            uint32_t outPortId,
            uint32_t inNodeI,
            uint32_t inPortId);
    void updateAudioLatencyUsage(const SINT framesPerBuffer);

    std::unordered_map<uint32_t, QSharedPointer<SoundDevicePipewire>> m_soundDevices;

    struct Link {
        uint32_t input;
        uint32_t output;
    };

    struct Port {
        // this is not global id, but port.id, which starts from 0
        // using this we can directly index into the port vector
        // on the node from nodeId
        // global id indexes into maps, port.id indexes into vectors
        uint32_t id;
        uint32_t nodeId;
        spa_direction direction;
    };

    struct Node {};
    using Object = std::variant<Node, Port, Link>;

    std::unordered_map<uint32_t, Object> m_objects;
    QList<mixxx::audio::SampleRate> m_samplerates;

    SoundManager* m_pSoundManager;
    UserSettingsPointer m_pConfig;

    pw_core* m_pCore;
    pw_registry* m_pRegistry;
    pw_context* m_pContext;
    pw_metadata* m_pMetadata;
    pw_thread_loop* m_pThreadLoop;
    spa_hook m_registryListener;
    spa_hook m_metadataListener;
    spa_hook m_filterListener;
    pw_filter* m_pFilter;

    struct Device {
        struct Port {
            void* pPortData;

            // these are our own indices, and are not assigned by
            // pipewire in any way
            uint32_t devicePort;
            uint32_t filterPort;
        };

        // inputs correspond to filter inputs and soundDevice outputs
        std::vector<Port> inputs;
        // outputs correspond to filter outputs and soundDevice inputs
        std::vector<Port> outputs;
    };

    std::unordered_map<uint32_t, Device> m_openedDevices;

    bool m_initialized;
    uint64_t xrun_duration;
    int m_invalidTimeInfoCount;
    double m_lastCallbackEntrytoDacSecs;
    PerformanceTimer m_clkRefTimer;
    mixxx::audio::SampleRate m_sampleRate;

    PollingControlProxy m_audioLatencyUsage;
    mixxx::Duration m_timeInAudioCallback;
    int m_framesSinceAudioLatencyUsageUpdate;
    uint32_t m_filterId;
};
