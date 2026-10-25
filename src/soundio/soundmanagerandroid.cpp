#include "soundio/soundmanagerandroid.h"

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniObject>
#endif
#include <QTimer>

#include "control/controlobject.h"
#include "moc_soundmanagerandroid.cpp"
#include "soundio/portaudioenumerator.h"
#include "soundio/soundmanager.h"

const ConfigKey SoundManagerAndroid::kUsbDirectModeKey =
        ConfigKey(QStringLiteral("[Soundcard]"), QStringLiteral("AndroidUsbDirectMode"));
const ConfigKey SoundManagerAndroid::kAutoRouteControllerKey =
        ConfigKey(QStringLiteral("[Soundcard]"), QStringLiteral("AndroidAutoRouteController"));

namespace {

bool isConfiguredOutput(const SoundManagerConfig& config, const SoundDevicePointer& pDevice) {
    return config.getOutputs().contains(pDevice->getDeviceId());
}

} // namespace

SoundManagerAndroid::SoundManagerAndroid(SoundManager* pSoundManager)
        : QObject(pSoundManager),
          m_pSoundManager(pSoundManager) {
    SoundManagerConfig& config = m_pSoundManager->m_config;
    if (config.getOutputs().isEmpty()) {
        config.loadDefaults(m_pSoundManager,
                SoundManagerConfig::API | SoundManagerConfig::DEVICES |
                        SoundManagerConfig::OTHER);
        config.writeToDisk();
    }

// NOTE(djsugar): USB hotplug listener not ported yet (needs
    // mixxx::android::addDevicesChangedListener + PortAudioEnumerator::
    // refreshAndroidUsbDevices from Javi's tree). Auto-route + fallback
    // below work without it.
}

SoundManagerAndroid::~SoundManagerAndroid() {
// NOTE(djsugar): see ctor note - nothing to unregister yet.
}

void SoundManagerAndroid::slotUsbDevicesChanged() {
    // NOTE(djsugar): hotplug reconfigure needs Javi's USB stack (not ported).
    qInfo() << "SoundManagerAndroid: USB devices changed (hotplug NYI)";
}

void SoundManagerAndroid::autoRouteToController() {
    if (!m_pSoundManager->m_pConfig->getValue(kAutoRouteControllerKey, true)) {
        return;
    }
    SoundManagerConfig& config = m_pSoundManager->m_config;
    // The Android enumerator suffixes USB devices with ": USB"
    SoundDevicePointer pController;
    for (const auto& pDevice : m_pSoundManager->m_pEnumerator->queryDevices()) {
        if (pDevice->getNumOutputChannels() >= 4 &&
                pDevice->getDisplayName().endsWith(QStringLiteral(": USB")) &&
                (!pController ||
                        pDevice->getNumOutputChannels() > pController->getNumOutputChannels())) {
            pController = pDevice;
        }
    }
    if (!pController || isConfiguredOutput(config, pController)) {
        return;
    }

    qInfo() << "DJ controller sound card detected, routing Main to channels 1-2 "
               "and Headphones to channels 3-4 of"
            << pController->getDisplayName();
    config.clearOutputs();
    config.addOutput(pController->getDeviceId(),
            AudioOutput(AudioPathType::Main, 0, mixxx::audio::ChannelCount::stereo(), 0));
    config.addOutput(pController->getDeviceId(),
            AudioOutput(AudioPathType::Headphones, 2, mixxx::audio::ChannelCount::stereo(), 0));
    // Play at the controller's own rate to avoid resampling
    const auto sampleRate = pController->getDefaultSampleRate();
    if (sampleRate.isValid()) {
        config.setSampleRate(sampleRate);
    }
    // The cable splitter preset is not used with a controller
    ControlObject::set(ConfigKey(QStringLiteral("[Master]"), QStringLiteral("headSplit")), 0.0);
    config.writeToDisk();
}

void SoundManagerAndroid::prepareSetup() {
    SoundManagerConfig& config = m_pSoundManager->m_config;
    if (config.getOutputs().isEmpty()) {
        qWarning() << "SoundManager on Android: No outputs configured in "
                      "SoundManagerConfig. Loading defaults...";
        config.loadDefaults(m_pSoundManager,
                SoundManagerConfig::API | SoundManagerConfig::DEVICES |
                        SoundManagerConfig::OTHER);
        config.writeToDisk();
    }
    autoRouteToController();
// NOTE(djsugar): direct-USB sample-rate clamp needs Javi's
    // SoundDeviceAndroidUsb (not ported, see class doc).
}

