#pragma once

#include <QObject>
#include <vector>

#include "soundio/sounddevice.h"

class SoundDeviceEnumerator : public QObject {
  public:
    SoundDeviceEnumerator();
    ~SoundDeviceEnumerator() override;

    virtual std::vector<SoundDevicePointer> queryDevices() const = 0;
    virtual QList<mixxx::audio::SampleRate> getSampleRates(bool jackSampleRate) const = 0;
    virtual std::vector<std::pair<uint32_t, QString>> queryHardwareDevices() {
        return {};
    }

    virtual std::unordered_map<uint32_t, QString> queryHardwareVolumes(
            [[maybe_unused]] uint32_t deviceIndex) {
        return {};
    }

    virtual QList<QString> getAPIs() const = 0;
    virtual void initialize() = 0;
    virtual void deinitialize() = 0;

    bool initialized() const {
        return m_initialized;
    }

  protected:
    bool m_initialized;
};
