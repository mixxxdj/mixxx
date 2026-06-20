#include "sounddevicepipewire.h"

#include <spa/utils/defs.h>

#include "soundio/pipewireenumerator.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanagerconfig.h"
#include "util/sample.h"

SoundDevicePipewire::SoundDevicePipewire(UserSettingsPointer pConfig,
        SoundManager* pManager,
        PipewireEnumerator* pEnumerator,
        uint32_t id,
        const std::string_view name)
        : SoundDevice(pConfig, pManager),
          m_pEnumerator(pEnumerator) {
    m_hostAPI = "PipeWire";
    m_deviceId.name = name.data();
    m_deviceId.deviceIndex = id;
    m_strDisplayName = QString::fromUtf8(name);
    m_numInputChannels = mixxx::audio::ChannelCount(0);
    m_numOutputChannels = mixxx::audio::ChannelCount(0);
    m_sampleRate = getDefaultSampleRate();
}

SoundDeviceStatus SoundDevicePipewire::open(bool, int) {
    std::set<uint8_t> inChans;
    for (auto& input : m_audioInputs) {
        ChannelGroup channelGroup = input.getChannelGroup();
        uint8_t highChannel = channelGroup.getChannelBase() + channelGroup.getChannelCount();
        for (uint8_t i = channelGroup.getChannelBase(); i < highChannel; i++) {
            inChans.insert(i);
        }
    }

    std::set<uint8_t> outChans;
    for (auto& output : m_audioOutputs) {
        ChannelGroup channelGroup = output.getChannelGroup();
        uint8_t highChannel = channelGroup.getChannelBase() + channelGroup.getChannelCount();
        for (uint8_t i = channelGroup.getChannelBase(); i < highChannel; i++) {
            outChans.insert(i);
        }
    }

    m_pEnumerator->openDevice(m_deviceId.deviceIndex,
            inChans,
            outChans,
            m_sampleRate,
            m_configFramesPerBuffer);
    return SoundDeviceStatus::Ok;
}

bool SoundDevicePipewire::isOpen() const {
    return m_pEnumerator->isOpen(m_deviceId.deviceIndex);
}

SoundDeviceStatus SoundDevicePipewire::close() {
    m_pEnumerator->closeDevice(m_deviceId.deviceIndex);
    m_inPorts.clear();
    m_outPorts.clear();
    return SoundDeviceStatus::Ok;
}

void SoundDevicePipewire::writeOutput(float* output, int channel, int framesPerBuffer, int offset) {
    for (const auto& out : std::as_const(m_audioOutputs)) {
        ChannelGroup chanGroup = out.getChannelGroup();
        const int iChannelCount = chanGroup.getChannelCount();
        const int iChannelBase = chanGroup.getChannelBase();
        const int iChannelEnd = iChannelCount + iChannelBase;

        if (channel < iChannelBase || channel > iChannelEnd) {
            continue;
        }

        const CSAMPLE* pOutputBuffer = &out.getBuffer()[offset];

        if (iChannelCount == 1) {
            for (int i = 0; i < framesPerBuffer; i++) {
                output[i] = pOutputBuffer[i * 2];
            }
        } else {
            for (int i = 0; i < framesPerBuffer; i++) {
                output[i] = pOutputBuffer[i * iChannelCount + channel];
            }
        }
    }
}

void SoundDevicePipewire::writeInput(
        const float* input, int channel, int framesPerBuffer, int offset) {
    for (const auto& in : std::as_const(m_audioInputs)) {
        ChannelGroup chanGroup = in.getChannelGroup();
        const int iChannelCount = chanGroup.getChannelCount();
        const int iChannelBase = chanGroup.getChannelBase();
        const int iChannelEnd = iChannelCount + iChannelBase;

        if (channel < iChannelBase || channel > iChannelEnd) {
            continue;
        }

        CSAMPLE* pInputBuffer = &in.getBuffer()[offset];

        if (iChannelCount == 1) {
            if (input) {
                for (int i = 0; i < framesPerBuffer; i++) {
                    pInputBuffer[i] = input[i];
                    pInputBuffer[i + 1] = input[i];
                }
            } else {
                SampleUtil::fill(pInputBuffer, 0, framesPerBuffer * 2);
            }
        } else {
            if (input) {
                for (int i = 0; i < framesPerBuffer; i++) {
                    pInputBuffer[i * iChannelCount + channel] = input[i];
                }
            } else {
                for (int i = 0; i < framesPerBuffer; i++) {
                    pInputBuffer[i * iChannelCount + channel] = 0;
                }
            }
        }
    }
}

void SoundDevicePipewire::registerDevicePort(uint32_t id, const struct spa_dict* props) {
    std::string name = spa_dict_lookup(props, PW_KEY_PORT_ALIAS);
    const char* dir = spa_dict_lookup(props, PW_KEY_PORT_DIRECTION);

    if (name.empty()) {
        name = spa_dict_lookup(props, PW_KEY_PORT_NAME);
    }

    if (name.empty()) {
        name = dir;
        name += ":";
        name += spa_dict_lookup(props, PW_KEY_PORT_ID);
    }

    if (strcmp(dir, "in") == 0) {
        m_inPorts.emplace_back(id, name);
        m_numOutputChannels = mixxx::audio::ChannelCount::fromInt(m_inPorts.size());
    } else if (strcmp(dir, "out") == 0) {
        m_outPorts.emplace_back(id, name);
        m_numInputChannels = mixxx::audio::ChannelCount::fromInt(m_outPorts.size());
    }
}

void SoundDevicePipewire::unregisterDevicePort(uint32_t id) {
    for (auto it = m_inPorts.begin(); it != m_inPorts.end(); it++) {
        if (it->id == id) {
            m_inPorts.erase(it);
            return;
        }
    }
    for (auto it = m_outPorts.begin(); it != m_outPorts.end(); it++) {
        if (it->id == id) {
            m_outPorts.erase(it);
            return;
        }
    }
}

mixxx::audio::SampleRate SoundDevicePipewire::getDefaultSampleRate() const {
    auto defaultSampleRate = m_pEnumerator->getDefaultSampleRate();
    if (defaultSampleRate.isValid()) {
        return defaultSampleRate;
    }

    return SoundManagerConfig::kMixxxDefaultSampleRate;
}