bool SoundManagerAndroid::skipFailedDevice(
        const SoundDevicePointer& pDevice, SoundDevicePointer* pMainClockRef) {
    // Never end up without any sound: skip the device and let
    // openFallbackOutput() open another one
    qWarning() << "Unable to open" << pDevice->getDisplayName() << ":" << pDevice->getError();
    if (*pMainClockRef == pDevice) {
        pMainClockRef->clear();
    }
    return true;
}

void SoundManagerAndroid::finishSetup(const std::vector<SoundDevicePointer>& devices,
        SoundDevicePointer* pMainClockRef,
        int* pOutputDevicesOpened,
        QSet<SoundDeviceId>* pDevicesNotFound) {
    openFallbackOutput(devices, pMainClockRef, pOutputDevicesOpened, pDevicesNotFound);

    // Android attenuates USB sound cards with the media volume. DJ
    // controllers have their own level knobs, so play at full scale.
    const SoundManagerConfig& config = m_pSoundManager->m_config;
    for (const auto& pDevice : devices) {
        if (isConfiguredOutput(config, pDevice) &&
                pDevice->getDisplayName().endsWith(QStringLiteral(": USB"))) {
#ifdef Q_OS_ANDROID
            QNativeInterface::QAndroidApplication::runOnAndroidMainThread([]() {
                QJniObject context = QNativeInterface::QAndroidApplication::context();
                QJniObject::callStaticMethod<jboolean>("org/mixxx/AudioHelper",
                        "maximizeMediaVolumeForUsb",
                        "(Landroid/content/Context;)Z",
                        context.object<jobject>());
            });
#else
            // NOTE(djsugar): AudioHelper.maximizeMediaVolumeForUsb needs to be
            // added to MainActivity's Java side; no-op on desktop.
#endif
            break;
        }
    }
}

void SoundManagerAndroid::openFallbackOutput(const std::vector<SoundDevicePointer>& devices,
        SoundDevicePointer* pMainClockRef,
        int* pOutputDevicesOpened,
        QSet<SoundDeviceId>* pDevicesNotFound) {
    const SoundManagerConfig& config = m_pSoundManager->m_config;
    for (const auto& pDevice : devices) {
        if (pDevice && pDevice->isOpen() &&
                pDevice->getDeviceId().name != kNetworkDeviceInternalName &&
                isConfiguredOutput(config, pDevice)) {
            // A physical output is playing
            return;
        }
    }

    qWarning() << "SoundManager on Android: No physical output device opened! "
                  "Forcing first available output device...";
    SoundDevicePointer targetDevice;
    for (const auto& pDevice : devices) {
        if (pDevice && pDevice->getNumOutputChannels() > 0 &&
                pDevice->getDeviceId().name != kNetworkDeviceInternalName) {
            if (pDevice->getDisplayName().contains(QStringLiteral("earpiece"), Qt::CaseInsensitive)) {
                continue;
            }
            if (pDevice->getHostAPI().contains(QStringLiteral("Oboe"), Qt::CaseInsensitive)) {
                // Prefer the built-in speaker, which is always present
                if (pDevice->getDisplayName().contains(QStringLiteral("Speaker"))) {
                    targetDevice = pDevice;
                    break;
                }
                if (!targetDevice || !targetDevice->getHostAPI().contains(QStringLiteral("Oboe"), Qt::CaseInsensitive)) {
                    targetDevice = pDevice;
                }
                continue;
            }
            if (!targetDevice) {
                targetDevice = pDevice;
            }
        }
    }
    if (!targetDevice) {
        return;
    }

    const auto channelCount = targetDevice->getNumOutputChannels() >= 2
            ? mixxx::audio::ChannelCount::stereo()
            : mixxx::audio::ChannelCount::mono();
    AudioOutput forcedOut(AudioPathType::Main, 0, channelCount, 0);
    auto pSource = m_pSoundManager->m_registeredSources.value(forcedOut);
    const CSAMPLE* pBuffer = pSource ? pSource->buffer(forcedOut).data() : nullptr;
    if (!pBuffer) {
        return;
    }
    targetDevice->clearOutputs();
    AudioOutputBuffer aob(forcedOut, pBuffer);
    targetDevice->addOutput(aob);
    targetDevice->setSampleRate(config.getSampleRate());
    targetDevice->setConfigFramesPerBuffer(config.getFramesPerBuffer());
    if (targetDevice->open(true, config.getSyncBuffers()) == SoundDeviceStatus::Ok) {
        *pMainClockRef = targetDevice;
        ++*pOutputDevicesOpened;
        pDevicesNotFound->remove(targetDevice->getDeviceId());
        // Not saved to the configuration on purpose: the configured device
        // (e.g. a DJ controller that isn't plugged in yet) must be used
        // again next time.
        qInfo() << "Successfully opened fallback Android audio device:"
                << targetDevice->getDisplayName();
    }
}
