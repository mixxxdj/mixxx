#include "sounddevicepipewire.h"

#include <spa/utils/defs.h>

#include <QRegularExpression>
#include <string_view>

#include "audio/types.h"
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
    m_hostAPI = SoundManagerConfig::kAPIPipewire;
    m_deviceId.name = name.data();
    m_deviceId.alsaHwDevice = name.data();
    m_deviceId.deviceIndex = id;
    m_strDisplayName = QString::fromUtf8(name);
    m_numInputChannels = mixxx::audio::ChannelCount(0);
    m_numOutputChannels = mixxx::audio::ChannelCount(0);
    m_sampleRate = getDefaultSampleRate();
}

SoundDevicePipewire::~SoundDevicePipewire() {
    if (isOpen()) {
        close();
    }
}

SoundDeviceStatus SoundDevicePipewire::open(bool, int) {
    std::string error;
    for (auto& input : m_audioInputs) {
        error += m_pEnumerator->openDeviceInput(m_deviceId.deviceIndex, input);
    }

    for (auto& output : m_audioOutputs) {
        error += m_pEnumerator->openDeviceOutput(m_deviceId.deviceIndex, output);
    }

    if (error.empty()) {
        return SoundDeviceStatus::Ok;
    }

    m_error = error;
    return SoundDeviceStatus::Error;
}

bool SoundDevicePipewire::isOpen() const {
    return m_pEnumerator->isOpen(m_deviceId.deviceIndex);
}

SoundDeviceStatus SoundDevicePipewire::close() {
    m_pEnumerator->closeDevices();
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

mixxx::audio::SampleRate SoundDevicePipewire::getDefaultSampleRate() const {
    auto defaultSampleRate = m_pEnumerator->getDefaultSampleRate();
    if (defaultSampleRate.isValid()) {
        return defaultSampleRate;
    }

    return SoundManagerConfig::kMixxxDefaultSampleRate;
}

QString SoundDevicePipewire::getChannelString(ChannelGroup channelGroup, bool input) const {
    return m_pEnumerator->getChannelString(m_deviceId.deviceIndex, channelGroup, input);
}
