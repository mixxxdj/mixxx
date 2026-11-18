#pragma once

#include <QString>
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

#include "control/pollingcontrolproxy.h"
#include "soundio/sounddevice.h"
#include "util/duration.h"
#include "util/performancetimer.h"

class SoundManager;

class SoundDeviceJack final : public SoundDevice {
  public:
    SoundDeviceJack(
            UserSettingsPointer config,
            SoundManager* soundManager,
            const QString& deviceName,
            int maxInputChannels,
            int maxOutputChannels,
            mixxx::audio::SampleRate defaultSampleRate,
            unsigned int deviceIndex);
    ~SoundDeviceJack() override;

    SoundDeviceStatus open(bool isClkRefDevice, int syncBuffers) override;
    bool isOpen() const override;
    SoundDeviceStatus close() override;
    void readProcess(SINT framesPerBuffer) override;
    void writeProcess(SINT framesPerBuffer) override;
    QString getError() const override;
    mixxx::audio::SampleRate getDefaultSampleRate() const override;

  private:
    struct Impl;

    int processCallback(unsigned int framesPerBuffer);
    void updateCallbackEntryToDacTime(
            unsigned int framesPerBuffer, double timeSinceLastCallbackSecs);
    void updateAudioLatencyUsage(unsigned int framesPerBuffer);

    std::unique_ptr<Impl> m_pImpl;
    mixxx::audio::SampleRate m_defaultSampleRate;
    int m_inputChannelCount{0};
    int m_outputChannelCount{0};
    std::vector<CSAMPLE> m_inputBuffer;
    std::vector<CSAMPLE> m_outputBuffer;
    std::atomic_bool m_isOpen{false};
    std::atomic_bool m_serverShutdown{false};
    std::atomic_bool m_sampleRateChanged{false};
    std::atomic_bool m_bufferTooLarge{false};
    std::atomic<uint32_t> m_outputLatencyFrames{0};
    QString m_lastError;
    PollingControlProxy m_audioLatencyUsage;
    mixxx::Duration m_timeInAudioCallback;
    unsigned int m_framesSinceAudioLatencyUsageUpdate{0};
    uint32_t m_lastCallbackEntryToDacLatencyFrames{0};
    double m_lastCallbackEntryToDacSecs{0.0};
    PerformanceTimer m_clkRefTimer;
};
