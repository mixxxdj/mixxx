#pragma once

#include <QSharedPointer>
#include <vector>

#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"

class SoundDevicePortAudio;
class SoundManager;

/// The USB sound cards that Mixxx drives directly on Android (see
/// SoundDeviceAndroidUsb), next to the devices of PortAudio.
class AndroidUsbSoundDevices {
  public:
    /// Rescans the USB sound cards. Returns true if the set changed; the
    /// devices that disappeared are added to pRemoved. The Android devices
    /// of the cards driven directly are removed from pPortAudioDevices.
    bool refresh(const UserSettingsPointer& pConfig,
            SoundManager* pSoundManager,
            std::vector<QSharedPointer<SoundDevicePortAudio>>* pPortAudioDevices,
            std::vector<SoundDevicePointer>* pRemoved,
            bool waitForPermission);

    const std::vector<SoundDevicePointer>& devices() const {
        return m_devices;
    }
    void clear() {
        m_devices.clear();
    }

  private:
    std::vector<SoundDevicePointer> m_devices;
};
