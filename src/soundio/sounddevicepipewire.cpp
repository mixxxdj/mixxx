#include "sounddevicepipewire.h"

#include <spa/utils/defs.h>

#include "soundio/pipewireenumerator.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddevicestatus.h"
#include "soundio/soundmanagerconfig.h"
#include "soundio/soundmanagerutil.h"
#include "util/sample.h"

SoundDevicePipewire::SoundDevicePipewire(UserSettingsPointer pConfig,
        SoundManager* pManager,
        PipewireEnumerator* pEnumerator,
        uint32_t id,
        const std::string_view name)
        : SoundDevice(pConfig, pManager),
          m_pEnumerator(pEnumerator) {
    m_hostAPI = QStringLiteral("PipeWire");
    m_deviceId.name = name.data();
    m_deviceId.deviceIndex = id;
    m_strDisplayName = QString::fromUtf8(name);
    m_numInputChannels = mixxx::audio::ChannelCount(0);
    m_numOutputChannels = mixxx::audio::ChannelCount(0);
    m_sampleRate = getDefaultSampleRate();
}

SoundDeviceStatus SoundDevicePipewire::open(bool, int) {
    m_error = m_pEnumerator->openDevice(*this, m_sampleRate, m_configFramesPerBuffer);
    if (m_error.empty()) {
        return SoundDeviceStatus::Ok;
    } else {
        return SoundDeviceStatus::Error;
    }
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

void SoundDevicePipewire::registerPort(uint32_t id, const struct spa_dict* props) {
    const char* nameStr = spa_dict_lookup(props, PW_KEY_PORT_ALIAS);
    const char* direction = spa_dict_lookup(props, PW_KEY_PORT_DIRECTION);
    std::string name;

    if (!nameStr) {
        nameStr = spa_dict_lookup(props, PW_KEY_PORT_NAME);
    }

    if (nameStr) {
        name = nameStr;
    } else {
        name = direction;
        name += ":";
        name += spa_dict_lookup(props, PW_KEY_PORT_ID);
    }

    // m_numInputChannels, m_numOutputChannels, m_audioInputs, m_audioOutputs
    // are with respect to Mixxx and not the SoundDevice
    if (strcmp(direction, "in") == 0) {
        m_inPorts.emplace_back(id, name);
        m_numOutputChannels = mixxx::audio::ChannelCount::fromInt(m_inPorts.size());
    } else if (strcmp(direction, "out") == 0) {
        m_outPorts.emplace_back(id, name);
        m_numInputChannels = mixxx::audio::ChannelCount::fromInt(m_outPorts.size());
    }
}

void SoundDevicePipewire::unregisterPort(uint32_t id) {
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

AudioPath* SoundDevicePipewire::getInputAudioPath(uint32_t id) {
    auto it = std::ranges::find(m_inPorts, id, &Port::id);

    if (it != m_inPorts.end()) {
        auto channel = it - m_inPorts.begin();
        for (auto& output : m_audioOutputs) {
            const auto& channelGroup = output.getChannelGroup();
            const uint32_t channelBase = channelGroup.getChannelBase();
            uint32_t channelEnd = channelBase + channelGroup.getChannelCount();

            if (channelBase <= channel && channel < channelEnd) {
                return &output;
            }
        }
    }

    // its fine to return nullptr here, this can be called after
    // this device has closed, in that case we ignore the nullptr
    return nullptr;
}

AudioPath* SoundDevicePipewire::getOutputAudioPath(uint32_t id) {
    auto it = std::ranges::find(m_outPorts, id, &Port::id);
    if (it != m_outPorts.end()) {
        auto channel = it - m_outPorts.begin();
        for (auto& input : m_audioInputs) {
            const auto& channelGroup = input.getChannelGroup();
            const uint32_t channelBase = channelGroup.getChannelBase();
            uint32_t channelEnd = channelBase + channelGroup.getChannelCount();
            if (channelBase <= channel && channel < channelEnd) {
                return &input;
            }
        }
    }

    // its fine to return nullptr here, this can be called after
    // this device has closed, in that case we ignore the nullptr
    return nullptr;
}

mixxx::audio::SampleRate SoundDevicePipewire::getDefaultSampleRate() const {
    auto defaultSampleRate = m_pEnumerator->getDefaultSampleRate();
    if (defaultSampleRate.isValid()) {
        return defaultSampleRate;
    }

    return SoundManagerConfig::kMixxxDefaultSampleRate;
}

std::span<const SoundDevicePipewire::Port> SoundDevicePipewire::getPortsByPath(
        const AudioPath& path) const {
    ChannelGroup channelGroup = path.getChannelGroup();
    unsigned char channelBase = channelGroup.getChannelBase();
    unsigned char channelCount = channelGroup.getChannelCount().value();

    switch (path.getType()) {
    case AudioPath::AudioPathType::Main:
    case AudioPath::AudioPathType::Headphones:
    case AudioPath::AudioPathType::Booth:
    case AudioPath::AudioPathType::Bus:
    case AudioPath::AudioPathType::Deck:
        return std::span{m_inPorts}.subspan(channelBase, channelCount);
    case AudioPath::AudioPathType::VinylControl:
    case AudioPath::AudioPathType::Microphone:
    case AudioPath::AudioPathType::Auxiliary:
    case AudioPath::AudioPathType::RecordBroadcast:
        return std::span{m_outPorts}.subspan(channelBase, channelCount);
    default:
        return std::span<SoundDevicePipewire::Port>{};
    }
}

void SoundDevicePipewire::registerLink(uint32_t id, spa_direction direction) {
    if (direction == SPA_DIRECTION_INPUT) {
        m_inLinks.push_back(id);
    } else {
        m_outLinks.push_back(id);
    }
}

void SoundDevicePipewire::unregisterLink(uint32_t id, spa_direction direction) {
    if (direction == SPA_DIRECTION_INPUT) {
        auto it = std::ranges::find(m_inLinks, id);
        if (it != m_inLinks.end()) {
            m_inLinks.erase(it);
        }
    } else {
        auto it = std::ranges::find(m_outLinks, id);
        if (it != m_outLinks.end()) {
            m_outLinks.erase(it);
        }
    }
}
