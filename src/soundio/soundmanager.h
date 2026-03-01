#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "audio/types.h"
#include "control/pollingcontrolproxy.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanagerconfig.h"
#include "util/cmdlineargs.h"
#include "util/types.h"

class EngineMixer;
class ControlObject;

#define MIXXX_PORTAUDIO_JACK_STRING "JACK Audio Connection Kit"
#define MIXXX_PORTAUDIO_ALSA_STRING "ALSA"
#define MIXXX_PORTAUDIO_OSS_STRING "OSS"
#define MIXXX_PORTAUDIO_ASIO_STRING "ASIO"
#define MIXXX_PORTAUDIO_DIRECTSOUND_STRING "Windows DirectSound"
// NOTE: This is what our patched version of PortAudio uses for the Core Audio
// backend on iOS. If/when upstream supports iOS officially
// (https://github.com/PortAudio/portaudio/pull/881), we may have to update this
#define MIXXX_PORTAUDIO_IOSAUDIO_STRING "iOS Audio"
#define MIXXX_PORTAUDIO_COREAUDIO_STRING "Core Audio"

#define SOUNDMANAGER_DISCONNECTED 0
#define SOUNDMANAGER_CONNECTING 1
#define SOUNDMANAGER_CONNECTED 2


class SoundManager : public QObject {
    Q_OBJECT
  public:
    SoundManager(UserSettingsPointer pConfig, EngineMixer* pEngineMixer);
    ~SoundManager() override;

    // Returns a list of all devices we've enumerated that match the provided
    // filterApi, and have at least one output or input channel if the
    // bOutputDevices or bInputDevices are set, respectively.
    QList<SoundDevicePointer> getDeviceList(
            const QString& filterAPI, bool bOutputDevices, bool bInputDevices) const;

    // Creates a list of sound devices
    void clearAndQueryDevices();
    void queryDevices();
    void queryDevicesPortaudio();
    void queryDevicesMixxx();

    // Opens all the devices chosen by the user in the preferences dialog, and
    // establishes the proper connections between them and the mixing engine.
    SoundDeviceStatus setupDevices();

    // Playermanager will notify us when the number of decks changes.
    void setConfiguredDeckCount(int count);
    int getConfiguredDeckCount() const;

    SoundDevicePointer getErrorDevice() const;
    QString getErrorDeviceName() const;
    QString getLastErrorMessage(SoundDeviceStatus status) const;

    // Returns a list of samplerates we will attempt to support for a given API.
    QList<mixxx::audio::SampleRate> getSampleRates(const QString& api) const;

    // Convenience overload for SoundManager::getSampleRates(QString)
    QList<mixxx::audio::SampleRate> getSampleRates() const;

    // Get a list of host APIs supported by PortAudio.
    QList<QString> getHostAPIList() const;
    SoundManagerConfig getConfig() const;
    SoundDeviceStatus setConfig(const SoundManagerConfig& config);
    // Due to a bug in in PulseAudio, we must give at least 5 seconds of cool
    // down before performing further audio related operation. This sleep
    // happens during the function call by default (synchronous blocking), but
    // the caller may decide to use the async version, and must not performs any
    // audio operation till it received the `devicesClosed` signal
    void closeActiveConfig(bool async = false);
    void checkConfig();

    void onDeviceOutputCallback(const SINT iFramesPerBuffer);

    // Used by SoundDevices to "push" any audio from their inputs that they have
    // into the mixing engine.
    void pushInputBuffers(const QList<AudioInputBuffer>& inputs,
                          const SINT iFramesPerBuffer);

    void writeProcess(SINT framesPerBuffer) const;
    void readProcess(SINT framesPerBuffer) const;

    void registerOutput(const AudioOutput& output, AudioSource* src);
    void registerInput(const AudioInput& input, AudioDestination* dest);
    QList<AudioOutput> registeredOutputs() const;
    QList<AudioInput> registeredInputs() const;

    QSharedPointer<EngineNetworkStream> getNetworkStream() const {
        return m_pNetworkStream;
    }

    void underflowHappened(int code) {
        m_underflowHappened = 1;
        // Disable the engine warnings by default, because printing a warning is a
        // locking function that will make the problem worse
        if (CmdlineArgs::Instance().getDeveloper()) {
            qWarning() << "underflowHappened code:" << code;
        }
    }

    void processUnderflowHappened(SINT framesPerBuffer);

    UserSettingsPointer userSettings() const {
        return m_pConfig;
    }

  signals:
    void devicesUpdated(); // emitted when pointers to SoundDevices go stale
    void devicesSetup(); // emitted when the sound devices have been set up
    void devicesClosed(); // emitted when the sound devices have been closed and resources freed
    void outputRegistered(const AudioOutput& output, AudioSource* src);
    void inputRegistered(const AudioInput& input, AudioDestination* dest);

  private slots:
    void completeDevicesClosing();

  private:
    // Closes all the devices and empties the list of devices we have.
    void clearDeviceList(bool sleepAfterClosing);

    // Closes all the open sound devices. Because multiple soundcards might be
    // open, this method simply runs through the list of all known soundcards
    // (from PortAudio) and attempts to close them all. Closing a soundcard that
    // isn't open is safe.
    void closeDevices(bool sleepAfterClosing, bool async = false);

    void setJACKName() const;
    bool jackApiUsed() const {
        return m_config.getAPI() == MIXXX_PORTAUDIO_JACK_STRING;
    }

    EngineMixer* m_pEngineMixer;
    UserSettingsPointer m_pConfig;
    bool m_paInitialized;
    mixxx::audio::SampleRate m_jackSampleRate;
    QList<SoundDevicePointer> m_devices;
    QList<mixxx::audio::SampleRate> m_samplerates;
    QList<CSAMPLE*> m_inputBuffers;

    SoundManagerConfig m_config;
    SoundDevicePointer m_pErrorDevice;
    QHash<AudioOutput, AudioSource*> m_registeredSources;
    QMultiHash<AudioInput, AudioDestination*> m_registeredDestinations;
    ControlObject* m_pControlObjectSoundStatusCO;
    ControlObject* m_pControlObjectVinylControlGainCO;

    QSharedPointer<EngineNetworkStream> m_pNetworkStream;

    QAtomicInt m_underflowHappened;
    int m_underflowUpdateCount;
    PollingControlProxy m_audioLatencyOverloadCount;
    PollingControlProxy m_audioLatencyOverload;
};
