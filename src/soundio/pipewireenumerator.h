#pragma once

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>
#include <spa/utils/defs.h>

#include <QObject>
#include <cstdint>
#include <unordered_set>

#include "audio/types.h"
#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "control/controlproxy.h"
#include "preferences/dialog/dlgprefsound.h"
#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddeviceenumerator.h"
#include "soundio/sounddevicepipewire.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerconfig.h"
#include "soundio/soundmanagerutil.h"

class PipewireEnumerator : public SoundDeviceEnumerator {
    Q_OBJECT
  public:
    PipewireEnumerator(UserSettingsPointer pConfig,
            SoundManager* pManager);
    ~PipewireEnumerator() override;

    QList<mixxx::audio::SampleRate> getSampleRates(
            [[maybe_unused]] bool jackSampleRates) const override {
        return {};
    }

    std::vector<SoundDevicePointer> queryDevices() const override;

    void initialize() override;
    void deinitialize() override;

    bool isOpen(uint32_t id);
    std::string openDeviceInput(uint32_t id, const AudioInput& input);
    std::string openDeviceOutput(uint32_t id, const AudioOutput& output);
    void closeDevices();

    QList<QString> getAPIs() const override {
        return {SoundManagerConfig::kAPIPipewire};
    }

    std::vector<std::pair<uint32_t, QString>> queryHardwareDevices() override;
    std::unordered_map<uint32_t, QString> queryHardwareVolumes(uint32_t deviceIndex) override;

    mixxx::audio::SampleRate getDefaultSampleRate() const {
        return m_defaultSampleRate;
    }

    QString getChannelString(uint32_t id, ChannelGroup channelGroup, bool input) const;

  signals:
    void deviceAdded(SoundDevicePointer pDevice);
    void deviceRemoved(SoundDevicePointer pDevice);

  private slots:
    void registerInput(const AudioInput& input, AudioDestination* dest);
    void registerOutput(const AudioOutput& output, AudioSource* src);
    void setHardwareGain(uint32_t deviceIndex, uint32_t routeIndex, float gain);

  private:
    struct Link {
        uint32_t input;
        uint32_t output;
    };

    struct Port {
        uint32_t node;
        bool isInput;
        std::vector<uint32_t> links;

        std::string getDisplayName() const {
            return name + channel;
        }

        // this is port.name after stripping out channel and delimiter,
        // and appending a ':' to simplify logic
        std::string name;
        // in case port had no recognizable channel, entire name is put
        // here so SoundDevicePipewire::getChannelString logic works fine
        std::string channel;
    };

    struct Node {
        std::vector<uint32_t> inputs;
        std::vector<uint32_t> outputs;
        spa_hook listener;
        pw_node* proxy;
    };

    struct Device {
        struct Route {
            bool mute;
            uint32_t device;
            ControlPotmeter volume;
            uint32_t numChannels;
            QString description;

            Route(const ConfigKey& key, const QString& description)
                    : mute(false),
                      volume(key),
                      description(description) {
            }
        };

        pw_device* device;
        std::string name;
        spa_hook listener = {};
        bool serialFlag = false;
        bool enumSerialFlag = false;
        std::unordered_map<uint32_t, Route> routes = {};
    };

    struct PortPair {
        struct Port {
            void* data;
            uint32_t id;
        };

        Port left;
        Port right;
        uint32_t activeDevice = 0;
        std::atomic<bool> active = false;
    };

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

    void updateAudioLatencyUsage(SINT samplerate, SINT framesPerBuffer);
    void setLatency(unsigned int sampleRate, unsigned int framesPerBuffer);

    void createInputPorts(const AudioInput& path, PortPair& ports);
    void createOutputPorts(const AudioOutput& path, PortPair& ports);
    void createPorts(PortPair& ports, std::string_view name, spa_direction direction);
    void closePorts(PortPair& ports);

    void updateFilterLatency(const SINT samplerate, const SINT bufferSize);
    std::unordered_set<uint32_t> getPwDevicesByOutput(
            const AudioPath& path, spa_direction direction);

    bool nodeHasPorts(const Node& node);

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

    uint64_t xrun_duration;
    int m_invalidTimeInfoCount;
    double m_lastCallbackEntrytoDacSecs;
    PerformanceTimer m_clkRefTimer;
    mixxx::audio::SampleRate m_defaultSampleRate;

    std::unordered_map<AudioInput, PortPair> m_inputs;
    std::unordered_map<AudioOutput, PortPair> m_outputs;

    PollingControlProxy m_audioLatencyUsage;
    ControlObject m_coPipewirePatchbaySync;
    ControlObject m_coBufferSize;
    ControlObject m_coLatencyParamsMismatch;
    ControlProxy m_coOutputLatencyMs;
    ControlProxy m_coSamplerate;
    mixxx::Duration m_timeInAudioCallback;
    int m_framesSinceAudioLatencyUsageUpdate;
    uint32_t m_filterId;
    // Handle all connections made to/from Mixxx node
    // If we do not, then we only track connections made in the
    // preference page, and leave the external patchbay connections
    // If we do, then all connections to Mixxx will be affected, even
    // the ones made with external patchbay
    int m_coreRegistrySyncSeq;
    int m_coreDeviceSyncSeq;
    bool m_forceQuantum;
    bool m_forceSamplerate;
    uint32_t m_samplerate;
    uint32_t m_bufferSize;

    ControlPotmeter m_coMainVolume;
    ControlPotmeter m_coHeadVolume;
    ControlPotmeter m_coBoothVolume;
    ControlObject m_coMainVolumeRoute;
    ControlObject m_coHeadVolumeRoute;
    ControlObject m_coBoothVolumeRoute;
    ControlObject m_coMainVolumeDevice;
    ControlObject m_coHeadVolumeDevice;
    ControlObject m_coBoothVolumeDevice;
    ControlObject m_coManualVolumeDevice;
    ControlObject m_coGraphDriver;
    uint32_t m_enumParamDeviceId;
    uint32_t m_coreSync;
    // pw_device param enumeration seq, since one enumeration
    // request yields multiple callbacks, we manually track
    // and remove previous entries
    std::unordered_map<int, uint32_t> m_deviceParamSeq;
    int m_prevDeviceParamSeq = 0;
};
