#pragma once

#include <portaudio.h>

#include <QString>
#include <memory>

#include "control/pollingcontrolproxy.h"
#include "soundio/sounddevice.h"
#include "util/duration.h"
#include "util/performancetimer.h"

class SoundManager;
class ControlProxy;

class SoundDevicePortAudio : public SoundDevice {
  public:
    SoundDevicePortAudio(UserSettingsPointer config,
            SoundManager* sm,
            const PaDeviceInfo* deviceInfo,
            PaHostApiTypeId deviceTypeId,
            unsigned int devIndex);
    ~SoundDevicePortAudio() override;

    SoundDeviceStatus open(bool isClkRefDevice, int syncBuffers) override;
    bool isOpen() const override;
    SoundDeviceStatus close() override;
    void readProcess(SINT framesPerBuffer) override;
    void writeProcess(SINT framesPerBuffer) override;
    QString getError() const override;

    // This callback function gets called every time the sound device runs out of
    // samples (ie. when it needs more sound to play)
    int callbackProcess(const SINT framesPerBuffer,
                        CSAMPLE *output, const CSAMPLE* in,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags);
    // Same as above but with drift correction
    int callbackProcessDrift(const SINT framesPerBuffer,
                        CSAMPLE *output, const CSAMPLE* in,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags);
    // The same as above but drives the MixxEngine
    int callbackProcessClkRef(const SINT framesPerBuffer,
                        CSAMPLE *output, const CSAMPLE* in,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags);

    unsigned int getDefaultSampleRate() const override {
        return m_deviceInfo ? static_cast<unsigned int>(
            m_deviceInfo->defaultSampleRate) : 44100;
    }

  private:
    void updateCallbackEntryToDacTime(
            SINT framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo);
    void updateAudioLatencyUsage(const SINT framesPerBuffer);

    // PortAudio stream for this device.
    PaStream* volatile m_pStream;
    // Struct containing information about this device. Don't free() it, it
    // belongs to PortAudio.
    const PaDeviceInfo* m_deviceInfo;
    const PaHostApiTypeId m_deviceTypeId;
    // Description of the output stream going to the soundcard.
    PaStreamParameters m_outputParams;
    // Description of the input stream coming from the soundcard.
    PaStreamParameters m_inputParams;
    std::unique_ptr<FIFO<CSAMPLE>> m_outputFifo;
    std::unique_ptr<FIFO<CSAMPLE>> m_inputFifo;
    bool m_outputDrift;
    bool m_inputDrift;

    // A string describing the last PortAudio error to occur.
    QString m_lastError;
    // Whether we have set the thread priority to realtime or not.
    bool m_bSetThreadPriority;
    PollingControlProxy m_audioLatencyUsage;
    mixxx::Duration m_timeInAudioCallback;
    int m_framesSinceAudioLatencyUsageUpdate;
    int m_syncBuffers;
    int m_invalidTimeInfoCount;
    PerformanceTimer m_clkRefTimer;
    PaTime m_lastCallbackEntrytoDacSecs;
};
