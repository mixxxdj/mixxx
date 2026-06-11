#pragma once

#include <QSharedPointer>

#include "soundio/sounddevice.h"
#include "soundio/sounddeviceenumerator.h"
#include "soundio/sounddevicenetwork.h"

class NetworkEnumerator : public SoundDeviceEnumerator {
  public:
    NetworkEnumerator(UserSettingsPointer pConfig,
            SoundManager* pSoundManager);
    ~NetworkEnumerator() override;

    std::vector<SoundDevicePointer> queryDevices() const override;
    QList<mixxx::audio::SampleRate> getSampleRates() const override;
    std::vector<std::string> getAPIs() const override;

    QSharedPointer<EngineNetworkStream> getNetworkStream() const {
        return m_pNetworkStream;
    }

  private:
    QSharedPointer<EngineNetworkStream> m_pNetworkStream;
    QSharedPointer<SoundDeviceNetwork> m_pDevice;
};
