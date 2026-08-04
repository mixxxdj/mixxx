#pragma once

#include <QString>

#include "soundio/sounddevice.h"
#include "soundio/soundmanagerconfig.h"

class SoundManager;

// A lightweight mock device for UI testing. Does not perform any audio I/O.
// Constructed with predictable API name, display name, and channel counts
// so that the QML sound settings router can discover and display it.

class SoundDeviceMock : public SoundDevice {
  public:
    SoundDeviceMock(UserSettingsPointer config,
            SoundManager* sm,
            const QString& api,
            const QString& displayName,
            int outputChannels,
            int inputChannels)
            : SoundDevice(config, sm) {
        m_hostAPI = api;
        m_strDisplayName = displayName;
        m_deviceId.name = displayName;
        m_numOutputChannels =
                mixxx::audio::ChannelCount::fromInt(outputChannels);
        m_numInputChannels =
                mixxx::audio::ChannelCount::fromInt(inputChannels);
    }

    SoundDeviceStatus open(
            bool isClkRefDevice, int syncBuffers) override {
        Q_UNUSED(isClkRefDevice);
        Q_UNUSED(syncBuffers);
        return SoundDeviceStatus::Ok;
    };
    bool isOpen() const override {
        return false;
    };
    SoundDeviceStatus close() override {
        return SoundDeviceStatus::Ok;
    };
    void readProcess(SINT /*framesPerBuffer*/) override{};
    void writeProcess(SINT /*framesPerBuffer*/) override{};
    QString getError() const override {
        return {};
    };

    mixxx::audio::SampleRate getDefaultSampleRate() const override {
        return m_sampleRate;
    }
};
