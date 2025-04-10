#pragma once

#include <QString>

#include "soundio/sounddevice.h"
#include "soundio/soundmanagerconfig.h"

class SoundManager;
class EngineNetworkStream;

// This is a fake device, constructed from SoundMamagerConfig data.
// It is used for error reporting only when there is no real data from the
// sound API

class SoundDeviceNotFound : public SoundDevice {
  public:
    SoundDeviceNotFound(const QString& name)
            : SoundDevice(UserSettingsPointer(), nullptr) {
        m_deviceId.name = name;
        m_strDisplayName = name;
    }

    SoundDeviceStatus open(bool isClkRefDevice, int syncBuffers) override {
        Q_UNUSED(isClkRefDevice);
        Q_UNUSED(syncBuffers);
        return SoundDeviceStatus::Error;
    };
    bool isOpen() const  override { return false; };
    SoundDeviceStatus close() override {
        return SoundDeviceStatus::Error;
    };
    void readProcess(SINT /*framesPerbuffer*/) override{};
    void writeProcess(SINT /*framesPerbuffer*/) override{};
    QString getError() const override {
        return QObject::tr("Device not found");
    };

    mixxx::audio::SampleRate getDefaultSampleRate() const override {
        return SoundManagerConfig::kMixxxDefaultSampleRate;
    }
};
