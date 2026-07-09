#pragma once

#include <portaudio.h>

#include <QList>

#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddeviceenumerator.h"
#include "soundio/sounddeviceportaudio.h"

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
