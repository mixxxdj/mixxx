#include "soundio/audiodeviceassistant.h"

#include <QApplication>
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <memory>

#include "coreservices.h"
#include "controllers/controllermanager.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerconfig.h"
#include "util/systemdevices.h"

namespace mixxx {

namespace {

constexpr int kPollIntervalMillis = 2000;
/// A controller without a sound card cannot be used for the output, and the name
/// of a controller is matched against the names of the output devices, so short
/// names are ignored to avoid matching unrelated devices.
constexpr int kMinControllerNameLength = 5;

const ConfigKey kControllerAudioDeviceKey("[Sound]", "controller_audio_device");
const QString kPreferenceMixer = QStringLiteral("mixer");
const QString kPreferenceComputer = QStringLiteral("computer");

/// Only letters and digits, lower case, so that e.g. "Altavoces (MixTrack Pro
/// II)" and "MixTrack Pro II" are recognized as referring to the same device.
QString normalizedName(const QString& name) {
    QString normalized;
    normalized.reserve(name.size());
    for (const QChar& character : name) {
        if (character.isLetterOrNumber()) {
            normalized.append(character.toLower());
        }
    }
    return normalized;
}

bool namesReferToSameDevice(const QString& name1, const QString& name2) {
    const QString normalized1 = normalizedName(name1);
    const QString normalized2 = normalizedName(name2);
    if (normalized1.size() < kMinControllerNameLength ||
            normalized2.size() < kMinControllerNameLength) {
        return false;
    }
    return normalized1.contains(normalized2) || normalized2.contains(normalized1);
}

bool containsOutputDeviceNamed(
        const QList<SoundDevicePointer>& outputDevices, const QString& name) {
    for (const auto& pDevice : outputDevices) {
        if (pDevice->getDisplayName() == name) {
            return true;
        }
    }
    return false;
}

} // namespace

AudioDeviceAssistant::AudioDeviceAssistant(
        QWidget* pParentWidget, CoreServices* pCoreServices)
        : m_pParentWidget(pParentWidget),
          m_pCoreServices(pCoreServices) {
    m_lastSystemDevices = systemConnectedDeviceNames();
    qWarning() << "AudioDeviceAssistant: watching devices" << m_lastSystemDevices;

    m_pollTimer.setInterval(kPollIntervalMillis);
    connect(&m_pollTimer, &QTimer::timeout, this, &AudioDeviceAssistant::slotPollDevices);
    m_pollTimer.start();
}

AudioDeviceAssistant::~AudioDeviceAssistant() {
    m_pollTimer.stop();
}

void AudioDeviceAssistant::slotPollDevices() {
    const QStringList systemDevices = systemConnectedDeviceNames();
    if (systemDevices == m_lastSystemDevices) {
        return;
    }
    // Do not interrupt the user while a dialog is open, the change is then
    // noticed on the next poll.
    if (QApplication::activeModalWidget() != nullptr) {
        return;
    }

    qDebug() << "AudioDeviceAssistant: the devices of the system changed to"
             << systemDevices;
    m_lastSystemDevices = systemDevices;
    checkForDevicesNow();
}

void AudioDeviceAssistant::checkForDevicesNow(bool askUser) {
    qWarning() << "AudioDeviceAssistant: checking for devices now";
    if (m_updating) {
        return;
    }
    m_updating = true;
    const bool previousAskUser = m_askUser;
    m_askUser = askUser;
    updateDevices();
    m_askUser = previousAskUser;
    m_updating = false;
}
void AudioDeviceAssistant::updateDevices() {
    const std::shared_ptr<SoundManager> pSoundManager = m_pCoreServices->getSoundManager();
    // The device that is really playing, which is not necessarily the one in the
    // configuration: it keeps devices that are not connected so that they can be
    // used again, and a default device may have been substituted for them.
    const QString previousDeviceName = openedOutputDeviceName(pSoundManager);

    // The backend caches the device list, so it has to be restarted to see
    // devices that were connected while Mixxx was running.
    if (m_askUser) {
        // Looking for the devices takes a moment and blocks the user interface,
        // so show that Mixxx is busy. Done before the dialogs below so that they
        // do not get the wait cursor.
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }
    pSoundManager->reinitializeDevices();

    // A controller that has just been connected also has to be taken into use,
    // otherwise its mapping would only be loaded when Mixxx is started again.
    // Skip this while a dialog is open: the list of controllers must not change
    // under the preferences dialog.
    if (QApplication::activeModalWidget() == nullptr) {
        m_pCoreServices->getControllerManager()->setUpDevices();
    }
    if (m_askUser) {
        QApplication::restoreOverrideCursor();
    }

    const QList<SoundDevicePointer> outputDevices = listOutputDevices();
    const SoundDevicePointer pControllerDevice = findControllerSoundCard(outputDevices);
    const SoundDevicePointer pComputerDevice = findSystemDefaultDevice(outputDevices);

    SoundDevicePointer pTargetDevice;
    if (pControllerDevice) {
        if (m_askUser) {
            // The user asked for the assistant, so ask again instead of using
            // the remembered choice or the device that is already in use.
            pTargetDevice = askUserForDevice(pControllerDevice, pComputerDevice);
        } else if (previousDeviceName == pControllerDevice->getDisplayName()) {
            // It is already in use, there is nothing to ask about.
            pTargetDevice = pControllerDevice;
        } else if (loadPreference() == kPreferenceComputer) {
            pTargetDevice = pComputerDevice;
        } else if (loadPreference() == kPreferenceMixer) {
            pTargetDevice = pControllerDevice;
        } else {
            pTargetDevice = askUserForDevice(pControllerDevice, pComputerDevice);
        }
    } else {
        // No controller with a sound card. Play through the audio of the
        // computer, telling the user when the device that was in use is gone.
        pTargetDevice = pComputerDevice;
        if (!previousDeviceName.isEmpty() && pComputerDevice &&
                previousDeviceName != pComputerDevice->getDisplayName() &&
                !containsOutputDeviceNamed(outputDevices, previousDeviceName)) {
            informAboutDeviceChange(previousDeviceName, pComputerDevice->getDisplayName());
        } else if (m_askUser) {
            // Nothing changed, but the user asked what is going on.
            informAboutNoController(pComputerDevice);
        }
    }

    useDevice(pTargetDevice);
}

QString AudioDeviceAssistant::openedOutputDeviceName(
        const std::shared_ptr<SoundManager>& pSoundManager) const {
    for (const QString& api : pSoundManager->getHostAPIList()) {
        for (const auto& pDevice : pSoundManager->getDeviceList(api, true, false)) {
            if (pDevice->isOpen()) {
                return pDevice->getDisplayName();
            }
        }
    }
    return QString();
}

QList<SoundDevicePointer> AudioDeviceAssistant::listOutputDevices() const {
    const std::shared_ptr<SoundManager> pSoundManager = m_pCoreServices->getSoundManager();

    // The same sound card is listed by several APIs, e.g. "MME" and "Windows
    // WASAPI" on Windows. The API that is configured is looked at first, so that
    // a device that was set up in the preferences keeps its API instead of being
    // switched to another one behind the back of the user.
    QStringList apis = pSoundManager->getHostAPIList();
    const QString configuredApi = pSoundManager->getConfig().getAPI();
    if (apis.removeAll(configuredApi) > 0) {
        apis.prepend(configuredApi);
    }

    QList<SoundDevicePointer> outputDevices;
    for (const QString& api : std::as_const(apis)) {
        for (const auto& pDevice : pSoundManager->getDeviceList(api, true, false)) {
            if (!containsOutputDeviceNamed(outputDevices, pDevice->getDisplayName())) {
                outputDevices.append(pDevice);
            }
        }
    }
    return outputDevices;
}

SoundDevicePointer AudioDeviceAssistant::findControllerSoundCard(
        const QList<SoundDevicePointer>& outputDevices) const {
    const QStringList controllerNames = systemConnectedDeviceNames();
    for (const QString& controllerName : controllerNames) {
        for (const auto& pDevice : outputDevices) {
            if (namesReferToSameDevice(controllerName, pDevice->getDisplayName())) {
                qDebug() << "AudioDeviceAssistant: controller" << controllerName
                         << "provides the output device" << pDevice->getDisplayName();
                return pDevice;
            }
        }
    }
    return {};
}

SoundDevicePointer AudioDeviceAssistant::findSystemDefaultDevice(
        const QList<SoundDevicePointer>& outputDevices) const {
    const std::shared_ptr<SoundManager> pSoundManager = m_pCoreServices->getSoundManager();
    const SoundDevicePointer pDefaultDevice = pSoundManager->getDefaultOutputDevice();
    if (pDefaultDevice) {
        for (const auto& pDevice : outputDevices) {
            if (pDevice->getDeviceId().name == pDefaultDevice->getDeviceId().name) {
                return pDevice;
            }
        }
    }
    return outputDevices.isEmpty() ? SoundDevicePointer() : outputDevices.constFirst();
}

void AudioDeviceAssistant::useDevice(const SoundDevicePointer& pDevice) {
    const std::shared_ptr<SoundManager> pSoundManager = m_pCoreServices->getSoundManager();
    SoundManagerConfig config = pSoundManager->getConfig();

    if (pDevice) {
        if (pDevice->getHostAPI() != config.getAPI()) {
            // The devices are only listed per API, so an input of another API
            // cannot be used any more.
            config.clearInputs();
        }
        config.setAPI(pDevice->getHostAPI());
        config.clearOutputs();
        config.addOutput(pDevice->getDeviceId(),
                AudioOutput(AudioPathType::Main,
                        0,
                        mixxx::audio::ChannelCount::stereo()));
        if (pDevice->getNumOutputChannels() >= 4) {
            // Sound cards of DJ controllers usually provide a separate
            // headphone output on the remaining channels.
            config.addOutput(pDevice->getDeviceId(),
                    AudioOutput(AudioPathType::Headphones,
                            2,
                            mixxx::audio::ChannelCount::stereo()));
        }
        // The sample rate of the configuration is adapted to the device by
        // SoundManagerConfig::checkSampleRate(), which setConfig() calls.
        qDebug() << "AudioDeviceAssistant: using" << pDevice->getDisplayName()
                 << "for the output";
    }

    const SoundDeviceStatus status = pSoundManager->setConfig(config);
    if (status != SoundDeviceStatus::Ok) {
        qWarning() << "AudioDeviceAssistant: failed to set up the sound devices,"
                   << "status" << static_cast<int>(status);
    }
}

SoundDevicePointer AudioDeviceAssistant::askUserForDevice(
        const SoundDevicePointer& pControllerDevice,
        const SoundDevicePointer& pComputerDevice) {
    QMessageBox messageBox(m_pParentWidget);
    messageBox.setIcon(QMessageBox::Information);
    messageBox.setWindowTitle(tr("Audio setup"));
    messageBox.setText(tr("Controller detected: %1")
                               .arg(pControllerDevice->getDisplayName()));
    messageBox.setInformativeText(
            pComputerDevice
                    ? tr("Its sound card can be used for the main output and the "
                         "headphones.\n\nAudio of the computer: %1")
                              .arg(pComputerDevice->getDisplayName())
                    : tr("Its sound card can be used for the main output and the "
                         "headphones."));

    QPushButton* pUseControllerButton =
            messageBox.addButton(tr("Use the controller"), QMessageBox::AcceptRole);
    messageBox.addButton(tr("Use the computer audio"), QMessageBox::RejectRole);
    QCheckBox* pRememberCheckBox =
            new QCheckBox(tr("Remember this and do not ask again"), &messageBox);
    messageBox.setCheckBox(pRememberCheckBox);
    messageBox.setDefaultButton(pUseControllerButton);

    qDebug() << "AudioDeviceAssistant: asking about" << pControllerDevice->getDisplayName();
    messageBox.exec();

    const bool useController = messageBox.clickedButton() == pUseControllerButton;
    if (pRememberCheckBox->isChecked()) {
        storePreference(useController ? kPreferenceMixer : kPreferenceComputer);
    }
    if (useController) {
        return pControllerDevice;
    }
    return pComputerDevice ? pComputerDevice : pControllerDevice;
}

void AudioDeviceAssistant::informAboutDeviceChange(
        const QString& removedDeviceName, const QString& usedDeviceName) {
    QMessageBox messageBox(m_pParentWidget);
    messageBox.setIcon(QMessageBox::Information);
    messageBox.setWindowTitle(tr("Audio setup"));
    messageBox.setText(tr("The device %1 is no longer connected.").arg(removedDeviceName));
    messageBox.setInformativeText(tr("Using the audio of the computer: %1")
                                          .arg(usedDeviceName));
    messageBox.exec();
}

void AudioDeviceAssistant::informAboutNoController(
        const SoundDevicePointer& pUsedDevice) {
    QMessageBox messageBox(m_pParentWidget);
    messageBox.setIcon(QMessageBox::Information);
    messageBox.setWindowTitle(tr("Audio setup"));
    messageBox.setText(tr("No controller with its own sound card was found."));
    messageBox.setInformativeText(
            pUsedDevice
                    ? tr("Using the audio of the computer: %1")
                              .arg(pUsedDevice->getDisplayName())
                    : tr("No output device is available."));
    messageBox.exec();
}

QString AudioDeviceAssistant::loadPreference() const {
    return m_pCoreServices->getSettings()->getValue(kControllerAudioDeviceKey);
}

void AudioDeviceAssistant::storePreference(const QString& preference) {
    m_pCoreServices->getSettings()->setValue(kControllerAudioDeviceKey, preference);
}

} // namespace mixxx

#include "moc_audiodeviceassistant.cpp"
