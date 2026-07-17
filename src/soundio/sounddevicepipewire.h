#pragma once

#include <spa/utils/defs.h>

#include "sounddevice.h"
#include "soundio/soundmanagerconfig.h"
#include "soundio/soundmanagerutil.h"

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
        return m_error.c_str();
    }

    QString getChannelString(ChannelGroup channelGroup, bool input) override;
    mixxx::audio::SampleRate getDefaultSampleRate() const override;

    void writeOutput(float* output, int channel, int framesPerBuffer, int offset = 0);
    void writeInput(const float* input, int channel, int framesPerBuffer, int offset = 0);

    void createLink(uint32_t outNodeId,
            uint32_t outPortId,
            uint32_t inNodeId,
            uint32_t inPortId);
    void registerPort(uint32_t id, const struct spa_dict* props);
    void unregisterPort(uint32_t id);
    void registerLink(uint32_t id, spa_direction direction);
    void unregisterLink(uint32_t id, spa_direction direction);

    std::span<const uint32_t> getInLinks() const {
        return m_inLinks;
    }

    std::span<const uint32_t> getOutLinks() const {
        return m_outLinks;
    }

    struct Port {
        std::string getDisplayName() const {
            return name + channel;
        }

        uint32_t id;
        // this is port.name after stripping out channel and delimiter,
        // and appending a ':' to simplify logic
        std::string name;
        // in case port had no recognizable channel, entire name is put
        // here so SoundDevicePipewire::getChannelString logic works fine
        std::string channel;
    };

    std::span<const Port> getInPorts() const {
        return m_inPorts;
    }

    std::span<const Port> getOutPorts() const {
        return m_outPorts;
    }

  private:
    PipewireEnumerator* m_pEnumerator;
    std::vector<Port> m_inPorts;
    std::vector<Port> m_outPorts;

    std::vector<uint32_t> m_inLinks;
    std::vector<uint32_t> m_outLinks;
    std::string m_error;
};
