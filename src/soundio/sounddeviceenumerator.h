#pragma once

#include <QObject>
#include <QtGlobal>
#include <vector>

#include "soundio/sounddevice.h"

class SoundDeviceEnumerator : public QObject {
  public:
    SoundDeviceEnumerator();
    ~SoundDeviceEnumerator() override;

    virtual std::vector<SoundDevicePointer> queryDevices() const = 0;
    virtual QList<mixxx::audio::SampleRate> getSampleRates(bool jackSampleRate) const = 0;
    virtual QList<QString> getAPIs() const = 0;
    virtual void initialize() = 0;
    virtual void deinitialize() = 0;

    /// Returns the device that the operating system uses by default for output
    /// on the given API, or a null pointer if the backend cannot tell. Used to
    /// keep output working when the configured device is not connected.
    virtual SoundDevicePointer getDefaultOutputDevice(const QString& api) const {
        Q_UNUSED(api);
        return {};
    }

    bool initialized() const {
        return m_initialized;
    }

  protected:
    bool m_initialized;
};
