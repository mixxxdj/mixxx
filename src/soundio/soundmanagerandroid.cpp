#include "soundio/soundmanagerandroid.h"

#include <QDebug>
#include <QTimer>

#include "control/controlobject.h"
#include "engine/enginemixer.h"
#include "moc_soundmanagerandroid.cpp"
#include "preferences/configobject.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddeviceportaudio.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerconfig.h"
#include "soundio/soundmanagerutil.h"

const ConfigKey SoundManagerAndroid::kUsbDirectModeKey(
        QStringLiteral("[SoundManager]"), QStringLiteral("usb_direct_mode"));
const ConfigKey SoundManagerAndroid::kAutoRouteControllerKey(
        QStringLiteral("[SoundManager]"), QStringLiteral("auto_route_controller"));

SoundManagerAndroid::SoundManagerAndroid(SoundManager* pSoundManager)
        : m_pSoundManager(pSoundManager) {
    // USB hotplug listener: the Android USB permission flow calls
    // mixxx::android::usbDeviceAccessResult, which sets
    // s_usbPermissionGranted. We poll that flag and rescan when it changes.
    m_pUsbTimer = new QTimer(this);
    m_pUsbTimer->setInterval(2000);
    connect(m_pUsbTimer, &QTimer::timeout, this, &SoundManagerAndroid::slotUsbDevicesChanged);
    m_pUsbTimer->start();

    // Listen for USB permission grants to trigger an immediate rescan
    m_usbListenerSeen = mixxx::s_usbPermissionGranted.load();
}

SoundManagerAndroid::~SoundManagerAndroid() {
    if (m_pUsbTimer) {
        m_pUsbTimer->stop();
    }
}

void SoundManagerAndroid::prepareSetup() {
    // Called at the start of setupDevices, before any devices are opened.
    // Nothing to do here for now, but this is the hook for future
    // pre-setup Android-specific initialization.
}

bool SoundManagerAndroid::skipFailedDevice(
        const SoundDevicePointer& pDevice, SoundDevicePointer* pMainClockRef) {
    Q_UNUSED(pMainClockRef);
    // If the failed device is a network device, we can continue without it
    if (pDevice->getDeviceId().name == kNetworkDeviceInternalName) {
        qWarning() << "Network device failed to open, continuing without it";
        return true;
    }
    // For other devices, don't skip by default - let the caller handle it
    return false;
}

void SoundManagerAndroid::finishSetup(const std::vector<SoundDevicePointer>& devices,
        SoundDevicePointer* pMainClockRef,
        int* pOutputDevicesOpened,
        QSet<SoundDeviceId>* pDevicesNotFound) {
    Q_UNUSED(devices);
    Q_UNUSED(pMainClockRef);
    Q_UNUSED(pOutputDevicesOpened);
    Q_UNUSED(pDevicesNotFound);

    // Auto-route to DJ controller if enabled
    if (m_pSoundManager->userSettings()->getValue(kAutoRouteControllerKey, true)) {
        autoRouteToController();
    }

    // Open fallback output if no physical output was opened
    if (*pOutputDevicesOpened == 0) {
        openFallbackOutput(devices, pMainClockRef, pOutputDevicesOpened, pDevicesNotFound);
    }
}

void SoundManagerAndroid::slotUsbDevicesChanged() {
    // Check if USB permission was granted recently
    if (mixxx::s_usbPermissionGranted.load()) {
        mixxx::s_usbPermissionGranted = false;
        // Trigger a device rescan through SoundManager
        emit m_pSoundManager->configInvalidated();
    }
}

void SoundManagerAndroid::autoRouteToController() {
    // TODO: Implement automatic routing to DJ controllers
    // This would scan for USB sound cards with >= 4 output channels
    // and configure Main on channels 1-2, Headphones on channels 3-4
}

void SoundManagerAndroid::openFallbackOutput(const std::vector<SoundDevicePointer>& devices,
        SoundDevicePointer* pMainClockRef,
        int* pOutputDevicesOpened,
        QSet<SoundDeviceId>* pDevicesNotFound) {
    Q_UNUSED(devices);
    Q_UNUSED(pMainClockRef);
    Q_UNUSED(pOutputDevicesOpened);
    Q_UNUSED(pDevicesNotFound);

    // TODO: Implement fallback output (built-in speaker) opening
    // This ensures Mixxx never ends up without sound
}
