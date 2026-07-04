#pragma once

#include <portaudio.h>

#include <QList>

#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddeviceenumerator.h"
#include "soundio/sounddeviceportaudio.h"

#define MIXXX_PIPEWIRE_STRING "PipeWire"
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

class PortAudioEnumerator : public SoundDeviceEnumerator {
  public:
    PortAudioEnumerator(UserSettingsPointer config,
            SoundManager* sm);
    ~PortAudioEnumerator() override;

    std::vector<SoundDevicePointer> queryDevices() const override;
    std::vector<std::string> getAPIs() const override;
    QList<mixxx::audio::SampleRate> getSampleRates() const override;
    QList<mixxx::audio::SampleRate> getJackSampleRates() const;

    void initialize();
    void terminate();

  private:
    void setJACKName() const;

    bool m_initialized;
    std::vector<QSharedPointer<SoundDevicePortAudio>> m_devices;
    mixxx::audio::SampleRate m_jackSampleRate;

    UserSettingsPointer m_pConfig;
    SoundManager* m_pSoundManager;
};
