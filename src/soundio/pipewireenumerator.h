#pragma once

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>
#include <spa/utils/defs.h>

#include <QObject>
#include <unordered_set>

#include "audio/types.h"
#include "control/controlobject.h"
#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddeviceenumerator.h"
#include "soundio/sounddevicepipewire.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerconfig.h"

class PipewireEnumerator : public SoundDeviceEnumerator {
    Q_OBJECT
  public:
    PipewireEnumerator(UserSettingsPointer pConfig,
            SoundManager* pManager);
    ~PipewireEnumerator() override;

    QList<mixxx::audio::SampleRate> getSampleRates(
            [[maybe_unused]] bool jackSampleRates) const override {
        return m_samplerates;
    }

    std::vector<SoundDevicePointer> queryDevices() const override;

    void initialize() override;
    void deinitialize() override;

    bool isOpen(uint32_t id);
    std::string openDevice(const SoundDevicePipewire& device,
            mixxx::audio::SampleRate sampleRate,
            SINT framesPerBuffer);
    void closeDevice(uint32_t id);

    mixxx::audio::SampleRate getDefaultSampleRate() const {
        return m_defaultSampleRate;
    }

    QList<QString> getAPIs() const override {
        return {SoundManagerConfig::kAPIPipewire};
    }

  signals:
    void deviceAdded(SoundDevicePointer pDevice);
    void deviceRemoved(SoundDevicePointer pDevice);

  private slots:
    void registerInput(const AudioInput& input, AudioDestination* dest);
    void registerOutput(const AudioOutput& output, AudioSource* src);
    void setHardwareGain(float gain, bool isInput);

  private:
    static void coreEventDone(void* data, uint32_t id, int seq) {
        static_cast<PipewireEnumerator*>(data)->coreEventDone(id, seq);
    }

    static void coreEventError(void* data, uint32_t id, int seq, int res, const char* message) {
        static_cast<PipewireEnumerator*>(data)->coreEventError(id, seq, res, message);
    }

    static constexpr pw_core_events coreEvents = {
            .version = PW_VERSION_CORE_EVENTS,
            .info = nullptr,
            .done = coreEventDone,
            .ping = nullptr,
            .error = coreEventError,
            .remove_id = nullptr,
            .bound_id = nullptr,
            .add_mem = nullptr,
            .remove_mem = nullptr,
#if PW_CHECK_VERSION(0, 3, 68)
            .bound_props = nullptr,
#endif
    };

    void coreEventDone(uint32_t id, int seq);
    void coreEventError(uint32_t id, int seq, int res, const char* message);

    static void registryEventGlobal(void* data,
            uint32_t id,
            uint32_t permissions,
            const char* type,
            uint32_t version,
            const struct spa_dict* props) {
        static_cast<PipewireEnumerator*>(data)->registryEventGlobal(
                id, permissions, type, version, props);
    }

    static void registryEventGlobalRemove(void* data, uint32_t id) {
        static_cast<PipewireEnumerator*>(data)->registryEventGlobalRemove(id);
    }

    static constexpr pw_registry_events registryEvents = {
            .version = PW_VERSION_REGISTRY_EVENTS,
            .global = registryEventGlobal,
            .global_remove = registryEventGlobalRemove,
    };

    void registryEventGlobal(uint32_t id,
            uint32_t permissions,
            const char* type,
            uint32_t version,
            const struct spa_dict* props);
    void registryEventGlobalRemove(unsigned int id);

    static int metadataProperty(void* data,
            uint32_t id,
            const char* key,
            const char* type,
            const char* value) {
        return static_cast<PipewireEnumerator*>(data)->metadataProperty(id, key, type, value);
    }

    static constexpr struct pw_metadata_events metadataEvents = {
            .version = PW_VERSION_METADATA_EVENTS,
            .property = metadataProperty,
    };

    int metadataProperty(uint32_t id,
            const char* key,
            const char* type,
            const char* value);

    static void callback(void* data, spa_io_position* pos) {
        static_cast<PipewireEnumerator*>(data)->callback(pos);
    }

