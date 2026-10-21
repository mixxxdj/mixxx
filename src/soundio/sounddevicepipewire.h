#pragma once

#include <spa/utils/defs.h>

#include "audio/types.h"
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
    ~SoundDevicePipewire() override;

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

    QString getChannelString(ChannelGroup channelGroup, bool input) const override;
    mixxx::audio::SampleRate getDefaultSampleRate() const override;

    void writeOutput(float* output, int channel, int framesPerBuffer, int offset = 0);
    void writeInput(const float* input, int channel, int framesPerBuffer, int offset = 0);

    void createLink(uint32_t outNodeId,
            uint32_t outPortId,
            uint32_t inNodeId,
            uint32_t inPortId);

    void setNumInputs(SINT count) {
        m_numInputChannels = mixxx::audio::ChannelCount::fromInt(count);
    }

    void setNumOutputs(SINT count) {
        m_numOutputChannels = mixxx::audio::ChannelCount::fromInt(count);
    }

  private:
    PipewireEnumerator* m_pEnumerator;
    std::string m_error;
};
