#pragma once

#include <spa/utils/defs.h>

#include "sounddevice.h"
#include "soundio/soundmanagerconfig.h"

class SoundManager;
class PipewireEnumerator;

class SoundDevicePipewire : public SoundDevice {
  public:
    SoundDevicePipewire(UserSettingsPointer pConfig,
            SoundManager* pManager,
            PipewireEnumerator* pEnumerator,
            uint32_t id,
            const std::string_view name);
    SoundDeviceStatus open(bool isClkRefDevice, int syncBuffers) override;
    bool isOpen() const override;
    SoundDeviceStatus close() override;

    void readProcess(SINT) override {
    }
    void writeProcess(SINT) override {
    }
    QString getError() const override {
        return "";
    }

    mixxx::audio::SampleRate getDefaultSampleRate() const override {
        return SoundManagerConfig::kMixxxDefaultSampleRate;
    }

    void writeOutput(float* output, int channel, int framesPerBuffer, int offset = 0);
    void writeInput(const float* input, int channel, int framesPerBuffer, int offset = 0);

    void createLink(uint32_t outNodeId,
            uint32_t outPortId,
            uint32_t inNodeId,
            uint32_t inPortId);
    void registerDevicePort(uint32_t id, const struct spa_dict* props);
    void unregisterDevicePort(uint32_t id);

    AudioPath* getAudioPath(uint32_t channel, spa_direction direction);

    struct Port {
        uint32_t id;
        std::string name;
    };

    std::span<Port> getInPorts() {
        return m_inPorts;
    }

    std::span<Port> getOutPorts() {
        return m_outPorts;
    }

  private:
    PipewireEnumerator* m_pEnumerator;
    std::vector<Port> m_inPorts;
    std::vector<Port> m_outPorts;
};
