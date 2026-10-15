#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "soundio/sounddevice.h"

class QWidget;

namespace mixxx {

class CoreServices;

/// Notices that a device was connected or disconnected while Mixxx is running
/// and, like other DJ software, offers to use the sound card of a DJ controller
/// that has just been plugged in, or to go back to the audio of the computer
/// when it is unplugged.
///
/// The audio backend only enumerates the devices when it is started, so noticing
/// a device that was connected while Mixxx is running means shutting the backend
/// down and starting it again, which interrupts the sound for a moment.
class AudioDeviceAssistant : public QObject {
    Q_OBJECT
  public:
    AudioDeviceAssistant(QWidget* pParentWidget, CoreServices* pCoreServices);
    ~AudioDeviceAssistant() override;

    /// Looks for devices right away instead of waiting for the next poll. Used
    /// by the menu action, which asks the user again instead of using the
    /// remembered choice.
    void checkForDevicesNow(bool askUser = false);

  private slots:
    void slotPollDevices();

  private:
    void updateDevices();
    void useDevice(const SoundDevicePointer& pDevice);

    QList<SoundDevicePointer> listOutputDevices() const;
    /// Name of the output device that is currently open, or an empty string.
    QString openedOutputDeviceName(const std::shared_ptr<SoundManager>& pSoundManager) const;
    /// Sound card of a connected DJ controller, if there is one.
    SoundDevicePointer findControllerSoundCard(
            const QList<SoundDevicePointer>& outputDevices) const;
    /// Output device the operating system uses by default.
    SoundDevicePointer findSystemDefaultDevice(
            const QList<SoundDevicePointer>& outputDevices) const;

    SoundDevicePointer askUserForDevice(
            const SoundDevicePointer& pControllerDevice,
            const SoundDevicePointer& pComputerDevice);
    void informAboutDeviceChange(
            const QString& removedDeviceName, const QString& usedDeviceName);
    /// Tells the user that no controller was found, used when the assistant is
    /// started from the menu.
    void informAboutNoController(const SoundDevicePointer& pUsedDevice);

    QString loadPreference() const;
    void storePreference(const QString& preference);

    QWidget* m_pParentWidget;
    CoreServices* m_pCoreServices;
    QTimer m_pollTimer;
    /// Devices the operating system reported on the last poll, used to notice
    /// that something was connected or disconnected.
    QStringList m_lastSystemDevices;
    bool m_updating{false};
    /// True while the user asked for the assistant, see checkForDevicesNow().
    bool m_askUser{false};
};

} // namespace mixxx
