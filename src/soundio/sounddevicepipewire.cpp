#include "sounddevicepipewire.h"

#include <spa/utils/defs.h>

#include "soundio/pipewireenumerator.h"
#include "soundio/sounddevice.h"
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
}

SoundDeviceStatus SoundDevicePipewire::open(bool isClkRefDevice, int syncBuffers) {
    std::set<uint8_t> inChans;
    for (auto& input : m_audioInputs) {
        ChannelGroup channelGroup = input.getChannelGroup();
        uint8_t highChannel = channelGroup.getChannelBase() + channelGroup.getChannelCount();
        for (uint8_t i = channelGroup.getChannelBase(); i < highChannel; i++) {
            inChans.insert(i);
            qWarning() << "opening input idx:id" << i;
        }
    }

    std::set<uint8_t> outChans;
    for (auto& output : m_audioOutputs) {
        ChannelGroup channelGroup = output.getChannelGroup();
        uint8_t highChannel = channelGroup.getChannelBase() + channelGroup.getChannelCount();
        for (uint8_t i = channelGroup.getChannelBase(); i < highChannel; i++) {
            outChans.insert(i);
            qWarning() << "opening output idx:id" << i;
        }
    }

    m_pEnumerator->openDevice(m_deviceId.deviceIndex, inChans, outChans);
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
    for (auto i = m_audioOutputs.constBegin(), e = m_audioOutputs.constEnd(); i != e; ++i) {
        const AudioOutputBuffer& in = *i;
        ChannelGroup chanGroup = in.getChannelGroup();
        const int iChannelCount = chanGroup.getChannelCount();
        const int iChannelBase = chanGroup.getChannelBase();
        const int iChannelEnd = iChannelCount + iChannelBase;

        if (channel < iChannelBase || channel > iChannelEnd) {
            continue;
        }

        const CSAMPLE* pOutputBuffer = &in.getBuffer()[offset];

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
    for (auto i = m_audioInputs.constBegin(), e = m_audioInputs.constEnd(); i != e; ++i) {
        const AudioInputBuffer& in = *i;
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

void SoundDevicePipewire::unregisterDevicePort(uint32_t id, spa_direction direction) {
    switch (direction) {
    case SPA_DIRECTION_INPUT:
        m_inPorts.erase(m_inPorts.begin() + id);
        m_numInputChannels = mixxx::audio::ChannelCount::fromInt(m_outPorts.size());
        break;
    case SPA_DIRECTION_OUTPUT:
        m_outPorts.erase(m_outPorts.begin() + id);
        m_numOutputChannels = mixxx::audio::ChannelCount::fromInt(m_inPorts.size());
        break;
    }
}

AudioPath* SoundDevicePipewire::getAudioPath(uint32_t channel, spa_direction direction) {
    if (direction == SPA_DIRECTION_INPUT) {
        for (auto& input : m_audioInputs) {
            const auto& channelGroup = input.getChannelGroup();
            const uint32_t channelBase = channelGroup.getChannelBase();
            uint32_t channelEnd = channelBase + channelGroup.getChannelCount();
            if (channelBase <= channel && channel < channelEnd) {
                return &input;
            }
        }
    } else {
        for (auto& output : m_audioOutputs) {
            const auto& channelGroup = output.getChannelGroup();
            const uint32_t channelBase = channelGroup.getChannelBase();
            uint32_t channelEnd = channelBase + channelGroup.getChannelCount();
            if (channelBase <= channel && channel < channelEnd) {
                return &output;
            }
        }
    }

    qWarning() << "returning null AudioPath, inputs:" << m_audioInputs.size()
               << "outputs:" << m_audioOutputs.size();
    return nullptr;
}
