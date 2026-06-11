#pragma once

#include <pipewire/pipewire.h>
#include <spa/param/audio/raw.h>
#include <spa/utils/defs.h>

#include <QString>

#include "control/pollingcontrolproxy.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanagerconfig.h"
#include "util/duration.h"
#include "util/fifo.h"
#include "util/performancetimer.h"

class SoundManager;
class PipewireEnumerator;

class SoundDevicePipewire : public SoundDevice {
  public:
    SoundDevicePipewire(UserSettingsPointer pConfig,
            SoundManager* pManager,
            PipewireEnumerator* pEnumerator,
            uint32_t id,
            const std::string_view name);
    ~SoundDevicePipewire() override;

    SoundDeviceStatus open(bool isClkRefDevice, int syncBuffers) override;
    bool isOpen() const override;
    SoundDeviceStatus close() override;
    void readProcess(SINT framesPerBuffer) override;
    void writeProcess(SINT framesPerBuffer) override;

    mixxx::audio::SampleRate getDefaultSampleRate() const override {
        return SoundManagerConfig::kMixxxDefaultSampleRate;
    }

    void registerDevicePort(uint32_t id, const struct spa_dict* props);
    void unregisterDevicePort(uint32_t id, spa_direction direction);
    void registerLink(uint32_t id, spa_direction direction, const struct spa_dict* props);
    void unregisterLink(uint32_t id, uint32_t portIndex, spa_direction direction);

    void registerFilterPort(uint32_t id, const spa_dict* props);

    FIFO<CSAMPLE>* addFilterPort(spa_direction dir, uint8_t id);
    QString getError() const override;

    pw_filter* getFilter() {
        return m_pFilter;
    }

  private:
    static void callback(void* data, spa_io_position* pos);
    static void callbackDrift(void* data, spa_io_position* pos);
    static void callbackClkRef(void* data, spa_io_position* pos);

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

    static constexpr pw_filter_events filter_drift_events{
            .version = PW_VERSION_FILTER_EVENTS,
            .destroy = nullptr,
            .state_changed = nullptr,
            .io_changed = nullptr,
            .param_changed = nullptr,
            .add_buffer = nullptr,
            .remove_buffer = nullptr,
            .process = callbackDrift,
            .drained = nullptr,
            .command = nullptr,
    };

    static constexpr pw_filter_events filter_clkref_events{
            .version = PW_VERSION_FILTER_EVENTS,
            .destroy = nullptr,
            .state_changed = nullptr,
            .io_changed = nullptr,
            .param_changed = nullptr,
            .add_buffer = nullptr,
            .remove_buffer = nullptr,
            .process = callbackClkRef,
            .drained = nullptr,
            .command = nullptr,
    };

    void updateCallbackEntryToDacTime(const spa_io_position* timeInfo);
    void updateAudioLatencyUsage(const SINT framesPerBuffer);

    void callback(const spa_io_position* pos);
    void callbackDrift(const spa_io_position* pos);
    void callbackClkRef(const spa_io_position* pos);

    void createLink(uint32_t outNodeId, uint32_t outPortId, uint32_t inNodeId, uint32_t inPortId);

    void writeOutput(float* output, int channel, int framesPerBuffer, int offset);
    void writeInput(const float* input, int channel, int framesPerBuffer, int offset);

    struct DevicePort {
        struct Link {
            uint32_t id;
        };

        uint32_t id;
        std::string name;
        std::vector<Link> links = {};
    };

    DevicePort* findPort(uint32_t id);

    std::vector<DevicePort> m_inDevicePorts;
    std::vector<DevicePort> m_outDevicePorts;

    // each port gets one channel planar audio
    struct Port {
        uint32_t id = PW_ID_ANY;
        FIFO<CSAMPLE>* fifo;
    };

    pw_filter* m_pFilter;

    std::vector<Port> m_inPorts;
    std::vector<Port> m_outPorts;

    bool m_outputDrift;
    bool m_inputDrift;

    // Whether we have set the thread priority to realtime or not.
    bool m_bSetThreadPriority;
    PollingControlProxy m_audioLatencyUsage;
    mixxx::Duration m_timeInAudioCallback;
    int m_framesSinceAudioLatencyUsageUpdate;
    int m_syncBuffers;
    int m_invalidTimeInfoCount;
    PerformanceTimer m_clkRefTimer;
    double m_lastCallbackEntrytoDacSecs;
    PipewireEnumerator* m_pEnumerator;
    spa_hook m_listener;
    bool m_isClkRef;
    size_t m_fifoSize;
    bool m_closing;
    QString m_lastError;
};
