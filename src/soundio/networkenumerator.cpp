#include "soundio/networkenumerator.h"

#include "engine/sidechain/enginenetworkstream.h"
#include "soundio/sounddevice.h"

NetworkEnumerator::NetworkEnumerator(UserSettingsPointer config,
        SoundManager* sm)
        : m_pNetworkStream(QSharedPointer<EngineNetworkStream>::create(2, 0)),
          m_pDevice(QSharedPointer<SoundDeviceNetwork>::create(
                  config, sm, m_pNetworkStream)) {
}

NetworkEnumerator::~NetworkEnumerator() {
}

std::vector<SoundDevicePointer> NetworkEnumerator::queryDevices() const {
    return std::vector<SoundDevicePointer>{m_pDevice};
}

QList<mixxx::audio::SampleRate> NetworkEnumerator::getSampleRates() const {
    return QList<mixxx::audio::SampleRate>{};
}

std::vector<std::string> NetworkEnumerator::getAPIs() const {
    return std::vector<std::string>{};
}
