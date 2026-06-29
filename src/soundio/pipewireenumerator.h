#pragma once

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>
#include <spa/utils/defs.h>

#include <QObject>

#include "audio/types.h"
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
        return m_initialized ? std::vector<std::string>{"PipeWire"} : std::vector<std::string>{};
    }

    void initialize();

    bool isOpen(uint32_t id);
    std::string openDevice(const SoundDevicePipewire& device,
            mixxx::audio::SampleRate sampleRate,
            SINT framesPerBuffer);
    void closeDevice(uint32_t id);
    mixxx::audio::SampleRate getDefaultSampleRate() const {
        return m_defaultSampleRate;
    }

  signals:
    void deviceAdded(SoundDevicePointer pDevice);
    void deviceRemoved(SoundDevicePointer pDevice);

  private slots:
    void registerInput(const AudioInput& input, AudioDestination* dest);
    void registerOutput(const AudioOutput& output, AudioSource* src);

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

    std::string createLink(uint32_t outNodeId,
            uint32_t outPortId,
            uint32_t inNodeI,
            uint32_t inPortId);
    void destroyLink(uint32_t id);

    void updateAudioLatencyUsage(const SINT framesPerBuffer);
    void setLatency(unsigned int sampleRate, unsigned int framesPerBuffer);
    void createInputPorts(const AudioInput& path);
    void createOutputPorts(const AudioOutput& path);
    uint32_t* createPorts(const AudioPath& path, bool channel);

    struct Link {
        uint32_t input;
        uint32_t output;
    };

    struct Port {
        uint32_t node;
    };

    struct Node {};
    using Object = std::variant<Node, Port, Link>;

    std::unordered_map<uint32_t, Object> m_objects;
    QList<mixxx::audio::SampleRate> m_samplerates;

    SoundManager* m_pSoundManager;
    UserSettingsPointer m_pConfig;

    pw_thread_loop* m_pPwThreadLoop;
    pw_context* m_pPwContext;
    pw_core* m_pPwCore;
    pw_registry* m_pPwRegistry;
    pw_metadata* m_pPwMetadata;
    pw_filter* m_pPwFilter;
    spa_hook m_pwRegistryListener;
    spa_hook m_pwFilterListener;
    spa_hook m_pwMetadataListener;

    std::unordered_map<uint32_t, QSharedPointer<SoundDevicePipewire>> m_soundDevices;
    std::vector<uint32_t> m_openedDevices;

    bool m_initialized;
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
};
