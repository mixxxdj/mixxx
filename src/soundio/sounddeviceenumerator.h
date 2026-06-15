#pragma once

#include <QObject>
#include <string>
#include <vector>

#include "soundio/sounddevice.h"

class SoundDeviceEnumerator : public QObject {
  public:
    SoundDeviceEnumerator();
    ~SoundDeviceEnumerator() override;

    virtual std::vector<SoundDevicePointer> queryDevices() const = 0;
    virtual QList<mixxx::audio::SampleRate> getSampleRates() const = 0;
    virtual std::vector<std::string> getAPIs() const = 0;
};
