#pragma once

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>
#include <spa/utils/defs.h>

#include <QObject>
#include <cstdint>

#include "audio/types.h"
#include "control/controlobject.h"
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
        return m_samplerates;
    }

    std::vector<SoundDevicePointer> queryDevices() const override;

    void initialize() override;
    void deinitialize() override;

    bool isOpen(uint32_t id);
    std::string openDeviceInput(uint32_t id,
            const AudioInput& input,
            mixxx::audio::SampleRate sampleRate,
            SINT framesPerBuffer);
    std::string openDeviceOutput(uint32_t id,
            const AudioOutput& output,
            mixxx::audio::SampleRate sampleRate,
            SINT framesPerBuffer);
    void closeDevice(uint32_t id);

    mixxx::audio::SampleRate getDefaultSampleRate() const {
        return m_defaultSampleRate;
    }

    QList<QString> getAPIs() const override {
        return {SoundManagerConfig::kAPIPipewire};
    }

    QString getChannelString(uint32_t id, ChannelGroup channelGroup, bool input) const;
    static void patchbayWaitTimer(void* data, [[maybe_unused]] uint64_t expirations) {
        static_cast<PipewireEnumerator*>(data)->patchbayWaitTimer();
    }

  signals:
    void deviceAdded(SoundDevicePointer pDevice);
    void deviceRemoved(SoundDevicePointer pDevice);

  private slots:
    void registerInput(const AudioInput& input, AudioDestination* dest);
    void registerOutput(const AudioOutput& output, AudioSource* src);

  private:
    void patchbayWaitTimer();
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
    };

    struct PortPair {
        struct Port {
            void* data;
            uint32_t id;
        };

        struct ConnectedDevice {
            std::vector<uint32_t> leftPorts;
            std::vector<uint32_t> rightPorts;
        };

        // Registry for all connections to a PortPair, including devices which
        // are not shown on Preference page, but might become "visible" after
        // the visible device is removed
        std::unordered_map<uint32_t, ConnectedDevice> connectedDevices;
        // the visible device of the registry, what is shown on deviceComboBox
        uint32_t activeDevice = 0;

        Port left;
        Port right;
        std::atomic<bool> active = false;

        // add/remove port and return if visible device might have changed
        bool addPort(uint32_t deviceId, uint32_t portId, bool isLeft);
        bool removePort(uint32_t deviceId, uint32_t portId, bool isLeft);

        // Compute if current connections have a presentable device/channel representation
        // Blank channelComboBox represents non presentable config (non contiguous connections
        // or multiple devices).

        // These are separate functions because we consider different conditions
        // for mono connections. For inputs, mono connection is one device output
        // port connected to both left and right input ports, while for outputs,
        // mono connection is one L/R output port connected to same L/R input
        // device port
        ChannelGroup inputChannel(std::span<const uint32_t> ports);
        ChannelGroup outputChannel(std::span<const uint32_t> ports);
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

    std::string createLink(uint32_t outNodeId,
            uint32_t outPortId,
            uint32_t inNodeI,
            uint32_t inPortId);
    void destroyLink(uint32_t id);

    void updateAudioLatencyUsage(const SINT framesPerBuffer);
    void setLatency(unsigned int sampleRate, unsigned int framesPerBuffer);

    void createInputPorts(const AudioInput& path, PortPair& ports);
    void createOutputPorts(const AudioOutput& path, PortPair& ports);
    void createPorts(PortPair& ports, std::string_view name, spa_direction direction);
    void closePorts(PortPair& ports, bool isInput);

    void updateFilterLatency(unsigned int sampleRate, unsigned int framesPerBuffer);
    bool nodeHasPorts(const Node& node);
    bool portsEmpty();

    std::unordered_map<uint32_t, Node> m_nodes;
    std::unordered_map<uint32_t, Port> m_ports;
    std::unordered_map<uint32_t, Link> m_links;
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

    std::unordered_map<uint32_t, QSharedPointer<SoundDevicePipewire>> m_soundDevices;

    uint64_t xrun_duration;
    int m_invalidTimeInfoCount;
    double m_lastCallbackEntrytoDacSecs;
    PerformanceTimer m_clkRefTimer;
    mixxx::audio::SampleRate m_sampleRate;
    mixxx::audio::SampleRate m_defaultSampleRate;

    std::unordered_map<AudioInput, PortPair> m_inputs;
    std::unordered_map<AudioOutput, PortPair> m_outputs;

    PollingControlProxy m_audioLatencyUsage;
    ControlObject m_COmanageExternalLinks;
    mixxx::Duration m_timeInAudioCallback;
    int m_framesSinceAudioLatencyUsageUpdate;
    uint32_t m_filterId;
    uint32_t m_framesPerBuffer;
    // Handle all connections made to/from Mixxx node
    // If we do not, then we only track connections made in the
    // preference page, and leave the external patchbay connections
    // If we do, then all connections to Mixxx will be affected, even
    // the ones made with external patchbay
    bool m_manageExternalLinks;
    int m_coreSyncSeq;
    spa_source* m_patchbayWaitTimer;
};