    static constexpr pw_filter_events filterEvents{
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

    void callback(const spa_io_position* pos);

    static void nodeEventInfo(void* data, const struct pw_node_info* info) {
        static_cast<PipewireEnumerator*>(data)->nodeEventInfo(info);
    }

    static constexpr pw_node_events nodeEvents{
            .version = PW_VERSION_FILTER_EVENTS,
            .info = nodeEventInfo,
            .param = nullptr,
    };

    void nodeEventInfo(const struct pw_node_info* info);

    static void deviceEventInfo(void* data, const struct pw_device_info* info) {
        static_cast<PipewireEnumerator*>(data)->deviceEventInfo(info);
    }

    static void deviceEventParam(void* data,
            int seq,
            uint32_t id,
            uint32_t index,
            uint32_t next,
            const struct spa_pod* param) {
        static_cast<PipewireEnumerator*>(data)->deviceEventParam(seq, id, index, next, param);
    }

    static constexpr pw_device_events deviceEvents{
            .version = PW_VERSION_DEVICE_EVENTS,
            .info = deviceEventInfo,
            .param = deviceEventParam,
    };

    void deviceEventInfo(const struct pw_device_info* info);
    void deviceEventParam(int seq,
            uint32_t id,
            uint32_t index,
            uint32_t next,
            const struct spa_pod* param);

    void addDevice(uint32_t id);
    void removeDevice(uint32_t id);

    void writeInput(const float* input, int channel, int framesPerBuffer, int offset = 0);
    void writeOutput(float* output, int channel, int framesPerBuffer, int offset = 0);

    std::string createLink(uint32_t outNodeId,
            uint32_t outPortId,
            uint32_t inNodeI,
            uint32_t inPortId);
    void destroyLink(uint32_t id);

    void updateAudioLatencyUsage(const SINT framesPerBuffer);
    void setLatency(unsigned int sampleRate, unsigned int framesPerBuffer);
    std::pair<uint32_t*, uint32_t*> createInputPorts(const AudioInput& path);
    std::pair<uint32_t*, uint32_t*> createOutputPorts(const AudioOutput& path);
    std::pair<uint32_t*, uint32_t*> createPorts(std::string_view name, spa_direction direction);

    std::unordered_set<uint32_t> getPwDevicesByOutput(
            const AudioPath& path, spa_direction direction);
    std::vector<std::pair<uint32_t, QString>> queryHardwareDevices() override;

    struct Link {
        uint32_t input;
        uint32_t output;
    };

    struct Port {
        uint32_t node;
    };

    struct Node {
        uint32_t driver;
    };

    struct Device {
        pw_device* device;
        std::string name;
        spa_hook listener = {};
        uint32_t inRouteIndex = 0;
        uint32_t inRouteDevice = 0;
        uint32_t outRouteIndex = 0;
        uint32_t outRouteDevice = 0;
        float volumes[2] = {0, 0};
        bool serialFlag = false;
    };

    std::unordered_map<uint32_t, Node> m_nodes;
    std::unordered_map<uint32_t, Port> m_ports;
    std::unordered_map<uint32_t, Link> m_links;
    std::unordered_map<uint32_t, Device> m_devices;
    QList<mixxx::audio::SampleRate> m_samplerates;

    SoundManager* m_pSoundManager;
    UserSettingsPointer m_pConfig;

    pw_thread_loop* m_pPwThreadLoop;
    pw_context* m_pPwContext;
    pw_core* m_pPwCore;
    pw_registry* m_pPwRegistry;
    pw_metadata* m_pPwMetadata;
    pw_filter* m_pPwFilter;
    spa_hook m_pwCoreListener;
    spa_hook m_pwRegistryListener;
    spa_hook m_pwFilterListener;
    spa_hook m_pwMetadataListener;
    spa_hook m_pwDeviceListener;

    std::unordered_map<uint32_t, QSharedPointer<SoundDevicePipewire>> m_soundDevices;
    std::vector<uint32_t> m_openedDevices;

    uint64_t xrun_duration;
    int m_invalidTimeInfoCount;
    double m_lastCallbackEntrytoDacSecs;
    PerformanceTimer m_clkRefTimer;
    mixxx::audio::SampleRate m_sampleRate;
    mixxx::audio::SampleRate m_defaultSampleRate;

    QHash<AudioInput, std::pair<uint32_t*, uint32_t*>> m_inputs;
    QHash<AudioOutput, std::pair<uint32_t*, uint32_t*>> m_outputs;

    PollingControlProxy m_audioLatencyUsage;
    mixxx::Duration m_timeInAudioCallback;
    int m_framesSinceAudioLatencyUsageUpdate;
    uint32_t m_filterId;
    uint32_t m_framesPerBuffer;
    std::unique_ptr<ControlObject> m_COInputVolume;
    std::unique_ptr<ControlObject> m_COOutputVolume;
    ControlObject m_COVolumeDevice;
    uint32_t m_volumeDeviceIndex;
    // uint32_t m_deviceEventParamId;
    uint32_t m_enumParamDeviceId;
    uint32_t m_coreSync;
};
