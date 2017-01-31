#ifndef SOUNDDEVICENOTFOUND_H
#define SOUNDDEVICENOTFOUND_H

#include <QString>

#include "soundio/sounddevice.h"


class SoundManager;
class EngineNetworkStream;

// This is a fake device, constructed from SoundMamagerConfig data.
// It is used for error reporting only when there is no real data from the
// sound API

class SoundDeviceNotFound : public SoundDevice {
  public:
    SoundDeviceNotFound(QString internalName)
            : SoundDevice(UserSettingsPointer(), nullptr) {
        m_strInternalName = internalName;
        m_strDisplayName = internalName;
    }

    SoundDeviceError open(bool isClkRefDevice, int syncBuffers) override {
        Q_UNUSED(isClkRefDevice);
        Q_UNUSED(syncBuffers);
        return SOUNDDEVICE_ERROR_ERR;
    };
    bool isOpen() const  override { return false; };
    SoundDeviceError close() override {
        return SOUNDDEVICE_ERROR_ERR;
    };
    void readProcess() override { };
    void writeProcess() override { };
    QString getError() const override{ return QObject::tr("Device not found"); };

    unsigned int getDefaultSampleRate() const override {
        return 44100;
    }
};

#endif // SOUNDDEVICENOTFOUND_H
