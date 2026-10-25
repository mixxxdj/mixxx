#include "soundio/androidusbsounddevices.h"

#include <algorithm>

#include "soundio/androidusbaudiodevice.h"
#include "soundio/sounddeviceandroidusb.h"
#include "soundio/sounddeviceportaudio.h"
#include "soundio/soundmanagerandroid.h"

bool AndroidUsbSoundDevices::refresh(const UserSettingsPointer& pConfig,
        SoundManager* pSoundManager,
        std::vector<QSharedPointer<SoundDevicePortAudio>>* pPortAudioDevices,
        std::vector<SoundDevicePointer>* pRemoved,
        bool waitForPermission) {
    using mixxx::androidusb::AndroidUsbAudioDevice;
    using mixxx::androidusb::DirectMode;
    // Some Android devices (e.g. with Qualcomm USB audio offload) fail to play
    // to multichannel USB sound cards. Mixxx drives those directly instead,
    // and hides the Android device of the same sound card.
    const auto mode = static_cast<DirectMode>(pConfig->getValue(
            SoundManagerAndroid::kUsbDirectModeKey, static_cast<int>(DirectMode::Auto)));
    const auto usbDevices = AndroidUsbAudioDevice::scan(mode, waitForPermission);

    bool changed = false;
    std::vector<SoundDevicePointer> devices;
    for (const auto& pUsbDevice : usbDevices) {
        auto existing = std::find_if(m_devices.begin(),
                m_devices.end(),
                [&pUsbDevice](const SoundDevicePointer& pDevice) {
                    return pDevice.staticCast<SoundDeviceAndroidUsb>()->usbDevice() ==
                            pUsbDevice;
                });
        if (existing != m_devices.end()) {
            devices.push_back(*existing);
            continue;
        }
        changed = true;
        const QString productName = pUsbDevice->productName();
        pPortAudioDevices->erase(std::remove_if(pPortAudioDevices->begin(),
                                         pPortAudioDevices->end(),
                                         [&productName](const auto& pDevice) {
                                             const QString& name = pDevice->getDisplayName();
                                             return name.endsWith(QStringLiteral(": USB")) &&
                                                     name.contains(productName);
                                         }),
                pPortAudioDevices->end());
        devices.push_back(SoundDevicePointer(
                new SoundDeviceAndroidUsb(pConfig, pSoundManager, pUsbDevice)));
    }
    for (const auto& pDevice : m_devices) {
        if (std::find(devices.begin(), devices.end(), pDevice) == devices.end()) {
            changed = true;
            if (pRemoved) {
                pRemoved->push_back(pDevice);
            }
        }
    }
    m_devices = std::move(devices);
    return changed;
}
